#pragma once
#include <nlohmann/json.hpp>
#include <string>

nlohmann::json parse_bibjson(const std::string &filepath);
