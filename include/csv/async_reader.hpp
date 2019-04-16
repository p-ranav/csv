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
#include <unordered_map>
#include <thread>
#include <future>
#include <mutex>
#include <condition_variable>
#include <iterator>

namespace csv {

  struct TestEOL {
    bool operator()(char c) {
      last = c;
      return last == '\n';
    }
    char last;
  };

  class AsyncReader {
  public:
    AsyncReader() :
      filename_(""),
      columns_(0),
      ready_(false),
      current_dialect_("excel"),
      reading_thread_started_(false),
      processing_thread_started_(false),
      total_number_of_rows_(0),
      row_iterator_index_(0),
      expected_number_of_rows_(0) {

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

    ~AsyncReader() {
      if (reading_thread_started_) reading_thread_.join();
      if (processing_thread_started_) processing_thread_.join();
    }

    bool done() {
      done_mutex_.lock();
      bool processing_thread_started = processing_thread_started_;
      done_mutex_.unlock();
      if (processing_thread_started) {
        size_mutex_.lock();
        bool result = (row_iterator_index_ == expected_number_of_rows_);
        size_mutex_.unlock();
        return result;
      }
      else return false;
    }

    bool has_next() {
      size_mutex_.lock();
      bool result = (row_iterator_index_ < expected_number_of_rows_
        && row_iterator_index_ < total_number_of_rows_);
      size_mutex_.unlock();
      return result;
    }

    std::unordered_map<std::string, std::string> next() {
      rows_mutex_.lock();
      auto row = rows_[row_iterator_index_];
      row_iterator_index_ += 1;
      rows_mutex_.unlock();
      return row;
    }

    void read_async(const std::string& filename) {
      filename_ = filename;
      stream_ = std::ifstream(filename_);
      // new lines will be skipped unless we stop it from happening:    
      stream_.unsetf(std::ios_base::skipws);
      std::string line;
      while (std::getline(stream_, line))
        ++expected_number_of_rows_;

      if (dialects_[current_dialect_]->header_)
        expected_number_of_rows_ -= 1;

      stream_.clear();
      stream_.seekg(0, std::ios::beg);

      reading_thread_started_ = true;
      reading_thread_ = std::thread(&AsyncReader::read_internal, this);
      return;
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

  private:
    bool front(std::string& value) {
      return values_.try_dequeue(value);
    }

    void read_internal() {
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      if (!dialect) {
        throw std::runtime_error("error: Dialect " + current_dialect_ + " not found");
      }

      // Get current position
      std::streamoff length = stream_.tellg();

      // Get first line and find headers by splitting on delimiters
      std::string first_line;
      getline(stream_, first_line);

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
        stream_.seekg(length, std::ios_base::beg);
      }

      columns_ = headers_.size();

      for (auto& header : headers_)
        current_row_[header] = "";
      for (auto&[key, value] : dialects_[current_dialect_]->ignore_columns_)
        current_row_.erase(key);

      // Start processing thread
      processing_thread_ = std::thread(&AsyncReader::process_values, this);
      done_mutex_.lock();
      processing_thread_started_ = true;
      done_mutex_.unlock();

      // Get lines one at a time, split on the delimiter and 
      // enqueue the split results into the values_ queue
      std::string row;
      while (std::getline(stream_, row)) {
        if (row.size() > 0 && row[row.size() - 1] == '\r')
          row.pop_back();
        auto row_split = split(row, dialect);
        for (auto& value : row_split)
          values_.enqueue(value);
      }
      stream_.close();
    }

    void process_values() {
      size_t index = 0;
      size_t cols = headers_.size();
      std::shared_ptr<Dialect> dialect = dialects_[current_dialect_];
      auto ignore_columns = dialect->ignore_columns_;
      std::string value;
      while (true) {
        size_mutex_.lock();
        if (total_number_of_rows_ == expected_number_of_rows_) {
          size_mutex_.unlock();
          break;
        }
        size_mutex_.unlock();
        if (front(value)) {
          size_t i = index % cols;
          auto column_name = headers_[i];
          if (ignore_columns.count(column_name) == 0)
            current_row_[column_name] = value;
          index += 1;
          if (index != 0 && index % cols == 0) {
            rows_mutex_.lock();
            rows_.push_back(current_row_);
            rows_mutex_.unlock();
            size_mutex_.lock();
            total_number_of_rows_ += 1;
            size_mutex_.unlock();
          }
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
    std::ifstream stream_;
    std::vector<std::string> headers_;
    std::unordered_map<std::string, std::string> current_row_;
    std::vector<std::unordered_map<std::string, std::string>> rows_;
    bool ready_;
    std::condition_variable ready_cv_;
    std::mutex done_mutex_;

    // Member variables to keep track of rows/cols
    size_t columns_;
    size_t expected_number_of_rows_;
    std::mutex entries_mutex_;
    std::mutex rows_mutex_;

    // Member variables to enable streaming
    size_t total_number_of_rows_;
    size_t row_iterator_index_;
    std::mutex size_mutex_;

    std::thread reading_thread_;
    bool reading_thread_started_;

    std::thread processing_thread_;
    bool processing_thread_started_;

    moodycamel::ConcurrentQueue<std::string> values_;
    std::string current_dialect_;
    std::unordered_map<std::string, std::shared_ptr<Dialect>> dialects_;
  };

}