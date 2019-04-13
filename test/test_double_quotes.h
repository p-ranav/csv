#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Parse headers with double quotes", "[simple csv]") {
  csv::reader csv;

  if (csv.parse("inputs/test_06.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 0);
    auto cols = csv.cols();
    REQUIRE(cols.size() == 3);
    REQUIRE(cols[0] == "\"Free trip to A,B\"");
    REQUIRE(cols[1] == "\"5.89\"");
    REQUIRE(cols[2] == "\"Special rate \"\"1.79\"\"\"");
  }
}