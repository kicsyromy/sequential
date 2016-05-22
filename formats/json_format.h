#ifndef JSONFORMAT_H
#define JSONFORMAT_H

#include <json.hpp>

class JsonFormat
{
public:
    JsonFormat() : json_() {}
    JsonFormat(const nlohmann::json &json) : json_(json) { }

public:
    template <typename ValueType>
    void write(const std::pair<const char *, ValueType> &attribute)
    {
        json_[attribute.first] = attribute.second;
    }

    template<typename ValueType>
    const ValueType get(const char *key, const ValueType * = nullptr) const
    {
        ValueType value;
        auto it = json_.find(key);
        if (it != json_.end())
        {
            value = it.value();
        }

        return value;
    }

    nlohmann::json output() const
    {
        return json_;
    }

private:
    nlohmann::json json_;
};

#endif // JSONFORMAT_H
