#include <string>

std::string tohex(int n, int len) {
    std::string retval = "";
    for (int x = 0; x < len; x++) {
        retval = "0123456789ABCDEF"[n & 0xF] + retval;
        n >>= 4;
    }
    return retval;
}

std::string tobin(int n, int len) {
    std::string retval = "";
    for (int x = 0; x < len; x++) {
        retval = "01"[n & 1] + retval;
        n >>= 1;
    }
    return retval;
}
