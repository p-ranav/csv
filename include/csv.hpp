#include <queue.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>
#include <thread>
#include <future>
#include <chrono>
#include <mutex>
#include <condition_variable>

namespace csv {

// Upsert into std::map
template <class KeyType, class ElementType>
bool upsert(std::map<KeyType, ElementType>& aMap, KeyType const& aKey, ElementType const& aNewValue) {
  typedef typename std::map<KeyType, ElementType>::iterator Iterator;
  typedef typename std::pair<Iterator, bool> Result;
  Result tResult = aMap.insert(typename std::map<KeyType, ElementType>::value_type(aKey, aNewValue));
  if (!tResult.second) {
    if (!(tResult.first->second == aNewValue)) {
      tResult.first->second = aNewValue;
      return true;
    }
    else
      return false; // it was the same
  }
  else
    return true;  // changed cause not existing
}

class reader {
public:
  reader(const std::string& filename) :
    filename_(filename),
    delimiter_(","),
    newline_("\r\n"),
    columns_(0),
    ready_(false) {
    done_future_ = done_promise_.get_future();
    thread_ = std::thread(&reader::process_values, this, &done_future_);
    read_file();
    done();
    std::unique_lock<std::mutex> lock(ready_mutex_);
    while (!ready_) ready_cv_.wait(lock);
  }

  ~reader() {
    thread_.join();
  }

  size_t rows() {
    return rows_.size();
  }

  std::vector<std::map<std::string, std::string>> dict() {
    return rows_;
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
    size_t header_index = 0;
    std::map<std::string, std::string> row;
    while(true) {
      std::string value;
      if (front(value)) {
        if (header_index < columns_) {
          headers_.push_back(value);
          header_index += 1;
        }
        else {
          upsert<std::string, std::string>(row, 
            headers_[header_index % headers_.size()], value);
          header_index += 1;
          if (header_index % headers_.size() == 0) {
            rows_.push_back(row);
            row.clear();
          }
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

  std::string filename_;
  std::string delimiter_;
  std::string newline_;
  size_t columns_;
  std::vector<std::string> headers_;
  std::vector<std::map<std::string, std::string>> rows_;
  bool ready_;
  std::condition_variable ready_cv_;
  std::mutex ready_mutex_;
  std::thread thread_;
  std::promise<bool> done_promise_;
  std::future<bool> done_future_;
  moodycamel::ConcurrentQueue<std::string> values_;
};

}