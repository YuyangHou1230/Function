#include <iostream>
#include <functional>
#include <unordered_map>
#include <memory>
#include <tuple>
#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <cassert>

// --------------------------------------------------
// 核心 FunctionRegistry
// --------------------------------------------------
class FunctionRegistry {
    int currentId = 0;

    struct EntryBase {
        virtual ~EntryBase() = default;
        virtual std::type_index getReturnType() const = 0;
    };

    template <typename R, typename... Args>
    struct Entry : EntryBase {
        using FuncType = std::function<R(Args...)>;
        FuncType func;

        Entry(FuncType f) : func(std::move(f)) {}

        R invoke(Args&&... args) {
            return func(std::forward<Args>(args)...);
        }

        std::type_index getReturnType() const override {
            return std::type_index(typeid(R));
        }
    };

    template <typename... Args>
    struct Entry<void, Args...> : EntryBase {
        using FuncType = std::function<void(Args...)>;
        FuncType func;

        Entry(FuncType f) : func(std::move(f)) {}

        void invoke(Args&&... args) {
            func(std::forward<Args>(args)...);
        }

        std::type_index getReturnType() const override {
            return std::type_index(typeid(void));
        }
    };

    std::unordered_map<int, std::unique_ptr<EntryBase>> storage;

public:
    // 注册任意可调用对象（含 lambda）
    template <typename F>
    int registerFunction(F&& f) {
        using Traits = FunctionTraits<std::decay_t<F>>;
        using R = typename Traits::ReturnType;
        using ArgsTuple = typename Traits::ArgsTuple;
        constexpr size_t N = std::tuple_size<ArgsTuple>::value;

        return registerFunctionImpl<R>(std::forward<F>(f), ArgsTuple{});
    }

    // 注册成员函数
    template <typename C, typename R, typename... Args>
    int registerMember(C* obj, R(C::*method)(Args...)) {
        auto bound = [obj, method](Args... args) -> R {
            return (obj->*method)(std::forward<Args>(args)...);
        };
        return registerFunction<R, Args...>(std::move(bound));
    }

    // 调用函数
    template <typename R, typename... Args>
    R invoke(int id, Args&&... args) {
        auto* base = storage.at(id).get();
        auto* entry = dynamic_cast<Entry<R, Args...>*>(base);
        if (!entry) throw std::bad_cast();
        return entry->invoke(std::forward<Args>(args)...);
    }

    // void 返回值特化
    template <typename... Args>
    void invokeVoid(int id, Args&&... args) {
        auto* base = storage.at(id).get();
        auto* entry = dynamic_cast<Entry<void, Args...>*>(base);
        if (!entry) throw std::bad_cast();
        entry->invoke(std::forward<Args>(args)...);
    }

    // 获取返回类型
    std::type_index getReturnType(int id) const {
        return storage.at(id)->getReturnType();
    }

private:
    // 推导式注册实现（用于 lambda）
    template <typename R, typename F, typename... Args>
    int registerFunctionImpl(F&& f, std::tuple<Args...>) {
        int id = currentId++;
        storage[id] = std::make_unique<Entry<R, Args...>>(std::function<R(Args...)>(std::forward<F>(f)));
        return id;
    }

    // 显式注册函数
    template <typename R, typename... Args>
    int registerFunction(std::function<R(Args...)> f) {
        int id = currentId++;
        storage[id] = std::make_unique<Entry<R, Args...>>(std::move(f));
        return id;
    }

    // 用于提取函数类型 traits
    template <typename T>
    struct FunctionTraits;

    template <typename R, typename... Args>
    struct FunctionTraits<std::function<R(Args...)>> {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename R, typename... Args>
    struct FunctionTraits<R(*)(Args...)> {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename C, typename R, typename... Args>
    struct FunctionTraits<R(C::*)(Args...)> {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename F>
    struct FunctionTraits : FunctionTraits<decltype(&F::operator())> {}; // lambda
};
