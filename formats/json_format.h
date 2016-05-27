#ifndef JSONFORMAT_H
#define JSONFORMAT_H

#include <json.hpp>

class JsonFormat
{
public:
    typedef std::vector<nlohmann::json> ArrayType;

public:
    JsonFormat() : json_() {}
    JsonFormat(const nlohmann::json &json) : json_(json) { }
    JsonFormat(const std::string &json) : json_() { json_ = nlohmann::json::parse(json); }

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

    std::size_t length() const
    {
        std::size_t length = 0;
        if (json_.is_array())
        {
            length = json_.size();
        }

        return length;
    }

    JsonFormat at(std::size_t index) const
    {
        return JsonFormat(json_.at(index));
    }

    inline nlohmann::json output() const
    {
        return json_;
    }

    inline void clear()
    {
        json_.clear();
    }

private:
    nlohmann::json json_;
};

#endif // JSONFORMAT_H
