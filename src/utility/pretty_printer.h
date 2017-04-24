#ifndef XHB_PRETTY_PRINTER_H_
#define XHB_PRETTY_PRINTER_H_

#include <iostream>
#include <vector>
#include <tuple>

namespace xhb {
// http://stackoverflow.com/questions/6245735/pretty-print-stdtuple
// 辅助打印tuple
namespace aux {
	template<std::size_t...> struct seq{};
	
	template<std::size_t N, std::size_t... Is>
	struct gen_seq : gen_seq<N-1, N-1, Is...>{};
	
	template<std::size_t... Is>
	struct gen_seq<0, Is...> : seq<Is...>{};
	
	template<class Tuple, std::size_t... Is>
	void print_tuple(std::ostream& out, Tuple const& t, seq<Is...>){
		using swallow = int[];
		(void)swallow{0, (void(out << (Is == 0? "" : ", ") << std::get<Is>(t)), 0)...};
	}
} // aux namespace


} // xhb namespace

namespace std {

// 打印tuple
template<class... Args>
std::ostream& operator<<(std::ostream& out, std::tuple<Args...> const& t) {
	out << "(";
	xhb::aux::print_tuple(out, t, xhb::aux::gen_seq<sizeof...(Args)>());
	return out << ")";
}

// 打印vector
template<typename T>
std::ostream& operator <<( std::ostream& out, const std::vector<T>& object )
{
    out << "[";
    if ( !object.empty() )
    {
        for(typename std::vector<T>::const_iterator
            iter = object.begin();
            iter != --object.end();
            ++iter) {
                out << *iter << ", ";
        }
        out << *--object.end();
    }
    out << "]";
    return out;
}

} // std namespace
#endif // XHB_PRETTY_PRINTER_H_