#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <string>
#include <vector>
#include <tuple>
#include <type_traits>

#include <iostream>

#include "sequential_p.h"

#define ATTRIBUTE(type, name)                                    \
    private:                                                     \
    struct name                                                  \
    {                                                            \
        typedef type value_type;                                 \
        name() : name##_() {}                                    \
        name(const type &value) : name##_(value) {}              \
        name(type &&value) : name##_(std::move(value)) {}        \
        static constexpr const char *string() { return #name; }; \
        inline const type &value() const { return name##_; };    \
        inline type &value() { return name##_; };                \
        inline void set_value(const type &v) { name##_ = v; };   \
        inline void set_value(type &&v) { name##_ = v; };        \
        private: type name##_;                                   \
    };                                                           \
    public:                                                      \
    const type &get_##name() const {                             \
        return std::get<name>(attributes).value();               \
    }                                                            \
    void set_##name(const type &value) {                         \
        std::get<name>(attributes).set_value(value);             \
    }                                                            \
    void set_##name(type &&value) {                              \
        std::get<name>(attributes).set_value(value);             \
    }

#define INIT_ATTRIBUTES(...)                            \
    private:                                            \
    friend struct ::sequential;                         \
    friend struct ::sequential::attribute;              \
    typedef std::tuple<__VA_ARGS__> Attributes;         \
    Attributes attributes;                              \
    public:                                             \
    struct has_attributes{};                            \
    static std::vector<std::string> attribute_names() { \
        std::vector<std::string> v;                     \
        v.reserve(std::tuple_size<Attributes>::value);  \
        sequential_private::static_for_each<std::tuple_size<Attributes>::value -1, Attributes>([&](auto attribute) { \
            v.push_back(std::remove_pointer<typename std::remove_const<decltype(attribute)>::type>::type::string()); \
        });                                             \
        return std::move(v);                            \
    }


struct sequential
{
    template<typename Struct, typename Functor>
    inline static void for_each(const Struct &instance, Functor &&f)
    {
        sequential_private::for_each<std::tuple_size<typename Struct::Attributes>::value - 1>(instance.attributes, f);
    }

    template<typename Struct, typename Functor>
    inline static void for_each(Struct &instance, Functor &&f)
    {
        sequential_private::for_each<std::tuple_size<typename Struct::Attributes>::value - 1>(instance.attributes, f);
    }

    struct attribute
    {
        template<typename Attribute>
        inline static const char *name()
        {
            return Attribute::string();
        }

        template<typename Attribute, typename Struct>
        inline static const typename Attribute::value_type &value_of(const Struct &instance)
        {
            return std::get<Attribute>(instance.attributes).value();
        }

        template<typename Attribute, typename Struct>
        inline static void set_value(Struct &instance, const typename Attribute::value_type &value)
        {
            std::get<Attribute>(instance.attributes).set_value(value);
        }

        template<typename Attribute, typename Struct>
        inline static void set_value(Struct &instance, typename Attribute::value_type &&value)
        {
            std::get<Attribute>(instance.attributes).set_value(std::forward<typename Attribute::value_type>(value));
        }

        template<typename Attribute, typename Struct>
        inline static Attribute &get(Struct &instance)
        {
            return std::get<Attribute>(instance.attributes);
        }

        template<typename Attribute, typename Struct>
        inline static const Attribute &get(const Struct &instance)
        {
            return std::get<Attribute>(instance.attributes);
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                !sequential_private::has_attributes<Attribute>::value and
                (!sequential_private::is_sequence_container<typename Attribute::value_type>::value or
                 (sequential_private::is_sequence_container<typename Attribute::value_type>::value and
                 !sequential_private::has_attributes<typename Attribute::value_type>::value))
            >::type * = nullptr
        >
        static void to_format(Format &&format, const Attribute &attribute)
        {
            format.write(std::make_pair(attribute.string(), attribute.value()));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                sequential_private::has_attributes<Attribute>::value and
                !sequential_private::is_sequence_container<typename Attribute::value_type>::value
            >::type * = nullptr
        >
        static void to_format(Format &&format, const Attribute &attribute)
        {
            typedef typename std::remove_reference<Format>::type FormatType;
            FormatType ff;
            const auto &attr = attribute.value();
            ::sequential::for_each(attr, [&ff](const auto &attribute) {
                to_format(ff, attribute);
            });
            format.write(std::make_pair(attribute.string(), ff.output()));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                 !sequential_private::has_attributes<Attribute>::value and
                 (sequential_private::is_sequence_container<typename Attribute::value_type>::value and
                  sequential_private::has_attributes<typename Attribute::value_type>::value)
            >::type * = nullptr
        >
        static void to_format(Format &&format, Attribute &attribute)
        {
            typedef typename std::remove_reference<Format>::type FormatType;
            typename FormatType::ArrayType array;
            for (auto element: attribute.value())
            {
                FormatType ff;
                ::sequential::for_each(element, [&ff](const auto &attribute) {
                    to_format(ff, attribute);
                });
                array.push_back(ff.output());
            }
            format.write(std::make_pair(attribute.string(), array));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                !sequential_private::has_attributes<Attribute>::value and
                (!sequential_private::is_sequence_container<typename Attribute::value_type>::value or
                 (sequential_private::is_sequence_container<typename Attribute::value_type>::value and
                 !sequential_private::has_attributes<typename Attribute::value_type>::value))
            >::type * = nullptr
        >
        static void from_format(const Format &format, Attribute &attribute)
        {
            attribute.set_value(format.template get<typename Attribute::value_type>(attribute.string()));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                sequential_private::has_attributes<Attribute>::value and
                !sequential_private::is_sequence_container<typename Attribute::value_type>::value
            >::type * = nullptr
        >
        static void from_format(const Format &format, Attribute &attribute)
        {
            const auto &output = format.template get<decltype(format.output())>(attribute.string());
            Format ff(output);
            ::sequential::for_each(attribute.value(), [&ff](auto &attribute) {
                from_format(ff, attribute);
            });
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<
                 !sequential_private::has_attributes<Attribute>::value and
                 (sequential_private::is_sequence_container<typename Attribute::value_type>::value and
                  sequential_private::has_attributes<typename Attribute::value_type>::value)
            >::type * = nullptr
        >
        static void from_format(const Format &format, Attribute &attribute)
        {
            typedef typename std::remove_reference<Format>::type FormatType;
            const auto &output = format.template get<decltype(format.output())>(attribute.string());
            FormatType ff(output);
            const auto arrayLength = ff.length();
            typename Attribute::value_type array;

            for (std::size_t it = 0; it < arrayLength; ++it)
            {
                typename Attribute::value_type::value_type element;
                const auto &elementFormat = ff.at(it);
                ::sequential::for_each(element, [&elementFormat](auto &attribute) {
                    from_format(elementFormat, attribute);
                });
                array.push_back(std::move(element));
            }
            attribute.set_value(std::move(array));
        }
    };

    template<typename Format, typename Struct>
    static void to_format(Format &&format, const Struct &instance)
    {
        for_each(instance, [&format](const auto &attribute) {
            attribute::to_format(format, attribute);
        });
    }

    template<typename Format, typename Struct>
    static void from_format(const Format &format, Struct &instance)
    {
        ::sequential::for_each(instance, [&format](auto &attribute) {
            attribute::from_format(format, attribute);
        });
    }
};

#endif // SEQUENTIAL_H
