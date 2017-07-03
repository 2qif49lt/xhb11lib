#ifndef XHBLIB_UTILITY_PRINT_SAFE_H_
#define XHBLIB_UTILITY_PRINT_SAFE_H_

#include <stdio.h>
#include <string.h>
#include <unistd.h>
namespace xhb {
//
// Collection of async-signal safe printing functions.
//

// Outputs string to stderr.
// Async-signal safe.
inline
void print_safe(const char *str, size_t len) noexcept {
    while (len) {
        auto result = write(STDERR_FILENO, str, len);
        if (result > 0) {
            len -= result;
            str += result;
        } else if (result == 0) {
            break;
        } else {
            if (errno == EINTR) {
                // retry
            } else {
                break; // what can we do?
            }
        }
    }
}

// Outputs string to stderr.
// Async-signal safe.
inline
void print_safe(const char *str) noexcept {
    print_safe(str, strlen(str));
}

} // xhb namespace

#endif // XHBLIB_UTILITY_PRINT_SAFE_H_