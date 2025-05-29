#include "chicago_formatter.hpp"
#include <algorithm>
#include <sstream>
#include <vector>

// Helper: Italicize in HTML
static std::string html_italic(const std::string &s) {
  return "<i>" + s + "</i>";
}

// Helper: Get string from JSON object, or empty string
static std::string get_str(const nlohmann::json &obj, const std::string &key) {
  if (obj.contains(key) && obj[key].is_string())
    return obj[key];
  return "";
}

// Helper: Join author/editor names, handling BibJSON object or string
static std::string join_names(const nlohmann::json &people) {
  if (!people.is_array())
    return "";
  std::vector<std::string> names;
  for (const auto &p : people) {
    if (p.is_object() && p.contains("name"))
      names.push_back((std::string)p["name"]);
    else if (p.is_string())
      names.push_back((std::string)p);
  }
  if (names.empty())
    return "";
  // For 2+ authors: "Last, First, and Last, First"
  if (names.size() == 1)
    return names[0];
  if (names.size() == 2)
    return names[0] + " and " + names[1];
  std::string joined;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i == names.size() - 1)
      joined += "and " + names[i];
    else
      joined += names[i] + ", ";
  }
  return joined;
}

// Helper: Get last name of first author/editor for sorting
std::string
ChicagoFormatter::get_author_last_name(const nlohmann::json &entry) {
  if (!entry.contains("author") || !entry["author"].is_array() ||
      entry["author"].empty())
    return "Unknown";
  auto first = entry["author"][0];
  if (first.is_object()) {
    if (first.contains("lastname"))
      return (std::string)first["lastname"];
    if (first.contains("name")) {
      std::string name = first["name"];
      auto comma = name.find(',');
      if (comma != std::string::npos)
        return name.substr(0, comma);
      auto space = name.find(' ');
      return (space != std::string::npos) ? name.substr(0, space) : name;
    }
  } else if (first.is_string()) {
    std::string name = first;
    auto comma = name.find(',');
    if (comma != std::string::npos)
      return name.substr(0, comma);
    auto space = name.find(' ');
    return (space != std::string::npos) ? name.substr(0, space) : name;
  }
  return "Unknown";
}

std::string ChicagoFormatter::format(const nlohmann::json &entry) const {
  // Chicago bibliography: Last, First, Title. Journal (Year).
  std::ostringstream oss;
  // Author(s)
  oss << join_names(entry.value("author", nlohmann::json::array()));
  // Title
  std::string title = entry.value("title", "Untitled");
  oss << ". " << html_italic(title);
  // Journal/Book/etc.
  if (entry.contains("journal") && entry["journal"].is_object()) {
    std::string journal = get_str(entry["journal"], "name");
    if (!journal.empty())
      oss << ". " << journal;
    std::string volume = get_str(entry["journal"], "volume");
    if (!volume.empty())
      oss << " " << volume;
    std::string pages = get_str(entry["journal"], "pages");
    if (!pages.empty())
      oss << ", " << pages;
  } else if (entry.contains("publisher")) {
    oss << ". " << get_str(entry, "publisher");
  }
  // Year
  std::string year = entry.value("year", "");
  if (!year.empty())
    oss << " (" << year << ")";
  oss << ".";
  return oss.str();
}

std::string
ChicagoFormatter::format_long_footnote(const nlohmann::json &entry) const {
  // Footnote (long): First Last, Title (Journal, Year), [pg].
  std::ostringstream oss;
  // Author(s) - try to rearrange "Last, First" to "First Last"
  std::vector<std::string> formatted_names;
  for (const auto &p : entry.value("author", nlohmann::json::array())) {
    if (p.is_object() && p.contains("name")) {
      std::string name = p["name"];
      auto comma = name.find(',');
      if (comma != std::string::npos) {
        std::string last = name.substr(0, comma);
        std::string first = name.substr(comma + 1);
        while (!first.empty() && isspace(first[0]))
          first.erase(0, 1);
        formatted_names.push_back(first + " " + last);
      } else {
        formatted_names.push_back(name);
      }
    } else if (p.is_string()) {
      std::string name = p;
      auto comma = name.find(',');
      if (comma != std::string::npos) {
        std::string last = name.substr(0, comma);
        std::string first = name.substr(comma + 1);
        while (!first.empty() && isspace(first[0]))
          first.erase(0, 1);
        formatted_names.push_back(first + " " + last);
      } else {
        formatted_names.push_back(name);
      }
    }
  }
  // Join footnote author names
  for (size_t i = 0; i < formatted_names.size(); ++i) {
    if (i > 0)
      oss << ", ";
    oss << formatted_names[i];
  }
  // Title
  std::string title = entry.value("title", "Untitled");
  oss << ", " << html_italic(title);
  // Journal, publisher, etc.
  if (entry.contains("journal") && entry["journal"].is_object()) {
    std::string journal = get_str(entry["journal"], "name");
    std::string volume = get_str(entry["journal"], "volume");
    std::string pages = get_str(entry["journal"], "pages");
    oss << " (" << journal;
    if (!volume.empty())
      oss << " " << volume;
    if (!pages.empty())
      oss << ", " << pages;
    oss << ")";
  } else if (entry.contains("publisher")) {
    oss << " (" << get_str(entry, "publisher") << ")";
  }
  // Year
  std::string year = entry.value("year", "");
  if (!year.empty())
    oss << ", " << year;
  // Page placeholder
  oss << ", [pg].";
  return oss.str();
}

std::string
ChicagoFormatter::format_short_footnote(const nlohmann::json &entry) const {
  // Short: Last, Short Title, [pg].
  // Author last name
  std::string last = get_author_last_name(entry);
  // Short title
  std::string title = entry.value("title", "Untitled");
  std::istringstream iss(title);
  std::string short_title, word;
  int count = 0;
  while (iss >> word && count < 4) {
    if (!short_title.empty())
      short_title += " ";
    short_title += word;
    count++;
  }
  std::ostringstream oss;
  oss << last << ", " << html_italic(short_title) << ", [pg].";
  return oss.str();
}
