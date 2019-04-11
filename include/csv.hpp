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
    newline_("\r\n") {
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
    while (stream >> std::noskipws >> ch) {

      // Handle delimiter & line terminator
      auto cases = std::vector<std::string>{delimiter_, newline_};
      for (size_t i = 0; i < cases.size(); i++) {
        for (size_t j = 0; j < cases[i].size(); j++) {
          if (ch == cases[i][j]) {
            if (j + 1 == cases[i].size()) {
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
  std::thread thread_;
  std::promise<bool> done_promise_;
  std::future<bool> done_future_;
  moodycamel::ConcurrentQueue<std::string> values_;
};

}