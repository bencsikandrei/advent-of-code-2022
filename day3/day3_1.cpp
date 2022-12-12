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

  uint64_t mask = 0;
  unsigned prio = 0;
  static_assert('A' < 'a');

  while (true) {
    const char* bsn = findbyte(p, end, '\n');
    const int64_t distance = bsn - p;
    // it's always %2 == 0
    for (int i = 0; i < distance / 2; ++i) {
      const char current = *(p + i);
      const unsigned bitpos = current - 'A';
      mask |= (0x01ul << bitpos); // set the i'th bit
    }

    for (int i = distance / 2; i < distance; ++i) {
      const char current = *(p + i);
      const unsigned bitpos = current - 'A';

      const uint64_t bitMaskToCheck = 0x01ul << bitpos;
      if ((mask & bitMaskToCheck) != 0) {
        prio += (current < 'Z') ? (current - 'A' + 27) : (current - 'a' + 1);
      }
    }

    if (bsn == end) {
      break;
    }

    p = bsn + 1;
    mask = 0;
  }

  printf("Sum is %u\n", prio);
}