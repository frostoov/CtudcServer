#include "threadmanager.hpp"


using std::make_unique;
using std::thread;

ThreadManager::ThreadManager()
	: mIsActive(false) { }

bool ThreadManager::isActive() {
	return mIsActive;
}

ThreadManager::~ThreadManager() {
	stop();
}

bool ThreadManager::start() {
	if(!mThread && init()) {
		mIsActive = true;
		mThread = make_unique<thread>(&ThreadManager::workerLoop, this);
	}
	return mIsActive;
}

void ThreadManager::stop() {
	if(mThread) {
		mIsActive = false;
		shutDown();
		mThread->join();
		mThread.reset();
		flush();
	}
}

bool ThreadManager::hasProcess() const {
	return bool(mThread);
}

void ThreadManager::workerLoop() {
	while(mIsActive)
		workerFunc();
}
