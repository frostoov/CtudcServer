#include "appsettings.hpp"

using std::string;

void AppSettings::load(const std::string& fileName) {
	AppConfigParser parser;
	parser.load(fileName);

	unMarshal(parser.config());
}

void AppSettings::save(const std::string& fileName) {
	AppConfigParser parser(marshal());
	parser.save(fileName);
}

nlohmann::json AppSettings::marshal() const {
	return {
		{"address", ip },
		{"port", port },
		{"process", procSettings.marshal()}
	};
}

void AppSettings::unMarshal(const nlohmann::json& doc) {
	ip   = doc.at("address").get<string>();
	port = doc.at("port");
	procSettings.unMarshal(doc.at("process"));
}
