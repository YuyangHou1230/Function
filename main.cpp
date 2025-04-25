#include <iostream>

#include "function.h"
#include "param.h"

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


    // Argument<int, int> arg1(1, 2);
    // result2 = registry.invoke<int>(2, arg1);

    Param p1{2, 3, 4};

    

    return 0;
}
