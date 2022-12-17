#pragma once

namespace afb {

constexpr unsigned
extract_unsigned(const char** data, const char* end) noexcept
{
  auto digit_value = [](char c) -> unsigned {
    return static_cast<unsigned>(c - '0');
  };
  const char* p = *data;
  unsigned res = digit_value(*p);
  unsigned digit;
  ++p;
  while (p != end && (digit = digit_value(*p)) <= 9) {
    res = res * 10 + digit;
    ++p;
  }
  *data = p;
  return res;
}

} // namespace afb
