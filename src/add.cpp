#include "add.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>
#include <algorithm>

static std::string prompt(const std::string &q) {
  std::cout << q << ": ";
  std::string a;
  std::getline(std::cin, a);
  return a;
}

// libcurl write callback
static size_t write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

// Query CrossRef for DOI/title/author, or OpenLibrary for ISBN
nlohmann::json search_sources(const std::string &query) {
  CURL *curl = curl_easy_init();
  std::string readBuffer, url;
  
  if (query.rfind("10.", 0) == 0) { // DOI
    url = "https://api.crossref.org/works/" + query;
  } else if (query.size() >= 10 && query.size() <= 13 &&
             std::all_of(query.begin(), query.end(), [](char c) { 
               return std::isdigit(c) || c == '-' || c == 'X'; 
             })) { // ISBN
    std::string clean_isbn = query;
    clean_isbn.erase(std::remove(clean_isbn.begin(), clean_isbn.end(), '-'), 
                     clean_isbn.end());
    url = "https://openlibrary.org/api/books?bibkeys=ISBN:" + clean_isbn +
          "&format=json&jscmd=data";
  } else { // Title/author search (CrossRef)
    char *esc = curl_easy_escape(curl, query.c_str(), 0);
    url = "https://api.crossref.org/works?query=" + std::string(esc ? esc : "") +
          "&rows=10";
    if (esc)
      curl_free(esc);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_setopt(curl, CURLOPT_USERAGENT, "Cite/1.0 (mailto:user@example.com)");
  CURLcode res = curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  if (res != CURLE_OK || readBuffer.empty()) {
    return nlohmann::json::object();
  }

  try {
    return nlohmann::json::parse(readBuffer);
  } catch (...) {
    return nlohmann::json::object();
  }
}

// Convert CrossRef/OpenLibrary to BibJSON
static nlohmann::json convert_to_bibjson(const nlohmann::json &source, 
                                         const std::string &source_type) {
  nlohmann::json entry;
  
  if (source_type == "crossref") {
    // Set type
    if (source.contains("type")) {
      std::string type = source["type"];
      if (type == "journal-article")
        entry["type"] = "article";
      else if (type == "book" || type == "monograph")
        entry["type"] = "book";
      else if (type == "book-chapter")
        entry["type"] = "chapter";
      else
        entry["type"] = type;
    }
    
    // Title
    if (source.contains("title") && source["title"].is_array() && 
        !source["title"].empty()) {
      entry["title"] = source["title"][0].get<std::string>();
    }
    
    // Authors
    if (source.contains("author") && source["author"].is_array()) {
      nlohmann::json authors = nlohmann::json::array();
      for (const auto &a : source["author"]) {
        nlohmann::json author;
        if (a.contains("given"))
          author["firstname"] = a["given"];
        if (a.contains("family"))
          author["lastname"] = a["family"];
        
        // Construct full name
        std::string name;
        if (a.contains("family"))
          name = a["family"].get<std::string>();
        if (a.contains("given")) {
          if (!name.empty())
            name += ", ";
          name += a["given"].get<std::string>();
        }
        if (!name.empty())
          author["name"] = name;
        
        if (!author.empty())
          authors.push_back(author);
      }
      if (!authors.empty())
        entry["author"] = authors;
    }
    
    // Year
    if (source.contains("issued") && source["issued"].contains("date-parts") &&
        source["issued"]["date-parts"].is_array() && 
        !source["issued"]["date-parts"].empty()) {
      auto date_parts = source["issued"]["date-parts"][0];
      if (date_parts.is_array() && !date_parts.empty()) {
        entry["year"] = std::to_string(date_parts[0].get<int>());
      }
    }
    
    // Journal info
    if (source.contains("container-title") && source["container-title"].is_array() &&
        !source["container-title"].empty()) {
      nlohmann::json journal;
      journal["name"] = source["container-title"][0].get<std::string>();
      
      if (source.contains("volume"))
        journal["volume"] = source["volume"].get<std::string>();
      if (source.contains("issue"))
        journal["number"] = source["issue"].get<std::string>();
      if (source.contains("page"))
        journal["pages"] = source["page"].get<std::string>();
      
      if (source.contains("ISSN") && source["ISSN"].is_array() && 
          !source["ISSN"].empty()) {
        nlohmann::json identifiers = nlohmann::json::array();
        nlohmann::json issn_id;
        issn_id["type"] = "issn";
        issn_id["id"] = source["ISSN"][0].get<std::string>();
        identifiers.push_back(issn_id);
        journal["identifier"] = identifiers;
      }
      
      entry["journal"] = journal;
    }
    
    // Publisher (for books)
    if (source.contains("publisher"))
      entry["publisher"] = source["publisher"].get<std::string>();
    
    // DOI
    if (source.contains("DOI")) {
      nlohmann::json identifiers = nlohmann::json::array();
      nlohmann::json doi_id;
      doi_id["type"] = "doi";
      doi_id["id"] = source["DOI"].get<std::string>();
      doi_id["url"] = "https://doi.org/" + source["DOI"].get<std::string>();
      identifiers.push_back(doi_id);
      entry["identifier"] = identifiers;
    }
    
    // URL
    if (source.contains("URL"))
      entry["url"] = source["URL"].get<std::string>();
      
  } else if (source_type == "openlibrary") {
    entry["type"] = "book";
    
    // Title
    if (source.contains("title"))
      entry["title"] = source["title"].get<std::string>();
    
    // Authors
    if (source.contains("authors") && source["authors"].is_array()) {
      nlohmann::json authors = nlohmann::json::array();
      for (const auto &a : source["authors"]) {
        nlohmann::json author;
        if (a.contains("name")) {
          author["name"] = a["name"].get<std::string>();
          // Try to parse into first/last
          std::string name = a["name"].get<std::string>();
          auto space = name.rfind(' ');
          if (space != std::string::npos) {
            author["firstname"] = name.substr(0, space);
            author["lastname"] = name.substr(space + 1);
          }
        }
        if (!author.empty())
          authors.push_back(author);
      }
      if (!authors.empty())
        entry["author"] = authors;
    }
    
    // Publisher
    if (source.contains("publishers") && source["publishers"].is_array() &&
        !source["publishers"].empty()) {
      entry["publisher"] = source["publishers"][0]["name"].get<std::string>();
    }
    
    // Year
    if (source.contains("publish_date"))
      entry["year"] = source["publish_date"].get<std::string>();
    
    // ISBN
    if (source.contains("identifiers") && source["identifiers"].contains("isbn_13") &&
        source["identifiers"]["isbn_13"].is_array() && 
        !source["identifiers"]["isbn_13"].empty()) {
      nlohmann::json identifiers = nlohmann::json::array();
      nlohmann::json isbn_id;
      isbn_id["type"] = "isbn";
      isbn_id["id"] = source["identifiers"]["isbn_13"][0].get<std::string>();
      identifiers.push_back(isbn_id);
      entry["identifier"] = identifiers;
    }
    
    // URL
    if (source.contains("url"))
      entry["url"] = source["url"].get<std::string>();
  }
  
  return entry;
}

// Normalize results to vector of BibJSON entries
std::vector<nlohmann::json> parse_results(const nlohmann::json &src, 
                                          const std::string &mode) {
  std::vector<nlohmann::json> out;
  
  if (mode == "doi") {
    if (src.contains("message")) {
      out.push_back(convert_to_bibjson(src["message"], "crossref"));
    }
  } else if (mode == "isbn") {
    for (auto &el : src.items()) {
      out.push_back(convert_to_bibjson(el.value(), "openlibrary"));
    }
  } else if (mode == "search") {
    if (src.contains("message") && src["message"].contains("items")) {
      for (auto &item : src["message"]["items"]) {
        out.push_back(convert_to_bibjson(item, "crossref"));
      }
    }
  }
  
  return out;
}

int show_results(const std::vector<nlohmann::json> &entries) {
  int i = 0;
  for (const auto &e : entries) {
    std::string author = "[Unknown]";
    if (e.contains("author") && e["author"].is_array() && !e["author"].empty()) {
      const auto &a = e["author"][0];
      if (a.contains("name"))
        author = a["name"].get<std::string>();
      else if (a.contains("lastname")) {
        author = a["lastname"].get<std::string>();
        if (a.contains("firstname"))
          author += ", " + a["firstname"].get<std::string>();
      }
    }
    
    std::string title = e.value("title", "[No Title]");
    std::string year = e.value("year", "[Year]");
    
    std::cout << "[" << i + 1 << "] " << author << ". " << title << ". " 
              << year << ".\n";
    ++i;
  }
  
  std::cout << "\nSelect (1-" << entries.size() << ", 0 to cancel): ";
  int sel = 0;
  std::cin >> sel;
  std::cin.ignore();
  return sel > 0 && sel <= (int)entries.size() ? sel - 1 : -1;
}

// Show all fields for review
void show_details(const nlohmann::json &entry) {
  std::cout << "\n=== Entry Details ===\n";
  std::cout << entry.dump(2) << "\n";
  std::cout << "=====================\n\n";
}

// Allow user to edit fields
nlohmann::json edit_entry(nlohmann::json entry) {
  std::cout << "\nAvailable fields to edit:\n";
  std::cout << "  title, type, year, publisher, place\n";
  std::cout << "  (for complex fields like author, edit the JSON directly)\n\n";
  
  std::string field, val;
  while (true) {
    field = prompt("Field to edit (or blank to finish)");
    if (field.empty())
      break;
    
    std::cout << "Current value: ";
    if (entry.contains(field))
      std::cout << entry[field].dump();
    else
      std::cout << "(not set)";
    std::cout << "\n";
    
    val = prompt("New value (JSON for arrays/objects, or press Enter to skip)");
    if (!val.empty()) {
      try {
        entry[field] = nlohmann::json::parse(val);
      } catch (...) {
        entry[field] = val;
      }
      std::cout << "Updated.\n";
    }
  }
  return entry;
}

// Append entry to JSON file
void add_to_json(const std::string &filename, const nlohmann::json &entry) {
  nlohmann::json root;
  std::ifstream in(filename);
  if (in) {
    try {
      in >> root;
    } catch (...) {
      root = nlohmann::json::object();
    }
    in.close();
  }
  
  // Ensure proper structure
  if (root.is_null() || !root.is_object()) {
    root = {{"metadata", {{"collection", "my_collection"}}},
            {"records", nlohmann::json::array()}};
  }
  
  if (!root.contains("records") || !root["records"].is_array()) {
    root["records"] = nlohmann::json::array();
  }
  
  // Generate unique ID
  std::string id = "rec_" + std::to_string(root["records"].size() + 1);
  nlohmann::json new_entry = entry;
  new_entry["id"] = id;
  new_entry["collection"] = "my_collection";
  
  root["records"].push_back(new_entry);
  
  // Update metadata
  if (!root.contains("metadata"))
    root["metadata"] = nlohmann::json::object();
  root["metadata"]["records"] = root["records"].size();
  
  std::ofstream out(filename);
  out << root.dump(2) << std::endl;
  std::cout << "\n✓ Entry added to " << filename << " with ID: " << id << "\n";
}

// Entry point for add command
int add_entry(const std::string &filename) {
  std::cout << "\n=== Add New Citation ===\n\n";
  std::string query = prompt("Search by DOI, ISBN, or Title/Author");
  
  if (query.empty()) {
    std::cout << "Search cancelled.\n";
    return 1;
  }
  
  // Determine search mode
  std::string mode;
  if (query.rfind("10.", 0) == 0) {
    mode = "doi";
    std::cout << "Searching CrossRef by DOI...\n";
  } else if (query.size() >= 10 && query.size() <= 13 &&
             std::all_of(query.begin(), query.end(), [](char c) { 
               return std::isdigit(c) || c == '-' || c == 'X'; 
             })) {
    mode = "isbn";
    std::cout << "Searching OpenLibrary by ISBN...\n";
  } else {
    mode = "search";
    std::cout << "Searching CrossRef...\n";
  }
  
  nlohmann::json results = search_sources(query);
  auto entries = parse_results(results, mode);

  if (entries.empty()) {
    std::cout << "No results found. Try a different query.\n";
    return 1;
  }
  
  std::cout << "\n=== Search Results ===\n\n";
  int idx = show_results(entries);
  
  if (idx == -1) {
    std::cout << "Selection cancelled.\n";
    return 1;
  }

  auto entry = entries[idx];
  show_details(entry);

  std::string edit = prompt("Edit fields? (y/n)");
  if (edit == "y" || edit == "Y")
    entry = edit_entry(entry);

  std::string confirm = prompt("Add this entry to " + filename + "? (y/n)");
  if (confirm == "y" || confirm == "Y") {
    add_to_json(filename, entry);
    return 0;
  } else {
    std::cout << "Entry not added.\n";
    return 1;
  }
}
