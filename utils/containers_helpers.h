#pragma once
#include "marray.h"
#include "mydecay.h"
#include "type_checks.h"

#include <boost/pool/pool_alloc.hpp>

#include <algorithm>
#include <functional>
#include <map>
#include <random>
#include <set>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace types_ns {
template <class T>
void try_resize(T &cont, size_t size)
{
    static_assert(!types_ns::isvector<T>::value,
                  "Woops, something went wrong! Need another overload.");
    (void)cont;
    (void)size;
}

template <class T, class Alloc>
void try_resize(std::vector<T, Alloc> &cont, size_t size)
{
    cont.resize(size);
}

// transforms container of weak_ptr into another of shared_ptr
template <class Dst, class Src>
Dst copy_convert(const Src &src)
{
    using PtrT = typename Dst::value_type;
    static_assert(types_ns::issharedptr<PtrT>::value, "Dst must be container of shared_ptr");
    Dst dest;
    if (!src.empty())
    {
        try_resize(dest, src.size());
        std::transform(src.cbegin(), src.cend(), dest.begin(), [](const auto &v) {
            return v.lock();
        });
    }
    return dest;
}

template <typename T, size_t N>
constexpr size_t countof(const T (&)[N]) noexcept
{
    return N;
}

template <typename T, size_t Y, size_t X>
constexpr size_t countofY(const T (&)[Y][X]) noexcept
{
    return Y;
}

template <typename T, size_t Y, size_t X>
constexpr size_t countofX(const T (&)[Y][X]) noexcept
{
    return X;
}

template <typename T>
constexpr size_t countof(const T &v) noexcept
{
    return v.size();
}

template <typename T, size_t N>
constexpr size_t bytesize(const T (&)[N]) noexcept
{
    return N * sizeof(T);
}

template <typename T, typename Alloc>
constexpr size_t bytesize(const std::vector<T, Alloc> &v) noexcept
{
    return v.size() * sizeof(T);
}

template <class Cont>
void shuffle_container(Cont &src)
{
    // todo: do I need custom seeds here ? >:
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(std::begin(src), std::end(src), g);
}

namespace pickrnd_helper {

template <class Cont>
const auto &pickRandom(const Cont &src, std::true_type)
{
    static_assert(isvector<Cont>::value || isarray<Cont>::value,
                  "Only sequental numbered accepted.");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, countof(src) - 1);
    return src[dis(gen)];
}

template <class T>
const auto &ret_val(const T &v, std::true_type)
{
    return v.second;
}

template <class T>
const T &ret_val(const T &v, std::false_type)
{
    return v;
}

// this version works with anything else except vectors & arrays
template <class Cont>
const auto &pickRandom(const Cont &src, std::false_type)
{
    static_assert(!(isvector<Cont>::value || isarray<Cont>::value),
                  "Only non-sequental numbered accepted.");
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, countof(src) - 1);
    auto b = src.begin();
    std::advance(b, dis(gen));
    return ret_val(*b, ismap<Cont>::type());
}
} // namespace pickrnd_helper

template <class Cont>
const auto &pickRandom(const Cont &src)
{
    constexpr static bool isva = isvector<Cont>::value || isarray<Cont>::value;
    using tp = std::integral_constant<bool, isva>;
    return pickrnd_helper::pickRandom(src, tp());
}

template <class T, size_t N>
bool contains(const T (&arr)[N], const T &what)
{
    auto e = std::end(arr);
    return e != std::find(std::begin(arr), e, what);
}

template <class T, class Cont>
bool contains(const Cont &arr, const T &what)
{
    auto e = std::end(arr);
    return e != std::find(std::begin(arr), e, what);
}

template <class T>
bool contains(const std::set<T> &arr, const T &what)
{
    // guess it is faster then std::find (void Player::UpdateUnderwaterState()maybe)
    return arr.count(what);
}

template <class T>
bool contains(const std::unordered_set<T> &arr, const T &what)
{
    return arr.count(what);
}

template <class Container, class Pred>
void remove_if(Container &src, const Pred &pred)
{
    src.erase(std::remove_if(src.begin(), src.end(), pred), src.end());
}

template <class Map, class Pred>
void remove_if_frommap(Map &src, const Pred &func)
{
    static_assert(types_ns::ismap<Map>::value, "Maps only.");
    for (auto it = src.begin(); it != src.end();)
    {
        if (func(it->second)) // sending value to functor, as removing by key has no sence, as it
                              // could be used direct erase
            it = src.erase(it);
        else
            ++it;
    }
}

