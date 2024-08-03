#include "pch.h"
#include "JobQueue.h"
#include "GlobalQueue.h"


void JobQueue::Push(JobRef job, bool pushOnly)
{
	const int32 prevCount = _jobCount.fetch_add(1);
	_jobs.Push(job);

	if (pushOnly == true)
	{
		return;
	}

	// 첫번째 Job을 넣은 쓰레드가 실행까지 담당
	if (prevCount != 0)
	{
		return;
	}

	// 이미 다른 JobQueue를 점유하고 있다면
	if (LCurrentJobQueue != nullptr)
	{
		GGlobalQueue->Push(shared_from_this());
		return;
	}

	Execute();
}

void JobQueue::Execute()
{
	LCurrentJobQueue = this;

	while (true)
	{
		Vector<JobRef> jobs;
		_jobs.PopAll(OUT jobs);

		const int32 jobCount = static_cast<int32>(jobs.size());
		for (int32 i = 0; i < jobCount; i++)
		{
			jobs[i]->Execute();
		}

		if (_jobCount.fetch_sub(jobCount) == jobCount)
		{
			LCurrentJobQueue = nullptr;
			return;
		}

		uint64 now = ::GetTickCount64();
		if (now >= LEndTickCount)
		{
			LCurrentJobQueue = nullptr;
			GGlobalQueue->Push(shared_from_this());
			return;
		}
	}
}
