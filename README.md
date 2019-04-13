# CSV Parser for Modern C++

## Highlights

* Header-only library
* Requires C++17
* BSD 2-Clause "Simplified" License

## Quick Start

```cpp
csv::reader("test.csv");

reader.configure()
  .delimiter(", ")
  .line_terminator("\r\n")
  .trim_characters(' ', '\t')
  .ignore_columns("foo", "bar")
  .transform_column("baz", [](const std::string& value) { return std::stoi(value); });

if (reader.parse()) {
  for (auto& row : reader.rows()) {
    int baz_value = row["baz"];
    // do something
  }
}
```
