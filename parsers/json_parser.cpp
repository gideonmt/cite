#include "json_parser.hpp"
#include "../include/json_utils.hpp"

// Implementation
nlohmann::json parse_json(const std::string &filepath) {
  return load_json_file(filepath);
}
