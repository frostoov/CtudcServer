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
	bool receiving() const {return mRecvFlag == _Flag::waiting && isOpen();}

	void close() {
		if(mState == _State::open) {
			mMutex.lock();
			mState = _State::close;
			mBlocker.unblock();
			mMutex.unlock();
		}
	}

	bool isBlocked() const { return mBlocker.isBlocked(); }

	bool trySend(const Object& object) {
		if(isOpen() && mRecvFlag == _Flag::waiting && mBlocker.isBlocked()) {
			send(object);
			return true;
		} else
			return false;
	}

	bool tryRecv(Object& object) {
		if(isOpen() && mSendFlag == _Flag::waiting && mBlocker.isBlocked())
			return recv(object);
		else
			return false;

	}

	void send(const Object& object) {
		if(!isOpen()) return;
		Lock lock(mSendMutex);
		mMutex.lock();
		mObject = std::move(object);
		mHasObject = true;
		mSendFlag = _Flag::waiting;

		if(mRecvFlag == _Flag::empty && isOpen()) {
			mMutex.unlock();
			mBlocker.block();
		}
		else if(mRecvFlag == _Flag::waiting) {
			mBlocker.unblock();
			mMutex.unlock();
		}
		mSendFlag = _Flag::empty;
	}
	bool recv(Object& object) {
		if(!isOpen()) return false;
		Lock lock(mRecvMutex);
		mMutex.lock();
		mRecvFlag = _Flag::waiting;

		if(mSendFlag == _Flag::empty && isOpen()) {
			mMutex.unlock();
			mBlocker.block();
		} else if(mSendFlag == _Flag::waiting) {
			mBlocker.unblock();
			mMutex.unlock();
		}
		mRecvFlag = _Flag::empty;
		bool success = mHasObject;
		object = std::move(mObject);
		mHasObject = false;
		return success;
	}

  private:
	Object mObject;
	State  mState;
	Flag   mSendFlag;
	Flag   mRecvFlag;
	Mutex  mMutex;
	Mutex  mRecvMutex;
	Mutex  mSendMutex;

	ThreadBlocker mBlocker;
	std::atomic_bool mHasObject;
};

#endif // CHANNEL_HPP
