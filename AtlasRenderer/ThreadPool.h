#pragma once

#include <array>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

namespace atlas
{
	enum class ThreadedTaskState
	{
		WAITING,
		RUNNING,
		FINISHING,
		CLOSED
	};

	class ThreadedTask
	{
	public:
		virtual bool preExecute() = 0;
		virtual void execute() = 0;
		virtual void postExecute() = 0;

		void assignThread()	{ assignedThreadCount++; }
		bool removeThread()	{ return (--assignedThreadCount); }

		const std::atomic<ThreadedTaskState> &getState() const { return (state); }
		std::atomic<ThreadedTaskState> &getAtomicState() { return (state); }
		void setState(ThreadedTaskState newState) { state = newState; }

	private:
		std::atomic<ThreadedTaskState> state = ThreadedTaskState::WAITING;
		std::atomic<uint32_t> assignedThreadCount = 0;
	};

	template <uint32_t BufferSize = 8>
	class ThreadPool
	{
	public:
		void init(uint32_t threadCount = std::thread::hardware_concurrency() - 1)
		{
			isRunning = true;

			workers.reserve(threadCount);
			for (uint32_t i = 0; i < threadCount; i++)
			{
				workers.push_back(std::thread([this]()
					{
						while (isRunning)
						{
							{ // lock scope
								std::unique_lock<std::mutex> lock(guard);
								sleepCtrl.wait(lock, [this]()
									{
										return (!isRunning
											|| (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::WAITING)
											|| (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::RUNNING));
									});						
							
								if (!isRunning)
									break;

								if (taskBuffer[currentTask]->getState() == ThreadedTaskState::WAITING)
								{
									if (taskBuffer[currentTask]->preExecute())
										taskBuffer[currentTask]->setState(ThreadedTaskState::RUNNING);
									else
									{
										taskBuffer[currentTask]->setState(ThreadedTaskState::CLOSED);
										continue;
									}
								}
							}

							uint32_t taskIdx = currentTask;
							taskBuffer[taskIdx]->assignThread();
							taskBuffer[taskIdx]->execute();
							taskBuffer[taskIdx]->setState(ThreadedTaskState::FINISHING);

							if (taskBuffer[taskIdx]->removeThread() == 0)
							{
								if (currentTask != lastTask)
									currentTask = (currentTask + 1) % BufferSize;
								sleepCtrl.notify_all();
								taskBuffer[taskIdx]->postExecute();
								taskBuffer[taskIdx]->setState(ThreadedTaskState::CLOSED);
								sleepCtrl.notify_all();
							}
						}
					}));
			}
		}

		void shutdown()
		{
			isRunning = false;
			sleepCtrl.notify_all();

			for (auto &worker : workers)
			{
				if (worker.joinable())
					worker.join();
			}
		}

		void execute(ThreadedTask *task)
		{
			
			uint32_t newTask = (lastTask + 1) % BufferSize;
			taskBuffer[newTask] = task;
			lastTask = newTask;
			if (taskBuffer[currentTask]->getState() == ThreadedTaskState::CLOSED)
				currentTask = (currentTask + 1) % BufferSize;
			sleepCtrl.notify_all();
		}

		void join()
		{
			while (true)
			{
				{
					std::unique_lock<std::mutex> lock(guard);
					sleepCtrl.wait(lock, [this]()
						{
							return (!isRunning
								|| (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::WAITING)
								|| (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::RUNNING)
								|| (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::CLOSED && currentTask == lastTask));
						});
				
					if (!isRunning || (taskBuffer[currentTask] && taskBuffer[currentTask]->getState() == ThreadedTaskState::CLOSED && currentTask == lastTask))
						break;

					if (taskBuffer[currentTask]->getState() == ThreadedTaskState::WAITING)
					{
						if (taskBuffer[currentTask]->preExecute())
							taskBuffer[currentTask]->setState(ThreadedTaskState::RUNNING);
						else
						{
							taskBuffer[currentTask]->setState(ThreadedTaskState::CLOSED);
							continue;
						}
					}
				}

				uint32_t taskIdx = currentTask;
				taskBuffer[taskIdx]->assignThread();
				taskBuffer[taskIdx]->execute();
				taskBuffer[taskIdx]->setState(ThreadedTaskState::FINISHING);

				if (taskBuffer[taskIdx]->removeThread() == 0)
				{
					if (currentTask != lastTask)
						currentTask = (currentTask + 1) % BufferSize;
					sleepCtrl.notify_all();
					taskBuffer[taskIdx]->postExecute();
					taskBuffer[taskIdx]->setState(ThreadedTaskState::CLOSED);
				}
			}
		}

	private:
		bool isRunning = false;

		std::mutex guard;
		std::condition_variable sleepCtrl;
		std::vector<std::thread> workers;

		std::atomic<uint32_t> currentTask = 0;
		std::atomic<uint32_t> lastTask = BufferSize - 1;
		std::array<ThreadedTask *, BufferSize> taskBuffer = { 0 };
	};
}