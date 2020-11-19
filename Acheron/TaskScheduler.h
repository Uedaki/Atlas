#pragma once

#include <functional>
#include <mutex>
#include <thread>
#include <vector>

typedef std::function<void()> DuplicateThread;

class Task
{
public:
	virtual void execute() {};
	virtual void expExecute(DuplicateThread duplicateThread) {};
};

class TaskScheduler
{
public:
	// Start nbrThread threads to execute the tasks dispatched
	void startThreads(uint32_t nbrThread);

	// Dispatch the task at the index code to all the threads and block the execution until all threads have finished
	void dispatch(Task &task);
	// TODO: need to find an accurate description
	void expDispatch(Task &task);

	
	// Execute the current command and wait for each thread to finish executing the current task
	void asyncDispatch(Task &task);
	// TODO: need to find an accurate description
	void asyncExpDispatch(Task &task);
	// Dispatch the task at the index code to one the threads
	void asyncDispatchOne(Task &task);

	// Wait for all thread to finish executing the current task
	void wait();
	// Wait for one thread to finish executing the current task
	void waitOne();
	// Wait for all thread to finish executing the current task, then stop all thread
	void waitAndStop();

	// Stop all thread
	void stop();

private:
	const uint32_t waitCheckInterval = 10u; // in nanoseconds

	bool isExp = false;
	Task *currentTask = nullptr;

	std::mutex lock;
	std::condition_variable dispatcher;
	std::vector<std::thread> threads;
	std::atomic<uint8_t> threadToLaunch = 0;
	std::atomic<uint8_t> workingThread = 0;

	void runThread();
	void sleepUntilSignal();
	void expandThread();
};