#ifndef ARGUMENT_H
#define ARGUMENT_H

#include <memory>

template <typename... Args>
class ArgumentImpl
{
protected:
    std::tuple<Args...> args;

public:
    ArgumentImpl(Args &&...args) : args(std::forward<Args>(args)...)
    {
    }
    virtual ~ArgumentImpl() = default;

    std::tuple<Args...> getArgs() const
    {
        return args;
    }
};

// template <typename... Args>
// class ArgumentPrivate : public ArgumentImpl<Args...>
// {

// public:
//     ArgumentPrivate(Args &&...args) : ArgumentImpl(std::forward<Args>(args)...)
//     {
//     }
// };

template <typename... Args>
class Argument
{
    std::unique_ptr<ArgumentImpl<Args...>> impl;

public:
    Argument(Args &&...args)
    {
        impl = std::make_unique<ArgumentImpl<Args...>>(std::forward<Args>(args)...);
    }

    std::tuple<Args...> getArgs() const
    {
        return impl.get()->getArgs();
    }
};

#endif // ARGUMENT_H