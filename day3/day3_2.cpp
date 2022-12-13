#include "../at_scope_exit.h"

#include <cstdint>
#include <cstdio>

// linux stuff start
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
// linux stuff end

int
error(const char* what)
{
  printf("Error: %s\n", what);
  return 1;
}

void
usage(const char* exe)
{
  printf("%s <input file>\n", exe);
  printf("\twhere input file has one integer per line or empty lines\n");
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

  auto findbyte = [](const char* b, const char* e, char c) -> const char* {
    while (b != e) {
      if (c == *b) {
        return b;
      }
      ++b;
    }
    return e;
  };

  static_assert('A' < 'a');

  unsigned prio = 0;
  // use one mask per line, in a group of 3
  uint64_t masks[3] = { 0 };
  // the idea is to & together the masks and find the common position
  while (true) {
    const char* bsn;
    for (int i = 0; i < 3; ++i) {
      bsn = findbyte(p, end, '\n');
      const int64_t distance = bsn - p;
      // it's always %2 == 0
      for (int j = 0; j < distance; ++j) {
        const char current = *(p + j);
        const unsigned bitpos = current - 'A';
        masks[i] |= (0x01ul << bitpos); // set the i'th bit
      }
      p = bsn + 1; // skip \n
    }

    uint64_t result = masks[0] & masks[1] & masks[2];
    // use bsf (or equivalent) to count zeros
    uint64_t pos = __builtin_ctzl(result);

    // there are some chars between a and Z, plus, the problem wants lower case
    // letters to have lower prio
    constexpr unsigned offsetOfLowerCase = ('a' - 'Z' + 25);
    prio += (pos < 26 ? pos + 27 : pos - offsetOfLowerCase + 1);

    if (bsn >= end) {
      break;
    }

    for (int i = 0; i < 3; ++i) {
      masks[i] = 0u;
    }
  }

  printf("Sum is %u\n", prio);
}