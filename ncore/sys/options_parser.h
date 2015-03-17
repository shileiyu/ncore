#ifndef NCORE_SYS_OPTIONS_PARSER_H_
#define NCORE_SYS_OPTIONS_PARSER_H_

#include <ncore/ncore.h>

namespace ncore
{

class Options : public std::map<std::string, std::string>
{
public:
    bool HasOption(const char * key);
};

class OptionsParser
{
    enum TokenID
    {
        kEnd = 0,

        kMinus = '-',
        kEqual = '=',

        kReverse = 255,
        kString,

    };
public:
    OptionsParser(const std::string & cmd_line);

    const Options & GetOptions() const;
private:
    TokenID ReadToken(std::string & token);

    void Parse(const std::string & cmd_line);
private:
    size_t next_pos_;
    std::string cmd_line_;
    Options options_;
};

Options ParseOptions(const std::string & cmd_line);

}

#endif