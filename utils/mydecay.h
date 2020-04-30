#pragma once
#include <type_traits>
#include <memory>

template <class T>
struct my_decay
{
    using type = typename std::conditional<std::is_pointer<T>::value, typename std::remove_const<T>::type, typename std::decay<T>::type>::type;
};

static_assert (std::is_same<my_decay<const std::shared_ptr<int>&>::type, std::shared_ptr<int>>::value, "my_decay produces wrong shared pointer");
static_assert (std::is_same<my_decay<int*>::type, int*>::value, "my_decay produces wrong regular pointer");



#define CLASS_TYPE std::decay<decltype(*this)>::type
#define GET_CURRENT_SHARED_IMPL std::dynamic_pointer_cast<CLASS_TYPE>(const_cast<CLASS_TYPE*>(this)->shared_from_this())

#ifndef NDEBUG
#define GET_CURRENT_SHARED (assert(GET_CURRENT_SHARED_IMPL),GET_CURRENT_SHARED_IMPL)
#else
#define GET_CURRENT_SHARED GET_CURRENT_SHARED_IMPL
#endif
