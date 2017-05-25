#ifndef XHBLIB_UTILITY_OPTIONAL_H_
#define XHBLIB_UTILITY_OPTIONAL_H_

#if (defined __cplusplus) && (__cplusplus >= 201700L)
#include <optional>
using std::optional;
using std::nullopt;
using std::make_optional;
#else
#include <experimental/optional>
using std::experimental::optional;
using std::experimental::nullopt;
using std::make_optional;
#endif

#endif // XHBLIB_UTILITY_OPTIONAL_H_