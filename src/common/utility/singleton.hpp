#ifndef SINGLE_TON_H
#define SINGLE_TON_H
#include <mutex>
#include <memory>
#include "nocopy.hpp"
/**
 @brief 单例模板类
*/
template <typename T>
class Singleton : public NonCopyableMovable{
protected:
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;
    static std::shared_ptr<T> instance_;

public:
    static std::shared_ptr<T> getinstance() {
        static std::once_flag s_flag;  // 保证只执行一次
        std::call_once(s_flag, [&]() {
            // 构造函数是protected的，make_shared无法调用, 所以new
            instance_ = std::shared_ptr<T>(new T);
            });
        return instance_;
    }
    //void PrintAddress() {
    //    std::cout << instance_.get() << endl;
    //}

    ~Singleton() {
     //   std::cout << "this is singleton destruct" << '\n';
    }
};


template <typename T>
std::shared_ptr<T> Singleton<T>::instance_ { nullptr};
#endif