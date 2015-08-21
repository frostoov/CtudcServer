#ifndef CONTROLLER_HPP
#define CONTROLLER_HPP value

#include <string>

class Controller {
public:
    virtual ~Controller() = default;
    virtual std::string handleRequest(const std::string& request) = 0;
    virtual const std::string& getName() const = 0;
protected:
    Controller() = default;
};

#endif //CONTROLLER_HPP
