#include "log.h"
#include <utility>
#include <sstream>
#include <chrono>
#include <ctime> // localtime
#include <cstdio> // sprintf

namespace xhb{
    const char* log_level_name[] = {"error","warn","info","debug","trace"};
    const char* level_to_name(log_level lev){
        return log_level_name[static_cast<int>(lev)];
    }
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

namespace xhb{

std::atomic<int> logger::_idx = {0};

logger::logger(){
    _name = "default" + std::to_string(_idx++.load(std::memory_order_relaxed));
    log_registry::instance().register(this);  
}

logger::logger(const string& name):_name(name){
    log_registry::instance().register(this);
}

logger::~logger(){
    log_registry::instance().unregister(this);
}

void xhb_printf(std::ostringstream& out,const char* fmt){
    out<<fmt;
}

template<typename T,typename... E>
void xhb_printf(std::ostringstream& out,const char* fmt, T val,E else_args){
    for(; *fmt != '\0'; ++fmt){
        if(*fmt == '%'){
            if(*(fmt + 1) == '%')
                ++fmt;
            else{
                out << value;
                xhb_printf(out,fmt + 1,else_args...);
                return;
            }
        }
        out << *fmt;
    }
}
template<typename...A>
void logger::log(log_level lev,const char* fmt,A... args){
    if(is_logable(lev) == false) 
        return;
    bool is_std_enable = _stdout.load(std::memory_order_relaxed);
    if(is_std_enable == false)
        return;
    std::ostringstream out;
    out   <<level_to_name << " ";

    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::millisecond>(now.time_since_epoch()).count() % 1000;
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto* tm = std::localtime(&tt);
    char tp[128];
    std::snprintf(tp,sizeof(tp),"%04d-%02d-%02d %02d:%02d:%02d,%03d",
        tm->tm_year + 1900,tm->tm_mon + 1, tm->tm_mday,
        tm->tm_hour,tm->tm_min,tm->tm_sec,
        ms);
    out << tp << " " << std::this_thread::get_id() << ": ";
   
    xhb_printf(out,fmt,args...)
    
    out << "\n";
    std::cout << out.str();
}

// log_registry
log_registry& log_registry::instance(){
    static log_registry ins;
    return ins;
}

void log_registry::register(logger* log){
     std::lock_guard<std::mutex> lock(_mutex);
     _loggers[log->name()] = log;
}

void log_registry::unregister(logger* log){
    std::lock_guard<std::mutex> lock(_mutex);
    _loggers.erase(l.name());
}

void log_registry::move(logger* to,logger* from){
    std::lock_guard<std::mutex> lock(_mutex);

}


}// xhb namespace