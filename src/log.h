#ifndef XHBLIB_LOG_H_
#define XHBLIB_LOG_H_

#include <iostream>

namespace xhb{

enum class log_level{
    error,
    warn,
    info,
    debug,
    trace
};    
} // xhb namespace

std::ostream& operator<<(std::ostream& out,xhb::log_level lev);
std::istream& operator>>(std::istream& in,xhb::log_level lev);

#endif //XHBLIB_LOG_H_