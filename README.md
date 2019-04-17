# CSV Parser for Modern C++

## Highlights

* Header-only library
* Fast, asynchronous, multi-threaded processing using:
  - [Lock-free Concurrent Queues](https://github.com/cameron314/concurrentqueue)
  - [Robin hood Hashing](https://github.com/Tessil/robin-map)
* Requires C++11
* BSD 2-Clause "Simplified" License

## Quick Start

Simply include reader.hpp and you're good to go.

```cpp
#include <reader.hpp>
```
To start parsing CSV files, create a ```csv::Reader``` object and call  ```.read(filename)```. 

```cpp
csv::Reader foo;
foo.read("test.csv");
```

This ```.read``` method is non-blocking. The reader spawns multiple threads to tokenize the file stream and build a "list of dictionaries". While the reader is doing it's thing, you can start post-processing the rows it has parsed so far. 

```cpp
while(foo.busy()) {
  if (foo.has_row()) {
    auto row = foo.next_row();    // Each row is a tsl::robin_map (https://github.com/Tessil/robin-map)
    auto foo = row["foo"]         // You can use it just like an std::unordered_map
    auto bar = row["bar"];
    // do something
  }
}
```

If instead you'd like to wait for all the rows to get processed, you can call ```.rows()``` which is a convenience method that executes the above while loop

```cpp
auto rows = foo.rows();           // blocks until the CSV is fully processed
for (auto& row : rows) {          // Example: [{"foo": "1", "bar": "2"}, {"foo": "3", "bar": "4"}, ...] 
  auto foo = row["foo"];
  // do something
}
```

## Dialects

This csv library comes with three standard dialects:

 | Name | Description |	
|-----------|---------------------------------------------------------------------------------------------------------------------------------------------------|	
| excel | The excel dialect defines the usual properties of an Excel-generated CSV file |	
| excel_tab | The excel_tab dialect defines the usual properties of an Excel-generated TAB-delimited file |	
| unix | The unix dialect defines the usual properties of a CSV file generated on UNIX systems, i.e. using  '\n' as line terminator and quoting all fields |	

### Configuring Custom Dialects

Custom dialects can be constructed with ```.configure_dialect(...)```

```cpp
csv::Reader csv;
csv.configure_dialect("my fancy dialect")
  .delimiter("")
  .quote_character('"')
  .double_quote(true)
  .skip_initial_space(false)
  .trim_characters(' ', '\t')    // parameter packed
  .ignore_columns("foo", "bar")  // parameter packed
  .header(true);

csv.read("foo.csv");
for (auto& row : csv.rows()) {
  // do something
}
```

| Property | Data Type | Description |
|--------------------|-------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| delimiter | ```std::string``` | specifies the character sequence which should separate fields (aka columns). Default = ```","``` |
| quote_character | ```char``` | specifies a one-character string to use as the quoting character. Default = ```'"'``` |
| double_quote | ```bool``` | controls the handling of quotes inside fields. If true, two consecutive quotes should be interpreted as one. Default = ```true``` |
| skip_initial_space | ```bool``` | specifies how to interpret whitespace which immediately follows a delimiter; if false, it means that whitespace immediately after a delimiter should be treated as part of the following field. Default = ```false``` |
| trim_characters | ```std::vector<char>``` | specifies the list of characters to trim from every value in the CSV. Default = ```{}``` - nothing trimmed |
| ignore_columns | ```std::vector<std::string>``` | specifies the list of columns to ignore. These columns will be stripped during the parsing process. Default = ```{}``` - no column ignored |
| header | ```bool``` | indicates whether the file includes a header row. If true the first row in the file is a header row, not data. Default = ```true``` |

The line terminator is ```'\n'``` by default. I use std::getline and handle stripping out ```'\r'``` from line endings. So, for now, this is not configurable in custom dialects. 

## Multi-character Delimiters

Consider this strange, messed up log file: 

```csv
[Thread ID] :: [Log Level] :: [Log Message] :: {Timestamp}
04 :: INFO :: Hello World ::             1555164718
02        :: DEBUG :: Warning! Foo has happened                :: 1555463132
```

To parse this file, simply configure a new dialect that splits on "::" and trims whitespace, braces, and bracket characters.

```cpp
csv::reader csv;
csv.configure_dialect("my strange dialect")
  .delimiter("::")
  .trim_characters(' ', '[', ']', '{', '}');   

csv.read("test.csv");
for (auto& row : csv.rows()) {
  auto thread_id = row["Thread ID"];    // "04"
  auto log_level = row["Log Level"];    // "INFO"
  auto message = row["Log Message"];    // "Hello World"
  // do something
}
```

## Ignoring Columns

Consider the following CSV. Let's say you don't care about the columns ```age``` and ```gender```. Here, you can use ```.ignore_columns``` and provide a list of columns to ignore. 

```csv
name, age, gender, email, department
Mark Johnson, 50, M, mark.johnson@gmail.com, BA
John Stevenson, 35, M, john.stevenson@gmail.com, IT
Jane Barkley, 25, F, jane.barkley@gmail.com, MGT
```

You can configure the dialect to ignore these columns like so:

```cpp
csv::reader csv;
csv.configure_dialect("ignore meh and fez")
  .delimiter(", ")
  .ignore_columns("age", "gender");

csv.read("test.csv");
auto rows = csv.rows();
// Your rows are:
// [{"name": "Mark Johnson", "email": "mark.johnson@gmail.com", "department": "BA"},
//  {"name": "John Stevenson", "email": "john.stevenson@gmail.com", "department": "IT"},
//  {"name": "Jane Barkley", "email": "jane.barkley@gmail.com", "department": "MGT"}]
```

## No Header?

Sometimes you have CSV files with no header row:

```csv
9 52 1
52 91 0
91 135 0
135 174 0
174 218 0
218 260 0
260 301 0
301 341 0
341 383 0
...
```

If you want to prevent the reader from parsing the first row as a header, simply:

* Set ```.header``` to false
* Provide a list of column names with ```.column_names(...)```

```cpp
csv.configure_dialect("no headers")
  .header(false)
  .column_names("foo", "bar", "baz");
```

The CSV rows will now look like this:

```cpp
[{"foo": "9", "bar": "52", "baz": "1"}, {"foo": "52", "bar": "91", "baz": "0"}, ...]
```

If ```.column_names``` is not called, then the reader simply generates dictionary keys like so:

```cpp
[{"0": "9", "1": "52", "2": "1"}, {"0": "52", "1": "91", "2": "0"}, ...]
```

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
This project is available under the [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause) license.
