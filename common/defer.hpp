#ifndef DEFER_HPP
#define DEFER_HPP
#include <functional>
class Defer {
public:
  Defer(std::function<void()> &&_func) : func_(std::move(_func)) {}
  ~Defer() { func_(); }

private:
  std::function<void()> func_;
};
#endif