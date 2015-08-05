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
		std::unique_lock<std::mutex> syncLocker(mSyncMutex);
		if (mIsBlocked) return;
		mIsBlocked = true;
		std::unique_lock<std::mutex> blokcLocker(mBlockMutex);
		while (mIsBlocked) mBlocker.wait(blokcLocker);
	}
	void unblock() {
		std::unique_lock<std::mutex> syncLocker(mSyncMutex);
		if (mIsBlocked) {
			mIsBlocked = false;
			mBlocker.notify_one();
		}
	}
	bool isBlocked() const { return mIsBlocked; }
  private:
	std::mutex mBlockMutex;
	std::mutex mSyncMutex;
	std::condition_variable mBlocker;
	volatile bool mIsBlocked;
};

#endif  // THREADBLOCKER_H
