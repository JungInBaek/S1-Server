#include "pch.h"
#include "Lock.h"
#include "CoreTLS.h"
#include "DeadLockProfiler.h"


void Lock::WriteLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유 중이면 무조건 성공
	const uint32 lockThreadId = (_lockFlag.load() & WRITE_THREAD_MASK) >> 16;
	if (lockThreadId == LThreadId)
	{
		_writeCount++;
		return;
	}

	// 아무도 소유 및 공유하고 있지 않을 때, 경합 후 소유권 획득
	const int64 beginTick = ::GetTickCount64();
	const uint32 desired = (LThreadId << 16) & WRITE_THREAD_MASK;
	while (true)
	{
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = EMPTY_FLAG;
			if (_lockFlag.compare_exchange_strong(OUT expected, desired))
			{
				_writeCount++;
				return;
			}
		}
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIMEOUT");
		}
		this_thread::yield();
	}
}

void Lock::WriteUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	// ReadLock 전부 풀기 전에는 WriteUnlock 불가능
	const uint32 readCount = _lockFlag.load() & READ_COUNT_MASK;
	if (readCount != 0)
	{
		CRASH("INVALID_UNLOCK_ORDER");
	}

	const uint32 lockCount = --_writeCount;
	if (_writeCount == 0)
	{
		_lockFlag.store(EMPTY_FLAG);
		return;
	}
}

void Lock::ReadLock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PushLock(name);
#endif

	// 동일한 쓰레드가 소유 중이면 무조건 성공
	const uint32 lockThreadId = _lockFlag.load() & WRITE_THREAD_MASK;
	if (lockThreadId == LThreadId)
	{
		_lockFlag.fetch_add(1);
		return;
	}

	// 아무도 소유하고 있지 않을 때, 경합 후 공유 카운트 증가
	while (true)
	{
		const int64 beginTick = ::GetTickCount64();
		for (uint32 spinCount = 0; spinCount < MAX_SPIN_COUNT; spinCount++)
		{
			uint32 expected = _lockFlag.load() & READ_COUNT_MASK;
			if (_lockFlag.compare_exchange_strong(OUT expected, expected + 1))
			{
				return;
			}
		}
		if (::GetTickCount64() - beginTick >= ACQUIRE_TIMEOUT_TICK)
		{
			CRASH("LOCK_TIMEOUT");
		}
		this_thread::yield();
	}
}

void Lock::ReadUnlock(const char* name)
{
#if _DEBUG
	GDeadLockProfiler->PopLock(name);
#endif

	if ((_lockFlag.fetch_sub(1) & READ_COUNT_MASK) == 0)
	{
		CRASH("MULTIPLE_UNLOCK");
	}
}