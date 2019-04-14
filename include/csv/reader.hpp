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
#include <csv/dialect.hpp>
#include <csv/queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace csv {

  class Reader {
  public:
    Reader() :
      filename_(""),
      columns_(0),
      ready_(false),
      current_dialect_("excel"),
      thread_started_(false) {

      std::shared_ptr<Dialect> unix_dialect = std::make_shared<Dialect>();
      unix_dialect
        ->delimiter(",")
        .line_terminator("\n")
        .quote_character('"')
        .double_quote(true)
        .header(true);
      dialects_["unix"] = unix_dialect;

      std::shared_ptr<Dialect> excel_dialect = std::make_shared<Dialect>();
      excel_dialect
        ->delimiter(",")
#ifdef _WIN32
        .line_terminator("\n")
#else
        .line_terminator("\r\n")
#endif
        .quote_character('"')
        .double_quote(true)
        .header(true);
      dialects_["excel"] = excel_dialect;

      std::shared_ptr<Dialect> excel_tab_dialect = std::make_shared<Dialect>();
      excel_tab_dialect
        ->delimiter("\t")
#ifdef _WIN32
        .line_terminator("\n")
#else
        .line_terminator("\r\n")
#endif
        .quote_character('"')
        .double_quote(true)
        .header(true);
      dialects_["excel_tab"] = excel_tab_dialect;
    }

    ~Reader() {
      if (thread_started_) thread_.join();
    }

    bool read(const std::string& filename) {
      filename_ = filename;
      read_internal();
      done();
      std::unique_lock<std::mutex> lock(ready_mutex_);
      while (!ready_) ready_cv_.wait(lock);
      return true;
    }

    Dialect& configure_dialect(const std::string& dialect_name) {
      if (dialects_.find(dialect_name) != dialects_.end()) {
        return *dialects_[dialect_name];
      }
      else {
        std::shared_ptr<Dialect> dialect_object = std::make_shared<Dialect>();
        dialects_[dialect_name] = dialect_object;
        current_dialect_ = dialect_name;
        return *dialect_object;
      }
    }

    std::vector<std::string> list_dialects() {
      std::vector<std::string> result;
      for (auto&[key, value] : dialects_)
        result.push_back(key);
      return result;
    }

    Dialect& get_dialect(const std::string& dialect_name) {
      return *(dialects_[dialect_name]);
    }

    void use_dialect(const std::string& dialect_name) {
      current_dialect_ = dialect_name;
      if (dialects_.find(dialect_name) == dialects_.end()) {
        throw std::runtime_error("error: Dialect " + dialect_name + " not found");
      }
    }

    std::vector<std::map<std::string, std::string>> rows() {
      return rows_;
    }

    std::vector<std::string> cols() {
      return headers_;
    }

    std::vector<std::map<std::string, std::string>>
      filter(std::function<bool(std::map<std::string, std::string>)> filter_function) {
      std::vector<std::map<std::string, std::string>> result;
      std::copy_if(rows_.begin(), rows_.end(), std::back_inserter(result), filter_function);
      return result;
    }

  private:
    bool front(std::string& value) {
      return values_.try_dequeue(value);
    }

    void done() {
      done_promise_.set_value(true);
    }

    void read_internal() {
      std::fstream stream(filename_, std::fstream::in);
      stream.flags(stream.flags() & ~std::ios_base::skipws);
      if (!stream.is_open()) {
        throw std::runtime_error("error: Failed to open " + filename_);
      }

      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      if (!dialect) {
        throw std::runtime_error("error: Dialect " + current_dialect_ + " not found");
      }

      // Get current position
      std::streamoff length = stream.tellg();

      // Get first line and find headers by splitting on delimiters
      std::string first_line;
      getline(stream, first_line);
      auto first_line_split = split(first_line, dialect->delimiter_, 
        dialect->quote_character_, dialect->double_quote_, dialect);
      if (dialect->header_) {
        headers_ = first_line_split;
      }
      else {
        headers_.clear();
        for (size_t i = 0; i < first_line_split.size(); i++)
          headers_.push_back(std::to_string(i));
        // return to start before getline()
        stream.seekg(length, std::ios_base::beg);
      }

      // Start processing thread
      done_future_ = done_promise_.get_future();
      thread_ = std::thread(&Reader::process_values, this, &done_future_);
      thread_started_ = true;

      char ch;
      std::string current;
      size_t quotes_encountered = 0;

      while (stream >> std::noskipws >> ch) {
        // Handle delimiter
        std::string delimiter_substring = "";
        for (size_t i = 0; i < dialect->delimiter_.size(); i++) {
          if (ch == dialect->delimiter_[i]) {
            delimiter_substring += ch;
            if (i + 1 == dialect->delimiter_.size()) {
              // Make sure that an even number of quotes have been 
              // encountered so far
              // If not, then don't consider the delimiter
              if (quotes_encountered % 2 == 0) {
                values_.enqueue(trim(current));
                current = "";
                stream >> std::noskipws >> ch;
                if (ch == ' ' && dialect->skip_initial_space_) {
                  stream >> std::noskipws >> ch;
                }
                quotes_encountered = 0;
              }
              else {
                current += ch;
                stream >> std::noskipws >> ch;
              }
            }
            else {
              if (quotes_encountered % 2 != 0) {
                current += ch;
              }
              stream >> std::noskipws >> ch;
            }
          }
          else {
            current += delimiter_substring;
            break;
          }
        }

        // Handle line_terminator
        std::string line_terminator_substring = "";
        for (size_t i = 0; i < dialect->line_terminator_.size(); i++) {
          if (ch == dialect->line_terminator_[i]) {
            line_terminator_substring += ch;
            if (i + 1 == dialect->line_terminator_.size()) {
              values_.enqueue(trim(current));
              current = "";
              stream >> std::noskipws >> ch;
            }
            else {
              stream >> std::noskipws >> ch;
            }
          }
          else {
            current += line_terminator_substring;
            break;
          }
        }

        // Base case
        current += ch;
        if (ch == dialect->quote_character_)
          quotes_encountered += 1;
        if (ch == dialect->quote_character_ &&
          dialect->double_quote_ &&
          current.size() >= 2 &&
          current[current.size() - 2] == ch)
          quotes_encountered -= 1;
      }
      if (current != "")
        values_.enqueue(trim(current));
    }

    void process_values(std::future<bool> * future_object) {
      size_t index = 0;
      size_t cols = headers_.size();
      std::map<std::string, std::string> row;
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      while (true) {
        std::string value;
        if (front(value)) {
          size_t i = index % cols;
          auto column_name = headers_[i];
          row[column_name] = value;
          index += 1;
          if (index != 0 && index % cols == 0) {
            for (auto&[key, value] : dialect->ignore_columns_) {
              row.erase(key);
            }
            rows_.push_back(row);
          }
        }
        const auto future_status =
          future_object->wait_for(std::chrono::seconds(0));
        if (future_status == std::future_status::ready &&
          values_.size_approx() == 0) {
          std::unique_lock<std::mutex> lock(ready_mutex_);
          ready_ = true;
          ready_cv_.notify_all();
          break;
        }
      }
    }

    // trim white spaces from the left end of an input string
    std::string ltrim(std::string input) {
      std::string result = input;
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      result.erase(result.begin(), std::find_if(result.begin(), result.end(), [=](int ch) {
        return !(std::find(dialect->trim_characters_.begin(), dialect->trim_characters_.end(), ch)
          != dialect->trim_characters_.end());
      }));
      return result;
    }

    // trim white spaces from right end of an input string
    std::string rtrim(std::string input) {
      std::string result = input;
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      result.erase(std::find_if(result.rbegin(), result.rend(), [=](int ch) {
        return !(std::find(dialect->trim_characters_.begin(), dialect->trim_characters_.end(), ch)
          != dialect->trim_characters_.end());
      }).base(), result.end());
      return result;
    }

    // trim white spaces from either end of an input string
    std::string trim(std::string input) {
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      if (dialect->trim_characters_.size() == 0)
        return input;
      return ltrim(rtrim(input));
    }

    // split string based on a delimiter string
    // supports multi-character delimiter
    // returns a vector of substrings after split
    std::vector<std::string>
      split(const std::string& input_string, const std::string& delimiter,
        char quote_character, bool double_quote, std::shared_ptr<Dialect> dialect) {
      // Initialize result
      std::shared_ptr<std::vector<std::string>> result = std::make_shared<std::vector<std::string>>();
      std::string substring = "";

      size_t num_quotes = 0;
      for (size_t i = 0; i < input_string.size(); i++) {
        char current = input_string[i];
        if (current == quote_character) {
          num_quotes += 1;
          if (double_quote) {
            if (i > 0 && input_string[i - 1] == quote_character) {
              num_quotes -= 1;
            }
          }
        }

        // num_quotes -> number of quotes encountered SO FAR
        // If this is even, the quotes are closed - any delimiter is legit
        // If this is odd, disregard any delimiter encountered

        for (size_t j = 0; j < delimiter.size(); j++) {

          if (current == delimiter[j]) {
            if (j + 1 == delimiter.size()) {
              if (num_quotes % 2 == 0) {
                result->push_back(trim(substring));
                substring = "";
                num_quotes = 0;
                if (i + 1 < input_string.size()) {
                  if (input_string[i + 1] == ' ' && dialect->skip_initial_space_) {
                    i = i + 1;
                    current = input_string[i];
                  }
                }
              }
              else {
                // num_quotes is odd
                // save current
                substring += current;
              }
            }
            else {
              if (i + 1 < input_string.size()) {
                i = i + 1;
                current = input_string[i];
              }
              else {
                if (substring != "")
                  result->push_back(trim(substring));
              }
            }
          }
          else {
            substring += current;
            break; // current isn't a delimiter
          }

        }
      }

      if (substring != "")
        result->push_back(trim(substring));

      return *result;
    }

    std::string filename_;
    size_t columns_;
    std::vector<std::string> headers_;
    std::vector<std::map<std::string, std::string>> rows_;
    bool ready_;
    std::condition_variable ready_cv_;
    std::mutex ready_mutex_;
    std::thread thread_;
    bool thread_started_;
    std::promise<bool> done_promise_;
    std::future<bool> done_future_;
    moodycamel::ConcurrentQueue<std::string> values_;
    std::string current_dialect_;
    std::map<std::string, std::shared_ptr<Dialect>> dialects_;
  };

}