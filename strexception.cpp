#include "strexception.h"

StringException::StringException(std::string message) : msg(message) {
}

const char* StringException::what() const throw() {
    return msg.c_str();
}
