#ifndef SINGLETON_HPP
#define SINGLETON_HPP
#include "nocopy.hpp"
#include <memory>
#include <mutex>
template <typename T> class Singleton : public NoCopy {
public:
  static std::shared_ptr<T> get_instance() {
    static std::once_flag flag;
    std::call_once(flag, [&]() { instance_ = std::shared_ptr<T>(new T()); });
    return instance_;
  }

protected:
  Singleton() = default;
  static std::shared_ptr<T> instance_;
  ;
};

template <typename T> std::shared_ptr<T> Singleton<T>::instance_ = nullptr;
#endif