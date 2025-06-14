#pragma once

#include <mutex>
#include <type_traits>

#define LOCK_GUARD_ON(MUTEX_NAME)                                                                  \
    const std::lock_guard<std::decay_t<decltype(MUTEX_NAME)>> __guard_var##MUTEX_NAME(MUTEX_NAME)

// inside templates use this one macro, it adds "typename"
#define LOCK_GUARD_ON_TEMPL(MUTEX_NAME)                                                            \
    const std::lock_guard<typename std::decay_t<decltype(MUTEX_NAME)>> __guard_var##MUTEX_NAME(    \
      MUTEX_NAME)
