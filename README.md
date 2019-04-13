# CSV for Modern C++

## Highlights

* Header-only library to [read](#reading-csv-files) and [write](#writing-csv-files) CSV files
* Requires C++17
* BSD 2-Clause "Simplified" License

## Reading CSV files

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

## Default Dialects

This csv library comes with three stanard dialects:

| Name | Description |
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| excel | The excel dialect defines the usual properties of an Excel-generated CSV file |
| excel_tab | The excel_tab dialect defines the usual properties of an Excel-generated TAB-delimited file |
| unix | The unix dialect defines the usual properties of a CSV file generated on UNIX systems, i.e. using  '\n' as line terminator and quoting all fields |

Custom dialects can be constructed with ```.configure_dialect(...)``` method provided by both csv::Reader and csv::Writer. Below, you can see some examples of custom dialects. If no dialect is provided, the ```excel``` dialect is the default. 

### Trimming Characters

Consider this strange, messed up log file: 

```csv
[Thread ID] :: [Log Level] :: [Log Message] :: {Timestamp}
04 :: INFO :: Hello World ::             1555164718
02        :: DEBUG :: Warning! Foo has happened                :: 1555463132
```

To parse this file, simply:
* Configure a new ```dialect```
* Specify the delimiter as "::"
* Provide a list of characters that need to be trimmed

```cpp
csv::reader csv;

csv.configure_dialect("my strange dialect")
  .delimiter("::")
  .trim_characters(' ', '[', ']', '{', '}');   

if (reader.parse("test.csv")) {
  for (auto& row : reader.rows()) {
    auto thread_id = row["Thread ID"];
    auto log_level = row["Log Level"];
    auto message = row["Log Message"];
    // do something
  }
}
```

## Writing CSV files

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
This project is available under the [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause) license.
