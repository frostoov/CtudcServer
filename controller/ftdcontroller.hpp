#ifndef FTDCONTROLLER_HPP
#define FTDCONTROLLER_HPP

#include "ftd/ftdmodule.hpp"

#include <trek/net/jcontroller.hpp>

class FtdController : public trek::net::JController {
    using ModulePtr = std::shared_ptr<ftdi::Module>;
public:
    FtdController(uint32_t address);
    const std::string& name() const override;
protected:
    Methods createMethods();

    trek::net::Response init(const trek::net::Request& request);
    trek::net::Response close(const trek::net::Request& request);
    trek::net::Response isInit(const trek::net::Request& request);
    trek::net::Response setCodes(const trek::net::Request& request);
private:
    ModulePtr mDevice;
};


#endif // FTDCONTROLLER_HPP
