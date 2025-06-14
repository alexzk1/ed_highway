#pragma once
#include "utils/cm_ctors.h"

#include <locale>

class FloatsShouldUsePointAsString
{
  public:
    FloatsShouldUsePointAsString() = default;
    NO_COPYMOVE(FloatsShouldUsePointAsString);

    ~FloatsShouldUsePointAsString()
    {
        std::locale::global(oldLocale);
    }

  private:
    const std::locale oldLocale{std::locale::global(std::locale::classic())};
};
