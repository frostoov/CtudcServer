#ifndef THREADMANAGER_HPP
#define THREADMANAGER_HPP

#include <thread>
#include <memory>

#include "threadblocker.hpp"

class ThreadManager {
	using ThreadPtr = std::unique_ptr<std::thread>;
  public:
	virtual ~ThreadManager();
	ThreadManager(const ThreadManager& other) = delete;
	void operator=(const ThreadManager& other) = delete;

	bool start();
	void stop();

	bool hasProcess() const;
  protected:
	ThreadManager();
	bool isActive() const;
	virtual void workerFunc() = 0;
	virtual bool init() = 0;
	virtual void flush() = 0;
	virtual void shutDown() = 0;
	bool isActive();
  private:
	void workerLoop();
	ThreadPtr mThread;

	volatile bool mIsActive;
};

#endif // THREADMANAGER_HPP
