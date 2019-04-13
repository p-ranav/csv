# CSV for Modern C++

## Highlights

* Header-only library to [read](#reading-csv-files) and [write](#writing-csv-files) CSV files
* Requires C++17
* BSD 2-Clause "Simplified" License

# Reading CSV files

To parse CSV files, simply include ```<csv/reader.hpp>``` and configure a ```csv::Reader``` like so:

```cpp
#include <csv/reader.hpp>

int main() {
  csv::Reader csv;
  if (csv.parse("test.csv")) {
    for (auto& row : csv.rows()) {
      // do something
    }
  }
}
```

## Trimming Characters

Consider this strange, messed up log file: 

```csv
[Thread ID] :: [Log Level] :: [Log Message] :: {Timestamp}
04 :: INFO :: Hello World ::             1555164718
02        :: DEBUG :: Warning! Foo has happened                :: 1555463132
```

To parse such a file, simply:
* Configure a new ```dialect```
* Specify the delimiter
* Provide a list of characters that need to be trimmed. 

```cpp
csv::reader csv("test.csv");

csv.configure_dialect("my strange dialect")
  .delimiter("::")
  .trim_characters(' ', '[', ']', '{', '}');   

if (reader.parse()) {
  for (auto& row : reader.rows()) {
    auto thread_id = row["Thread ID"];
    auto log_level = row["Log Level"];
    auto message = row["Log Message"];
    // do something
  }
}
```

# Writing CSV files

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
This project is available under the [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause) license.
