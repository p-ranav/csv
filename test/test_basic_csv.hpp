#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>

TEST_CASE("Parse the most basic of CSV buffers", "[simple csv]") {
  csv::reader csv;

  if (csv.parse("inputs/test_01.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers with ', ' delimiter", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .delimiter(", ");

  if (csv.parse("inputs/test_02.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers with ', ' delimiter using skip_initial_space_", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .delimiter(",")
    .skip_initial_space(true);

  if (csv.parse("inputs/test_02.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers with '::' delimiter", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .delimiter("::");

  if (csv.parse("inputs/test_03.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers - Trim whitespace characters", "[simple csv]") {
  csv::reader csv;

  csv.configure_dialect()
    .trim_characters(' ', '\t');

  if (csv.parse("inputs/test_02.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers - Trim whitespace characters gone crazy", "[simple csv]") {
  csv::reader csv;

  csv.configure_dialect()
    .trim_characters(' ', '\t');

  if (csv.parse("inputs/test_04.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["a"] == "1");
    REQUIRE(rows[0]["b"] == "2");
    REQUIRE(rows[0]["c"] == "3");
    REQUIRE(rows[1]["a"] == "4");
    REQUIRE(rows[1]["b"] == "5");
    REQUIRE(rows[1]["c"] == "6");
  }
}

TEST_CASE("Parse the most basic of CSV buffers - Log messages", "[simple csv]") {
  csv::reader csv;
  csv.configure_dialect()
    .delimiter("::");
  if (csv.parse("inputs/test_05.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 3);
    REQUIRE(rows[0]["Thread_ID"] == "1");
    REQUIRE(rows[0]["Log_Level"] == "DEBUG");
    REQUIRE(rows[0]["Message"] == "Thread Started");
    REQUIRE(rows[1]["Thread_ID"] == "2");
    REQUIRE(rows[1]["Log_Level"] == "DEBUG");
    REQUIRE(rows[1]["Message"] == "Thread Started");
    REQUIRE(rows[2]["Thread_ID"] == "3");
    REQUIRE(rows[2]["Log_Level"] == "ERROR");
    REQUIRE(rows[2]["Message"] == "File not found");
  }
}

TEST_CASE("Parse the most basic of CSV buffers - No header row", "[simple csv]") {
  csv::reader csv;

  csv.configure_dialect()
    .header(false);

  if (csv.parse("inputs/test_08.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 3);
    REQUIRE(rows[0]["0"] == "1");
    REQUIRE(rows[0]["1"] == "2");
    REQUIRE(rows[0]["2"] == "3");
    REQUIRE(rows[1]["0"] == "4");
    REQUIRE(rows[1]["1"] == "5");
    REQUIRE(rows[1]["2"] == "6");
    REQUIRE(rows[2]["0"] == "7");
    REQUIRE(rows[2]["1"] == "8");
    REQUIRE(rows[2]["2"] == "9");
  }
}