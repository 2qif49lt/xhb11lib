#include "log.h"

#include <string>

namespace xhb{
    const char* log_level_name[] = {"error","warn","info","debug","trace"};
}

using namespace xhb;
using std::string;

std::ostream& operator<<(std::ostream& out,log_level lev){
    return out<<log_level_name[lev];
}

std::istream& operator>>(std::istream& in,log_level& lev){
     string tmp;
     in >> tmp;
     if(!in) 
        return in;
     for(int idx = 0; idx != sizeof(log_level_name)/sizeof(char*);++idx){
        if(tmp == log_level_name[idx]){
            lev = static_cast<log_level>(idx);
            return in;
        } 
     }
    in.setstate(std::ios::failbit);
    return in;
}