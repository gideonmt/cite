#include "add.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

std::string prompt(const std::string &q) {
  std::cout << q << ": ";
  std::string a;
  std::getline(std::cin, a);
  return a;
}

int cite_add(const std::string &filename) {
  std::string type = prompt("Type of source (book/article/other)");
  nlohmann::json entry;
  entry["type"] = type;

  entry["title"] = prompt("Title");

  std::vector<nlohmann::json> authors;
  std::cout << "Enter authors (empty to finish):\n";
  while (true) {
    std::string name = prompt("Author name (Last, First)");
    if (name.empty())
      break;
    authors.push_back({{"name", name}});
  }
  if (!authors.empty())
    entry["author"] = authors;

  if (type == "book") {
    entry["publisher"] = prompt("Publisher");
    entry["year"] = prompt("Year");
    std::string isbn = prompt("ISBN (optional)");
    if (!isbn.empty())
      entry["identifier"] = {{{"type", "isbn"}, {"id", isbn}}};
  } else if (type == "article") {
    entry["journal"] = {{"name", prompt("Journal name")}};
    entry["year"] = prompt("Year");
    entry["journal"]["volume"] = prompt("Volume (optional)");
    entry["journal"]["pages"] = prompt("Pages (optional)");
    std::string doi = prompt("DOI (optional)");
    if (!doi.empty())
      entry["identifier"] = {{{"type", "doi"}, {"id", doi}}};
  } else {
    entry["year"] = prompt("Year (optional)");
  }

  nlohmann::json root;
  std::ifstream in(filename);
  if (in) {
    in >> root;
    in.close();
  }
  if (root.is_null()) {
    root = {{"metadata", {{"collection", "my_collection"}}},
            {"records", nlohmann::json::array()}};
  }
  if (!root.contains("records") || !root["records"].is_array())
    root["records"] = nlohmann::json::array();

  root["records"].push_back(entry);

  std::ofstream out(filename);
  out << root.dump(2) << std::endl;
  std::cout << "Entry added to " << filename << "\n";
  return 0;
}
