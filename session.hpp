#ifndef SESSION_HPP
#define SESSION_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include <boost/asio.hpp>

#include "caenTDC/tdcmodule.hpp"
#include "handler.hpp"
#include <json.hpp>

class Session : public std::enable_shared_from_this<Session>, public ProcessHandler {

	using DestroyCallback = std::function<void(std::shared_ptr<Session>)>;
	using ModulePtr = std::shared_ptr<caen::Module>;
	using TCP = boost::asio::ip::tcp;
	using Socket = TCP::socket;
	struct Query {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   input;
	};
	struct Response {
		nlohmann::json::string_t  procedure;
		nlohmann::json::array_t   output;
		nlohmann::json::boolean_t status;
	};
	using Procedure = std::function<Response(const Query&)>;
	using Procedures = std::unordered_map<std::string, Procedure>;
public:
	Session(ModulePtr device,Socket&& socket, DestroyCallback callback);
protected:
	std::string createAnswer(const Response& response);
	Procedures createProcedures();
	void workerLoop() override;
	std::string handleMessage(const nlohmann::json& message);

	Response isInit(const Query& query);
	Response getSettings(const Query& query);
	Response updateSettings(const Query& query);
	Response getLog(const Query& query);
	Response init(const Query& query);
	Response close(const Query& query);
	Response setSettings(const Query& query);
	Response setLog(const Query& query);

	Response setTriggerMode(const Query& query);
	Response setTriggerSubtraction(const Query& query);
	Response setTdcMeta(const Query& query);
	Response setWindowWidth(const Query& query);
	Response setWindowOffset(const Query& query);
	Response setEdgeDetection(const Query& query);
	Response setLsb(const Query& query);
	Response setAlmostFull(const Query& query);
	Response setControl(const Query& query);
	Response setDeadTime(const Query& query);
	Response setEventBLT(const Query& query);
private:
	ModulePtr mDevice;
	Socket mSocket;
	DestroyCallback mCallback;
	Procedures mProcedures;
};

#endif // SESSION_HPP
