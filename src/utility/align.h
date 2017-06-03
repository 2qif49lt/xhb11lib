#ifndef XHBLIB_UTILITY_ALIGN_H_
#define XHBLIB_UTILITY_ALIGN_H_

namespace xhb {
/*
cout << align_up<2 << 4>(122) << " " << align_down<16>(122) <<endl;
cout << align_up(122,16) << " " << align_down(122,16) <<endl;
*/

// 适用2的幂
template<size_t A, typename T>
inline constexpr T align_up(T v) {
	static_assert((A & (A - 1)) == 0, "parameter must be  a power of 2.");
    return (v + A - 1) & (~(A - 1));
}

// 指针类型
template <size_t A, typename T>
inline constexpr
T* align_up(T* v) {
    static_assert(sizeof(T) == 1, "align byte pointers only");
    return reinterpret_cast<T*>(align_up<A>(reinterpret_cast<uintptr_t>(v)));
}

template<size_t A, typename T>
inline T constexpr align_down(T v) {
	static_assert((A & (A - 1)) == 0, "parameter must be  a power of 2.");
    return v & (~(A - 1));
}

template <size_t A, typename T>
inline constexpr
T* align_down(T* v) {
    static_assert(sizeof(T) == 1, "align byte pointers only");
    return reinterpret_cast<T*>(align_down<A>(reinterpret_cast<uintptr_t>(v)));
}

// 通用
template<typename T>
T align_up(T num, size_t mod) {
	return mod * ((num + mod - 1) / mod);
}

template <typename T>
inline constexpr
T* align_up(T* v, size_t mod) {
    static_assert(sizeof(T) == 1, "align byte pointers only");
    return reinterpret_cast<T*>(align_up(reinterpret_cast<uintptr_t>(v), mod));
}

template<typename T>
T align_down(T num, size_t mod) {
	return mod * (num / mod);
}

template <typename T>
inline constexpr
T* align_down(T* v, size_t mod) {
    static_assert(sizeof(T) == 1, "align byte pointers only");
    return reinterpret_cast<T*>(align_down(reinterpret_cast<uintptr_t>(v), mod));
}
} // xhb namespace


#endif // XHBLIB_UTILITY_ALIGN_H_