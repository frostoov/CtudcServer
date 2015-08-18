#include <fstream>
#include "appconfigparser.hpp"

using std::ifstream;
using std::ofstream;
using std::string;
using std::getline;
using std::istreambuf_iterator;
using nlohmann::json;

AppConfigParser::AppConfigParser(const json&& defaultConfig) {
    mConfig = std::move(defaultConfig);
}

void AppConfigParser::load(const string& fileName) {
    ifstream configFile;
    configFile.exceptions(configFile.badbit | configFile.failbit);
    configFile.open(fileName, configFile.binary);
    load(configFile);
}

void AppConfigParser::save(const string& fileName) {
    ofstream configFile;
    configFile.exceptions(configFile.failbit | configFile.badbit);
    configFile.open(fileName, configFile.binary);
    save(configFile);
}

void AppConfigParser::load(std::istream& stream) {
    string jsonText({istreambuf_iterator<char> (stream), istreambuf_iterator<char>() });
    mConfig = json::parse(jsonText);
}

void AppConfigParser::save(std::ostream& stream) {
    stream << mConfig.dump(4);
}
