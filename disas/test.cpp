// Independent from the project.

#include <iostream>

std::string signedtohex(int n, int binlen) {
    binlen--;
    std::string retval = (n >> binlen) == 0 ? "" : "-";
    binlen = (binlen + 3) / 4; // ceil of binlen/4
    // now binlen <- hexlen
    for (int x = 0; x < binlen; x++) {
        retval = "0123456789ABCDEF"[n & 0xF] + retval;
        n >>= 4;
    }
    return retval;
}

int main(int argc, char** argv) {
    std::cout << 0xf0 << " " << (0xf0 << 24 >> 24);
}
