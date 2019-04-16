#pragma once
#include <catch.hpp>
#include <csv/reader.hpp>
#include <csv/async_reader.hpp>

TEST_CASE("Parse the most basic of CSV buffers", "[simple csv]") {
  csv::Reader csv;

  if (csv.read("inputs/test_01.csv")) {
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

TEST_CASE("Parse the most basic of CSV buffers (Async)", "[simple csv]") {
  csv::AsyncReader csv;
  csv.read("inputs/test_01.csv");
  std::vector<std::unordered_map<std::string, std::string>> rows;
  while (!csv.done()) {
    if (csv.has_next()) {
      auto row = csv.next();
      rows.push_back(row);
    }
  }

  REQUIRE(rows.size() == 2);
  REQUIRE(rows[0]["a"] == "1");
  REQUIRE(rows[0]["b"] == "2");
  REQUIRE(rows[0]["c"] == "3");
  REQUIRE(rows[1]["a"] == "4");
  REQUIRE(rows[1]["b"] == "5");
  REQUIRE(rows[1]["c"] == "6");
}

TEST_CASE("Parse the most basic of CSV buffers with ', ' delimiter", "[simple csv]") {
  csv::Reader csv;
  auto foo = csv.configure_dialect("test_dialect");
  csv.configure_dialect("test_dialect")
    .delimiter(", ");

  if (csv.read("inputs/test_02.csv")) {
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
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .delimiter(", ")
    .skip_initial_space(true);

  if (csv.read("inputs/test_02.csv")) {
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
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .delimiter("::");

  if (csv.read("inputs/test_03.csv")) {
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
  csv::Reader csv;

  csv.configure_dialect("test_dialect")
    .trim_characters(' ', '\t');

  if (csv.read("inputs/test_02.csv")) {
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
  csv::Reader csv;

  csv.configure_dialect("test_dialect")
    .trim_characters(' ', '\t');

  if (csv.read("inputs/test_04.csv")) {
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
  csv::Reader csv;
  csv.configure_dialect("test_dialect")
    .delimiter("::");
  if (csv.read("inputs/test_05.csv")) {
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
  csv::Reader csv;

  csv.configure_dialect("test_dialect")
    .header(false);

  if (csv.read("inputs/test_08.csv")) {
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

TEST_CASE("Parse the most basic of CSV buffers - Space delimiter", "[simple csv]") {
  csv::Reader csv;

  csv.configure_dialect("test_dialect")
    .delimiter(" ");

  if (csv.read("inputs/test_09.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["first_name"] == "Eric");
    REQUIRE(rows[0]["last_name"] == "Idle");
    REQUIRE(rows[1]["first_name"] == "John");
    REQUIRE(rows[1]["last_name"] == "Cleese");
  }
}

TEST_CASE("Parse the most basic of CSV buffers - Log with header", "[simple csv]") {
  csv::Reader csv;

  csv.configure_dialect("test_dialect")
    .delimiter("::")
    .trim_characters(' ', '[', ']');

  if (csv.read("inputs/test_10.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["Timestamp"] == "1555164718");
    REQUIRE(rows[0]["Thread ID"] == "04");
    REQUIRE(rows[0]["Log Level"] == "INFO");
    REQUIRE(rows[0]["Log Message"] == "Hello World");
    REQUIRE(rows[1]["Timestamp"] == "1555463132");
    REQUIRE(rows[1]["Thread ID"] == "02");
    REQUIRE(rows[1]["Log Level"] == "DEBUG");
    REQUIRE(rows[1]["Log Message"] == "Warning! Foo has happened");
  }
}

TEST_CASE("Parse Excel CSV", "[simple csv]") {
  csv::Reader csv;
  csv.use_dialect("excel");

  if (csv.read("inputs/test_11_excel.csv")) {
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

TEST_CASE("Parse Unix CSV", "[simple csv]") {
  csv::Reader csv;
  csv.use_dialect("unix");

  if (csv.read("inputs/test_12_unix.csv")) {
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

TEST_CASE("Parse messed up log files", "[simple csv]") {
  csv::Reader csv;

  csv.configure_dialect("my strange dialect")
    .delimiter("::")
    .trim_characters(' ', '[', ']', '{', '}');

  if (csv.read("inputs/test_13.csv")) {
    auto rows = csv.rows();
    REQUIRE(rows.size() == 2);
    REQUIRE(rows.size() == 2);
    REQUIRE(rows[0]["Timestamp"] == "1555164718");
    REQUIRE(rows[0]["Thread ID"] == "04");
    REQUIRE(rows[0]["Log Level"] == "INFO");
    REQUIRE(rows[0]["Log Message"] == "Hello World");
    REQUIRE(rows[1]["Timestamp"] == "1555463132");
    REQUIRE(rows[1]["Thread ID"] == "02");
    REQUIRE(rows[1]["Log Level"] == "DEBUG");
    REQUIRE(rows[1]["Log Message"] == "Warning! Foo has happened");
  }
}