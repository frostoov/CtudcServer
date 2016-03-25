#pragma once

#include "tdc/caenv2718.hpp"

#include <trek/net/controller.hpp>

class TdcController : public trek::net::Controller {
	using Module = CaenV2718;
	using ModulePtr = std::shared_ptr<Module>;
public:
	TdcController(const std::string& name, const ModulePtr& module);
protected:
	Methods createMethods();

	void open(const trek::net::Request& request, const SendCallback& send);
	void close(const trek::net::Request& request, const SendCallback& send);
	void isOpen(const trek::net::Request& request, const SendCallback& send);

	void clear(const trek::net::Request& request, const SendCallback& send);
	void reset(const trek::net::Request& request, const SendCallback& send);
	void stat(const trek::net::Request& request, const SendCallback& send);
	void ctrl(const trek::net::Request& request, const SendCallback& send);
	void mode(const trek::net::Request& request, const SendCallback& send);
	void tdcMeta(const trek::net::Request& request, const SendCallback& send);
	void setMode(const trek::net::Request& request, const SendCallback& send);
	void setWindowWidth(const trek::net::Request& request, const SendCallback& send);
	void setWindowOffset(const trek::net::Request& request, const SendCallback& send);
	void setEdgeDetection(const trek::net::Request& request, const SendCallback& send);
	void setLsb(const trek::net::Request& request, const SendCallback& send);
	void setCtrl(const trek::net::Request& request, const SendCallback& send);
	void setTdcMeta(const trek::net::Request& request, const SendCallback& send);
	void updateSettings(const trek::net::Request& request, const SendCallback& send);
	void settings(const trek::net::Request& request, const SendCallback& send);
private:
	ModulePtr mDevice;
};
