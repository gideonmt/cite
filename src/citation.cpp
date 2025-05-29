#include "../include/citation.hpp"
#include "../formatters/chicago_formatter.hpp"
#include "../formatters/mla_formatter.hpp"
#include <algorithm>

std::unique_ptr<CitationFormatter> create_formatter(const std::string &style) {
  if (style == "chicago") {
    return std::make_unique<ChicagoFormatter>();
  } else if (style == "mla") {
    return std::make_unique<MLAFormatter>();
  }
  return nullptr;
}

std::vector<std::string> format_bibliography(const nlohmann::json &entries,
                                             const std::string &style) {
  auto formatter = create_formatter(style);
  std::vector<std::string> results;
  if (!formatter)
    return results;

  if (style == "chicago") {
    // Chicago bibliography should be sorted by last name
    std::vector<nlohmann::json> sorted_entries = entries;
    std::sort(sorted_entries.begin(), sorted_entries.end(),
              [](const nlohmann::json &a, const nlohmann::json &b) {
                return ChicagoFormatter::get_author_last_name(a) <
                       ChicagoFormatter::get_author_last_name(b);
              });
    for (const auto &entry : sorted_entries) {
      results.push_back(formatter->format(entry));
    }
  } else {
    for (const auto &entry : entries) {
      results.push_back(formatter->format(entry));
    }
  }
  return results;
}

std::vector<ChicagoCitationBundle>
format_chicago_with_footnotes(const nlohmann::json &entries) {
  ChicagoFormatter formatter;
  // Sort entries by last name
  std::vector<nlohmann::json> sorted_entries = entries;
  std::sort(sorted_entries.begin(), sorted_entries.end(),
            [](const nlohmann::json &a, const nlohmann::json &b) {
              return ChicagoFormatter::get_author_last_name(a) <
                     ChicagoFormatter::get_author_last_name(b);
            });
  std::vector<ChicagoCitationBundle> bundles;
  for (const auto &entry : sorted_entries) {
    bundles.push_back({formatter.format(entry),
                       formatter.format_long_footnote(entry),
                       formatter.format_short_footnote(entry)});
  }
  return bundles;
}
