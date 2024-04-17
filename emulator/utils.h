//
// Created by bczhc on 15/04/24.
//

#ifndef CASIOEMUX_UTILS_H
#define CASIOEMUX_UTILS_H

#include <sstream>
#include <fstream>
#include <exception>
#include <utility>
#include <vector>
#include "Gui/hex.hpp"

namespace casioemu {
    class Exception : public std::exception {
    private:
        std::string msg;
    public:
        explicit Exception(std::string _msg);

        [[nodiscard]] const char *what() const noexcept override;
    };

    bool starts_with(const std::string &str, const std::string &prefix);

    std::vector<MemoryEditor::MarkedSpan> parseColoredSpansConfig(const std::string &path);

    class FileSystem {
    public:
        static bool exists(const std::string &path);

        static timespec mtime(const std::string &path);

        static uint64_t mtime_ms(const std::string &path);
    };
}

#endif //CASIOEMUX_UTILS_H
