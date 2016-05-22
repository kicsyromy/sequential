#ifndef SEQUENTIAL_H
#define SEQUENTIAL_H

#include <string>
#include <tuple>
#include <type_traits>

#include "sequential_p.h"

#define ATTRIBUTE(type, name)                                             \
    struct name                                                           \
    {                                                                     \
        typedef type value_type;                                          \
        static constexpr const char *string() { return #name; };          \
        inline const type &value() const { return name##_; };             \
        inline type &value() { return name##_; };                         \
        inline void set_value(const type &v) { name##_ = v; };            \
        inline void set_value(type &&v) { name##_ = std::move(v); };      \
        private: type name##_;                                            \
    };

#define INIT_ATTRIBUTES(...)                                               \
    struct has_attributes{};                                               \
    typedef std::tuple<__VA_ARGS__> Attributes;                            \
    Attributes attributes;                                                 \


namespace sequential
{
    template<typename Struct, typename Functor>
    inline static void for_each(const Struct &instance, Functor &&f)
    {
        impl_priv::for_each<std::tuple_size<typename Struct::Attributes>::value - 1>(instance.attributes, f);
    }

    template<typename Struct, typename Functor>
    inline static void for_each(Struct &instance, Functor &&f)
    {
        impl_priv::for_each<std::tuple_size<typename Struct::Attributes>::value - 1>(instance.attributes, f);
    }

    namespace attribute
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
            typename std::enable_if<!(impl_priv::has_attributes<Attribute>::value)>::type * = nullptr
        >
        void to_format(Format &&format, const Attribute &attribute)
        {
            format.write(std::make_pair(attribute.string(), attribute.value()));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<impl_priv::has_attributes<Attribute>::value>::type * = nullptr
        >
        void to_format(Format &&format, const Attribute &attribute)
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
            typename std::enable_if<!(impl_priv::has_attributes<Attribute>::value)>::type * = nullptr
        >
        void from_format(const Format &format, Attribute &attribute)
        {
            attribute.set_value(format.template get<typename Attribute::value_type>(attribute.string()));
        }

        template<typename Format, typename Attribute,
            typename std::enable_if<impl_priv::has_attributes<Attribute>::value>::type * = nullptr
        >
        void from_format(const Format &format, Attribute &attribute)
        {
            const auto &output = format.template get<decltype(format.output())>(attribute.string());
            Format ff(output);
            ::sequential::for_each(attribute.value(), [&ff](auto &attribute) {
                from_format(ff, attribute);
            });
        }
    }

    template<typename Format, typename Struct>
    void to_format(Format &&format, const Struct &instance)
    {
        for_each(instance, [&format](const auto &attribute) {
            attribute::to_format(format, attribute);
        });
    }

    template<typename Format, typename Struct>
    void from_format(const Format &format, Struct &instance)
    {
        ::sequential::for_each(instance, [&format](auto &attribute) {
            attribute::from_format(format, attribute);
        });
    }

}

#endif // SEQUENTIAL_H
