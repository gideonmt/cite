#include "add.hpp"
#include <curl/curl.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

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
  } else if (query.size() > 7 &&
             std::all_of(query.begin(), query.end(), ::isdigit)) { // ISBN
    url = "https://openlibrary.org/api/books?bibkeys=ISBN:" + query +
          "&format=json&jscmd=data";
  } else { // Title/author search (CrossRef)
    char *esc = curl_easy_escape(curl, query.c_str(), 0);
    url =
        "https://api.crossref.org/works?query=" + std::string(esc ? esc : "") +
        "&rows=10";
    if (esc)
      curl_free(esc);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_perform(curl);
  curl_easy_cleanup(curl);

  return nlohmann::json::parse(readBuffer.empty() ? "{}" : readBuffer);
}

// Normalize results to vector of entries
std::vector<nlohmann::json> parse_results(const nlohmann::json &src,
                                          const std::string &mode) {
  std::vector<nlohmann::json> out;
  if (mode == "doi") {
    if (src.contains("message"))
      out.push_back(src["message"]);
  } else if (mode == "isbn") {
    for (auto &el : src.items())
      out.push_back(el.value());
  } else if (mode == "search") {
    if (src.contains("message") && src["message"].contains("items")) {
      for (auto &item : src["message"]["items"])
        out.push_back(item);
    }
  }
  return out;
}

// Display list as: Author Last, Author First. Title. Year.
// Returns selected index, or -1 if none
int show_results(const std::vector<nlohmann::json> &entries) {
  int i = 0;
  for (const auto &e : entries) {
    std::string author = "[Unknown]";
    if (e.contains("author") && e["author"].is_array() &&
        !e["author"].empty()) {
      const auto &a = e["author"][0];
      std::string last = a.value("family", "");
      std::string first = a.value("given", "");
      author = last + (last.empty() ? "" : ", ") + first;
    } else if (e.contains("authors") && e["authors"].is_array() &&
               !e["authors"].empty()) {
      const auto &a = e["authors"][0];
      author = a.value("name", "");
    }
    std::string title = "[No Title]";
    if (e.contains("title")) {
      if (e["title"].is_array() && !e["title"].empty())
        title = e["title"][0].get<std::string>();
      else if (e["title"].is_string())
        title = e["title"].get<std::string>();
    }
    std::string year = "[Year]";
    if (e.contains("issued") && e["issued"].contains("date-parts"))
      year = std::to_string(e["issued"]["date-parts"][0][0].get<int>());
    else if (e.contains("publish_date"))
      year = e.value("publish_date", "[Year]");

    std::cout << "[" << i + 1 << "] " << author << ". " << title << ". " << year
              << ".\n";
    ++i;
  }
  std::cout << "Select (1-" << entries.size() << ", 0 to cancel): ";
  int sel = 0;
  std::cin >> sel;
  std::cin.ignore();
  return sel > 0 && sel <= (int)entries.size() ? sel - 1 : -1;
}
// Show all fields for review
void show_details(const nlohmann::json &entry) {
  std::cout << entry.dump(2) << "\n";
}

// Allow user to edit fields
nlohmann::json edit_entry(nlohmann::json entry) {
  std::string field, val;
  while (true) {
    field = prompt("Field to edit (or blank to finish)");
    if (field.empty())
      break;
    val = prompt("New value (JSON for arrays/objects)");
    if (!val.empty()) {
      try {
        entry[field] = nlohmann::json::parse(val);
      } catch (...) {
        entry[field] = val;
      }
    }
  }
  return entry;
}

// Append entry to JSON file
void add_to_json(const std::string &filename, const nlohmann::json &entry) {
  nlohmann::json root;
  std::ifstream in(filename);
  if (in) {
    in >> root;
    in.close();
  }
  if (root.is_null())
    root = {{"metadata", {{"collection", "my_collection"}}},
            {"records", nlohmann::json::array()}};
  if (!root.contains("records") || !root["records"].is_array())
    root["records"] = nlohmann::json::array();
  root["records"].push_back(entry);
  std::ofstream out(filename);
  out << root.dump(2) << std::endl;
  std::cout << "Entry added to " << filename << "\n";
}

// Entry point for new add flow
int add_entry(const std::string &filename) {
  std::string query = prompt("Search by DOI, ISBN, or Title/Author");
  std::string mode =
      (query.rfind("10.", 0) == 0) ? "doi"
      : (query.size() > 7 && std::all_of(query.begin(), query.end(), ::isdigit))
          ? "isbn"
          : "search";
  nlohmann::json results = search_sources(query);
  auto entries = parse_results(results, mode);

  if (entries.empty()) {
    std::cout << "No results found.\n";
    return 1;
  }
  int idx = show_results(entries);
  if (idx == -1)
    return 1;

  auto entry = entries[idx];
  show_details(entry);

  std::string edit = prompt("Edit fields? (y/n)");
  if (edit == "y" || edit == "Y")
    entry = edit_entry(entry);

  add_to_json(filename, entry);
  return 0;
}
