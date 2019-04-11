#include <queue.hpp>

class csv {
public:
  csv(const std::string& filename) :
    filename_(filename),
    delimiter_(","),
    newline_("\r\n")
    {}

private:
  std::string filename_;
  std::string delimiter_;
  std::string newline_;
  moodycamel::ConcurrentQueue<std::string> values_;
};