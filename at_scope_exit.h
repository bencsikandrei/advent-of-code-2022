#pragma once

#include "forward.h"

namespace afb {

template<typename F>
struct [[nodiscard]] at_scope_exit
{
  template<typename FF = F>
  at_scope_exit(FF&& f) noexcept
    : m_f(afb::forward<FF>(f))
  {
  }
  ~at_scope_exit() noexcept { m_f(); }
  at_scope_exit(const at_scope_exit&) = delete;
  at_scope_exit(at_scope_exit&&) = delete;

private:
  F m_f;
};

template<typename F>
auto
make_at_scope_exit(F&& f)
{
  return at_scope_exit<F>(afb::forward<F>(f));
}

} // namespace afb
