#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Parse the most basic of CSV buffers and ignore 1 column", "[simple csv]") {
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .ignore_columns("a");

  if (csv.read("inputs/test_01.csv")) {
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
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .ignore_columns("a", "b");

  if (csv.read("inputs/test_01.csv")) {
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
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .ignore_columns("a", "b", "c");

  if (csv.read("inputs/test_01.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0].size() == 0);
    REQUIRE(rows[1].size() == 0);
  }
}

TEST_CASE("Parse the most basic of CSV buffers and ignore age/gender columns", "[simple csv]") {
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .delimiter(", ")
    .ignore_columns("age", "gender");

  if (csv.read("inputs/test_14.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 3);
    REQUIRE(rows[0]["name"] == "Mark Johnson");
    REQUIRE(rows[0]["email"] == "mark.johnson@gmail.com");
    REQUIRE(rows[0]["department"] == "BA");
    REQUIRE(rows[0].count("age") == 0);
    REQUIRE(rows[0].count("gender") == 0);
    REQUIRE(rows[1]["name"] == "John Stevenson");
    REQUIRE(rows[1]["email"] == "john.stevenson@gmail.com");
    REQUIRE(rows[1]["department"] == "IT");
    REQUIRE(rows[1].count("age") == 0);
    REQUIRE(rows[1].count("gender") == 0);
    REQUIRE(rows[2]["name"] == "Jane Barkley");
    REQUIRE(rows[2]["email"] == "jane.barkley@gmail.com");
    REQUIRE(rows[2]["department"] == "MGT");
    REQUIRE(rows[2].count("age") == 0);
    REQUIRE(rows[2].count("gender") == 0);
  }
}