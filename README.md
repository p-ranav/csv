# CSV Parser for Modern C++

## Highlights

* Header-only library
* Requires C++17
* BSD 2-Clause "Simplified" License

## Reading CSV files

```cpp
csv::reader("test.csv");

reader.configure_dialect()
  .delimiter(", ")
  .line_terminator("\r\n")
  .trim_characters(' ', '\t')
  .ignore_columns("foo", "bar");

if (reader.parse()) {
  for (auto& row : reader.rows()) {
    int baz_value = row["baz"];
    // do something
  }
}
```

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
This project is available under the [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause) license.
