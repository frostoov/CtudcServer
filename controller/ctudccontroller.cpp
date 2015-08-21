#include "ctudccontroller.hpp"

namespace ctudc {

using std::string;

using nlohmann::json;

CtudcController::CtudcController(const Methods& method)
    : mMethods(method) { }

std::string CtudcController::handleRequest(const std::string& rawRequest) {
    Lock lock(mMutex);
    Request request(rawRequest);
    auto method = getMethod(request);
    return method(request);
}

string CtudcController::getObject(const std::string& rawRequest) {
    return Request(rawRequest).getObject();
}

CtudcController::Method CtudcController::getMethod(const Request& request) const {
    if(request.getObject() == getName())
        return mMethods.at(request.getMethod());
    else
        throw std::logic_error("Invalid request object");
}

/**********************************************************************************************************************/


CtudcController::Request::Request(const std::string& request) {
    auto jsonRequest = json::parse(request);
    mObject = jsonRequest.at("object").get<JsonString>();
    mMethod = jsonRequest.at("method").get<JsonString>();
    mInputs = jsonRequest.at("inputs").get<JsonArray>();
}

const CtudcController::Request::JsonString& CtudcController::Request::getObject() const {
    return mObject;
}

const CtudcController::Request::JsonString& CtudcController::Request::getMethod() const {
    return mMethod;
}

const CtudcController::Request::JsonArray& CtudcController::Request::getInputs() const {
    return mInputs;
}

CtudcController::Response::Response(const JsonString& object,
                                    const JsonString& method,
                                    const JsonArray& outputs,
                                    const JsonBool& status)
    : mObject(object),
      mMethod(method),
      mOutputs(outputs),
      mStatus(status) { }

ctudc::CtudcController::Response::operator string() const {
    return json{
        {"object",  mObject},
        {"method",  mMethod},
        {"outputs", mOutputs},
        {"status",  mStatus},
    } .dump();
}



}
