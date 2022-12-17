#include "../at_scope_exit.h"
#include "../io.h"

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

  const auto overlaps =
    [](long int a1, long int b1, long int a2, long int b2) {
      // [a1 --- b1]      1
      // [   a2 ---- b2]
      
      // [   a1 - b1]      2
      // [a2 ------ b2]

      return a1 <= b2 && a2 <= b1;
      
    };
  unsigned overlaped_pairs = 0;
  while (p != end) {
    // 14-50,14-50
    // 43-44,43-87
    // int dash int comma int dash int \n
    // last line same, but not \n
    unsigned a1, b1, a2, b2;

    a1 = afb::extract_unsigned(&p, end);
    ++p; // dash
    b1 = afb::extract_unsigned(&p, end);
    ++p; // comma

    a2 = afb::extract_unsigned(&p, end);
    ++p; // dash
    b2 = afb::extract_unsigned(&p, end);

    overlaped_pairs += overlaps(a1, b1, a2, b2);

    if (p == end) {
      break;
    }
    ++p; // \n
  }

  printf("Fully contained pairs: %u\n", overlaped_pairs);
}