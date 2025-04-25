#ifndef PARAM_H
#define PARAM_H
#include <vector>
#include <string>
#include <functional>

template <typename T>
struct FiledInfo
{
    /* data */
    std::string name;
    std::function<T()> getter;
};

template <typename T>
class XParam
{

public:
    static void registerField(std::string name, std::function<T()> getter)
    {
        fileds().push_back(FiledInfo<T> {name, getter});
    }


    static std::vector<FiledInfo<T>>& fileds()
    {
        static std::vector<FiledInfo<T>> fileds;
        return fileds;
    }

    bool toString(std::string& str)
    {
        str = "{";
        for (auto& filed : fileds())
        {
            str += filed.name + ":" + std::to_string(filed.getter()) + ",";
        }
        str += "}";
        return true;
    }
};


#endif