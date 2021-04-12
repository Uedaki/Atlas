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
		CLOSED,
		WAITING,
		RUNNING,
		FINISHING
	};

	class ThreadedTask
	{
	public:
		virtual bool preExecute() = 0;
		virtual void execute() = 0;
		virtual void postExecute() = 0;

		void release()
		{
			delete this;
		}

		inline void assignThread()
		{
			assignedThreadCount++;
		}
		inline bool removeThread()
		{
			return (--assignedThreadCount);
		}
	private:
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
							uint32_t taskIdx;
							{ // lock scope
								std::unique_lock<std::mutex> lock(guard);
								sleepCtrl.wait(lock, [this]()
									{
										uint32_t taskId = currentTask;
										return (!isRunning
											|| (taskStates[taskId] == ThreadedTaskState::WAITING)
											|| (taskStates[taskId] == ThreadedTaskState::RUNNING));
									});						
							
								if (!isRunning)
									break;

								taskIdx = currentTask;  // it is better to stock currentTask inside a variable to reduce access to an atomic variable
								if (taskStates[taskIdx] == ThreadedTaskState::WAITING)
								{
									if (taskBuffer[taskIdx]->preExecute())
										taskStates[taskIdx] = ThreadedTaskState::RUNNING;
									else
									{
										taskBuffer[taskIdx]->release();
										taskBuffer[taskIdx] = nullptr;
										taskStates[taskIdx] = ThreadedTaskState::CLOSED;
										continue;
									}
								}
							}

							if (taskStates[taskIdx] == ThreadedTaskState::FINISHING || taskStates[taskIdx] == ThreadedTaskState::CLOSED)
								continue;
							taskBuffer[taskIdx]->assignThread();
							taskBuffer[taskIdx]->execute();
							taskStates[taskIdx] = ThreadedTaskState::FINISHING;

							if (taskBuffer[taskIdx]->removeThread() == 0)
							{
								currentTask = (currentTask + 1) % BufferSize;
								sleepCtrl.notify_all();
								taskBuffer[taskIdx]->postExecute();
								taskBuffer[taskIdx]->release();
								taskBuffer[taskIdx] = nullptr;
								taskStates[taskIdx] = ThreadedTaskState::CLOSED;
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

		template <typename T, typename ...Args>
		void execute(Args... args)
		{
			uint32_t newTask = (lastTask + 1) % BufferSize;
			taskBuffer[newTask] = new T(args...);
			taskStates[newTask] = ThreadedTaskState::WAITING;
			lastTask = newTask;
			sleepCtrl.notify_all();
		}

		void join()
		{
			//while (taskStates[currentTask] != ThreadedTaskState::CLOSED)
			//	;
			//return;

			while (true)
			{
				uint32_t taskIdx;
				{
					std::unique_lock<std::mutex> lock(guard);
					sleepCtrl.wait(lock, [this]()
						{
							uint32_t taskIdx = currentTask;
							return (!isRunning
								|| taskStates[taskIdx] == ThreadedTaskState::WAITING
								|| taskStates[taskIdx] == ThreadedTaskState::RUNNING
								|| taskStates[taskIdx] == ThreadedTaskState::CLOSED);
						});
				
					taskIdx = currentTask;
					if (!isRunning || taskStates[taskIdx] == ThreadedTaskState::CLOSED)
						break;

					if (taskStates[taskIdx] == ThreadedTaskState::WAITING)
					{
						if (taskBuffer[taskIdx]->preExecute())
							taskStates[taskIdx] = ThreadedTaskState::RUNNING;
						else
						{
							taskBuffer[taskIdx]->release();
							taskBuffer[taskIdx] = nullptr;
							taskStates[taskIdx] = ThreadedTaskState::CLOSED;
							continue;
						}
					}
				}

				if (taskStates[taskIdx] == ThreadedTaskState::FINISHING || taskStates[taskIdx] == ThreadedTaskState::CLOSED)
					continue;
				taskBuffer[taskIdx]->assignThread();
				taskBuffer[taskIdx]->execute();
				taskStates[taskIdx] = ThreadedTaskState::FINISHING;

				if (taskBuffer[taskIdx]->removeThread() == 0)
				{
					currentTask = (currentTask + 1) % BufferSize;
					sleepCtrl.notify_all();
					taskBuffer[taskIdx]->postExecute();
					taskBuffer[taskIdx]->release();
					taskBuffer[taskIdx] = nullptr;
					taskStates[taskIdx] = ThreadedTaskState::CLOSED;
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
		std::array<ThreadedTaskState, BufferSize> taskStates = { ThreadedTaskState::CLOSED };
	};
}