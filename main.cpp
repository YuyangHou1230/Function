#include <iostream>
#include <functional>
#include <memory>
#include <utility>
#include <map>

#include <unordered_map>
#include <typeindex>

template <typename T>
struct FunctionTraits;

// 普通函数类型
template <typename R, typename... Args>
struct FunctionTraits<R(Args...)>
{
    using ReturnType = R;
    using FunctionType = R(Args...);
    static constexpr size_t Arity = sizeof...(Args);
};

// 函数指针
template <typename R, typename... Args>
struct FunctionTraits<R (*)(Args...)> : FunctionTraits<R(Args...)>
{
};

// std::function
template <typename R, typename... Args>
struct FunctionTraits<std::function<R(Args...)>> : FunctionTraits<R(Args...)>
{
};

// 成员函数（非 const）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...)> : FunctionTraits<R(Args...)>
{
};

// 成员函数（const）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const> : FunctionTraits<R(Args...)>
{
};

// 成员函数（volatile）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) volatile> : FunctionTraits<R(Args...)>
{
};

// 成员函数（const volatile）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const volatile> : FunctionTraits<R(Args...)>
{
};

// 成员函数（带 ref-qualifier 的 const &）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const &> : FunctionTraits<R(Args...)>
{
};

// 成员函数（带 ref-qualifier 的 const &&）
template <typename C, typename R, typename... Args>
struct FunctionTraits<R (C::*)(Args...) const &&> : FunctionTraits<R(Args...)>
{
};

// Lambda、函数对象：调用 operator()
template <typename T>
struct FunctionTraits : FunctionTraits<decltype(&T::operator())>
{
};

template <typename R, typename... Args>
class EntryBase
{
public:
    virtual ~EntryBase() = default;
    virtual R invoke(Args &&...args) = 0;
};

template <typename F, typename R, typename... Args>
class Entry : public EntryBase<R, Args...>
{
    F func;

public:
    Entry(F &&f) : func(std::move(f)) {}
    R invoke(Args &&...args) override
    {
        return func(std::forward<Args>(args)...);
    }
};

template <typename T>
class MyEntry;

class FunctionBase
{
public:
    virtual ~FunctionBase() = default;
};

template <typename R, typename... Args>
class MyEntry<R(Args...)> : public FunctionBase
{
    std::unique_ptr<EntryBase<R, Args...>> base;

public:
    template <typename F>
    MyEntry(F &&f) : base(std::make_unique<Entry<F, R, Args...>>(std::forward<F>(f))) {}

    R operator()(Args &&...args) const
    {
        return base->invoke(std::forward<Args>(args)...);
    }
};

class FunctionRegistry
{
public:
    // template <typename T, typename R, typename... Args>
    // void registerFunction(int id, T &&f) {
    //     storage[id] = std::make_unique<MyEntry<R(Args...)>>(std::forward<T>(f));
    // }

    // template <typename R, typename... Args>
    // void registerFunction(int id, MyEntry<R(Args...)> &&f)
    // {
    //     storage[id] = std::make_unique<MyEntry<R(Args...)>>(std::forward<MyEntry<R(Args...)>>(f));
    // }

    template <typename F>
    void registerFunction(int id, F &&f)
    {
        using Traits = FunctionTraits<F>;
        using Signature = typename Traits::FunctionType;
        using R = typename Traits::ReturnType;
        storage[id] = std::make_unique<MyEntry<Signature>>(std::forward<F>(f));
        // types[id] = std::type_index(typeid(R));
    }

    // 非 const 成员函数注册
    template <typename R, typename C, typename... Args>
    void registerFunction(int id, C *obj, R (C::*func)(Args...))
    {
        // 用 std::function 封装成员函数
        auto wrapper = [obj, func](Args &&...args) -> R
        {
            return (obj->*func)(std::forward<Args>(args)...);
        };

        using FuncType = R (C::*)(Args...);
        using Traits = FunctionTraits<FuncType>;
        using Signature = typename Traits::FunctionType;
        using WrapperType = std::function<R(Args...)>;
        storage[id] = std::make_unique<MyEntry<Signature>>(std::move(wrapper));
    }

    template <typename R, typename... Args>
    R invoke(int id, Args...args)
    {
        // auto type = types.at(id);
        // if (types[id] != std::type_index(typeid(R)))
        //     throw std::runtime_error("Type mismatch");
        auto *base = storage.at(id).get();
        auto *entry = dynamic_cast<MyEntry<R(Args...)> *>(base);
        if (!entry)
            throw std::bad_cast();
        return (*entry)(std::forward<Args>(args)...);
    }

    std::unordered_map<int, std::unique_ptr<FunctionBase>> storage;
    std::map<int, std::type_index> types;
};

int add(int a, int b)
{
    return a + b;
}
int add2(int a, int b, int c)
{
    return a + b + c;
}


struct Param
{
    int a;
    int b;
    int c;
};
class Foo
{
public:
    int add(int a, int b)
    {
        return a + b;
    }
    double add2(int a, int b, int c)
    {
        return double(a + b + c);
    }

    int add3(int a, double b, int c)
    {
        return double(a + b + c);
    }
    
    std::string add4(std::string a)
    {
        return a + " world";
    }

    int add5(Param p)
    {
        return p.a * p.b * p.c;
    }
};

int main()
{
    FunctionRegistry registry;

    // 普通函数注册
    registry.registerFunction(1, &add2);

    // 成员函数注册
    Foo foo;
    registry.registerFunction(2, &foo, &Foo::add);
    registry.registerFunction(3, &foo, &Foo::add2);
    registry.registerFunction(4, &foo, &Foo::add3);
    registry.registerFunction(5, &foo, &Foo::add4);
    registry.registerFunction(6, &foo, &Foo::add5);

    int result1 = registry.invoke<int>(1, 3, 8, 6);
    std::cout << "Result1: " << result1 << std::endl;

    int result2 = registry.invoke<int>(2, 8, 6);
    std::cout << "Resulte:" << result2 << std::endl;

    double result3 = registry.invoke<double>(3, 8, 6, 7);
    std::cout << "Result3:" << result3 << std::endl;

    int result4 = registry.invoke<int>(4, 8, 6.8, 7);
    std::cout << "Result4:" << result4 << std::endl;

    std::string result5 = registry.invoke<std::string>(5, std::string("hello"));
    std::cout << "Result5:" << result5 << std::endl;

    Param p{2, 3, 4};   
    int result6 = registry.invoke<int>(6, p);
    std::cout << "Result6:" << result6 << std::endl;

    return 0;
}
