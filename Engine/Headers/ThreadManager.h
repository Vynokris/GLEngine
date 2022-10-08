#pragma once
#include <vector>
#include <thread>
#include <atomic>
#include <string>
#include "IResource.h"
#include <functional>

namespace Resources
{
	class ResourceManager;
}

namespace Core
{
	class ThreadManager
	{
	private:
		// Used to make all threads end.
		std::atomic_bool stopThreads = false;

		// Use as a substitue as lock / unlock.
		std::atomic_flag lock = ATOMIC_FLAG_INIT;

		// List of tasks and list of threads.
		std::vector<Resources::IResource*> tasks;
		std::vector<std::thread>           threads;

		// Used to start all of the threads.
		void LaunchThreads(int maxThread);

	public:
		ThreadManager(const int& maxThread);
		~ThreadManager();

		// Delete all the copy constructors.
		ThreadManager(const ThreadManager&)			   = delete;
		ThreadManager(ThreadManager&&)				   = delete;
		ThreadManager& operator=(const ThreadManager&) = delete;
		ThreadManager& operator=(ThreadManager&&)      = delete;

		// Adds new task to the list.
		void AddTask(Resources::IResource* resource);

		// Function on which the threads run.
		void Life();
	};
}
