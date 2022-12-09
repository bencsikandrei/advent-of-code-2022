#pragma once

#include "traits.h"

namespace afb {

template<typename T>
inline T&&
forward(typename afb::remove_reference_t<T>& t) noexcept
{
  return static_cast<T&&>(t);
}

template<typename T>
inline T&&
forward(typename afb::remove_reference_t<T>&& t) noexcept
{
  return static_cast<T&&>(t);
}

} // namespace afb
