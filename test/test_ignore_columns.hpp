#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Parse the most basic of CSV buffers and ignore 1 column", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .ignore_columns("a");

  if (csv.parse("inputs/test_01.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].count("a") == 0);
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1].count("a") == 0);
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers and ignore 2 columns", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .ignore_columns("a", "b");

  if (csv.parse("inputs/test_01.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].count("a") == 0);
    REQUIRE(rows[0].count("b") == 0);
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1].count("a") == 0);
    REQUIRE(rows[1].count("b") == 0);
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers and ignore all columns", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .ignore_columns("a", "b", "c");

  if (csv.parse("inputs/test_01.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].size() == 0);
    REQUIRE(rows[1].size() == 0);
  }
}