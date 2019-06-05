#pragma once

#include <unordered_map>
#include <nlohmann/json.hpp>

#include "configparser/configparser.hpp"

class AppConfigParser : public AbstractConfigParser {
public:
    AppConfigParser() {};
    AppConfigParser(const nlohmann::json&& defaultConfig);

    void load(const std::string& fileName) override;
    void save(const std::string& fileName) override;
    void load(std::istream& stream) override;
    void save(std::ostream& stream) override;

    const nlohmann::json& config() const {
        return mConfig;
    }
private:
    nlohmann::json mConfig;
};
