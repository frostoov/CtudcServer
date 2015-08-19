#ifndef CHANNELSCONFIGPARSER_HPP
#define CHANNELSCONFIGPARSER_HPP

#include "configparser.hpp"
#include "managers/channelconfig.hpp"

class ChannelsConfigParser : public AbstractConfigParser {
public:
    void load(const std::string& fileName) override;
    void save(const std::string& fileName) override;
    void load(std::istream& stream) override;
    void save(std::ostream& stream) override;
    const caen::ChannelConfig& getConfig() const;
private:
    caen::ChannelConfig mConfig;

    static const std::string channelsIdent;
    static const std::string numberIdent;
    static const std::string wireIdent;
    static const std::string chamberIdent;
};

#endif // CHANNELSCONFIGPARSER_HPP
