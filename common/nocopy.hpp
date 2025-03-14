#ifndef NO_COPY_HPP
#define NO_COPY_HPP
class NoCopy
{
public:
    NoCopy() = default;
    NoCopy(NoCopy const& _other) = delete;
    NoCopy& operator=(NoCopy const& _other) = delete;
};
#endif