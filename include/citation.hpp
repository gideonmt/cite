#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class CitationFormatter {
public:
  virtual ~CitationFormatter() = default;
  virtual std::string format(const nlohmann::json &entry) const = 0;
};

std::unique_ptr<CitationFormatter> create_formatter(const std::string &style);
std::vector<std::string> format_bibliography(const nlohmann::json &entries,
                                             const std::string &style);

// --- Add this struct definition ---
struct ChicagoCitationBundle {
  std::string bibliography;
  std::string long_footnote;
  std::string short_footnote;
};

std::vector<ChicagoCitationBundle>
format_chicago_with_footnotes(const nlohmann::json &entries);
