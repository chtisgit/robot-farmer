#ifndef STREXCEPTION_H
#define STREXCEPTION_H

#include <string>

class StringException: public std::exception
{
    public:
        virtual const char* what() const throw();

        StringException(std::string);
    private:
        std::string msg;
};

#endif
