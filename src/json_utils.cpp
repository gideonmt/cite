#include "json_utils.hpp"
#include <fstream>
#include <nlohmann/json.hpp>

nlohmann::json load_json_file(const std::string& filepath) {
    std::ifstream file(filepath);
    nlohmann::json j;
    file >> j;
    return j;
}
