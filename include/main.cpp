#include <csv.hpp>

int main() {
  csv::reader foo;

  foo.configure()
    .delimiter(",")
    .newline("\r\n");

  if (foo.parse("test.csv")) {
    std::cout << "Rows: " << foo.shape().first << std::endl;
  }

  return 0;
}