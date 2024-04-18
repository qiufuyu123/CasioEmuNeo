//
// Created by bczhc on 15/04/24.
//

#include "utils.h"
#include <unistd.h>
#include <sys/stat.h>

const char *casioemu::Exception::what() const noexcept {
    return msg.c_str();
}

casioemu::Exception::Exception(std::string _msg) : msg(std::move(_msg)) {}

std::vector<MemoryEditor::MarkedSpan> casioemu::ParseColoredSpansConfig(const std::string &path) {
    auto result = std::vector<MemoryEditor::MarkedSpan>();

    std::ifstream file(path);
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
        if (parts.size() < 3) continue;

        auto range_start = strtoul(parts[0].c_str(), nullptr, 16);
        size_t range_end;
        if (starts_with(parts[1], "0x")) {
            range_end = strtoul(parts[1].c_str(), nullptr, 16);
        } else {
            range_end = range_start + strtoul(parts[1].c_str(), nullptr, 10) - 1;
        }
        int r = 0, g = 0, b = 0, a = 0;

        if (parts[2].length() == 6) {
            sscanf(parts[2].c_str(), "%02x%02x%02x", &r, &g, &b); // NOLINT(*-err34-c)
        } else if (parts[2].length() == 8) {
            sscanf(parts[2].c_str(), "%02x%02x%02x%02x", &a, &r, &g, &b); // NOLINT(*-err34-c)
        }

        MemoryEditor::MarkedSpan span = {
                .start = range_start,
                .length = range_end - range_start + 1,
                .color = ImColor(r, g, b, a == 0 ? 50 : a),
        };
        if (parts.size() == 4) {
            span.desc = parts[3];
        }
        result.push_back(span);
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

bool casioemu::FileSystem::exists(const std::string &path) {
    return access(path.c_str(), F_OK) == 0;
}

timespec casioemu::FileSystem::mtime(const std::string &path) {
    struct stat stat_r{};
    if (stat(path.c_str(), &stat_r) != 0) {
        perror("stat");
        return {};
    }
#ifdef _WIN32
    return {stat_r.st_mtime,0};
#else
    return stat_r.st_mtim;
#endif
}

uint64_t casioemu::FileSystem::mtime_ms(const std::string &path) {
    const timespec &ts = FileSystem::mtime(path);
    return (uint64_t) ts.tv_sec * 1000 + (uint64_t) ts.tv_nsec / 1000000;
}
