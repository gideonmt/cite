#pragma once
#include "../include/citation.hpp"
#include <nlohmann/json.hpp>
#include <string>

class ChicagoFormatter : public CitationFormatter {
public:
  // Bibliography entry
  std::string format(const nlohmann::json &entry) const override;

  // Long footnote (full, 1st use)
  std::string format_long_footnote(const nlohmann::json &entry) const;

  // Short footnote (subsequent)
  std::string format_short_footnote(const nlohmann::json &entry) const;

  // Extract last name for sorting
  static std::string get_author_last_name(const nlohmann::json &entry);
};
