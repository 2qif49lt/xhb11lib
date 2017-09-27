#ifndef XHBLIB_STRING_H_
#define XHBLIB_STRING_H_

#include <vector>
#include <string>
#include <sstream>
#include <iterator>

namespace xhb {
template <typename It>
void split(const std::string& s, char delim, It inserter, bool empty) {
    std::stringstream ss(s);
    std::string block;
    
    while (std::getline(ss, block, delim)) {
        if (empty == false && block == "") {
            continue;
        }
        *(inserter++) = block;
    }

}
inline std::vector<std::string> split(const std::string& s, char delim, bool empty = false) {
    std::vector<std::string> vec;
    split(s, delim, std::back_inserter(vec), empty);
    return vec;
}

} // xhb namespace

#endif // XHBLIB_STRING_H_
