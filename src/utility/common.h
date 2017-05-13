#ifndef XHBLIB_UTILITY_COMMON_H_
#define XHBLIB_UTILITY_COMMON_H_

// 获取唯一名字
#define PP_CAT(a, b) PP_CAT_I(a, b)
#define PP_CAT_I(a, b) PP_CAT_II(~, a ## b)
#define PP_CAT_II(p, res) res
#define UNIQUE_NAME(prefix) PP_CAT(prefix, __COUNTER__)
#endif // XHBLIB_UTILITY_COMMON_H_