#include "options_parser.h"


namespace ncore
{

Options ParseOptions(const std::string & cmd_line)
{
    OptionsParser parser(cmd_line);
    return parser.GetOptions();
}

bool Options::HasOption(const char * key)
{
    return find(key) != end();
}

OptionsParser::OptionsParser(const std::string & cmd_line)
    :next_pos_(0)
{
    Parse(cmd_line);
}

const Options & OptionsParser::GetOptions() const
{
    return options_;
}

void OptionsParser::Parse(const std::string & cmd_line)
{
    next_pos_ = 0;
    options_.clear();
    cmd_line_ = cmd_line;

    std::string token;
    TokenID id = kEnd;
    std::string option_key;
    do 
    { 
        if(kMinus == id)
        {//开始解析参数
            id = ReadToken(token);
            if(kString != id || token.empty())
                return;
            option_key = token;

            id = ReadToken(token);
            if(kEqual != id)
            {//如果没读到等号
                options_[option_key] = "";
                continue;
            }
            else
            {//读到等号后
                //value允许空字符串
                id = ReadToken(token);
                if(kString != id)
                    return;
                options_[option_key] = token;
            }
        }
        id = ReadToken(token);
    } while (kEnd != id);

}

OptionsParser::TokenID OptionsParser::ReadToken(std::string & token)
{
    if(next_pos_ >= cmd_line_.size())
        return kEnd;

    char cur = 0;
    //跳过开头的不可见字符
    while(0 != isspace( cur = cmd_line_[next_pos_++] ) )
    {//字符串 全是不可见字符 则退出
        if(next_pos_ >= cmd_line_.size())
            return kEnd;
    }

    assert(cur != ' ');
    //
    if('-' == cur)
    {
        token = cur;
        return kMinus;
    }
    else if('=' == cur)
    {
        token = cur;
        return kEqual;
    }
    else
    {
        std::string cur_token;
        if('"' != cur)
        {//读取字符直到遇到空格
            cur_token += cur;
            while(cmd_line_.size() > next_pos_) 
            {
                char next = cmd_line_[next_pos_];
                //看下一个字符决定是否返回
                if('-' == next)
                    break;
                if('=' == next)
                    break;
                if(0 != isspace(next))
                    break;//不可见字符

                cur = cmd_line_[next_pos_++];
                cur_token += cur;
            }
        }
        else
        {//读取字符直到遇到"
            while(cmd_line_.size() > next_pos_)
            {
                cur = cmd_line_[next_pos_++];
                if('"' == cur)//找到配对字符就返回
                    break;
                else
                {//如果已经到末尾还为找到匹配字符 就直接返回
                    if(cmd_line_.size() <= next_pos_)
                        return kEnd;
                    cur_token += cur;
                }
            }
        }
        token = cur_token;
        return kString;
    }
}


}