template <class Set, class Pred>
void remove_if_fromset(Set &src, const Pred &func)
{
    static_assert(types_ns::isset<Set>::value, "Sets only.");
    for (auto it = src.begin(); it != src.end();)
    {
        if (func(*it))
            it = src.erase(it);
        else
            ++it;
    }
}

template <class T, class Pred>
void remove_if(std::set<T> &src, const Pred &func)
{
    remove_if_fromset(src, func);
}

template <class T, class Pred>
void remove_if(std::unordered_set<T> &src, const Pred &func)
{
    remove_if_fromset(src, func);
}

template <class Key, class T, class Pred>
void remove_if(std::map<Key, T> &src, const Pred &func)
{
    remove_if_frommap(src, func);
}

template <class Key, class T, class Pred>
void remove_if(std::multimap<Key, T> &src, const Pred &func)
{
    remove_if_frommap(src, func);
}

template <class Key, class T, class Pred>
void remove_if(std::unordered_map<Key, T> &src, const Pred &func)
{
    remove_if_frommap(src, func);
}

// usable to make some std algorithms parallel
template <class T, class Alloc = typename std::vector<T>::allocator_type>
auto generateSeq(T min, size_t size)
{
    std::vector<T, Alloc> res;
    res.resize(size);
    for (size_t offset = 0; offset < size; ++offset)
        res[offset] = min + offset;

    return res;
}

template <class T, size_t N, class Pred>
auto foreach_apply(const T (&src)[N], Pred pred)
{
    using PooledVector = std::vector<T, boost::pool_allocator<T>>;
    PooledVector res;
    res.reserve(N);
    std::transform(src.begin(), src.end(), std::back_inserter(res), pred);
    return res;
}

template <class Cont, class Pred>
auto foreach_apply(const Cont &src, const Pred &pred)
{
    using T = typename Cont::value_type;
    using PooledVector = std::vector<T, boost::pool_allocator<T>>;
    PooledVector res;

    res.reserve(src.size());
    std::transform(src.begin(), src.end(), std::back_inserter(res), pred);

    return res;
}

template <class K, class V>
const V &findorempty(const std::map<K, V> &src, const K &key)
{
    const static V empty;
    if (src.count(key))
        return src.at(key);
    return empty;
}

template <class K, class V>
const V &findorempty(const std::unordered_map<K, V> &src, const K &key)
{
    const static V empty;
    if (src.count(key))
        return src.at(key);
    return empty;
}

template <class T, size_t N>
T max_value(const T (&arr)[N])
{
    return *std::max_element(std::begin(arr), std::end(arr));
}

template <class T, class Cont>
T max_value(const Cont &arr)
{
    return *std::max_element(std::begin(arr), std::end(arr));
}

template <class T, size_t N>
T min_value(const T (&arr)[N])
{
    return *std::min_element(std::begin(arr), std::end(arr));
}

template <class T, class Cont>
T min_value(const Cont &arr)
{
    return *std::min_element(std::begin(arr), std::end(arr));
}

template <class T, size_t N, class Pred>
void for_each_array(T (&arr)[N], const Pred &func)
{
    for (size_t i = 0; i < N; ++i)
        func(arr[i], i);
}

template <class T, size_t N, class Pred>
void for_each_array(array1d<T, N> &arr, const Pred &func)
{
    for (size_t i = 0; i < N; ++i)
        func(arr[i], i);
}

template <class T, size_t N, size_t M, class Pred>
void for_each_array(T (&arr)[N][M], const Pred &func)
{
    for (size_t y = 0; y < N; ++y)
        for (size_t x = 0; x < M; ++x)
            func(arr[y][x], x, y);
}

template <class T, size_t N, size_t M, class Pred>
void for_each_array(array2d<T, N, M> &arr, const Pred &func)
{
    for (size_t y = 0; y < N; ++y)
        for (size_t x = 0; x < M; ++x)
            func(arr[y][x], x, y);
}

template <class T, size_t N, size_t M, class Pred>
void for_each_array_left_index(T (&arr)[N][M], const Pred &func)
{
    for (size_t y = 0; y < N; ++y)
        func(arr[y], y);
}

template <class Pred, class T, size_t N, size_t M>
void for_each_array_left_index(array2d<T, N, M> &arr, const Pred &func)
{
    for (size_t y = 0; y < N; ++y)
    {
        array1d<T, M> &ref = arr[y];
        func(ref, y);
    }
}

