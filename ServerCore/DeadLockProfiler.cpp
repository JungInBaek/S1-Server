#include "pch.h"
#include "DeadLockProfiler.h"


/*----------------------
	DeadLockProfiler
----------------------*/

void DeadLockProfiler::PushLock(const char* name)
{
	LockGuard guard(_lock);

	// 아이디를 찾거나 발급
	int32 lockId = 0;

	auto findIt = _nameToId.find(name);
	if (findIt == _nameToId.end())
	{
		lockId = static_cast<int32>(_nameToId.size());
		_nameToId[name] = lockId;
		_idToName[lockId] = name;
	}
	else
	{
		lockId = findIt->second;
	}

	// 이전에 잡은 락이 있는지 확인
	if (LLockStack.empty()  == false)
	{
		const int32 prevId = LLockStack.top();
		// 서로 다른 락을 잡으려 한다면
		if (lockId != prevId)
		{
			set<int32>& history = _lockHistory[prevId];
			
			// 기존에 방문하지 않은 케이스라면 데드락 여부를 다시 확인
			if (history.find(lockId) == history.end())
			{
				history.insert(lockId);
				CheckCycle();
			}
		}
	}
	LLockStack.push(lockId);
}

void DeadLockProfiler::PopLock(const char* name)
{
	LockGuard guard(_lock);

	if (LLockStack.empty())
	{
		CRASH("MULTIPLE_UNLOCK");
	}

	int32 lockId = _nameToId[name];
	if (lockId != LLockStack.top())
	{
		CRASH("INVALID_UNLOCK");
	}

	LLockStack.pop();
}

void DeadLockProfiler::CheckCycle()
{
	const int32 lockCount = static_cast<int32>(_nameToId.size());
	_discorveredCount = 0;
	_discorveredOrder = vector<int32>(lockCount, -1);
	_finished = vector<bool>(lockCount, false);
	_parent = vector<int32>(lockCount, -1);

	for (int32 lockId = 0; lockId < lockCount; lockId++)
	{
		Dfs(lockId);
	}

	// 연산이 끝났으면 정리
	_discorveredOrder.clear();
	_finished.clear();
	_parent.clear();
}

void DeadLockProfiler::Dfs(int32 here)
{
	if (_discorveredOrder[here] != -1)
	{
		return;
	}
	_discorveredOrder[here] = _discorveredCount++;

	// 인접한 모든 정점
	auto findIt = _lockHistory.find(here);

	if (findIt == _lockHistory.end())
	{
		_finished[here] = true;
		return;
	}

	// 방문한 적이 없다면 방문
	set<int32>& nextSet = findIt->second;
	for (int32 there : nextSet)
	{
		// �湮�� ���� ���ٸ� �湮
		if (_discorveredOrder[there] == -1)
		{
			_parent[there] = here;
			Dfs(there);
			continue;
		}

		// here가 there보다 먼저 발견되었다면 순방향 간선
		if (_discorveredOrder[here] < _discorveredOrder[there])
		{
			continue;
		}

		// 순방향이 아니고, Dfs(there)가 아직 종료되지 않았다면 역방향 간선
		if (_finished[there] == false)
		{
			printf("%s -> %s\n", _idToName[here], _idToName[there]);

			int32 now = here;
			while (true)
			{
				printf("%s -> %s\n", _idToName[_parent[now]], _idToName[now]);
				now = _parent[now];
				if (now == there)
				{
					break;
				}
			}

			CRASH("DEADLOCK_DETECTED");
		}
	}

	_finished[here] = true;
}