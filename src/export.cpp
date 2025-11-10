#include "export.hpp"
#include "../include/citation.hpp"
#include "../parsers/json_parser.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

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

static std::string html_escape(const std::string &in) {
  std::string out;
  for (char c : in) {
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      case '\'': out += "&#39;"; break;
      default: out += c;
    }
  }
  return out;
}

int cite_export(const std::string &filename, const std::string &style,
                const std::string &output_file) {
  // Parse input file
  nlohmann::json root = parse_json(filename);
  nlohmann::json entries;
  
  if (root.contains("records") && root["records"].is_array()) {
    entries = root["records"];
  } else if (root.is_array()) {
    entries = root;
  } else {
    std::cerr << "Error: Could not find any BibJSON records in " << filename << "\n";
    std::cerr << "Expected 'records' array or top-level array.\n";
    return 2;
  }

  if (entries.empty()) {
    std::cerr << "Warning: No entries found in " << filename << "\n";
    return 2;
  }

  std::cout << "Loaded " << entries.size() << " entries from " << filename << "\n";

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
        std::cerr << "Error: Cannot open " << output_file << " for writing\n";
        return 3;
      }
      out = &outfile;
    } else if (output_file.size() > 3 &&
               output_file.substr(output_file.size() - 3) == ".md") {
      to_md = true;
      outfile.open(output_file);
      if (!outfile) {
        std::cerr << "Error: Cannot open " << output_file << " for writing\n";
        return 3;
      }
      out = &outfile;
    } else {
      std::cerr << "Error: Output file must end in .html or .md\n";
      return 3;
    }
  }

  // Generate citations
  if (style == "chicago") {
    auto bundles = format_chicago_with_footnotes(entries);
    
    if (to_html) {
      // HTML output with styling
      *out << "<!DOCTYPE html>\n";
      *out << "<html lang=\"en\">\n";
      *out << "<head>\n";
      *out << "  <meta charset=\"UTF-8\">\n";
      *out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
      *out << "  <title>Chicago Style Bibliography</title>\n";
      *out << "  <style>\n";
      *out << "    body { font-family: 'Times New Roman', Times, serif; max-width: 800px; margin: 40px auto; padding: 0 20px; line-height: 1.6; }\n";
      *out << "    h1 { font-size: 24px; font-weight: bold; margin-top: 40px; margin-bottom: 20px; border-bottom: 2px solid #333; padding-bottom: 10px; }\n";
      *out << "    h2 { font-size: 20px; font-weight: bold; margin-top: 30px; margin-bottom: 15px; }\n";
      *out << "    ol { padding-left: 0; }\n";
      *out << "    li { margin-bottom: 12px; margin-left: 2em; text-indent: -2em; }\n";
      *out << "    i { font-style: italic; }\n";
      *out << "    .note { color: #666; font-size: 0.9em; margin-top: 30px; padding: 10px; background: #f5f5f5; border-left: 3px solid #ccc; }\n";
      *out << "  </style>\n";
      *out << "</head>\n";
      *out << "<body>\n";
      *out << "  <h1>Chicago Style Citations</h1>\n";
      
      *out << "  <h2>Bibliography</h2>\n";
      *out << "  <ol>\n";
      for (const auto &c : bundles) {
        *out << "    <li>" << c.bibliography << "</li>\n";
      }
      *out << "  </ol>\n";
      
      *out << "  <h2>Footnotes (First Reference)</h2>\n";
      *out << "  <ol>\n";
      for (const auto &c : bundles) {
        *out << "    <li>" << c.long_footnote << "</li>\n";
      }
      *out << "  </ol>\n";
      
      *out << "  <h2>Footnotes (Subsequent References)</h2>\n";
      *out << "  <ol>\n";
      for (const auto &c : bundles) {
        *out << "    <li>" << c.short_footnote << "</li>\n";
      }
      *out << "  </ol>\n";
      
      *out << "  <div class=\"note\">\n";
      *out << "    <strong>Note:</strong> Replace <code>[pg]</code> with actual page numbers when citing.\n";
      *out << "  </div>\n";
      *out << "</body>\n";
      *out << "</html>\n";
      
    } else {
      // Markdown output (to file or terminal)
      if (to_md) {
        *out << "# Chicago Style Citations\n\n";
        *out << "Generated from: " << filename << "\n\n";
      } else {
        *out << "\n=== Chicago Style Citations ===\n\n";
      }
      
      *out << "## Bibliography\n\n";
      int i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". " << html_to_md(c.bibliography) << "\n\n";
      }
      
      *out << "## Footnotes (First Reference)\n\n";
      i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". " << html_to_md(c.long_footnote) << "\n\n";
      }
      
      *out << "## Footnotes (Subsequent References)\n\n";
      i = 1;
      for (const auto &c : bundles) {
        *out << i++ << ". " << html_to_md(c.short_footnote) << "\n\n";
      }
      
      *out << "---\n\n";
      *out << "*Note: Replace `[pg]` with actual page numbers when citing.*\n";
    }
    
  } else {
    std::cerr << "Error: Style '" << style << "' is not yet implemented.\n";
    std::cerr << "Currently supported: chicago\n";
    return 4;
  }
  
  if (outfile) {
    outfile.close();
    std::cout << "Output written to: " << output_file << "\n";
  }
  
  return 0;
}
