#include <thread>
#include <iostream>
#include "process.hpp"

using std::thread;

Process::Process()
	: mIsActive(false),
	  mIsInLoop(false) { }

Process::~Process() {
	stop();
}

bool Process::start() {
	if(mIsActive == true)
		return false;
	setActive(true);
	thread(&Process::workerLoop, this).detach();
	return true;
}

void Process::stop() {
	setActive(false);
	while(isInLoop());
}

bool Process::isActive() const {
	return mIsActive;
}

void Process::setActive(bool flag) {
	mIsActive = flag;
}

void Process::setLoop(bool flag) {
	mIsInLoop = flag;
}

bool Process::isInLoop() const {
	return mIsInLoop;
}

