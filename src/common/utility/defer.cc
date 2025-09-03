
#include "defer.hpp"


Defer::Defer(std::function<void()> callback): callback_(std::move(callback)){
}

Defer::~Defer(){
    callback_();
}