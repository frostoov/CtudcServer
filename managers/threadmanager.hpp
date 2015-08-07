#ifndef THREADMANAGER_HPP
#define THREADMANAGER_HPP

#include <thread>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

class ThreadManager {
	using ThreadPtr = std::unique_ptr<std::thread>;
  public:
	virtual ~ThreadManager();
	ThreadManager(const ThreadManager& other) = delete;
	void operator=(const ThreadManager& other) = delete;

	virtual bool start();
	virtual void stop();

	bool hasProcess() const;
  protected:
	ThreadManager();
	virtual void workerFunc() = 0;
	virtual bool init() = 0;
	virtual void shutDown() = 0;
	void joinThread();
	void resetThread();
	bool startThread(std::function<void()>&&func);
  private:
	void workerLoop();
	ThreadPtr mThread;
	std::mutex mThreadMutex;
	std::atomic<bool> mIsActive;
};

#endif // THREADMANAGER_HPP
