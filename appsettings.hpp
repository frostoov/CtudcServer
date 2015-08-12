#ifndef APPSETTINGS_HPP
#define APPSETTINGS_HPP

#include "configparser/appconfigparser.hpp"
#include "managers/facilitymanager.hpp"

class AppSettings {
public:
	AppSettings() = default;
	void load (const std::string& fileName);
	void save (const std::string& fileName);

	nlohmann::json marshal() const;
	void unMarshal (const nlohmann::json& doc);

	const std::string& getIpAddress() const;
	uint16_t getPort() const;
	const FacilitySettings& getFacilitySettings() const;

	void setIpAddress (const std::string& address);
	void setPort (uint16_t port);
	void setFacilitySettings (const FacilitySettings& settings);

private:
	std::string  mIpAddress;
	uint16_t     mPort;
	FacilitySettings mFacilitySettings;
};

#endif // APPSETTINGS_HPP
