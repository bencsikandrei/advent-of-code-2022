#include "../at_scope_exit.h"

#include <cstdio>

// linux stuff start
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
// linux stuff end

void
usage(const char* exe)
{
  printf("%s <input file>\n", exe);
  printf("\twhere input file has one integer per line or empty lines\n");
}

int
error(const char* what)
{
  printf("Error: %s\n", what);
  return 1;
}

#include <immintrin.h>

// this is the version that should be the fastest
const __m128i pass1_add4 = _mm_setr_epi32(1, 1, 3, 3);
const __m128i pass2_add4 = _mm_setr_epi32(2, 3, 2, 3);
const __m128i pass3_add4 = _mm_setr_epi32(0, 2, 2, 3);

void
simdsort4(unsigned* __restrict v)
{
  __m128i a = _mm_load_si128(reinterpret_cast<const __m128i*>(v));
  __m128i b;

  b = _mm_shuffle_epi32(a, _MM_SHUFFLE(2, 3, 0, 1)); // 10 6 3 9 -> 6 10 9 3
  b = _mm_cmpgt_epi32(b, a);
  b = _mm_add_epi32(b, pass1_add4);
  a = _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(a), b));

  b = _mm_shuffle_epi32(a, _MM_SHUFFLE(1, 0, 3, 2));
  b = _mm_cmpgt_epi32(b, a);
  b = _mm_add_epi32(b, b);
  b = _mm_add_epi32(b, pass2_add4);
  a = _mm_castps_si128(_mm_permutevar_ps(_mm_castsi128_ps(a), b));

  b = _mm_shuffle_epi32(a, _MM_SHUFFLE(3, 1, 2, 0));
  b = _mm_cmpgt_epi32(b, a);
  b = _mm_add_epi32(b, pass3_add4);
  __m128 ret = _mm_permutevar_ps(_mm_castsi128_ps(a), b);

  _mm_storeu_ps(reinterpret_cast<float*>(v), ret);
}

int
main(int argc, char** argv)
{
  if (argc < 2) {
    usage(argv[0]);
    return 1;
  }

  void* mapped_file = nullptr;
  unsigned long long mapped_file_size = 0;
  auto unmapfile = afb::make_at_scope_exit([mapped_file, mapped_file_size]() {
    if (mapped_file) {
      munmap(mapped_file, mapped_file_size);
    }
  });

  {
    // open file
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
      return error("open");
    }

    auto closefile = afb::make_at_scope_exit([fd]() { close(fd); });

    struct stat sb;
    // memory map it
    if (fstat(fd, &sb) == -1) {
      return error("stat");
    }

    mapped_file_size = sb.st_size;
    mapped_file =
      mmap(nullptr, mapped_file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
      return error("mmap");
    }
  }

  printf("File %s has size %llu\n", argv[1], mapped_file_size);
  const char* end = static_cast<const char*>(mapped_file) + mapped_file_size;
  const char* p = static_cast<const char*>(mapped_file);

  // we only need unsinged values
  auto extract_unsigned = [](const char** data, const char* end) -> unsigned {
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
  };

  unsigned most_calories_index = 0;
  unsigned most_calories = 0;
  unsigned current_index = 0;
  unsigned current_most_calories = 0;

  alignas(16) unsigned top3_most_calories[4]{ 0, 0, 0, 0 };
  // simdsort4(top3_most_calories);

  while (true) {
    unsigned calories = extract_unsigned(&p, end);
    // printf("Calories is: %u\n", calories);

    current_most_calories += calories;

    if (p == end) {
      break;
    }

    if (*p == '\n') {
      ++p;
      if (p == end) {
        break;
      }
      // empty line, move to next
      if (*p == '\n') {
        top3_most_calories[0] = current_most_calories;
        simdsort4(top3_most_calories);
        ++current_index;
        current_most_calories = 0;
        ++p;
      }
    }
  }

  // top 3 values
  printf("top 3 are %d %d %d\n",
         top3_most_calories[3],
         top3_most_calories[2],
         top3_most_calories[1]);
  printf("Sum of top 3: %d\n",
         top3_most_calories[3] + top3_most_calories[2] + top3_most_calories[1]);
}
