#ifndef NO_COPY_HPP
#define NO_COPY_HPP

/**
@brief 禁止拷贝。可通过继承禁止子类的拷贝行为
*/
class NonCopyable {
public:
  NonCopyable () = default;
  NonCopyable (NonCopyable  const &) = delete;
  NonCopyable  &operator=(NonCopyable  const &) = delete;
  NonCopyable (NonCopyable  &&) = default;
  NonCopyable  &operator=(NonCopyable  &&) = default;
};

/**
@brief 禁止移动。可通过继承禁止子类的移动行为
*/
class NonMovable {
public:
  NonMovable() = default;
  NonMovable(NonMovable const&) = default;
  NonMovable &operator=(NonMovable const &) = default;
  NonMovable(NonMovable &&) = delete;
  NonMovable &operator=(NonMovable &&) = delete;
};

/**
@brief 禁止拷贝和移动。可通过继承禁止子类的拷贝和移动行为
*/
class NonCopyableMovable : public NonCopyable, public NonMovable {
public:
  NonCopyableMovable() = default;
};

#endif