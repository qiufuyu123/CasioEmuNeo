// Independent from the project.

#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <cstring>

#define ip (totallen - length - buf.size())

std::string tohex(int n, int len) {
    std::string retval = "";
    for (int x = 0; x < len; x++) {
        retval = "0123456789ABCDEF"[n & 0xF] + retval;
        n >>= 4;
    }
    return retval;
}
// Those are important to the disassembler.

// ------------------------------------------------------------------ 1
// ------------------------------------------------------------------ 2

int main(int argc, char** argv) {
    char* st = new char[0x400];
    if (argc != 5) { // argv[0] = executable file name
        std::ifstream fi {"help.txt"};
        do {
            fi.getline(st, 0x400);
        } while (std::strcmp(st, "*") != 0);
        while (true) {
            fi.getline(st, 0x400);
            if (fi.fail()) {
                fi.close();
                return 0;
            }
            std::cout << st << "\n";
        }
    }

    std::ifstream in {argv[1], std::ios_base::binary};
    in.seekg(std::stoi(argv[2], nullptr, 0));
    int length = std::stoi(argv[3], nullptr, 0), l1, i, totallen = length;
    std::ofstream out {argv[4]};
    std::deque<std::uint8_t> buf {};
    char* readbuf = new char[0x10000];

    l1 = std::min(length, 0x10000);
    in.read(readbuf, l1);
    length -= l1;
    for (i = 0; i < l1; i++) {
        buf.push_back(readbuf[i]);
    }

    while (buf.size() > 0) { // assume the block is valid.
        // That part read number in dequeue, pop_front it for some bytes and write to (ofstream) out.

// ------------------------------------------------------------------ 3

// #1

// aaabbb00 f1 #{a}, #{b}
if ((buf[0] & 0b00000011) == 0b00000000) {
    int a = buf[0] >> 5 & 0b111, b = buf[0] >> 2 & 0b111; // >> has higher precedence than &
    out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << "          " // IP and opcode
        << "f1 #" << a << ", #" << b  // command
        << "\n";
    buf.pop_front();
    goto done;
}

// aaabbb01 f2 #{a}, #{b}
if ((buf[0] & 0b00000011) == 0b00000001) {
    int a = buf[0] >> 5 & 0b111, b = buf[0] >> 2 & 0b111;
    out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << "          " // def convenience: exceed 7 spaces, plus one
        << "f2 #" << a << ", #" << b  // command
        << "\n";
    buf.pop_front();
    goto done;
}

// #2

// aaaaaa11 bbbbbbbb f3 #{a}, #{b}
if ((buf[0] & 0b00000011) == 0b00000011 && (buf[1] & 0b00000000) == 0b00000000) {
    int a = buf[0] >> 2 & 0b111111, b = buf[1] >> 0 & 0b11111111;
    out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "       "
        << "f3 #" << a << ", #" << b  // command
        << "\n";
    buf.pop_front(); buf.pop_front();
    goto done;
}

// *

    out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << "          "
        << "Unrecognized command"
        << "\n";
    buf.pop_front();

// ------------------------------------------------------------------ 4

done:
        if (buf.size() < 0x20 && length != 0) { // assume all opcode is shorter than 0x20 bytes
            l1 = std::min(length, 0x10000);
            in.read(readbuf, l1);
            length -= l1;
            for (i = 0; i < l1; i++) {
                buf.push_back(readbuf[i]);
            }
        }
    }

    in.close();
    out.close();

    return 0;
}
