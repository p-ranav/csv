#include <csv.hpp>

int main() {
  csv::reader reader;

  reader.configure()
    .delimiter(", ");

  if (reader.parse("test.csv")) {
    std::cout << "Rows: " << reader.rows() << std::endl;
  }

  return 0;
}