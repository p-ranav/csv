#include <csv.hpp>

int main() {
  csv::reader reader("test.csv");
  std::cout << "Rows: " << reader.rows() << std::endl;
  return 0;
}