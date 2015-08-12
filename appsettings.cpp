#include "appsettings.hpp"

using std::string;

void AppSettings::load (const std::string& fileName) {
	AppConfigParser parser;
	parser.load (fileName);

	unMarshal (parser.getConfig() );
}

void AppSettings::save (const std::string& fileName) {
	AppConfigParser parser (marshal() );
	parser.save (fileName);
}

nlohmann::json AppSettings::marshal() const {
	return {
		{"address", getIpAddress() },
		{"port", getPort() },
		{"facility", mFacilitySettings.marshal() }
	};
}

void AppSettings::unMarshal (const nlohmann::json& doc) {
	setIpAddress (doc.at ("address").get<string>() );
	setPort (doc.at ("port").get<uint16_t>() );
	mFacilitySettings.unMarshal (doc.at ("facility") );
}

const std::string& AppSettings::getIpAddress() const {
	return mIpAddress;
}

uint16_t AppSettings::getPort() const {
	return mPort;
}

const FacilitySettings& AppSettings::getFacilitySettings() const {
	return mFacilitySettings;
}

void AppSettings::setIpAddress (const std::string& address) {
	mIpAddress = address;
}

void AppSettings::setPort (uint16_t port) {
	mPort = port;
}

void AppSettings::setFacilitySettings (const FacilitySettings& settings) {
	mFacilitySettings = settings;
}