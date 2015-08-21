#ifndef CTUDCCONTROLLER_HPP
#define CTUDCCONTROLLER_HPP

#include <functional>
#include <unordered_map>
#include <mutex>

#include <json.hpp>

#include "controller.hpp"

namespace ctudc {

class CtudcController : public Controller {
    using Mutex = std::mutex;
    using Lock  = std::lock_guard<Mutex>;
protected:
    class Request;
    class Response;
    using Method  = std::function<Response(const Request&)>;
    using Methods = std::unordered_map<std::string, Method>;
public:
    std::string handleRequest(const std::string& rawRequest) override;
    static std::string getObject(const std::string& rawRequest);
protected:
    CtudcController(const Methods& method);
    virtual Methods createMethods() = 0;
private:
    Method getMethod(const Request& request) const;
    Methods mMethods;
    Mutex mMutex;
};

class CtudcController::Request {
    using Json       = nlohmann::json;
    using JsonString = Json::string_t;
    using JsonArray  = Json::array_t;
public:
    Request(const std::string& request);
    const JsonString& getObject() const;
    const JsonString& getMethod() const;
    const JsonArray&  getInputs() const;
private:
    JsonString  mObject;
    JsonString  mMethod;
    JsonArray   mInputs;
};

class CtudcController::Response {
    using Json       = nlohmann::json;
    using JsonString = Json::string_t;
    using JsonArray  = Json::array_t;
    using JsonBool   = Json::boolean_t;
public:
    Response(const JsonString& object,
             const JsonString& method,
             const JsonArray&  outputs,
             const JsonBool&   status);
    operator std::string() const;
private:
    JsonString  mObject;
    JsonString  mMethod;
    JsonArray   mOutputs;
    JsonBool    mStatus;
};

}

#endif // CTUDCCONTROLLER_HPP