template <class T>
using enum_hash = typename std::conditional<std::is_enum<T>::value,
                                            std::hash<std::underlying_type<T>>, std::hash<T>>::type;

template <class T, size_t N, class V>
void init(T (&arr)[N], const V &val)
{
    std::fill_n(std::begin(arr), N, val);
}

template <class T, size_t N, class V>
void init(array1d<T, N> &arr, const V &val)
{
    std::fill_n(std::begin(arr), N, val);
}

template <class T, size_t N, size_t M, class V>
void init(T (&arr)[N][M], const V &val)
{
    for (size_t y = 0; y < N; ++y)
        init(arr[y], val);
}

template <class T, size_t N, size_t M, class V>
void init(array2d<T, N, M> &arr, const V &val)
{
    for (size_t y = 0; y < N; ++y)
        init(arr[y], val);
}

template <class Any, class V>
void init(Any &src, const V &val)
{
    std::fill(std::begin(src), std::end(src), val);
}

template <class T, size_t N, class Pred>
bool anyof(const T (&arr)[N], const Pred &val)
{
    return std::any_of(std::begin(arr), std::end(arr), val);
}

template <class T, size_t N, class Pred>
bool allof(const T (&arr)[N], const Pred &val)
{
    return std::all_of(std::begin(arr), std::end(arr), val);
}

template <class Cont, class Pred>
bool anyof(const Cont &arr, const Pred &val)
{
    return std::any_of(std::begin(arr), std::end(arr), val);
}

template <class Cont, class Pred>
bool allof(const Cont &arr, const Pred &val)
{
    return std::all_of(std::begin(arr), std::end(arr), val);
}

// if Cont is array of pointers and Value is nullptr then we get 2 different types
// and returning reference is to temporary object
// so we want to check and avoid copies when possible
// http://cpp.sh/64fa53

template <class Iter, class Value>
struct ref_or_copy_it
{
    using it_type = typename std::iterator_traits<Iter>::value_type;
    using type =
      typename std::conditional<std::is_same<it_type, Value>::value, const Value &, it_type>::type;
};

template <class Cont, class Value>
struct ref_or_copy
{
    static Cont tmp;
    using decl_it = typename std::decay<decltype(std::begin(tmp))>::type;
    using it_type = typename ref_or_copy_it<decl_it, Value>::it_type;
    using type = typename ref_or_copy_it<decl_it, Value>::type;
};

template <class Iterator, class Value, class Pred>
typename ref_or_copy_it<Iterator, Value>::type findif_value(Iterator begin, Iterator end,
                                                            const Value &def, const Pred &func)
{
    const auto it = std::find_if(begin, end, func);
    return (it == end) ? def : *it;
}

template <class Cont, class Value, class Pred>
typename ref_or_copy<Cont, Value>::type findif_value(const Cont &where, const Value &def,
                                                     const Pred &func)
{
    return findif_value(std::begin(where), std::end(where), def, func);
}

// this returns copy always
template <class Cont, class Value, class Pred>
std::pair<typename ref_or_copy<Cont, Value>::it_type, int64_t>
findif_value_and_index(const Cont &where, const Value &def, const Pred &func)
{
    const auto end = std::end(where);
    const auto beg = std::begin(where);
    const auto it = std::find_if(beg, end, func);
    return (it == end) ? std::make_pair(def, -1) : std::make_pair(*it, std::distance(beg, it));
}

template <class ContainerT>
auto pop_front(ContainerT &q)
{
    typename ContainerT::value_type v{std::move(q.front())};
    q.pop_front();
    return v;
}

template <class Vec>
auto pop_back(Vec &vec)
{
    typename Vec::value_type res{std::move(vec.back())};
    vec.pop_back();
    return res;
}

template <class Ret, class K, class V, class Less, class Alloc>
void keyset(Ret &ret, const std::map<K, V, Less, Alloc> &m)
{
    for (auto &kv : m)
        ret.insert(kv.first);
}

template <class Ret, class K, class V, class Less, class Alloc>
void keyset(Ret &ret, const std::multimap<K, V, Less, Alloc> &m)
{
    for (auto &kv : m)
        ret.insert(kv.first);
}

template <class K, class V, class Less, class Alloc>
std::set<K> keyset(const std::map<K, V, Less, Alloc> &m)
{
    std::set<K> r;
    keyset(r, m);
    return r;
}

template <class K, class V, class Less, class Alloc>
std::set<K> keyset(const std::multimap<K, V, Less, Alloc> &m)
{
    std::set<K> r;
    keyset(r, m);
    return r;
}

