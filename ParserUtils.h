#ifndef MM_PARSER_UTILS
#define MM_PARSER_UTILS
#include <vector>
#include <string>
#include <functional>
namespace mm {
struct Stream {
  Stream(const char* p, int i, int max) : ptr(p), index(i), bounds(max) {}
  const char *ptr;
  int index;
  int bounds;
  char get(bool ignoreWhiteSpace = true) {
    if (index >= bounds)
      return '\0';
    if (std::isspace(ptr[index]) && ignoreWhiteSpace) {
      index++;
      return get(ignoreWhiteSpace);
    }
    return ptr[index++];
  }
  std::string loop(std::function<bool(char)> Pred) {
    skipWhiteSpace();
    std::string Result;
    while (Pred(ptr[index])) {
      Result += ptr[index];
      index++;
    }
    return Result;
  }
  bool fixed(std::string Str) {
    skipWhiteSpace();
    bool failed = false;
    for (unsigned int i = 0; i < Str.length(); ++i) {
      if (Str[i] != ptr[index+i])
        failed = true;
    }
    if (!failed) {
      index += Str.length();
      return true;
    }
    else return false;
  }
  std::string fixed(std::vector<std::string> Choices) {
    for (auto Choice : Choices) {
      if (fixed(Choice)) {
        return Choice;
      }
    }
    return "";
  }
  void skipWhiteSpace() {
    while (index < bounds && std::isspace(ptr[index]))
      index++;
  }
};
struct Result {
  Result(void *Ptr, Stream Str, std::string Err = "") : Ptr(Ptr), Str(Str), Err(Err) {}
  template <typename T>
  T *getAs() {return static_cast<T*>(Ptr);}
  void *Ptr;
  Stream Str;
  std::string Err;
  
};
typedef std::function<Result(Stream)> Parser;

}
#endif
