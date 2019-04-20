/*
 _______  _______  __   __
|      _||       ||  | |  |  Fast CSV Parser for Modern C++
|     |  |  _____||  |_|  |  http://github.com/p-ranav/csv
|     |  | |_____ |       |
|     |  |_____  ||       |
|     |_  _____| | |     |
|_______||_______|  |___|

Licensed under the MIT License <http://opensource.org/licenses/MIT>.
SPDX-License-Identifier: MIT
Copyright (c) 2019 Pranav Srinivas Kumar <pranav.srinivas.kumar@gmail.com>.

Permission is hereby  granted, free of charge, to any  person obtaining a copy
of this software and associated  documentation files (the "Software"), to deal
in the Software  without restriction, including without  limitation the rights
to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#pragma once
#include <dialect.hpp>
#include <concurrent_queue.hpp>
#include <robin_map.hpp>
#include <thread>
#include <iostream>
#include <fstream>
#include <string>
#include <future>
#include <chrono>

namespace csv {

class Writer {
public:
  explicit Writer(const std::string& file_name) : 
    header_written_(false),
    current_dialect_name_("excel") {
    file_stream.open(file_name);
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

    done_future = done_promise.get_future();
    thread = std::thread(&Writer::write_to_file, this, &done_future);
  }

  ~Writer() {
    thread.join();
  }

  Dialect& configure_dialect(const std::string& dialect_name = "excel") {
    if (dialects_.find(dialect_name) != dialects_.end()) {
      return *dialects_[dialect_name];
    }
    else {
      std::shared_ptr<Dialect> dialect_object = std::make_shared<Dialect>();
      dialects_[dialect_name] = dialect_object;
      current_dialect_name_ = dialect_name;
      return *dialect_object;
    }
  }

  std::vector<std::string> list_dialects() {
    std::vector<std::string> result;
    for (auto&kvpair : dialects_)
      result.push_back(kvpair.first);
    return result;
  }

  Dialect& get_dialect(const std::string& dialect_name) {
    return *(dialects_[dialect_name]);
  }

  void use_dialect(const std::string& dialect_name) {
    current_dialect_name_ = dialect_name;
    if (dialects_.find(dialect_name) == dialects_.end()) {
      throw std::runtime_error("error: Dialect " + dialect_name + " not found");
    }
  }

  void write_row() {
    if (!header_written_) {
      write_header();
      header_written_ = true;
    }
    std::string row;
    for (size_t i = 0; i < current_row_entries_.size(); i++) {
      row += current_row_entries_[i];
      if (i + 1 < current_row_entries_.size())
        row += dialects_[current_dialect_name_]->delimiter_;
    }
    row += dialects_[current_dialect_name_]->line_terminator_;
    queue.enqueue(row);
    current_row_entries_.clear();
  }

  // Parameter packed write_row method
  // Accepts a variadic number of entries
  template<typename T, typename... Targs>
  Dialect& write_row(T entry, Targs... Fargs) {
    current_row_entries_.push_back(entry);
    write_row(Fargs...);
  }

  void close() {
    done_promise.set_value(true);
  }

private:

  void write_header() {
    auto dialect = dialects_[current_dialect_name_];
    auto column_names = dialect->column_names_;
    if (column_names.size() == 0)
      return;
    auto delimiter = dialect->delimiter_;
    auto line_terminator = dialect->line_terminator_;
    std::string row;
    for (size_t i = 0; i < column_names.size(); i++) {
      row += column_names[i];
      if (i + 1 < column_names.size())
        row += delimiter;
    }
    row += line_terminator;
    queue.enqueue(row);
  }

  bool front(std::string& message) {
    return queue.try_dequeue(message);
  }

  void write_to_file(std::future<bool> * future_object) { 
    while(true) {
      std::string message;
      if (front(message)) {
          file_stream << message;
      }
      const auto future_status = future_object->wait_for(std::chrono::seconds(0));
      if (future_status == std::future_status::ready && queue.size_approx() == 0)
        break;
    }
    file_stream.close();
  }

  std::ofstream file_stream;
  std::thread thread;
  std::promise<bool> done_promise;
  std::future<bool> done_future;
  ConcurrentQueue<std::string> queue;
  std::string current_dialect_name_;
  robin_map<std::string, std::shared_ptr<Dialect>> dialects_;
  std::shared_ptr<Dialect> current_dialect_;
  std::vector<std::string> current_row_entries_;
  bool header_written_;
};

}