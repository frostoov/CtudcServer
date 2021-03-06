#include <fstream>
#include <json.hpp>

#include "channelsconfigparser.hpp"

using std::ifstream;
using std::ofstream;
using std::string;
using std::istreambuf_iterator;
using nlohmann::json;

const std::string ChannelsConfigParser::chamberIdent("chamber");
const std::string ChannelsConfigParser::channelsIdent("channels");
const std::string ChannelsConfigParser::wireIdent("wire");
const std::string ChannelsConfigParser::numberIdent("number");

void ChannelsConfigParser::load(const std::string& fileName) {
    mConfig.clear();
    ifstream stream;
    stream.exceptions(stream.badbit | stream.failbit);
    stream.open(fileName);

    load(stream);
}

void ChannelsConfigParser::save(const std::string& fileName) {
    ofstream stream;
    stream.exceptions(ofstream::badbit | ofstream::failbit);
    stream.open(fileName);

    save(stream);
}

void ChannelsConfigParser::load(std::istream& stream) {
    string configString({istreambuf_iterator<char> (stream), istreambuf_iterator<char>() });
    auto jsonConfig = json::parse(configString);

    const auto& channelsConfig = jsonConfig.at(channelsIdent);
    for(const auto& channelConfig : channelsConfig) {
        const auto channelNumber = channelConfig.at(numberIdent).get<unsigned>() - 1;
        if(mConfig.count(channelNumber))
            throw std::runtime_error("ChannelsConfigParser::load: channel config already exists");
        const auto wireNumber    = channelConfig.at(wireIdent).get<unsigned>() - 1;
        const auto chamberNumber = channelConfig.at(chamberIdent).get<unsigned>() - 1;

        mConfig.insert({channelNumber, {chamberNumber, wireNumber} });
    }
}

void ChannelsConfigParser::save(std::ostream& stream) {
    json config;
    for(const auto& configPair : mConfig) {
        const auto& channelNumber = configPair.first;
        const auto& channelConfig = configPair.second;
        json channel{
            {numberIdent,  channelNumber + 1},
            {chamberIdent, channelConfig.chamber + 1},
            {wireIdent,    channelConfig.wire + 1}
        };
        config[channelsIdent].push_back(channel);
    }
    stream << config.dump(4);
}

const ChannelConfig& ChannelsConfigParser::getConfig() const {
    return mConfig;
}
