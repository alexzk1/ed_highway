#pragma once

//helper macros to declare/delete copy-moves
#define DEFAULT_COPYMOVE(TYPE) TYPE(const TYPE&) = default;TYPE(TYPE&&) = default;TYPE& operator=(const TYPE&) = default;TYPE& operator=(TYPE&&) = default
#define NO_COPYMOVE(TYPE) TYPE(const TYPE&) = delete;TYPE(TYPE&&) = delete;TYPE& operator=(const TYPE&) = delete;TYPE& operator=(TYPE&&) = delete
#define MOVEONLY_ALLOWED(TYPE) TYPE(const TYPE&) = delete;TYPE(TYPE&&) = default;TYPE& operator=(const TYPE&) = delete;TYPE& operator=(TYPE&&) = default
