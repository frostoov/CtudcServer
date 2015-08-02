#ifndef OBSERVER_H
#define OBSERVER_H

#include <list>
#include <string>
#include <queue>
#include <mutex>
#include <chrono>

class Subject;

class Observer {
  public:
	virtual ~Observer() {}
	virtual void observerUpdate(const Subject* subject) = 0;
  protected:
	Observer() {}
};

class Subject {
  public:
	using SystemClock  = std::chrono::high_resolution_clock;
	using TimePoint    = SystemClock::time_point;
	using Message      = std::pair<std::string, TimePoint>;
	using ObserverList = std::list<Observer*>;
  private:
	using MessageQueue = std::queue<Message>;
  public:
	virtual ~Subject();
	void attach(Observer* observer);
	void detach(Observer* observer);
	void notify();
	bool hasMessages() const;
	Message popMessage() const;
	void setLog(bool flag) {mNeedLog = flag;}
	virtual const char* getTitle() const = 0;
  protected:
	Subject();
	void pushMessage(const std::string& text) const;
  private:
	ObserverList mObservers;
	MessageQueue mutable mMessages;
	std::mutex mutable mMessagesMutex;
	volatile bool mNeedLog;
};

#endif  // OBSERVER_H
