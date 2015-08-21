#ifndef TDCCONTROLLER_HPP
#define TDCCONTROLLER_HPP

#include <trekdata/tdcsettings.hpp>

#include "ctudccontroller.hpp"
#include "caen/tdcmodule.hpp"

namespace ctudc {

class TdcController : public CtudcController {
    using ModulePtr = std::shared_ptr<caen::Module>;
public:
    TdcController(int32_t vmeAddress);
    const std::string& getName() const override;
    ModulePtr& getModule();

protected:
    Methods createMethods() override;

    Response init(const Request& request);
    Response close(const Request& request);
    Response isInit(const Request& request);

    Response setLog(const Request& request);
    Response getLog(const Request& request);
    Response softwareClear(const Request& request);
    Response getSettings(const Request& request);
    Response updateSettings(const Request& request);
    Response setSettings(const Request& request);
    Response setTriggerMode(const Request& request);
    Response setTriggerSubtraction(const Request& request);
    Response setTdcMeta(const Request& request);
    Response setWindowWidth(const Request& request);
    Response setWindowOffset(const Request& request);
    Response setEdgeDetection(const Request& request);
    Response setLsb(const Request& request);
    Response setAlmostFull(const Request& request);
    Response setControl(const Request& request);
    Response setDeadTime(const Request& request);
    Response setEventBLT(const Request& request);

    trekdata::Settings createSettings(const Request& request) const;
private:
    ModulePtr mDevice;
    const std::string mName;
};

}

#endif // TDCCONTROLLER_HPP
