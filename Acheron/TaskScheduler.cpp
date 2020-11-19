#include "pch.h"
#include "TaskScheduler.h"

#include <cassert>

void TaskScheduler::startThreads(uint32_t nbrThread)
{
	if (nbrThread < threads.size())
		waitAndStop();

	uint32_t startIdx = static_cast<uint32_t>(threads.size());
	for (uint32_t i = startIdx; i < nbrThread; i++)
	{
		threads.emplace_back([this]()
			{
				this->runThread();
				printf("Exit thread\n");
			});
	}
}

void TaskScheduler::dispatch(Task &task)
{
	isExp = false;
	currentTask = &task;
	threadToLaunch = static_cast<uint8_t>(threads.size());
	dispatcher.notify_all();
	task.execute();
	wait();
}

void TaskScheduler::expDispatch(Task &task)
{
	isExp = true;
	currentTask = &task;
	task.expExecute([this]() {this->expandThread(); });
	wait();
}

void TaskScheduler::asyncDispatch(Task &task)
{
	isExp = false;
	currentTask = &task;
	threadToLaunch = static_cast<uint8_t>(threads.size());
	dispatcher.notify_all();
}

void TaskScheduler::asyncExpDispatch(Task &task)
{
	isExp = true;
	currentTask = &task;
	threadToLaunch = static_cast<uint8_t>(threads.size());
	dispatcher.notify_one();
}

void TaskScheduler::asyncDispatchOne(Task &task)
{
	isExp = false;
	currentTask = &task;
	threadToLaunch = 1;
	dispatcher.notify_one();
}

void TaskScheduler::wait()
{
	//while (threadToLaunch != 0)
	//{
	//	std::this_thread::sleep_for(std::chrono::nanoseconds(waitCheckInterval));
	//}

	while (workingThread > 0)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(waitCheckInterval));
	}
	//currentTask = nullptr;
}

void TaskScheduler::waitOne()
{
	//while (threadToLaunch != static_cast<uint8_t>(threads.size()))
	//{
	//	std::this_thread::sleep_for(std::chrono::nanoseconds(waitCheckInterval));
	//}

	uint8_t nbrThreadWorking = workingThread;
	while (workingThread >= nbrThreadWorking)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(waitCheckInterval));
	}
}

void TaskScheduler::waitAndStop()
{
	wait();
	stop();
}

void TaskScheduler::stop()
{
	currentTask = nullptr;
	dispatcher.notify_all();
	for (auto &thread : threads)
	{
		if (thread.joinable())
			thread.join();
	}
	threads.clear();
}

void TaskScheduler::runThread()
{
	while (true)
	{
		sleepUntilSignal();

		if (!currentTask)
			return;

		workingThread += 1;
		threadToLaunch -= 1;

		if (isExp)
			currentTask->expExecute([this]() {this->expandThread(); });
		else
			currentTask->execute();

		workingThread -= 1;
	}
}

void TaskScheduler::sleepUntilSignal()
{
	std::unique_lock<std::mutex> lk(lock);
	dispatcher.wait(lk);
}

void TaskScheduler::expandThread()
{
	dispatcher.notify_one();
}