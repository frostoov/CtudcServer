#ifndef FTDCONTROLLER_HPP
#define FTDCONTROLLER_HPP

#include "ftd/ftdmodule.hpp"
#include "ctudccontroller.hpp"

namespace ctudc {

class FtdController : public CtudcController {
    using ModulePtr = std::shared_ptr<ftdi::Module>;
public:
    FtdController(uint32_t address);
    const std::string& getName() const override;
protected:
    Methods createMethods() override;

    Response init(const Request& request);
    Response close(const Request& request);
    Response isInit(const Request& request);
    Response setCodes(const Request& request);
private:
    ModulePtr mDevice;
    const std::string mName;
};

}

#endif // FTDCONTROLLER_HPP
