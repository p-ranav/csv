# CSV for Modern C++

## Highlights

* Header-only library to read/write CSV files
* Requires C++17
* BSD 2-Clause "Simplified" License

## Table of Contents

- [Reading CSV files](#reading-csv-files)
  * [Standard Dialects](#standard-dialects)
  * [Configuring Custom Dialects](#configuring-custom-dialects)
  * [Trimming Characters](#trimming-characters)
  * [Ignoring Columns](#ignoring-columns)
  * [Filtering Rows](#filtering-rows)

## Reading CSV files

To parse CSV files, simply include ```<csv/reader.hpp>``` and configure a ```csv::Reader``` like so:

```cpp
#include <csv/reader.hpp>

int main() {
  csv::Reader csv;
  if (csv.read("test.csv")) {         // reads a CSV file and builds a list of dictionaries
    for (auto& row : csv.rows()) {    // test.csv => [{"foo": "1", "bar": "2"}, {"foo": "3", "bar": "4"}, ...] 
      auto foo = row["foo"];
      // do something
    }
  }
}
```

### Standard Dialects

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
  .header(true)

if (csv.read("foo.csv") {
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

### Trimming Characters

Consider this strange, messed up log file: 

```csv
[Thread ID] :: [Log Level] :: [Log Message] :: {Timestamp}
04 :: INFO :: Hello World ::             1555164718
02        :: DEBUG :: Warning! Foo has happened                :: 1555463132
```

To parse this file, configure a new dialect that splits on "::" and trims whitespace, braces, and bracket characters.

```cpp
csv::reader csv;
csv.configure_dialect("my strange dialect")
  .delimiter("::")
  .trim_characters(' ', '[', ']', '{', '}');   

if (csv.read("test.csv")) {
  for (auto& row : csv.rows()) {
    auto thread_id = row["Thread ID"];
    auto log_level = row["Log Level"];
    auto message = row["Log Message"];
    // do something
  }
}
```

### Ignoring Columns

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

if (csv.read("test.csv")) {
  auto rows = csv.rows();
  // Your rows are:
  // [{"name": "Mark Johnson", "email": "mark.johnson@gmail.com", "department": "BA"},
  //  {"name": "John Stevenson", "email": "john.stevenson@gmail.com", "department": "IT"},
  //  {"name": "Jane Barkley", "email": "jane.barkley@gmail.com", "department": "MGT"}]
}  
```

### Filtering Rows

Once parsed, you can filter the CSV and obtain a list of rows that meet some condition using ```.filter(...)```

```csv
first_name, last_name, grade
Alex, Brian, A
Tom, Smith, F
John, Doe, B
Jane, Dee, F,
Tony, Hawk, A,
```

Let's say you want a shortlist of all students who received an 'F' grade. 

```cpp
csv::reader csv;
csv.configure_dialect("student records")
  .skip_initial_space(true);

if (csv.read("test.csv")) {
  auto rows = csv.filter([](auto rows) {
    return rows["grade"] == "F";
  });
  // Your rows are:
  // [{"first_name": "Tom", "last_name": "Smith", "grade": "F"},
  //  {"first_name": "Jane", "last_name": "Doe", "grade": "F"}]
}  
```

NOTE: Filtering does not mutate the original list of rows. ```csv.rows()``` will still return all the original rows. This call simply returns a copy with the filtered results. 

## Supported Compilers
* GCC >= 7.0.0
* Clang >= 4.0
* MSVC >= 2017

## Contributing
Contributions are welcomed, have a look at the [CONTRIBUTING.md](CONTRIBUTING.md) document for more information.

## License
This project is available under the [BSD-2-Clause](https://opensource.org/licenses/BSD-2-Clause) license.
