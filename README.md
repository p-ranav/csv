# CSV for Modern C++

## Highlights

* Header-only library to read/write CSV files
* Requires C++17
* BSD 2-Clause "Simplified" License

## Quick Start

### Reading CSV files

To parse CSV files, simply include ```<csv/reader.hpp>``` and configure a ```csv::Reader``` like so:

```cpp
#include <csv/reader.hpp>

int main() {
  csv::Reader csv;
  if (csv.parse("test.csv")) {
    for (auto& row : csv.rows()) {
      auto foo = row["foo"];
      auto bar = row["bar"];
      // do something
    }
  }
}
```

## Dialects

This csv library comes with three standard dialects:

| Name | Description |
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------|
| excel | The excel dialect defines the usual properties of an Excel-generated CSV file |
| excel_tab | The excel_tab dialect defines the usual properties of an Excel-generated TAB-delimited file |
| unix | The unix dialect defines the usual properties of a CSV file generated on UNIX systems, i.e. using  '\n' as line terminator and quoting all fields |

Custom dialects can be constructed with ```.configure_dialect(...)``` method provided by both ```csv::Reader``` and ```csv::Writer```. Below, you can see some examples of custom dialects. If no dialect is provided, the ```excel``` dialect is used. 

### Configuring custom dialects

The CSV dialect has numerous properties that can be configured using ```.configure_dialect```

```cpp
csv.configure_dialect("my fancy dialect")
  .delimiter("")
  .line_terminator("")
  .quote_character('"')
  .double_quote(true)
  .skip_initial_space(false)
  .trim_characters(' ', '\t')
  .ignore_columns("foo", "bar")
  .header(true)
```

| Property | Data Type | Description |
|--------------------|-------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| delimiter | ```std::string``` | specifies the character sequence which should separate fields (aka columns). Default = ```","``` |
| line_terminator | ```std::string``` | specifies the character sequence which should terminate rows. Default = ```"\r\n"``` |
| quote_character | ```char``` | specifies a one-character string to use as the quoting character. Default = ```'"'``` |
| double_quote | ```bool``` | controls the handling of quotes inside fields. If true, two consecutive quotes should be interpreted as one. Default = ```true``` |
| skip_initial_space | ```bool``` | specifies how to interpret whitespace which immediately follows a delimiter; if false, it means that whitespace immediately after a delimiter should be treated as part of the following field. Default = ```true``` |
| header | ```bool``` | indicates whether the file includes a header row. If true the first row in the file is a header row, not data. Default = ```true``` |
| trim_characters | ```std::vector<char>``` | specifies the list of characters to trim from every value in the CSV. Default = ```{}``` - nothing trimmed |
| ignore_columns | ```std::vector<std::string>``` | specifies the list of columns to ignore. These columns will be stripped during the parsing process. Default = ```{}``` - no column ignored |

## Trimming Characters

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

if (csv.parse("test.csv")) {
  for (auto& row : csv.rows()) {
    auto thread_id = row["Thread ID"];
    auto log_level = row["Log Level"];
    auto message = row["Log Message"];
    // do something
  }
}
```

## Ignoring Columns

Consider in the following CSV, you don't care about the column ```meh```.

```csv
foo, bar, meh, baz, fez
1, 2, 3, 4, 5
a, b, c, d, e
```

You can configure the dialect to ignore this column like so:

```cpp
csv::reader csv;
csv.configure_dialect("ignore meh")
  .delimiter(", ")
  .ignore_columns("meh");

if (csv.parse("test.csv")) {
  auto rows = csv.rows();
  // Your rows are:
  // [{"foo": "1", "bar": "2", "baz": "4"},
  //  {"foo": "a", "bar": "b", "baz": "d")]
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
