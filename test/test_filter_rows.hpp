#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Filter a simple csv", "[simple csv]") {
  csv::Reader csv;

  if (csv.read("inputs/test_01.csv")) {
    auto rows = csv.filter([](auto row) {
      return std::stoi(row["a"]) > 1;
    });
    REQUIRE(rows.size() == 1);
    REQUIRE(rows[0]["a"] == "4");
    REQUIRE(rows[0]["b"] == "5");
    REQUIRE(rows[0]["c"] == "6");
  }
}

TEST_CASE("Filter a simple csv II", "[simple csv]") {
  csv::Reader csv;

  csv.configure_dialect("students")
    .skip_initial_space(true);

  if (csv.read("inputs/test_15.csv")) {
    auto rows = csv.filter([](auto row) {
      return row["grade"] == "F";
    });
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["first_name"] == "Tom");
    REQUIRE(rows[0]["last_name"] == "Smith");
    REQUIRE(rows[0]["grade"] == "F");
    REQUIRE(rows[1]["first_name"] == "Jane");
    REQUIRE(rows[1]["last_name"] == "Dee");
    REQUIRE(rows[1]["grade"] == "F");
  }
}