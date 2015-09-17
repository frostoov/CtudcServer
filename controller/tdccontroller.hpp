#ifndef TDCCONTROLLER_HPP
#define TDCCONTROLLER_HPP

#include "caen/tdcmodule.hpp"

#include <trek/data/tdcsettings.hpp>
#include <trek/net/jcontroller.hpp>

class TdcController : public trek::net::JController {
    using ModulePtr = std::shared_ptr<caen::Module>;
public:
    TdcController(int32_t vmeAddress);
    const std::string& name() const override;
    ModulePtr& getModule();

protected:
    Methods createMethods();

    trek::net::Response init(const trek::net::Request& request);
    trek::net::Response close(const trek::net::Request& request);
    trek::net::Response isInit(const trek::net::Request& request);

    trek::net::Response setLog(const trek::net::Request& request);
    trek::net::Response getLog(const trek::net::Request& request);
    trek::net::Response softwareClear(const trek::net::Request& request);
    trek::net::Response getSettings(const trek::net::Request& request);
    trek::net::Response updateSettings(const trek::net::Request& request);
    trek::net::Response setSettings(const trek::net::Request& request);
    trek::net::Response setTriggerMode(const trek::net::Request& request);
    trek::net::Response setTriggerSubtraction(const trek::net::Request& request);
    trek::net::Response setTdcMeta(const trek::net::Request& request);
    trek::net::Response setWindowWidth(const trek::net::Request& request);
    trek::net::Response setWindowOffset(const trek::net::Request& request);
    trek::net::Response setEdgeDetection(const trek::net::Request& request);
    trek::net::Response setLsb(const trek::net::Request& request);
    trek::net::Response setAlmostFull(const trek::net::Request& request);
    trek::net::Response setControl(const trek::net::Request& request);
    trek::net::Response setDeadTime(const trek::net::Request& request);
    trek::net::Response setEventBLT(const trek::net::Request& request);

    trek::data::Settings createSettings(const trek::net::Request& request) const;
private:
    ModulePtr mDevice;
};

#endif // TDCCONTROLLER_HPP
