#ifndef TREKVIEWER_APPCONFIGPARSER_HPP
#define TREKVIEWER_APPCONFIGPARSER_HPP

#include <unordered_map>
#include <json.hpp>

#include "configparser/configparser.hpp"

class AppConfigParser : public AbstractConfigParser {
public:
    AppConfigParser() {};
    AppConfigParser(const nlohmann::json&& defaultConfig);

    void load(const std::string& fileName) override;
    void save(const std::string& fileName) override;
    void load(std::istream& stream) override;
    void save(std::ostream& stream) override;

    const nlohmann::json& getConfig() const {
        return mConfig;
    }
private:
    nlohmann::json mConfig;
};

#endif // TREKVIEWER_APPCONFIGPARSER_HPP
