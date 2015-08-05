#ifndef THREADBLOCKER_H
#define THREADBLOCKER_H

#include <condition_variable>
#include <mutex>
#include <atomic>
#include <thread>

class ThreadBlocker {
	using ThreadId = std::thread::id;
	using Mutex = std::mutex;
	using Lock = std::unique_lock<Mutex>;
	using ConditionVar = std::condition_variable;
  public:
	ThreadBlocker() : mIsBlocked(false) {}
	~ThreadBlocker() {unblock();}
	void block() {
		Lock syncLock(mSyncMutex);
		if (mIsBlocked) return;
		mIsBlocked = true;
		Lock blockLock(mBlockMutex);
		while (mIsBlocked) mBlockVar.wait(blockLock);
	}
	void unblock() {
		if (mIsBlocked) {
			mIsBlocked = false;
			mBlockVar.notify_one();
		}
	}
	bool isBlocked() const { return mIsBlocked; }
  private:
	Mutex mBlockMutex;
	Mutex mSyncMutex;
	ConditionVar mBlockVar;
	std::atomic<bool> mIsBlocked;
};

#endif  // THREADBLOCKER_H