template <class Ret, class K, class V, class Hash, class Pred, class Alloc>
void keyset(Ret &ret, const std::unordered_map<K, V, Hash, Pred, Alloc> &m)
{
    for (auto &kv : m)
        ret.insert(kv.first);
}

template <class K, class V, class Hash, class Pred, class Alloc>
std::unordered_set<K> keyset(const std::unordered_map<K, V, Hash, Pred, Alloc> &m)
{
    std::unordered_set<K> r;
    keyset(r, m);
    return r;
}

// https://stackoverflow.com/questions/12030538/calling-a-function-for-each-variadic-template-argument-and-an-array
template <class F, class... Args>
void for_each_argument(F f, Args &&...args)
{
    [](...) {
    }((f(std::forward<Args>(args)), 0)...);
}

// traverses many containers, calls func for same positioned iterators, out-of-bound check is not
// performed func must accept references to contained elements
template <class Pred, class... Iters>
void for_each_many(const Pred &func, const size_t count, Iters... begin)
{
    for (size_t i = 0; i < count; ++i)
    {
        func((*begin)...);
        for_each_argument(
          [](auto &v) {
              std::advance(v, 1);
          },
          begin...);
    }
}

// returns true if was interrupted by returning true from Pred
template <class Pred, class... Iters>
bool for_each_many_breakable(const Pred &func, const size_t count, Iters... begin)
{
    bool result = false;
    for (size_t i = 0; i < count; ++i)
    {
        if ((result = func((*begin)...)))
            break;
        for_each_argument(
          [](auto &v) {
              std::advance(v, 1);
          },
          begin...);
    }
    return result;
}

/*
 * foreach_container_breakable([](const auto &i){
    // do something here
    return false; //if return true - then stop
}, vecA, vecB, vecC, vecD, vecE, vecF);

Allows to iterate many containers (vectors) as it is single one.
from:
https://stackoverflow.com/questions/12552277/whats-the-best-way-to-iterate-over-two-or-more-containers-simultaneously
 * */
template <typename Func, typename T, typename Alloc>
void foreach_container_breakable(Func callback, std::vector<T, Alloc> &v)
{
    (void)std::find_if(v.begin(), v.end(), callback);
}

template <typename Func, typename T, typename Alloc, typename... Args>
void foreach_container_breakable(Func callback, std::vector<T, Alloc> &v, Args... args)
{
    if (v.end() == std::find_if(v.begin(), v.end(), callback))
        foreach_container_breakable(callback, args...);
}

template <typename Func, typename T, typename Alloc>
void foreach_container_breakable(Func callback, std::initializer_list<std::vector<T, Alloc>> list)
{
    for (auto &vec : list)
        if (vec.end() != std::find_if(vec.begin(), vec.end(), callback))
            break;
}

template <typename Func, typename T, typename Alloc>
void foreach_container_breakable(Func callback, std::initializer_list<std::vector<T, Alloc> *> list)
{
    for (auto &vec : list)
        if (vec->end() != std::find_if(vec->begin(), vec->end(), callback))
            break;
}

// Joins one or more container chunks specified by iterators into a single container
template <class Cont, class ItA, class ItB>
void join(Cont &res, const ItA begin, const ItB end)
{
    // recursion bottom
    std::copy(begin, end, std::back_inserter(res));
}

template <class Cont, class ItA, class ItB, class... Args>
void join(Cont &res, const ItA begin, const ItB end, Args... args)
{
    join(res, begin, end);
    join(res, args...);
}

// taken from
// http://stackoverflow.com/questions/6899392/generic-hash-function-for-all-stl-containers
template <class T>
inline void hash_combine(std::size_t &seed, const T &v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// hashes any struct with float fields x-y-z
template <class T>
struct Point3DHasher
{
    size_t operator()(const T &v) const
    {
        return hash(v);
    }
    static size_t hash(const T &v)
    {
        size_t h = std::hash<float>()(v.x);
        hash_combine(h, v.y);
        hash_combine(h, v.z);
        return h;
    }
};

} // namespace types_ns

#define VALUE_COMP_PRED(WHAT)                                                                      \
    [&](const auto &v) -> bool {                                                                   \
        return v == WHAT;                                                                          \
    }
#define VALUE_BOOL_PRED                                                                            \
    [](const auto &v) -> bool {                                                                    \
        return static_cast<bool>(v);                                                               \
    }
#define VALUE_BOOL_NOT_PRED                                                                        \
    [](const auto &v) -> bool {                                                                    \
        return !static_cast<bool>(v);                                                              \
    }
