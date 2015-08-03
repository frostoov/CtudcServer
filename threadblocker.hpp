#ifndef THREADBLOCKER_H
#define THREADBLOCKER_H

#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadBlocker {
  public:
	ThreadBlocker() : mIsBlocked(false) {}
	~ThreadBlocker() {unblock();}
	void block() {
		if (mIsBlocked) return;
		mIsBlocked = true;
		std::unique_lock<std::mutex> locker(mSyncMutex);
		while (mIsBlocked) mBlocker.wait(locker);
	}
	void unblock() {
		if (mIsBlocked) {
			mIsBlocked = false;
			mBlocker.notify_one();
		}
	}
	bool isBlocked() const { return mIsBlocked; }
  private:
	std::mutex mSyncMutex;
	std::condition_variable mBlocker;
	volatile bool mIsBlocked;
};

#endif  // THREADBLOCKER_H
