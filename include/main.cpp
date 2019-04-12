#include <csv.hpp>

int main() {
  csv::reader csv;

  csv.configure_dialect()
    .delimiter(" ")
    .newline("\r\n")
    .quotechar('|')
    .trim_whitespace(true);

  if (csv.parse("quoted.csv")) {
    for (auto& row : csv.rows()) {
      for (auto& [k, v] : row) {
        std::cout << k << ": " << v << std::endl;
      }
      std::cout << "-----------\n";
    }
  }

  return 0;
}