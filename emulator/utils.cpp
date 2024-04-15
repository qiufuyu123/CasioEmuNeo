//
// Created by bczhc on 15/04/24.
//

#include "utils.h"

const char *casioemu::Exception::what() const noexcept {
    return msg.c_str();
}

casioemu::Exception::Exception(std::string _msg) : msg(std::move(_msg)) {}

std::vector<MemoryEditor::MarkedSpan> casioemu::parseColoredSpansConfig(const char *path) {
    auto result = std::vector<MemoryEditor::MarkedSpan>();

    std::ifstream file("/home/bczhc/code/c/text");
    if (!file.is_open()) {
        throw Exception("Failed to open the file.");
    }

    std::string line;
    while (std::getline(file, line)) {
        if (starts_with(line, "#")) {
            // ignore "comments"
            continue;
        }

        std::stringstream ss(line);
        std::vector<std::string> parts;
        std::string part;
        while (std::getline(ss, part, ',')) {
            parts.push_back(part);
        }
        if (parts.size() != 3) continue;

        auto range_start = strtoul(parts[0].c_str(), nullptr, 16);
        size_t range_end;
        if (starts_with(parts[1], "0x")) {
            range_end = strtoul(parts[1].c_str(), nullptr, 16);
        } else {
            range_end = range_start + strtoul(parts[1].c_str(), nullptr, 10) - 1;
        }
        int r = 0, g = 0, b = 0;
        sscanf(parts[2].c_str(), "%02x%02x%02x", &r, &g, &b); // NOLINT(*-err34-c)
        result.push_back({
                                 .start = range_start,
                                 .length = range_end - range_start + 1,
                                 .color = ImColor(r, g, b, 50),
                         });
    }
    file.close();
    return result;
}

bool casioemu::starts_with(const std::string &str, const std::string &prefix) {
    if (str.length() < prefix.length()) {
        return false;
    }
    return str.substr(0, prefix.length()) == prefix;
}
