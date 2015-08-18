#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include <string>
#include <istream>
#include <ostream>

class AbstractConfigParser {
public:
    virtual ~AbstractConfigParser() {}
    virtual void load(const std::string& fileName) = 0;
    virtual void save(const std::string& fileName) = 0;
    virtual void load(std::istream& stream) = 0;
    virtual void save(std::ostream& stream) = 0;
protected:
    AbstractConfigParser() = default;

private:
};

#endif // CONFIG_PARSER_H
