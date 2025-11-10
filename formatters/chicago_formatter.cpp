#include "chicago_formatter.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

static std::string html_italic(const std::string &s) {
  return "<i>" + s + "</i>";
}

static std::string get_str(const nlohmann::json &obj, const std::string &key) {
  if (obj.contains(key) && obj[key].is_string())
    return obj[key];
  return "";
}

static std::string title_case(const std::string &s) {
  std::string result = s;
  bool new_word = true;
  for (char &c : result) {
    if (std::isspace(c) || c == '-') {
      new_word = true;
    } else if (new_word) {
      c = std::toupper(c);
      new_word = false;
    }
  }
  return result;
}

struct ParsedName {
  std::string first;
  std::string last;
  std::string full;
};

static ParsedName parse_name(const nlohmann::json &person) {
  ParsedName pn;
  
  if (person.is_object()) {
    // Try structured fields first
    if (person.contains("firstname") && person.contains("lastname")) {
      pn.first = person["firstname"].get<std::string>();
      pn.last = person["lastname"].get<std::string>();
      pn.full = pn.last + ", " + pn.first;
    } else if (person.contains("given") && person.contains("family")) {
      // CrossRef format
      pn.first = person["given"].get<std::string>();
      pn.last = person["family"].get<std::string>();
      pn.full = pn.last + ", " + pn.first;
    } else if (person.contains("name")) {
      pn.full = person["name"].get<std::string>();
      // Try to parse "Last, First" format
      auto comma = pn.full.find(',');
      if (comma != std::string::npos) {
        pn.last = pn.full.substr(0, comma);
        pn.first = pn.full.substr(comma + 1);
        // Trim whitespace
        while (!pn.first.empty() && std::isspace(pn.first[0]))
          pn.first.erase(0, 1);
      } else {
        // Try "First Last" format
        auto space = pn.full.rfind(' ');
        if (space != std::string::npos) {
          pn.first = pn.full.substr(0, space);
          pn.last = pn.full.substr(space + 1);
        } else {
          pn.last = pn.full;
        }
      }
    }
  } else if (person.is_string()) {
    pn.full = person.get<std::string>();
    auto comma = pn.full.find(',');
    if (comma != std::string::npos) {
      pn.last = pn.full.substr(0, comma);
      pn.first = pn.full.substr(comma + 1);
      while (!pn.first.empty() && std::isspace(pn.first[0]))
        pn.first.erase(0, 1);
    } else {
      auto space = pn.full.rfind(' ');
      if (space != std::string::npos) {
        pn.first = pn.full.substr(0, space);
        pn.last = pn.full.substr(space + 1);
      } else {
        pn.last = pn.full;
      }
    }
  }
  
  return pn;
}

static std::string join_names_biblio(const nlohmann::json &people) {
  if (!people.is_array() || people.empty())
    return "";
  
  std::vector<ParsedName> names;
  for (const auto &p : people) {
    names.push_back(parse_name(p));
  }
  
  if (names.empty())
    return "";
  
  std::ostringstream oss;
  oss << names[0].last;
  if (!names[0].first.empty())
    oss << ", " << names[0].first;
  
  if (names.size() == 2) {
    oss << ", and " << names[1].first;
    if (!names[1].first.empty())
      oss << " ";
    oss << names[1].last;
  } else if (names.size() > 2) {
    for (size_t i = 1; i < names.size(); ++i) {
      oss << ", ";
      if (i == names.size() - 1)
        oss << "and ";
      oss << names[i].first;
      if (!names[i].first.empty())
        oss << " ";
      oss << names[i].last;
    }
  }
  
  return oss.str();
}

static std::string join_names_footnote(const nlohmann::json &people) {
  if (!people.is_array() || people.empty())
    return "";
  
  std::vector<ParsedName> names;
  for (const auto &p : people) {
    names.push_back(parse_name(p));
  }
  
  if (names.empty())
    return "";
  
  std::ostringstream oss;
  for (size_t i = 0; i < names.size(); ++i) {
    if (i > 0) {
      if (i == names.size() - 1)
        oss << ", and ";
      else
        oss << ", ";
    }
    oss << names[i].first;
    if (!names[i].first.empty())
      oss << " ";
    oss << names[i].last;
  }
  
  return oss.str();
}

std::string ChicagoFormatter::get_author_last_name(const nlohmann::json &entry) {
  if (entry.contains("author") && entry["author"].is_array() && 
      !entry["author"].empty()) {
    ParsedName pn = parse_name(entry["author"][0]);
    return pn.last.empty() ? "Unknown" : pn.last;
  }
  if (entry.contains("editor") && entry["editor"].is_array() && 
      !entry["editor"].empty()) {
    ParsedName pn = parse_name(entry["editor"][0]);
    return pn.last.empty() ? "Unknown" : pn.last;
  }
  return "Unknown";
}

