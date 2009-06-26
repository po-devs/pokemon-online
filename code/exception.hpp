#ifndef EXCEPTION_HPP_INCLUDED
#define EXCEPTION_HPP_INCLUDED

#include <exception>
#include <string>

class MF_Exception : public std::exception
{
    private:
        std::string msg;
    public:
        template <class T>
        MF_Exception(T what):
            std::exception(),
            msg(what)
        {}
        virtual ~MF_Exception() throw()
        {}
        virtual const char* what() const throw()
        {    return msg.c_str();    }
};

typedef MF_Exception InterfaceException;

#endif // EXCEPTION_HPP_INCLUDED
