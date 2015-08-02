#include "observer.hpp"

using std::unique_lock;
using std::mutex;
using std::string;

Subject::Subject() :
	mNeedLog(false) { }

Subject::~Subject() { }

void Subject::attach(Observer* observer) {
	mObservers.push_back(observer);
}

void Subject::detach(Observer* observer) {
	mObservers.remove(observer);
}

void Subject::notify() {
	for(auto& observer : mObservers) {
		if(observer)
			observer->observerUpdate(this);
	}
}

bool Subject::hasMessages() const {
	return !mMessages.empty();
}

void Subject::pushMessage(const string& text) const {
	if(mNeedLog) {
		unique_lock<mutex> locker(mMessagesMutex);
		if(mMessages.size() >= 1000)
			mMessages.pop();

		mMessages.push({text, SystemClock::now()});
	}
}

Subject::Message Subject::popMessage() const {
	unique_lock<mutex> locker(mMessagesMutex);
	auto message = mMessages.front();
	mMessages.pop();
	return message;
}
