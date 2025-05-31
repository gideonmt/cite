#pragma once
#include <string>
#include <nlohmann/json.hpp>

// Entry point for new add flow
int add_entry(const std::string &filename);

nlohmann::json search_sources(const std::string &query);
std::vector<nlohmann::json> parse_results(const nlohmann::json &src, const std::string &mode);
int show_results(const std::vector<nlohmann::json> &entries);
void show_details(const nlohmann::json &entry);
nlohmann::json edit_entry(nlohmann::json entry);
void add_to_json(const std::string &filename, const nlohmann::json &entry);
