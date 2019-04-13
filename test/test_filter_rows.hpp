#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Filter a simple csv", "[simple csv]") {
  csv::Reader csv;

  if (csv.read("inputs/test_01.csv")) {
    csv.filter([](auto row) {
      return std::stoi(row["a"]) > 1;
    });
    auto rows = csv.rows();
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0]["a"] == "4");
    REQUIRE(rows[0]["b"] == "5");
    REQUIRE(rows[0]["c"] == "6");
  }
}