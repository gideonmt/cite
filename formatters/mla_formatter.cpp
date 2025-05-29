#include "mla_formatter.hpp"

// Stub MLA formatter
std::string MLAFormatter::format(const nlohmann::json& entry) const {
    return "[MLA] " + (entry.value("title", "Unknown Title"));
}
