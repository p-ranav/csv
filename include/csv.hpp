#include <queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <future>
#include <chrono>

namespace csv {

class reader {
public:
  reader(const std::string& filename) :
    filename_(filename),
    delimiter_(","),
    newline_("\r\n"),
    columns_(0) {
    done_future_ = done_promise_.get_future();
    thread_ = std::thread(&reader::process_values, this, &done_future_);
    read_file();
    done();
  }

  ~reader() {
    thread_.join();
  }

private:
  bool front(std::string& value) {
    return values_.try_dequeue(value);
  }

  void done() {
    done_promise_.set_value(true);
  }

  void read_file() {
    char ch;
    std::fstream stream(filename_, std::fstream::in);
    std::string current;
    bool first_row = true;
    while (stream >> std::noskipws >> ch) {
      // Handle delimiter
      for (size_t i = 0; i < delimiter_.size(); i++) {
        if (ch == delimiter_[i]) {
          if (i + 1 == delimiter_.size()) {
            if (first_row) columns_ += 1;
            values_.enqueue(current);
            current = "";
            stream >> std::noskipws >> ch;
          }
          else {
            stream >> std::noskipws >> ch;
          }
        } 
        else { 
          break;
        }
      }
      // Handle newline
      for (size_t i = 0; i < newline_.size(); i++) {
        if (ch == newline_[i]) {
          if (i + 1 == newline_.size()) {
            if (first_row) columns_ += 1;
            values_.enqueue(current);
            current = "";
            stream >> std::noskipws >> ch;
            if (first_row) first_row = false;
          }
          else {
            stream >> std::noskipws >> ch;
          }
        } 
        else { 
          break;
        }
      }
      // Base case
      current += ch;
    }
  }

  void process_values(std::future<bool> * future_object) {
    while(true) {
      std::string value;
      if (front(value)) {
        // std::cout << value << std::endl;
      }
      const auto future_status = future_object->wait_for(std::chrono::seconds(0));
      if (future_status == std::future_status::ready && values_.size_approx() == 0)
        break;
    }
  }

  std::string filename_;
  std::string delimiter_;
  std::string newline_;
  size_t columns_;
  std::thread thread_;
  std::promise<bool> done_promise_;
  std::future<bool> done_future_;
  moodycamel::ConcurrentQueue<std::string> values_;
};

}