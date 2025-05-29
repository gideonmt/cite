#include "export.hpp"
#include <iostream>
#include <string>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage:\n";
    std::cerr << "  cite add <file.json>\n";
    std::cerr << "  cite export <file.json> <style> [output.html|output.md]\n";
    return 1;
  }

  std::string command = argv[1];
  if (command == "add") {
    if (argc < 3) {
      std::cerr << "Usage: cite add <file.json>\n";
      return 1;
    }
    std::string filename = argv[2];
    std::cout << "TODO" << std::endl;
    return 0;
  } else if (command == "export") {

    if (argc < 4) {
      std::cerr
          << "Usage: cite export <file.json> <style> [output.html|output.md]\n";
      return 1;
    }
    std::string filename = argv[2];
    std::string style = argv[3];
    std::string output = (argc >= 5 ? argv[4] : "");
    return cite_export(filename, style, output);
  } else {
    std::cerr << "Unknown command: " << command << "\n";
    return 1;
  }
}
