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
  while (true) {
    unsigned calories = extract_unsigned(&p, end);
    // printf("Calories is: %u\n", calories);

    current_most_calories += calories;
    if (current_most_calories > most_calories) {
      most_calories = current_most_calories;
      most_calories_index = current_index;
    }

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
        ++current_index;
        current_most_calories = 0;
        ++p;
      }
    }
  }

  printf("Index with most calories is %u and value is %u\n",
         most_calories_index,
         most_calories);
}
