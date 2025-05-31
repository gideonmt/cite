#include "export.hpp"
#include "../include/citation.hpp"
#include "../parsers/bibjson_parser.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

// Simple helper: convert <i>...</i> to *...* for Markdown
static std::string html_to_md(const std::string &in) {
  std::string out = in;
  size_t pos = 0;
  while ((pos = out.find("<i>", pos)) != std::string::npos) {
    out.replace(pos, 3, "*");
  }
  pos = 0;
  while ((pos = out.find("</i>", pos)) != std::string::npos) {
    out.replace(pos, 4, "*");
  }
  return out;
}

int cite_export(const std::string &filename, const std::string &style,
                const std::string &output_file) {
  nlohmann::json root = parse_bibjson(filename);
  nlohmann::json entries;
  if (root.contains("records") && root["records"].is_array()) {
    entries = root["records"];
  } else if (root.is_array()) {
    entries = root;
  } else {
    std::cerr << "Could not find any BibJSON records.\n";
    return 2;
  }

  // Prepare output stream
  std::ostream *out = &std::cout;
  std::ofstream outfile;
  bool to_html = false, to_md = false;
  if (!output_file.empty()) {
    if (output_file.size() > 5 &&
        output_file.substr(output_file.size() - 5) == ".html") {
      to_html = true;
      outfile.open(output_file);
      if (!outfile) {
        std::cerr << "Cannot open " << output_file << "\n";
        return 3;
      }
      out = &outfile;
    } else if (output_file.size() > 3 &&
               output_file.substr(output_file.size() - 3) == ".md") {
      to_md = true;
      outfile.open(output_file);
      if (!outfile) {
        std::cerr << "Cannot open " << output_file << "\n";
        return 3;
      }
      out = &outfile;
    }
  }

  auto print_md = [&](const std::string &text) {
    *out << html_to_md(text) << "\n";
  };
  auto print_html = [&](const std::string &text) { *out << text << "<br/>\n"; };

  // Output with markup
  if (style == "chicago") {
    auto bundles = format_chicago_with_footnotes(entries);
    if (to_html) {
      *out << "<h2>Bibliography</h2>\n<ol>\n";
      for (const auto &c : bundles)
        *out << "<li>" << c.bibliography << "</li>\n";
      *out << "</ol>\n<h2>Long footnotes</h2>\n<ol>\n";
      for (const auto &c : bundles)
        *out << "<li>" << c.long_footnote << "</li>\n";
      *out << "</ol>\n<h2>Short footnotes</h2>\n<ol>\n";
      for (const auto &c : bundles)
        *out << "<li>" << c.short_footnote << "</li>\n";
      *out << "</ol>\n";
    } else {
      // Markdown (to_md or terminal)
      *out << "## Bibliography\n";
      int i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". ";
        print_md(c.bibliography);
      }
      *out << "\n## Long footnotes\n";
      i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". ";
        print_md(c.long_footnote);
      }
      *out << "\n## Short footnotes\n";
      i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". ";
        print_md(c.short_footnote);
      }
    }
  } else {
    std::vector<std::string> formatted = format_bibliography(entries, style);
    int i = 1;
    if (to_html) {
      *out << "<ol>\n";
      for (const auto &f : formatted)
        *out << "<li>" << f << "</li>\n";
      *out << "</ol>\n";
    } else {
      for (const auto &f : formatted) {
        *out << i++ << ". ";
        print_md(f);
      }
    }
  }
  if (outfile)
    outfile.close();
  return 0;
}
