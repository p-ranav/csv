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
        .quote_character('"')
        .double_quote(true)
        .header(true);
      dialects_["unix"] = unix_dialect;

      std::shared_ptr<Dialect> excel_dialect = std::make_shared<Dialect>();
      excel_dialect
        ->delimiter(",")
        .quote_character('"')
        .double_quote(true)
        .header(true);
      dialects_["excel"] = excel_dialect;

      std::shared_ptr<Dialect> excel_tab_dialect = std::make_shared<Dialect>();
      excel_tab_dialect
        ->delimiter("\t")
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

      // Under Linux, getline removes \n from the input stream. 
      // However, it does not remove the \r
      // Let's remove it
      if (first_line.size() > 0 && first_line[first_line.size() - 1] == '\r') {
        first_line.pop_back();
      }
      
      auto first_line_split = split(first_line, dialect);
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

      std::string row;
      while (std::getline(stream, row)) {
        if (row.size() > 0 && row[row.size() - 1] == '\r')
          row.pop_back();
        auto row_split = split(row, dialect);
        for (auto& value : row_split)
          values_.enqueue(value);
      }
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
            for (auto&[key, value] : dialect->ignore_columns_)
              row.erase(key);
            rows_.push_back(row);
          }
        }
        const auto future_status = future_object->wait_for(std::chrono::seconds(0));
        if (future_status == std::future_status::ready && values_.size_approx() == 0) {
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
    std::vector<std::string> split(const std::string& input_string, std::shared_ptr<Dialect> dialect) {
      
      std::shared_ptr<std::vector<std::string>> result = std::make_shared<std::vector<std::string>>();
      std::string sub_result = "";
      bool discard_delimiter = false;
      size_t quotes_encountered = 0;
      
      for (size_t i = 0; i < input_string.size(); ++i) {
                
        // Check if ch is the start of a delimiter sequence
        bool delimiter_detected = false;
        for (size_t j = 0; j < dialect->delimiter_.size(); ++j) {

          char ch = input_string[i];
          if (ch != dialect->delimiter_[j]) {
            delimiter_detected = false;
            break;
          }
          else {
            // ch *might* be the start of a delimiter sequence
            if (j + 1 == dialect->delimiter_.size()) {
              if (quotes_encountered % 2 == 0) {
                // Reached end of delimiter sequence without breaking
                // delimiter detected!
                delimiter_detected = true;
                result->push_back(trim(sub_result));
                sub_result = "";

                // If enabled, skip initial space right after delimiter
                if (i + 1 < input_string.size()) {
                  if (dialect->skip_initial_space_ && input_string[i + 1] == ' ') {
                    i = i + 1;
                  }
                }
                quotes_encountered = 0;
              }
              else {
                sub_result += input_string[i];
                i = i + 1;
                if (i == input_string.size()) break;
              }
            }
            else {
              // Keep looking
              i = i + 1;
              if (i == input_string.size()) break;
            }
          }
        }

        // base case
        if (!delimiter_detected)
          sub_result += input_string[i];

        if (input_string[i] == dialect->quote_character_)
          quotes_encountered += 1;
        if (input_string[i] == dialect->quote_character_ &&
          dialect->double_quote_ &&
          sub_result.size() >= 2 &&
          sub_result[sub_result.size() - 2] == input_string[i])
          quotes_encountered -= 1;
      }

      if (sub_result != "")
        result->push_back(trim(sub_result));

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