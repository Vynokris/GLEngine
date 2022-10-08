#include "ThreadManager.h"
#include "App.h"
using namespace Resources;
using namespace Core;


Core::ThreadManager::ThreadManager(const int& maxThread)
{
	LaunchThreads(maxThread);
}

ThreadManager::~ThreadManager()
{
	stopThreads.store(true);
	for (std::thread& t : threads)
		t.join();
	threads.clear();
}

void ThreadManager::LaunchThreads(int maxthread)
{
	for (int i = 0; i < maxthread; i++)
	{
		std::thread b  { &ThreadManager::Life, this};
		threads.push_back(std::move(b));
	}
}

void ThreadManager::AddTask(IResource* resource)
{
	while (lock.test_and_set()) {}
	tasks.push_back(resource);
	lock.clear();
}

void ThreadManager::Life()
{
	while (!stopThreads)
	{
		while (lock.test_and_set()) {}
		if(tasks.size() > 0)
		{
			IResource* resource = tasks[0];
			tasks.erase(tasks.begin());
			lock.clear();
			if (resource != nullptr && !resource->IsLoaded())
				resource->Load();
		}
		else
		{
			lock.clear();
		}
	}
}
