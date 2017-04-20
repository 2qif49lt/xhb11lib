#ifndef XHBLIB_LOGGER_H_
#define XHBLIB_LOGGER_H_


// 简单日志,不能用于跨线程

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <vector>
#include <utility>
#include <thread>
#include <ctime> // localtime
#include <cstdio> // sprintf
#include <chrono>
#include <sstream>
#include <iomanip>

//#include "utility/sprinter.h"

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
std::istream& operator>>(std::istream& in,xhb::log_level& lev);


namespace xhb{
   
extern void safe_printf(std::ostringstream& out,const char* fmt);

template<typename T,typename... E>
void safe_printf(std::ostringstream& out,const char* fmt, T val,E... else_args){
    for(; *fmt != '\0'; ++fmt){
        if(*fmt == '%'){
            if(*(fmt + 1) == '%')
                ++fmt;
            else{
                out << val;
                safe_printf(out,fmt + 1,else_args...);
                return;
            }
        }
        out << *fmt;
    }
}
  
class logger final{
    std::string _name;
    std::atomic<log_level> _level = {log_level::info};
    static std::atomic<bool> _stdout;
    static std::atomic<int> _idx;
public:
    logger();
    logger(const std::string& name);
    ~logger();

    inline std::string name() const {return _name;}
    inline log_level level() const { return _level.load(std::memory_order_relaxed);}
    inline void set_level(log_level lev) {_level.store(lev,std::memory_order_relaxed);}
    inline bool is_logable(log_level lev) const {
        return lev <= _level.load(std::memory_order_relaxed);
    }

    inline static void set_stdout_enable(bool b){
        _stdout.store(b,std::memory_order_relaxed);
    }

    // usage: log(info,"hello%,here you are!",name);
    template<typename... A>
    void log(log_level lev,const char* fmt, A&&... args){
        if(is_logable(lev) == false)
            return;
        bool is_std_enable = _stdout.load(std::memory_order_relaxed);
        if(is_std_enable == false)
            return;
        std::ostringstream out;
        out << "[" <<  std::left << std::setw(5) << lev << " ";
        
        auto now = std::chrono::system_clock::now();
        auto ms = (int)(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000);
        auto tt = std::chrono::system_clock::to_time_t(now);
        auto* tm = std::localtime(&tt);
        char tp[128];
        std::snprintf(tp,sizeof(tp),"%04d-%02d-%02d %02d:%02d:%02d,%03d",
                      tm->tm_year + 1900,tm->tm_mon + 1, tm->tm_mday,
                      tm->tm_hour,tm->tm_min,tm->tm_sec,
                      ms);
        out << tp << " " << std::this_thread::get_id() << "/" << _name<<"]: ";
        
        safe_printf(out,fmt,std::forward<A>(args)...);
        
        out << "\n";
        std::cout << out.str();
        
    }
  
    template<typename... A>
    void error(const char* fmt, A&&... args){
        log(log_level::error,fmt,std::forward<A>(args)...);
    }
    
    template<typename... A>
    void warn(const char* fmt, A&&... args){
        log(log_level::warn,fmt,std::forward<A>(args)...);
    }

    template<typename... A>
    void info(const char* fmt, A&&... args){
        log(log_level::info,fmt,std::forward<A>(args)...);
    }

    template<typename... A>
    void debug(const char* fmt, A&&... args){
        log(log_level::debug,fmt,std::forward<A>(args)...);
    }

    template<typename... A>
    void trace(const char* fmt, A&&... args){
        log(log_level::trace,fmt,std::forward<A>(args)...);
    }
};

class log_registry final{
    mutable std::mutex _mutex;
    std::unordered_map<std::string,logger*> _loggers;
private:
    log_registry(){};
public:
    static log_registry& instance();

    void registers(logger* log);
    void unregister(logger* log);

    void set_levels(log_level lev);
    
    log_level get_level(const std::string& name)const;
    void set_level(const std::string& name,log_level lev);

    std::vector<std::string> get_names()const;

};


} // xhb namespace
#endif //XHBLIB_LOGGER_H_
