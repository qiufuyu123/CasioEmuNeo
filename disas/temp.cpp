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
// This program doesn't have IP, but the disassembler use ip (instruction pointer).

#define pc ip

/* Have zero padding at first if necessary.
   The number with maximum length is (1 << (binlen - 1)), equal to the smallest
   number. It is a power of 2, thus first digit cannot be hexadecimal.
   The number has binlen digits. So, hexlen = ceil(binlen/4).
*/
std::string signedtohex(int n, int binlen) {
	// n satisfy (unsigned)n < (1 << binlen)
    binlen--;
    bool ispositive = (n >> binlen) == 0;
	if (!ispositive) n = (2 << binlen) - n;
	std::string retval = "";
    binlen = 1 + binlen / 4; // ceil of (old)binlen/4
    // now binlen <- hexlen
    for (int x = 0; x < binlen; x++) {
        retval = "0123456789ABCDEF"[n & 0xF] + retval;
        n >>= 4;
    }
    return ispositive ? retval : ("-" + retval);
}

std::string cond [16] = {"GE", "LT", "GT", "LE", "GES", "LTS", "GTS", "LES",
		"NE", "EQ", "NV", "OV", "PS", "NS", "AL", "<Unrecognized>"};

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
// # 2
// iiiiiiii 11100011 DSR<-   0{tohex(i, 2)}h
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100011) {
	int i = buf[0] >> 0 & 0b11111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DSR<-   0" << (tohex(i, 2)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// dddd1111 10010000 DSR<-   R{d}
if ((buf[0] & 0b00001111) == 0b00001111 && (buf[1] & 0b11111111) == 0b10010000) {
	int d = buf[0] >> 4 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DSR<-   R" << (d) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10011111 11111110 DSR<-   DSR
if ((buf[0] & 0b11111111) == 0b10011111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DSR<-   DSR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0001 1000nnnn ADD     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADD     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0001nnnn ADD     R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00010000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADD     R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00110 1111nnn0 ADD     ER{n*2}, ER{m*2}
if ((buf[0] & 0b00011111) == 0b00000110 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADD     ER" << (n*2) << ", ER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 1iiiiiii 1110nnn0 ADD     ER{n*2}, #{i << 25 >> 25}
if ((buf[0] & 0b10000000) == 0b10000000 && (buf[1] & 0b11110001) == 0b11100000) {
	int i = buf[0] >> 0 & 0b1111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADD     ER" << (n*2) << ", #" << (i << 25 >> 25) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0110 1000nnnn ADDC    R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000110 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADDC    R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0110nnnn ADDC    R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01100000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADDC    R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0010 1000nnnn AND     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000010 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "AND     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0010nnnn AND     R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00100000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "AND     R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0111 1000nnnn CMP     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000111 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CMP     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0111nnnn CMP     R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01110000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CMP     R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0101 1000nnnn CMPC    R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000101 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CMPC    R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0101nnnn CMPC    R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01010000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CMPC    R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00101 1111nnn0 MOV     ER{n*2}, ER{m*2}
if ((buf[0] & 0b00011111) == 0b00000101 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ER" << (n*2) << ", ER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0iiiiiii 1110nnn0 MOV     ER{n*2}, #{i}
if ((buf[0] & 0b10000000) == 0b00000000 && (buf[1] & 0b11110001) == 0b11100000) {
	int i = buf[0] >> 0 & 0b1111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ER" << (n*2) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0000 1000nnnn MOV     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0000nnnn MOV     R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00000000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0011 1000nnnn OR      R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000011 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "OR      R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0011nnnn OR      R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b00110000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "OR      R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0100 1000nnnn XOR     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00000100 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "XOR     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 0100nnnn XOR     R{n}, #{i}
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b01000000) {
	int i = buf[0] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "XOR     R" << (n) << ", #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00111 1111nnn0 CMP     ER{n*2}, ER{m*2}
if ((buf[0] & 0b00011111) == 0b00000111 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CMP     ER" << (n*2) << ", ER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1000 1000nnnn SUB     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001000 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SUB     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1001 1000nnnn SUBC    R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001001 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SUBC    R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1010 1000nnnn SLL     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001010 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SLL     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0www1010 1001nnnn SLL     R{n}, #{w}
if ((buf[0] & 0b10001111) == 0b00001010 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SLL     R" << (n) << ", #" << (w) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1011 1000nnnn SLLC    R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001011 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SLLC    R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0www1011 1001nnnn SLLC    R{n}, #{w}
if ((buf[0] & 0b10001111) == 0b00001011 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SLLC    R" << (n) << ", #" << (w) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1110 1000nnnn SRA     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRA     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0www1110 1001nnnn SRA     R{n}, #{w}
if ((buf[0] & 0b10001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRA     R" << (n) << ", #" << (w) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1100 1000nnnn SRL     R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001100 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRL     R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0www1100 1001nnnn SRL     R{n}, #{w}
if ((buf[0] & 0b10001111) == 0b00001100 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRL     R" << (n) << ", #" << (w) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1101 1000nnnn SRLC    R{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001101 && (buf[1] & 0b11110000) == 0b10000000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRLC    R" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0www1101 1001nnnn SRLC    R{n}, #{w}
if ((buf[0] & 0b10001111) == 0b00001101 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111, w = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SRLC    R" << (n) << ", #" << (w) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110010 1001nnn0 L       ER{n*2}, [EA]
if ((buf[0] & 0b11111111) == 0b00110010 && (buf[1] & 0b11110001) == 0b10010000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       ER" << (n*2) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010010 1001nnn0 L       ER{n*2}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010010 && (buf[1] & 0b11110001) == 0b10010000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       ER" << (n*2) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00010 1001nnn0 L       ER{n*2}, [ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00000010 && (buf[1] & 0b11110001) == 0b10010000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       ER" << (n*2) << ", [ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00DDDDDD 1011nnn0 L       ER{n*2}, {signedtohex(D, 6)}h[BP]
if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11110001) == 0b10110000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       ER" << (n*2) << ", " << (signedtohex(D, 6)) << "h[BP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01DDDDDD 1011nnn0 L       ER{n*2}, {signedtohex(D, 6)}h[FP]
if ((buf[0] & 0b11000000) == 0b01000000 && (buf[1] & 0b11110001) == 0b10110000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       ER" << (n*2) << ", " << (signedtohex(D, 6)) << "h[FP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110000 1001nnnn L       R{n}, [EA]
if ((buf[0] & 0b11111111) == 0b00110000 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       R" << (n) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010000 1001nnnn L       R{n}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010000 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       R" << (n) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00000 1001nnnn L       R{n}, [ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10010000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       R" << (n) << ", [ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00DDDDDD 1101nnnn L       R{n}, {signedtohex(D, 6)}h[BP]
if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b11010000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       R" << (n) << ", " << (signedtohex(D, 6)) << "h[BP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01DDDDDD 1101nnnn L       R{n}, {signedtohex(D, 6)}h[FP]
if ((buf[0] & 0b11000000) == 0b01000000 && (buf[1] & 0b11110000) == 0b11010000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       R" << (n) << ", " << (signedtohex(D, 6)) << "h[FP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110100 1001nn00 L       XR{n*4}, [EA]
if ((buf[0] & 0b11111111) == 0b00110100 && (buf[1] & 0b11110011) == 0b10010000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       XR" << (n*4) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010100 1001nn00 L       XR{n*4}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010100 && (buf[1] & 0b11110011) == 0b10010000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       XR" << (n*4) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110110 1001n000 L       QR{n*8}, [EA]
if ((buf[0] & 0b11111111) == 0b00110110 && (buf[1] & 0b11110111) == 0b10010000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       QR" << (n*8) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010110 1001n000 L       QR{n*8}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010110 && (buf[1] & 0b11110111) == 0b10010000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "L       QR" << (n*8) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110011 1001nnn0 ST      ER{n*2}, [EA]
if ((buf[0] & 0b11111111) == 0b00110011 && (buf[1] & 0b11110001) == 0b10010000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      ER" << (n*2) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010011 1001nnn0 ST      ER{n*2}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010011 && (buf[1] & 0b11110001) == 0b10010000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      ER" << (n*2) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00011 1001nnn0 ST      ER{n*2}, [ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00000011 && (buf[1] & 0b11110001) == 0b10010000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      ER" << (n*2) << ", [ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10DDDDDD 1011nnn0 ST      ER{n*2}, {signedtohex(D, 6)}h[BP]
if ((buf[0] & 0b11000000) == 0b10000000 && (buf[1] & 0b11110001) == 0b10110000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      ER" << (n*2) << ", " << (signedtohex(D, 6)) << "h[BP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11DDDDDD 1011nnn0 ST      ER{n*2}, {signedtohex(D, 6)}h[FP]
if ((buf[0] & 0b11000000) == 0b11000000 && (buf[1] & 0b11110001) == 0b10110000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      ER" << (n*2) << ", " << (signedtohex(D, 6)) << "h[FP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110001 1001nnnn ST      R{n}, [EA]
if ((buf[0] & 0b11111111) == 0b00110001 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      R" << (n) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010001 1001nnnn ST      R{n}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010001 && (buf[1] & 0b11110000) == 0b10010000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      R" << (n) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm00001 1001nnnn ST      R{n}, [ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10010000) {
	int m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      R" << (n) << ", [ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10DDDDDD 1101nnnn ST      R{n}, {signedtohex(D, 6)}h[BP]
if ((buf[0] & 0b11000000) == 0b10000000 && (buf[1] & 0b11110000) == 0b11010000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      R" << (n) << ", " << (signedtohex(D, 6)) << "h[BP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11DDDDDD 1101nnnn ST      R{n}, {signedtohex(D, 6)}h[FP]
if ((buf[0] & 0b11000000) == 0b11000000 && (buf[1] & 0b11110000) == 0b11010000) {
	int D = buf[0] >> 0 & 0b111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      R" << (n) << ", " << (signedtohex(D, 6)) << "h[FP]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110101 1001nn00 ST      XR{n*4}, [EA]
if ((buf[0] & 0b11111111) == 0b00110101 && (buf[1] & 0b11110011) == 0b10010000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      XR" << (n*4) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010101 1001nn00 ST      XR{n*4}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010101 && (buf[1] & 0b11110011) == 0b10010000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      XR" << (n*4) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00110111 1001n000 ST      QR{n*8}, [EA]
if ((buf[0] & 0b11111111) == 0b00110111 && (buf[1] & 0b11110111) == 0b10010000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      QR" << (n*8) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01010111 1001n000 ST      QR{n*8}, [EA+]
if ((buf[0] & 0b11111111) == 0b01010111 && (buf[1] & 0b11110111) == 0b10010000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ST      QR" << (n*8) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 11100001 ADD     SP, #{signedtohex(i, 8)}h
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100001) {
	int i = buf[0] >> 0 & 0b11111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "ADD     SP, #" << (signedtohex(i, 8)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1111 10100000 MOV     ECSR, R{m}
if ((buf[0] & 0b00001111) == 0b00001111 && (buf[1] & 0b11111111) == 0b10100000) {
	int m = buf[0] >> 4 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ECSR, R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001101 1010mmm0 MOV     ELR, ER{m*2}
if ((buf[0] & 0b11111111) == 0b00001101 && (buf[1] & 0b11110001) == 0b10100000) {
	int m = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ELR, ER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1100 10100000 MOV     EPSW, R{m}
if ((buf[0] & 0b00001111) == 0b00001100 && (buf[1] & 0b11111111) == 0b10100000) {
	int m = buf[0] >> 4 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     EPSW, R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000101 1010nnn0 MOV     ER{n*2}, ELR
if ((buf[0] & 0b11111111) == 0b00000101 && (buf[1] & 0b11110001) == 0b10100000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ER" << (n*2) << ", ELR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00011010 1010nnn0 MOV     ER{n*2}, SP
if ((buf[0] & 0b11111111) == 0b00011010 && (buf[1] & 0b11110001) == 0b10100000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     ER" << (n*2) << ", SP"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1011 10100000 MOV     PSW, R{m}
if ((buf[0] & 0b00001111) == 0b00001011 && (buf[1] & 0b11111111) == 0b10100000) {
	int m = buf[0] >> 4 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     PSW, R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// iiiiiiii 11101001 MOV     PSW, #{i} 
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11101001) {
	int i = buf[0] >> 0 & 0b11111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     PSW, #" << (i) << " "
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000111 1010nnnn MOV     R{n}, ECSR
if ((buf[0] & 0b11111111) == 0b00000111 && (buf[1] & 0b11110000) == 0b10100000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", ECSR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000100 1010nnnn MOV     R{n}, EPSW
if ((buf[0] & 0b11111111) == 0b00000100 && (buf[1] & 0b11110000) == 0b10100000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", EPSW"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000011 1010nnnn MOV     R{n}, PSW
if ((buf[0] & 0b11111111) == 0b00000011 && (buf[1] & 0b11110000) == 0b10100000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", PSW"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01010 10100001 MOV     SP, ER{m*2}
if ((buf[0] & 0b00011111) == 0b00001010 && (buf[1] & 0b11111111) == 0b10100001) {
	int m = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     SP, ER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01011110 1111nnn0 PUSH    ER{n*2}
if ((buf[0] & 0b11111111) == 0b01011110 && (buf[1] & 0b11110001) == 0b11110000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    ER" << (n*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01111110 1111n000 PUSH    QR{n*8}
if ((buf[0] & 0b11111111) == 0b01111110 && (buf[1] & 0b11110111) == 0b11110000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    QR" << (n*8) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01001110 1111nnnn PUSH    R{n}
if ((buf[0] & 0b11111111) == 0b01001110 && (buf[1] & 0b11110000) == 0b11110000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    R" << (n) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01101110 1111nn00 PUSH    XR{n*4}
if ((buf[0] & 0b11111111) == 0b01101110 && (buf[1] & 0b11110011) == 0b11110000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    XR" << (n*4) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001110 1111lep1 PUSH    {l==1?"LR, ":""}{e==1?"EPSW, ":""}{p==1?"ELR, ":""}EA
if ((buf[0] & 0b11111111) == 0b11001110 && (buf[1] & 0b11110001) == 0b11110001) {
	int e = buf[1] >> 2 & 0b1, l = buf[1] >> 3 & 0b1, p = buf[1] >> 1 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    " << (l==1?"LR, ":"") << "" << (e==1?"EPSW, ":"") << "" << (p==1?"ELR, ":"") << "EA"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001110 1111le10 PUSH    {l==1?"LR, ":""}{e==1?"EPSW, ":""}ELR
if ((buf[0] & 0b11111111) == 0b11001110 && (buf[1] & 0b11110011) == 0b11110010) {
	int e = buf[1] >> 2 & 0b1, l = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    " << (l==1?"LR, ":"") << "" << (e==1?"EPSW, ":"") << "ELR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001110 1111l100 PUSH    {l==1?"LR, ":""}EPSW
if ((buf[0] & 0b11111111) == 0b11001110 && (buf[1] & 0b11110111) == 0b11110100) {
	int l = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    " << (l==1?"LR, ":"") << "EPSW"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001110 11111000 PUSH    LR
if ((buf[0] & 0b11111111) == 0b11001110 && (buf[1] & 0b11111111) == 0b11111000) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "PUSH    LR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00011110 1111nnn0 POP     ER{n*2}
if ((buf[0] & 0b11111111) == 0b00011110 && (buf[1] & 0b11110001) == 0b11110000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     ER" << (n*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00111110 1111n000 POP     QR{n*8}
if ((buf[0] & 0b11111111) == 0b00111110 && (buf[1] & 0b11110111) == 0b11110000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     QR" << (n*8) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001110 1111nnnn POP     R{n}
if ((buf[0] & 0b11111111) == 0b00001110 && (buf[1] & 0b11110000) == 0b11110000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     R" << (n) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00101110 1111nn00 POP     XR{n*4}
if ((buf[0] & 0b11111111) == 0b00101110 && (buf[1] & 0b11110011) == 0b11110000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     XR" << (n*4) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001110 1111lep1 POP     {l==1?"LR, ":""}{e==1?"PSW, ":""}{p==1?"PC, ":""}EA
if ((buf[0] & 0b11111111) == 0b10001110 && (buf[1] & 0b11110001) == 0b11110001) {
	int e = buf[1] >> 2 & 0b1, l = buf[1] >> 3 & 0b1, p = buf[1] >> 1 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     " << (l==1?"LR, ":"") << "" << (e==1?"PSW, ":"") << "" << (p==1?"PC, ":"") << "EA"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001110 1111le10 POP     {l==1?"LR, ":""}{e==1?"PSW, ":""}PC
if ((buf[0] & 0b11111111) == 0b10001110 && (buf[1] & 0b11110011) == 0b11110010) {
	int e = buf[1] >> 2 & 0b1, l = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     " << (l==1?"LR, ":"") << "" << (e==1?"PSW, ":"") << "PC"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001110 1111l100 POP     {l==1?"LR, ":""}PSW
if ((buf[0] & 0b11111111) == 0b10001110 && (buf[1] & 0b11110111) == 0b11110100) {
	int l = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     " << (l==1?"LR, ":"") << "PSW"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001110 11111000 POP     LR
if ((buf[0] & 0b11111111) == 0b10001110 && (buf[1] & 0b11111111) == 0b11111000) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "POP     LR"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1110 1010nnnn MOV     CR{n}, R{m}
if ((buf[0] & 0b00001111) == 0b00001110 && (buf[1] & 0b11110000) == 0b10100000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CR" << (n) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00101101 1111nnn0 MOV     CER{n*2}, [EA]
if ((buf[0] & 0b11111111) == 0b00101101 && (buf[1] & 0b11110001) == 0b11110000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CER" << (n*2) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00111101 1111nnn0 MOV     CER{n*2}, [EA+]
if ((buf[0] & 0b11111111) == 0b00111101 && (buf[1] & 0b11110001) == 0b11110000) {
	int n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CER" << (n*2) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001101 1111nnnn MOV     CR{n}, [EA]
if ((buf[0] & 0b11111111) == 0b00001101 && (buf[1] & 0b11110000) == 0b11110000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CR" << (n) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00011101 1111nnnn MOV     CR{n}, [EA+]
if ((buf[0] & 0b11111111) == 0b00011101 && (buf[1] & 0b11110000) == 0b11110000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CR" << (n) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01001101 1111nn00 MOV     CXR{n*4}, [EA]
if ((buf[0] & 0b11111111) == 0b01001101 && (buf[1] & 0b11110011) == 0b11110000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CXR" << (n*4) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01011101 1111nn00 MOV     CXR{n*4}, [EA+]
if ((buf[0] & 0b11111111) == 0b01011101 && (buf[1] & 0b11110011) == 0b11110000) {
	int n = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CXR" << (n*4) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01101101 1111n000 MOV     CQR{n*8}, [EA]
if ((buf[0] & 0b11111111) == 0b01101101 && (buf[1] & 0b11110111) == 0b11110000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CQR" << (n*8) << ", [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01111101 1111n000 MOV     CQR{n*8}, [EA+]
if ((buf[0] & 0b11111111) == 0b01111101 && (buf[1] & 0b11110111) == 0b11110000) {
	int n = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     CQR" << (n*8) << ", [EA+]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0110 1010nnnn MOV     R{n}, CR{m}
if ((buf[0] & 0b00001111) == 0b00000110 && (buf[1] & 0b11110000) == 0b10100000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     R" << (n) << ", CR" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10101101 1111mmm0 MOV     [EA], CER{m*2}
if ((buf[0] & 0b11111111) == 0b10101101 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA], CER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10111101 1111mmm0 MOV     [EA+], CER{m*2}
if ((buf[0] & 0b11111111) == 0b10111101 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA+], CER" << (m*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001101 1111mmmm MOV     [EA], CR{m}
if ((buf[0] & 0b11111111) == 0b10001101 && (buf[1] & 0b11110000) == 0b11110000) {
	int m = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA], CR" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10011101 1111mmmm MOV     [EA+], CR{m}
if ((buf[0] & 0b11111111) == 0b10011101 && (buf[1] & 0b11110000) == 0b11110000) {
	int m = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA+], CR" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001101 1111mm00 MOV     [EA], CXR{m*4}
if ((buf[0] & 0b11111111) == 0b11001101 && (buf[1] & 0b11110011) == 0b11110000) {
	int m = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA], CXR" << (m*4) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11011101 1111mm00 MOV     [EA+], CXR{m*4}
if ((buf[0] & 0b11111111) == 0b11011101 && (buf[1] & 0b11110011) == 0b11110000) {
	int m = buf[1] >> 2 & 0b11;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA+], CXR" << (m*4) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11101101 1111m000 MOV     [EA], CQR{m*8}
if ((buf[0] & 0b11111111) == 0b11101101 && (buf[1] & 0b11110111) == 0b11110000) {
	int m = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA], CQR" << (m*8) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11111101 1111m000 MOV     [EA+], CQR{m*8}
if ((buf[0] & 0b11111111) == 0b11111101 && (buf[1] & 0b11110111) == 0b11110000) {
	int m = buf[1] >> 3 & 0b1;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MOV     [EA+], CQR" << (m*8) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01010 11110000 LEA     [ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001010 && (buf[1] & 0b11111111) == 0b11110000) {
	int m = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "LEA     [ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00011111 1000nnnn DAA     R{n}
if ((buf[0] & 0b11111111) == 0b00011111 && (buf[1] & 0b11110000) == 0b10000000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DAA     R" << (n) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00111111 1000nnnn DAS     R{n}
if ((buf[0] & 0b11111111) == 0b00111111 && (buf[1] & 0b11110000) == 0b10000000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DAS     R" << (n) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01011111 1000nnnn NEG     R{n}
if ((buf[0] & 0b11111111) == 0b01011111 && (buf[1] & 0b11110000) == 0b10000000) {
	int n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "NEG     R" << (n) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0bbb0000 1010nnnn SB      R{n}.{b}
if ((buf[0] & 0b10001111) == 0b00000000 && (buf[1] & 0b11110000) == 0b10100000) {
	int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SB      R" << (n) << "." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0bbb0010 1010nnnn RB      R{n}.{b}
if ((buf[0] & 0b10001111) == 0b00000010 && (buf[1] & 0b11110000) == 0b10100000) {
	int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "RB      R" << (n) << "." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 0bbb0001 1010nnnn TB      R{n}.{b}
if ((buf[0] & 0b10001111) == 0b00000001 && (buf[1] & 0b11110000) == 0b10100000) {
	int b = buf[0] >> 4 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "TB      R" << (n) << "." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001000 11101101 EI
if ((buf[0] & 0b11111111) == 0b00001000 && (buf[1] & 0b11111111) == 0b11101101) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "EI"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11110111 11101011 DI
if ((buf[0] & 0b11111111) == 0b11110111 && (buf[1] & 0b11111111) == 0b11101011) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DI"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10000000 11101101 SC
if ((buf[0] & 0b11111111) == 0b10000000 && (buf[1] & 0b11111111) == 0b11101101) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SC"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 01111111 11101011 RC
if ((buf[0] & 0b11111111) == 0b01111111 && (buf[1] & 0b11111111) == 0b11101011) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "RC"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11001111 11111110 CPLC
if ((buf[0] & 0b11111111) == 0b11001111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "CPLC"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// rrrrrrrr 1100cccc BC      {cond[c]}, {tohex(2 + pc + ((int)(signed char)r << 1), 4+1)}h
if ((buf[0] & 0b00000000) == 0b00000000 && (buf[1] & 0b11110000) == 0b11000000) {
	int c = buf[1] >> 0 & 0b1111, r = buf[0] >> 0 & 0b11111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "BC      " << (cond[c]) << ", " << (tohex(2 + pc + ((int)(signed char)r << 1), 4+1)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// nnn01111 1000mmm1 {m == n ? "" : "Wrong format - "}EXTBW   ER{n*2}
if ((buf[0] & 0b00011111) == 0b00001111 && (buf[1] & 0b11110001) == 0b10000001) {
	int m = buf[1] >> 1 & 0b111, n = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "" << (m == n ? "" : "Wrong format - ") << "EXTBW   ER" << (n*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00iiiiii 11100101 SWI     #{i}
if ((buf[0] & 0b11000000) == 0b00000000 && (buf[1] & 0b11111111) == 0b11100101) {
	int i = buf[0] >> 0 & 0b111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "SWI     #" << (i) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 11111111 11111111 BRK
if ((buf[0] & 0b11111111) == 0b11111111 && (buf[1] & 0b11111111) == 0b11111111) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "BRK"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// nnn00010 11110000 B       ER{n*2}
if ((buf[0] & 0b00011111) == 0b00000010 && (buf[1] & 0b11111111) == 0b11110000) {
	int n = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "B       ER" << (n*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// nnn00011 11110000 BL      ER{n*2}
if ((buf[0] & 0b00011111) == 0b00000011 && (buf[1] & 0b11111111) == 0b11110000) {
	int n = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "BL      ER" << (n*2) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm0100 1111nnn0 MUL     ER{n*2}, R{m}
if ((buf[0] & 0b00001111) == 0b00000100 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "MUL     ER" << (n*2) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// mmmm1001 1111nnn0 DIV     ER{n*2}, R{m}
if ((buf[0] & 0b00001111) == 0b00001001 && (buf[1] & 0b11110001) == 0b11110000) {
	int m = buf[0] >> 4 & 0b1111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DIV     ER" << (n*2) << ", R" << (m) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00101111 11111110 INC     [EA]
if ((buf[0] & 0b11111111) == 0b00101111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "INC     [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00111111 11111110 DEC     [EA]
if ((buf[0] & 0b11111111) == 0b00111111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "DEC     [EA]"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00011111 11111110 RT
if ((buf[0] & 0b11111111) == 0b00011111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "RT"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001111 11111110 RTI
if ((buf[0] & 0b11111111) == 0b00001111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "RTI"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// 10001111 11111110 NOP
if ((buf[0] & 0b11111111) == 0b10001111 && (buf[1] & 0b11111111) == 0b11111110) {
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "NOP"
		<< "\n";
    buf.pop_front(); buf.pop_front();
	goto done;
}
// # 4
// mmm01000 1010nnn0 DDDDDDDD EEEEEEEE L       ER{n*2}, {signedtohex(E*256+D, 16)}h[ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001000 && (buf[1] & 0b11110001) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "L       ER" << (n*2) << ", " << (signedtohex(E*256+D, 16)) << "h[ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00010010 1001nnn0 DDDDDDDD EEEEEEEE L       ER{n*2}, 0{tohex(E*256+D, 4)}h
if ((buf[0] & 0b11111111) == 0b00010010 && (buf[1] & 0b11110001) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "L       ER" << (n*2) << ", 0" << (tohex(E*256+D, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01000 1001nnnn DDDDDDDD EEEEEEEE L       R{n}, {signedtohex(E*256+D, 16)}h[ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001000 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "L       R" << (n) << ", " << (signedtohex(E*256+D, 16)) << "h[ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00010000 1001nnnn DDDDDDDD EEEEEEEE L       R{n}, 0{tohex(E*256+D, 4)}h
if ((buf[0] & 0b11111111) == 0b00010000 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "L       R" << (n) << ", 0" << (tohex(E*256+D, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01001 1010nnn0 DDDDDDDD EEEEEEEE ST      ER{n*2}, {signedtohex(E*256+D, 16)}h[ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001001 && (buf[1] & 0b11110001) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "ST      ER" << (n*2) << ", " << (signedtohex(E*256+D, 16)) << "h[ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00010011 1001nnn0 DDDDDDDD EEEEEEEE ST      ER{n*2}, 0{tohex(E*256+D, 4)}h
if ((buf[0] & 0b11111111) == 0b00010011 && (buf[1] & 0b11110001) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 1 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "ST      ER" << (n*2) << ", 0" << (tohex(E*256+D, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01001 1001nnnn DDDDDDDD EEEEEEEE ST      R{n}, {signedtohex(E*256+D, 16)}h[ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001001 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "ST      R" << (n) << ", " << (signedtohex(E*256+D, 16)) << "h[ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00010001 1001nnnn DDDDDDDD EEEEEEEE ST      R{n}, 0{tohex(E*256+D, 4)}h
if ((buf[0] & 0b11111111) == 0b00010001 && (buf[1] & 0b11110000) == 0b10010000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, n = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "ST      R" << (n) << ", 0" << (tohex(E*256+D, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// mmm01011 11110000 DDDDDDDD EEEEEEEE LEA     {signedtohex(E*256+D, 16)}h[ER{m*2}]
if ((buf[0] & 0b00011111) == 0b00001011 && (buf[1] & 0b11111111) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, m = buf[0] >> 5 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "LEA     " << (signedtohex(E*256+D, 16)) << "h[ER" << (m*2) << "]"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00001100 11110000 DDDDDDDD EEEEEEEE LEA     0{tohex(E*256+D, 4)}h
if ((buf[0] & 0b11111111) == 0b00001100 && (buf[1] & 0b11111111) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "LEA     0" << (tohex(E*256+D, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 1bbb0000 10100000 DDDDDDDD EEEEEEEE SB      0{tohex(E*256+D, 4)}h.{b}
if ((buf[0] & 0b10001111) == 0b10000000 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "SB      0" << (tohex(E*256+D, 4)) << "h." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 1bbb0010 10100000 DDDDDDDD EEEEEEEE RB      0{tohex(E*256+D, 4)}h.{b}
if ((buf[0] & 0b10001111) == 0b10000010 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "RB      0" << (tohex(E*256+D, 4)) << "h." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 1bbb0001 10100000 DDDDDDDD EEEEEEEE TB      0{tohex(E*256+D, 4)}h.{b}
if ((buf[0] & 0b10001111) == 0b10000001 && (buf[1] & 0b11111111) == 0b10100000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int D = buf[2] >> 0 & 0b11111111, E = buf[3] >> 0 & 0b11111111, b = buf[0] >> 4 & 0b111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "TB      0" << (tohex(E*256+D, 4)) << "h." << (b) << ""
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000000 1111gggg CCCCCCCC DDDDDDDD B       0{tohex(g, 1)}h:0{tohex(D*256+C, 4)}h
if ((buf[0] & 0b11111111) == 0b00000000 && (buf[1] & 0b11110000) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int C = buf[2] >> 0 & 0b11111111, D = buf[3] >> 0 & 0b11111111, g = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "B       0" << (tohex(g, 1)) << "h:0" << (tohex(D*256+C, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
// 00000001 1111gggg CCCCCCCC DDDDDDDD BL      0{tohex(g, 1)}h:0{tohex(D*256+C, 4)}h
if ((buf[0] & 0b11111111) == 0b00000001 && (buf[1] & 0b11110000) == 0b11110000 && (buf[2] & 0b00000000) == 0b00000000 && (buf[3] & 0b00000000) == 0b00000000) {
	int C = buf[2] >> 0 & 0b11111111, D = buf[3] >> 0 & 0b11111111, g = buf[1] >> 0 & 0b1111;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << tohex(buf[2], 2) << ' ' << tohex(buf[3], 2) << ' ' << "       "
		<< "BL      0" << (tohex(g, 1)) << "h:0" << (tohex(D*256+C, 4)) << "h"
		<< "\n";
    buf.pop_front(); buf.pop_front(); buf.pop_front(); buf.pop_front();
	goto done;
}
;
	out << tohex(ip, 6) << "   " << tohex(buf[0], 2) << ' ' << tohex(buf[1], 2) << ' ' << "             "
		<< "Unrecognized command"
		<< "\n";
    buf.pop_front();
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
