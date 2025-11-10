#include "add.hpp"
#include "export.hpp"
#include <iostream>
#include <string>

void print_usage() {
  std::cout << "\n";
  std::cout << "cite - Citation Management Tool\n";
  std::cout << "================================\n\n";
  std::cout << "USAGE:\n";
  std::cout << "  cite add <file.json>\n";
  std::cout << "  cite export <file.json> <style> [output]\n";
  std::cout << "  cite help\n";
  std::cout << "  cite version\n\n";
  std::cout << "COMMANDS:\n";
  std::cout << "  add      Search and add a citation to your bibliography\n";
  std::cout << "  export   Generate formatted bibliography and footnotes\n";
  std::cout << "  help     Show this help message\n";
  std::cout << "  version  Show version information\n\n";
  std::cout << "ADD COMMAND:\n";
  std::cout << "  cite add mybibliography.json\n\n";
  std::cout << "  Interactively search for and add citations:\n";
  std::cout << "  - By DOI: 10.1186/1758-2946-3-47\n";
  std::cout << "  - By ISBN: 978-0-226-45808-3\n";
  std::cout << "  - By title/author: quantum computing feynman\n\n";
  std::cout << "EXPORT COMMAND:\n";
  std::cout << "  cite export mybibliography.json chicago\n";
  std::cout << "  cite export mybibliography.json chicago output.md\n";
  std::cout << "  cite export mybibliography.json chicago output.html\n\n";
  std::cout << "  Styles: chicago (mla and apa coming soon)\n";
  std::cout << "  Formats: terminal (default), .md (Markdown), .html (HTML)\n\n";
  std::cout << "EXAMPLES:\n";
  std::cout << "  # Add a citation by DOI\n";
  std::cout << "  cite add my_papers.json\n";
  std::cout << "  > Search: 10.1093/mind/LIX.236.433\n\n";
  std::cout << "  # Export to terminal\n";
  std::cout << "  cite export my_papers.json chicago\n\n";
  std::cout << "  # Export to HTML file\n";
  std::cout << "  cite export my_papers.json chicago bibliography.html\n\n";
  std::cout << "For more information, see readme.md\n\n";
}

void print_version() {
  std::cout << "\ncite version 1.0.0\n";
  std::cout << "Chicago Manual of Style (17th edition)\n";
  std::cout << "Built with C++17, libcurl, nlohmann/json\n\n";
}

int main(int argc, char *argv[]) {
  // No arguments - show help
  if (argc < 2) {
    print_usage();
    return 1;
  }

  std::string command = argv[1];
  
  // Help command
  if (command == "help" || command == "-h" || command == "--help") {
    print_usage();
    return 0;
  }
  
  // Version command
  if (command == "version" || command == "-v" || command == "--version") {
    print_version();
    return 0;
  }
  
  // Add command
  if (command == "add") {
    if (argc < 3) {
      std::cerr << "Error: Missing filename\n";
      std::cerr << "Usage: cite add <file.json>\n";
      std::cerr << "Example: cite add mybibliography.json\n\n";
      return 1;
    }
    std::string filename = argv[2];
    return add_entry(filename);
  }
  
  // Export command
  if (command == "export") {
    if (argc < 4) {
      std::cerr << "Error: Missing arguments\n";
      std::cerr << "Usage: cite export <file.json> <style> [output]\n";
      std::cerr << "Example: cite export mybibliography.json chicago output.html\n\n";
      return 1;
    }
    std::string filename = argv[2];
    std::string style = argv[3];
    std::string output = (argc >= 5 ? argv[4] : "");
    
    // Validate style
    if (style != "chicago") {
      std::cerr << "Error: Unknown style '" << style << "'\n";
      std::cerr << "Currently supported: chicago\n";
      std::cerr << "Coming soon: mla, apa\n\n";
      return 1;
    }
    
    return cite_export(filename, style, output);
  }
  
  // Unknown command
  std::cerr << "Error: Unknown command '" << command << "'\n";
  std::cerr << "Run 'cite help' for usage information\n\n";
  return 1;
}
