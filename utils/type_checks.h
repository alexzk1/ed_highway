#pragma once
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace types_ns {
template <class T, class S>
bool isTypePtr(S *ptr)
{
    return dynamic_cast<T *>(ptr) != nullptr;
}

template <class T, class S>
bool isTypePtr(const S *ptr)
{
    return dynamic_cast<const T *>(ptr) != nullptr;
}

template <class T, class S>
bool isTypePtr(const std::shared_ptr<S> &ptr)
{
    return std::dynamic_pointer_cast<T>(ptr) != nullptr;
}

// some templates to make proper conditional compiling with explicit instantiation

template <typename T>
using isarray = typename std::is_array<T>::type;
template <typename T>
struct isset : std::false_type
{
};
template <typename... Args>
struct isset<std::set<Args...>> : std::true_type
{
};

template <typename T>
struct isqtmap : std::false_type
{
};

template <typename T>
struct ismap : std::false_type
{
};
template <typename... Args>
struct ismap<std::map<Args...>> : std::true_type
{
};
template <typename... Args>
struct ismap<std::unordered_map<Args...>> : std::true_type
{
};
template <typename... Args>
struct ismap<std::multimap<Args...>> : std::true_type
{
};

template <typename T>
struct ispair : std::false_type
{
};
template <typename... Args>
struct ispair<std::pair<Args...>> : std::true_type
{
};

template <typename T>
struct isclassarray : std::false_type
{
};
template <class T, std::size_t N>
struct isclassarray<std::array<T, N>> : std::true_type
{
};

template <typename T>
struct isvector : std::false_type
{
};
template <typename... Args>
struct isvector<std::vector<Args...>> : std::true_type
{
};

template <typename T>
struct issharedptr : std::false_type
{
};
template <typename... Args>
struct issharedptr<std::shared_ptr<Args...>> : std::true_type
{
};

template <typename T>
struct isweakptr : std::false_type
{
};
template <typename... Args>
struct isweakptr<std::weak_ptr<Args...>> : std::true_type
{
};

template <typename T>
struct islockguard : std::false_type
{
};
template <typename... Args>
struct islockguard<std::lock_guard<Args...>> : std::true_type
{
};

template <class T, size_t N>
struct TestAlignForStruct
{
    using is_align1 =
      typename std::conditional<alignof(T) == N, std::true_type, std::false_type>::type;
    static constexpr bool value{
      std::conditional<std::is_class<T>::value, is_align1, std::true_type>::type::value};
};

template <class T>
using TestAlign1ForStruct = TestAlignForStruct<T, 1>;

// taken from here:
// https://stackoverflow.com/questions/15393938/find-out-whether-a-c-object-is-callable
template <typename T, typename U = void>
struct is_callable
{
    static bool const constexpr value =
      std::conditional_t<std::is_class<std::remove_reference_t<T>>::value,
                         is_callable<std::remove_reference_t<T>, int>, std::false_type>::value;
};

template <typename T, typename U, typename... Args>
struct is_callable<T(Args...), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T (*)(Args...), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T (&)(Args...), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T (*)(Args......), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T (&)(Args......), U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) volatile, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const volatile, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) volatile, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const volatile, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) volatile &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const volatile &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) volatile &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const volatile &, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) volatile &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args...) const volatile &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) volatile &&, U> : std::true_type
{
};
template <typename T, typename U, typename... Args>
struct is_callable<T(Args......) const volatile &&, U> : std::true_type
{
};

template <typename T>
struct is_callable<T, int>
{
    using YesType = char (&)[1];
    using NoType = char (&)[2];

    struct Fallback
    {
        void operator()();
    };

    struct Derived : T, Fallback
    {
    };

    template <typename U, U>
    struct Check;

    template <typename>
    static YesType Test(...);

    template <typename C>
    static NoType Test(Check<void (Fallback::*)(), &C::operator()> *);

    static bool const constexpr value = sizeof(Test<Derived>(0)) == sizeof(YesType);
};

// allows to get element type from C-arrays, std::array and array1d
template <class T>
struct ArrayTest
{
    static_assert(types_ns::isarray<T>::value || types_ns::isclassarray<T>::value
                    || types_ns::isvector<T>::value,
                  "T must be an array or vector type.");

    template <class A>
    struct wrap
    {
        using type = typename std::remove_all_extents<A>::type;
    };

    template <class A, std::size_t N>
    struct wrap<std::array<A, N>>
    {
        using type = typename std::array<A, N>::value_type;
    };

    template <class A, class Alloc>
    struct wrap<std::vector<A, Alloc>>
    {
        using type = typename std::vector<A, Alloc>::value_type;
    };

    using type = typename wrap<T>::type;
};

template <class T>
struct array_size
{
    constexpr static size_t size = std::extent<T>::value;
};

template <class A, std::size_t N>
struct array_size<std::array<A, N>>
{
    constexpr static size_t size = N;
};

template <typename... Args>
constexpr bool return_void(std::function<void(Args...)>)
{
    return true;
}

template <typename R, typename... Args>
constexpr bool return_void(std::function<R(Args...)>)
{
    return false;
}
} // namespace types_ns

// usage like  int arr[5]; VALUE_TYPE(arr) new_var=0;
#define VALUE_TYPE(ARR) types_ns::ArrayTest<decltype(ARR)>::type
#define COUNTOF(ARR) types_ns::array_size<decltype(ARR)>::size
