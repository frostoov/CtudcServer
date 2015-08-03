#include <thread>
#include <iostream>
#include "handler.hpp"

using std::thread;

ProcessHandler::ProcessHandler()
	: mIsActive(false),
	  mIsInLoop(false) { }

ProcessHandler::~ProcessHandler() {
	stop();
}

bool ProcessHandler::start() {
	if(mIsActive == true)
		return false;
	setActive(true);
	thread(&ProcessHandler::workerLoop, this).detach();
	return true;
}

void ProcessHandler::stop() {
	setActive(false);
	while(isInLoop());
}

bool ProcessHandler::isActive() const {
	return mIsActive;
}

void ProcessHandler::setActive(bool flag) {
	mIsActive = flag;
}

void ProcessHandler::setLoop(bool flag) {
	mIsInLoop = flag;
}

bool ProcessHandler::isInLoop() const {
	return mIsInLoop;
}

