#pragma once

#include "configparser.hpp"
#include "exposition/channelconfig.hpp"

class ChannelsConfigParser : public AbstractConfigParser {
public:
    void load(const std::string& fileName) override;
    void save(const std::string& fileName) override;
    void load(std::istream& stream) override;
    void save(std::ostream& stream) override;
    const ChannelConfig& getConfig() const;
private:
    ChannelConfig mConfig;

    static const std::string channelsIdent;
    static const std::string numberIdent;
    static const std::string wireIdent;
    static const std::string chamberIdent;
};
