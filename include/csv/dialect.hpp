/*
Simplified BSD license:
Copyright (c) 2019, Pranav Srinivas Kumar
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice, this
list of conditions and the following disclaimer in the documentation and/or
other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#pragma once
#include <string>
#include <vector>

namespace csv {

struct Dialect {
  std::string delimiter_;
  bool skip_initial_space_;
  std::string line_terminator_;
  char quote_character_;
  bool double_quote_;
  std::vector<std::string> ignore_columns_;
  std::vector<char> trim_characters_;
  bool header_;

  Dialect() :
    delimiter_(","),
    skip_initial_space_(false),
#ifdef _WIN32
    line_terminator_("\n"),
#else
    line_terminator_("\r\n"),
#endif
    quote_character_('"'),
    double_quote_(true),
    trim_characters_({}),
    header_(true) {}

  Dialect& delimiter(const std::string& delimiter) {
    delimiter_ = delimiter;
    return *this;
  }

  Dialect& skip_initial_space(bool skip_initial_space) {
    skip_initial_space_ = skip_initial_space;
    return *this;
  }

  Dialect& line_terminator(const std::string& line_terminator) {
    line_terminator_ = line_terminator;
    return *this;
  }

  Dialect& quote_character(char quote_character) {
    quote_character_ = quote_character;
    return *this;
  }

  Dialect& double_quote(bool double_quote) {
    double_quote_ = double_quote;
    return *this;
  }

  // Base case for trim_characters parameter packing
  Dialect& trim_characters() {
    return *this;
  }

  // Parameter packed trim_characters method
  // Accepts a variadic number of characters
  template<typename T, typename... Targs>
  Dialect& trim_characters(T character, Targs... Fargs) {
    trim_characters_.push_back(character);
    trim_characters(Fargs...);
    return *this;
  }

  // Base case for ignore_columns parameter packing
  Dialect& ignore_columns() {
    return *this;
  }

  // Parameter packed trim_characters method
  // Accepts a variadic number of columns
  template<typename T, typename... Targs>
  Dialect& ignore_columns(T column, Targs... Fargs) {
    ignore_columns_.push_back(column);
    ignore_columns(Fargs...);
    return *this;
  }

  Dialect& header(bool header) {
    header_ = header;
    return *this;
  }
};

}