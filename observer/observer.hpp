#ifndef OBSERVER_H
#define OBSERVER_H

#include <list>
#include <string>
#include <queue>
#include <mutex>
#include <chrono>
#include <memory>

class Subject;

class Observer {
  public:
	virtual ~Observer() {}
	virtual void observerUpdate(const Subject* subject) = 0;
  protected:
	Observer() = default;
};

class Subject {
  public:
	using SystemClock  = std::chrono::high_resolution_clock;
	using TimePoint    = SystemClock::time_point;
	using Message      = std::pair<std::string, TimePoint>;
	using ObserverPtr  = std::shared_ptr<Observer>;
	using ObserverList = std::list<ObserverPtr>;
  private:
	using MessageQueue = std::queue<Message>;
  public:
	virtual ~Subject() = default;
	void attach(ObserverPtr observer);
	void detach(ObserverPtr observer);
	void notify();
	bool hasMessages() const;
	Message&& popMessage() const;
	void setLog(bool flag) {mNeedLog = flag;}
	virtual const char* getTitle() const = 0;
  protected:
	Subject();
	void pushMessage(std::string&& text) const;
  private:
	ObserverList mObservers;
	MessageQueue mutable mMessages;
	std::mutex mutable mMessagesMutex;
	volatile bool mNeedLog;
};

#endif  // OBSERVER_H
