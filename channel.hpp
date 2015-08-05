#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <atomic>
#include <memory>
#include <mutex>

#include "threadblocker.hpp"

template<typename T>
class Channel {
	enum class _State {
		open,
		close,
	};
	enum class _Flag {
		empty,
		waiting,
	};
	using Object  = T;
	using Flag    = std::atomic<_Flag>;
	using State   = std::atomic<_State>;
	using Mutex   = std::mutex;
	using Lock  = std::unique_lock<Mutex>;
  public:
	Channel() :
		mState(_State::open),
		mSendFlag(_Flag::empty),
		mRecvFlag(_Flag::empty),
		mHasObject(false)	{ }
	Channel(const Channel<T>& other) = delete;
	void operator=(const Channel<T>& other) = delete;

	bool isOpen() const {return mState == _State::open;}

	bool sending() const {return mSendFlag == _Flag::waiting && isOpen();}
	bool receiving() const {return mRecvFlag == _Flag::Waiting && isOpen();}

	void close() {
		if(mState == _State::open) {
			mState = _State::close;
			mBlocker.unblock();
		}
	}


	bool trySend(const Object& object) {
		if(!isOpen() || mRecvFlag != _Flag::waiting)
			return false;
		send(std::move(object));
		return true;
	}

	bool tryRecv(Object& object) {
		if(!isOpen() || mSendFlag != _Flag::waiting)
			return false;
		return recv(object);
	}

	void send(const Object& object) {
		Lock lock(mSendMutex);
		checkOpen();
		mObject = std::move(object);
		mHasObject = true;
		mSendFlag = _Flag::waiting;
		if(mRecvFlag == _Flag::empty)
			mBlocker.block();
		else if(mRecvFlag == _Flag::waiting)
			mBlocker.unblock();
		mSendFlag = _Flag::empty;
	}
	bool recv(Object& object) {
		Lock lock(mRecvMutex);
		checkOpen();
		mRecvFlag = _Flag::waiting;
		if(mSendFlag == _Flag::empty)
			mBlocker.block();
		else if(mSendFlag == _Flag::waiting)
			mBlocker.unblock();
		mRecvFlag = _Flag::empty;
		bool success = mHasObject;
		object = std::move(mObject);
		return success;
	}

  protected:
	void checkOpen() {
		if(mState == _State::close)
			throw std::runtime_error("Channel: channel is closed");
	}

  private:
	Object mObject;
	State  mState;
	Flag   mSendFlag;
	Flag   mRecvFlag;
	Mutex  mSendMutex;
	Mutex  mRecvMutex;

	ThreadBlocker mBlocker;
	std::atomic_bool mHasObject;
};

#endif // CHANNEL_HPP
