#include "threadmanager.hpp"

using std::unique_lock;
using std::mutex;

using std::make_unique;
using std::thread;

ThreadManager::ThreadManager()
	: mIsActive(false) { }

ThreadManager::~ThreadManager() {
	stop();
}

void ThreadManager::joinThread() {
	unique_lock<mutex> lock(mThreadMutex);
	mThread->join();
}

void ThreadManager::resetThread() {
	unique_lock<mutex> lock(mThreadMutex);
	if(mThread && mThread->joinable()) {
		mThread->join();
	}
	mThread.reset();
}

bool ThreadManager::startThread(std::function<void()>&& func) {
	unique_lock<mutex> lock(mThreadMutex);
	if(mThread)
		return false;
	else {
		mThread = make_unique<thread>(std::move(func));
		return true;
	}
}



bool ThreadManager::start() {
	if(!mThread && init()) {
		mIsActive = startThread([this]() {
			workerLoop();
		});
	}
	return mIsActive;
}

void ThreadManager::stop() {
	if(mThread) {
		mIsActive = false;
		resetThread();
		shutDown();
	}
}

bool ThreadManager::hasProcess() const {
	return bool(mThread);
}

void ThreadManager::workerLoop() {
	while(mIsActive)
		workerFunc();
}
