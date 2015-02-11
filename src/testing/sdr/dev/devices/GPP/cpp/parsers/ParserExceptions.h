#ifndef PARSER_EXCEPTIONS_H_
#define PARSER_EXCEPTIONS_H_

#include <stdexcept>

namespace ParserExceptions
{

class ParseError : public std::runtime_error
{
public:
	explicit ParseError(const std::string& arg): std::runtime_error(arg){};
};

}

#endif
