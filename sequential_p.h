#ifndef SEQUENTIAL_P_H
#define SEQUENTIAL_P_H

#include <type_traits>

namespace sequential
{
    namespace impl_priv
    {
        template<size_t I, typename Tuple, typename F>
        struct ForEachImpl
        {
            static void for_each(Tuple &t, F &f)
            {
                ForEachImpl<I - 1, Tuple, F>::for_each(t, f);
                f(std::get<I>(t));
            }
        };

        template<class Tuple, typename F>
        struct ForEachImpl<0, Tuple, F>
        {
            static void for_each(Tuple &t, F &f)
            {
                f(std::get<0>(t));
            }
        };

        template<size_t count, class Tuple, typename F>
        void for_each(Tuple &t, F &&f)
        {
            ForEachImpl<count, Tuple, F>::for_each(t, f);
        }
    }

    namespace attribute
    {
        namespace impl_priv
        {
            template<class Attribute>
            class has_attributes
            {
                template<typename A> static std::true_type test(typename A::value_type::has_attributes *);
                template<typename A> static std::false_type test(...);
            public:
                static constexpr bool value = decltype(test<Attribute>(0))::value;
            };
        }
    }
}

#endif // SEQUENTIAL_P_H
