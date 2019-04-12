#include <csv.hpp>

int main() {
  csv::reader foo;

  foo.dialect()
    .delimiter(", ")
    .newline("\r\n")
    .quote('"');

  if (foo.parse("quoted.csv")) {
    std::cout << "Rows: " << foo.shape().first << std::endl;
    for (auto& rows : foo.dict()) {
      for (auto& [k, v] : rows) {
        std::cout << k << ": " << v << std::endl;
      }
      std::cout << "----------------" << std::endl;
    }
  }

  return 0;
}