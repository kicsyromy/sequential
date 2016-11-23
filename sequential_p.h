#ifndef SEQUENTIAL_P_H
#define SEQUENTIAL_P_H

#include <type_traits>
#include <array>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>

namespace sequential_private
{
    template<std::size_t I, typename Tuple, typename F>
    struct for_each_impl
    {
        static constexpr void for_each(Tuple &t, F &f)
        {
            for_each_impl<I - 1, Tuple, F>::for_each(t, f);
            f(std::get<I>(t));
        }
    };

    template<class Tuple, typename F>
    struct for_each_impl<0, Tuple, F>
    {
        static constexpr void for_each(Tuple &t, F &f)
        {
            f(std::get<0>(t));
        }
    };

    template<std::size_t I, typename Tuple, typename F>
    struct static_for_each_impl
    {
        static constexpr void static_for_each(F &f)
        {
            static_for_each_impl<I - 1, Tuple, F>::static_for_each(f);
            f(static_cast<typename std::tuple_element<I, Tuple>::type *>(nullptr));
        }
    };

    template<class Tuple, typename F>
    struct static_for_each_impl<0, Tuple, F>
    {
        static constexpr void static_for_each(F &f)
        {
            f(static_cast<typename std::tuple_element<0, Tuple>::type *>(nullptr));
        }
    };

    template<std::size_t count, class Tuple, typename F>
    void constexpr for_each(Tuple &t, F &&f)
    {
        for_each_impl<count, Tuple, F>::for_each(t, f);
    }

    template<std::size_t count, class Tuple, typename F>
    void constexpr static_for_each(F &&f)
    {
        static_for_each_impl<count, Tuple, F>::static_for_each(f);
    }

    template<typename Attribute>
    class has_attributes
    {
        template<typename A> static std::true_type test(typename A::value_type::has_attributes *);
        template<typename A> static std::false_type test(...);
    public:
        static constexpr bool value = decltype(test<Attribute>(0))::value;
    };

    template<typename ContainerType>
    struct is_variable_size_container
    {
        static constexpr bool value = false;
    };

    template<typename ValueType>
    struct is_variable_size_container<std::vector<ValueType>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<const std::vector<ValueType>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<const std::vector<ValueType> &>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<std::vector<ValueType> &>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<std::list<ValueType>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<const std::list<ValueType>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<const std::list<ValueType> &>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType>
    struct is_variable_size_container<std::list<ValueType> &>
    {
        static constexpr bool value = true;
    };

    template<typename ArrayType>
    struct is_fixed_size_container
    {
        static constexpr bool value = false;
    };

    template<typename ValueType, std::size_t N>
    struct is_fixed_size_container<std::array<ValueType, N>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType, std::size_t N>
    struct is_fixed_size_container<const std::array<ValueType, N>>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType, std::size_t N>
    struct is_fixed_size_container<const std::array<ValueType, N> &>
    {
        static constexpr bool value = true;
    };

    template<typename ValueType, std::size_t N>
    struct is_fixed_size_container<std::array<ValueType, N> &>
    {
        static constexpr bool value = true;
    };

    template<typename, typename Enable = void>
    struct is_sequence_container
    {
        static constexpr bool value = false;
    };

    template<typename ContainerType>
    struct is_sequence_container<ContainerType,
            typename std::enable_if<
            is_variable_size_container<ContainerType>::value ||
            is_fixed_size_container<ContainerType>::value
            >::type
            >
    {
        static constexpr bool value = true;
    };
}

#endif // SEQUENTIAL_P_H
