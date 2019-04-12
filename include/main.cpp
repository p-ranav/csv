#include <csv.hpp>

int main() {
  csv::reader foo;

  foo.configure_dialect()
    .delimiter(", ")
    .newline("\r\n")
    .quotechar('"')
    .trim_whitespace(true);

  if (foo.parse("quoted.csv")) {
    for (auto& row : foo.rows()) {
      for (auto& [k, v] : row) {
        std::cout << k << ": " << v << std::endl;
      }
      std::cout << "-----------\n";
    }
  }

  return 0;
}