#include <csv.hpp>

int main() {
  csv::reader foo;

  foo.configure_dialect()
    .delimiter(",")
    .newline("\r\n")
    .quotechar('"')
    .trim_whitespace(true);

  if (foo.parse("quoted.csv")) {
    std::cout << "Rows: " << foo.shape().first << std::endl;
  }

  return 0;
}