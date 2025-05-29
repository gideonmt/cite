#pragma once
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

// Loads a JSON file from disk and returns the parsed nlohmann::json object
nlohmann::json load_json_file(const std::string &filepath);
