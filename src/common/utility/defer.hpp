#ifndef DEFER_HPP
#define DEFER_HPP
#include <functional>
#include "nocopy.hpp"
struct Defer : public NonCopyable{
  Defer(std::function<void()> callback);
  ~Defer();
 std::function<void()> callback_;
};

#endif