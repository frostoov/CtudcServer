#ifndef HANDLER_HPP
#define HANDLER_HPP


class ProcessHandler
{
public:
	virtual ~ProcessHandler();
	virtual bool start();
	virtual void stop();
	bool isActive() const;
protected:
	ProcessHandler();
	virtual void workerLoop() = 0;
	void setActive(bool flag);
	void setLoop(bool flag);
	bool isInLoop() const;
private:
	volatile bool mIsActive;
	volatile bool mIsInLoop;
};

#endif // HANDLER_HPP