std::string ChicagoFormatter::format(const nlohmann::json &entry) const {
  std::ostringstream oss;
  
  bool has_author = entry.contains("author") && entry["author"].is_array() && 
                    !entry["author"].empty();
  bool has_editor = entry.contains("editor") && entry["editor"].is_array() && 
                    !entry["editor"].empty();
  
  if (has_author) {
    oss << join_names_biblio(entry["author"]);
  } else if (has_editor) {
    oss << join_names_biblio(entry["editor"]);
    oss << (entry["editor"].size() > 1 ? ", eds" : ", ed");
  } else {
    oss << "Unknown Author";
  }
  
  oss << ". ";
  
  // Title (italicized for books, quoted for articles)
  std::string title = entry.value("title", "Untitled");
  std::string type = entry.value("type", "");
  
  if (type == "article" || type == "paper") {
    oss << "\"" << title << ".\"";
  } else {
    oss << html_italic(title) << ".";
  }
  
  // Container (journal, book, etc.)
  if (entry.contains("journal") && entry["journal"].is_object()) {
    oss << " ";
    std::string journal_name = get_str(entry["journal"], "name");
    if (!journal_name.empty())
      oss << html_italic(journal_name);
    
    std::string volume = get_str(entry["journal"], "volume");
    std::string issue = get_str(entry["journal"], "number");
    
    if (!volume.empty()) {
      oss << " " << volume;
      if (!issue.empty())
        oss << ", no. " << issue;
    }
    
    // Year in parentheses for journal articles
    std::string year = entry.value("year", "");
    if (!year.empty())
      oss << " (" << year << ")";
    
    std::string pages = get_str(entry["journal"], "pages");
    if (!pages.empty())
      oss << ": " << pages;
    
    oss << ".";
  } else if (entry.contains("publisher")) {
    // Book format
    std::string place = entry.value("place", "");
    std::string publisher = get_str(entry, "publisher");
    std::string year = entry.value("year", "");
    
    if (!place.empty() || !publisher.empty()) {
      oss << " ";
      if (!place.empty())
        oss << place << ": ";
      if (!publisher.empty())
        oss << publisher;
      if (!year.empty())
        oss << ", " << year;
      oss << ".";
    } else if (!year.empty()) {
      oss << " " << year << ".";
    } else {
      oss << ".";
    }
  } else {
    // Just year if nothing else
    std::string year = entry.value("year", "");
    if (!year.empty())
      oss << " " << year << ".";
    else
      oss << ".";
  }
  
  // DOI or URL
  if (entry.contains("identifier") && entry["identifier"].is_array()) {
    for (const auto &id : entry["identifier"]) {
      if (id.contains("type") && id["type"] == "doi") {
        std::string doi = get_str(id, "id");
        if (!doi.empty())
          oss << " https://doi.org/" << doi << ".";
        break;
      }
    }
  } else if (entry.contains("url")) {
    std::string url = get_str(entry, "url");
    if (!url.empty())
      oss << " " << url << ".";
  }
  
  return oss.str();
}

// Long footnote (full citation, first use)
std::string ChicagoFormatter::format_long_footnote(const nlohmann::json &entry) const {
  std::ostringstream oss;
  
  // Author(s) in First Last format
  bool has_author = entry.contains("author") && entry["author"].is_array() && 
                    !entry["author"].empty();
  bool has_editor = entry.contains("editor") && entry["editor"].is_array() && 
                    !entry["editor"].empty();
  
  if (has_author) {
    oss << join_names_footnote(entry["author"]);
  } else if (has_editor) {
    oss << join_names_footnote(entry["editor"]);
    oss << (entry["editor"].size() > 1 ? ", eds." : ", ed.");
  } else {
    oss << "Unknown Author";
  }
  
  oss << ", ";
  
  // Title
  std::string title = entry.value("title", "Untitled");
  std::string type = entry.value("type", "");
  
  if (type == "article" || type == "paper") {
    oss << "\"" << title << ",\"";
  } else {
    oss << html_italic(title);
  }
  
  // Container info
  if (entry.contains("journal") && entry["journal"].is_object()) {
    oss << " ";
    std::string journal_name = get_str(entry["journal"], "name");
    if (!journal_name.empty())
      oss << html_italic(journal_name);
    
    std::string volume = get_str(entry["journal"], "volume");
    std::string issue = get_str(entry["journal"], "number");
    
    if (!volume.empty()) {
      oss << " " << volume;
      if (!issue.empty())
        oss << ", no. " << issue;
    }
    
    std::string year = entry.value("year", "");
    if (!year.empty())
      oss << " (" << year << ")";
    
    oss << ": [pg].";
  } else if (entry.contains("publisher")) {
    std::string place = entry.value("place", "");
    std::string publisher = get_str(entry, "publisher");
    std::string year = entry.value("year", "");
    
    oss << " (";
    if (!place.empty())
      oss << place << ": ";
    if (!publisher.empty())
      oss << publisher;
    if (!year.empty())
      oss << ", " << year;
    oss << "), [pg].";
  } else {
    std::string year = entry.value("year", "");
    if (!year.empty())
      oss << " (" << year << ")";
    oss << ", [pg].";
  }
  
  return oss.str();
}

// Short footnote (subsequent references)
std::string ChicagoFormatter::format_short_footnote(const nlohmann::json &entry) const {
  std::ostringstream oss;
  
  // Last name only
  std::string last = get_author_last_name(entry);
  oss << last << ", ";
  
  // Shortened title (first 4 words, no articles)
  std::string title = entry.value("title", "Untitled");
  std::istringstream iss(title);
  std::string word, short_title;
  int count = 0;
  
  while (iss >> word && count < 4) {
    // Skip articles at the beginning
    if (count == 0 && (word == "The" || word == "A" || word == "An"))
      continue;
    
    if (!short_title.empty())
      short_title += " ";
    short_title += word;
    count++;
  }
  
  std::string type = entry.value("type", "");
  if (type == "article" || type == "paper") {
    oss << "\"" << short_title << ",\"";
  } else {
    oss << html_italic(short_title);
  }
  
  oss << " [pg].";
  return oss.str();
}
