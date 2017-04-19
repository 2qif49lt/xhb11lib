#ifndef XHBLIB_LOG_H_
#define XHBLIB_LOG_H_

#include <iostream>
#include <string>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <vector>

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

class log_registry;

class logger final{
    std::string _name;
    std::atomic<log_level> _level = {log_level::info};
    static std::atomic<bool> _stdout;
    static std::atomic<int> _idx;
public:
    logger();
    logger(const string& name);
    ~logger();

    inline string name() const {return _name;}
    inline log_level level() const { return _level.load(std::memory_order_relaxed);}
    inline void set_level(log_level lev) {_level.store(lev,std::memory_order_relaxed);}
    inline bool is_logable(log_level lev) const {
        return lev <= _level.load(std::memory_order_relaxed);
    }

    inline static void set_stdout_enable(bool b){
        _stdout.store(b,std::memory_order_relaxed);
    }

    template<typename... A>
    void log(log_level lev,const char* fmt, A&&... args);
};

class log_registry final{
    mutable std::mutex _mutex;
    std::unordered_map<std::string,logger*> _loggers;
private:
    log_registry(){};
public:
    static log_registry& instance();

    void register(logger* log);
    void unregister(logger* log);

    void set_levels(log_level lev);
    
    log_level get_level(const std::string& name)const;
    void set_level(const std::string& name,log_level lev);

    std::vector<std::string> get_names()const;

};


} // xhb namespace
#endif //XHBLIB_LOG_H_