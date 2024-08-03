#include "pch.h"
#include "MemoryPool.h"


MemoryPool::MemoryPool(int32 allocSize) : _allocSize(allocSize)
{
	::InitializeSListHead((PSLIST_HEADER)&_header);
}

MemoryPool::~MemoryPool()
{
	while (SLIST_ENTRY* memory = ::InterlockedPopEntrySList((PSLIST_HEADER)&_header))
	{
		::_aligned_free(memory);
	}
}

void MemoryPool::Push(MemoryHeader* ptr)
{
	ptr->allocSize = 0;
	::InterlockedPushEntrySList(&_header, (PSLIST_ENTRY)ptr);
	_useCount.fetch_sub(1);
	_reserveCount.fetch_add(1);
}

MemoryHeader* MemoryPool::Pop()
{
	MemoryHeader* memory = static_cast<MemoryHeader*>(::InterlockedPopEntrySList(&_header));

	if (memory == nullptr)
	{
		memory = reinterpret_cast<MemoryHeader*>(::_aligned_malloc(_allocSize, SLIST_ALIGNMENT));
	}
	else
	{
		ASSERT_CRASH(memory->allocSize == 0);
		_reserveCount.fetch_sub(1);
	}

	_useCount.fetch_add(1);

	return memory;
}
