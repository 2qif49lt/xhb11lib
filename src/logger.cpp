#include "logger.h"

#include <utility>
#include <algorithm>


namespace xhb{
    const char* log_level_name[] = {"error","warn","info","debug","trace"};
    const char* level_to_name(log_level lev){
        return log_level_name[static_cast<int>(lev)];
    }
}

using namespace xhb;
using std::string;

std::ostream& operator<<(std::ostream& out,log_level lev){
    return out<<log_level_name[static_cast<int>(lev)];
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

namespace xhb{

    void safe_printf(std::ostringstream& out,const char* fmt){
        out<<fmt;
    }

std::atomic<int> logger::_idx = {0};
std::atomic<bool> logger::_stdout = {true};

logger::logger(){
    _name = "default" + std::to_string(_idx.load(std::memory_order_relaxed));
    _idx++;
    log_registry::instance().registers(this);
}

logger::logger(const string& name):_name(name){
    log_registry::instance().registers(this);
}

logger::~logger(){
    log_registry::instance().unregister(this);
}

// log_registry
log_registry& log_registry::instance(){
    static log_registry ins;
    return ins;
}

void log_registry::registers(logger* log){
     std::lock_guard<std::mutex> lock(_mutex);
     _loggers[log->name()] = log;
}

void log_registry::unregister(logger* log){
    std::lock_guard<std::mutex> lock(_mutex);
    _loggers.erase(log->name());
}

void log_registry::set_levels(log_level lev){
    std::lock_guard<std::mutex> lock(_mutex);
    for(auto& lp : _loggers){
        lp.second->set_level(lev);
    }
}

log_level log_registry::get_level(const std::string& name)const{
    std::lock_guard<std::mutex> lock(_mutex);
    return _loggers.at(name)->level();
}


void log_registry::set_level(const std::string& name,log_level lev){
    std::lock_guard<std::mutex> lock(_mutex);
    _loggers.at(name)->set_level(lev);
}

std::vector<std::string> log_registry::get_names()const{
    std::lock_guard<std::mutex> lock(_mutex);
    std::vector<std::string> vec;
    // c++ 14
    std::for_each(_loggers.begin(),_loggers.end(),[&vec](const auto& p){vec.push_back(p.first);});
    return  vec;
}

}// xhb namespace
