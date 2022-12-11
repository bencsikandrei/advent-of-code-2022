#include "../at_scope_exit.h"

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

  int scoresForRPS[]{ 1, 2, 3 };
  int scoresForOutcome[]{ 0, 3, 6 };
  int score = 0;
  // X lose, Y draw, Z win
  // 0 lose, 1 draw, 2 win
  int iLose[]{ 2, 0, 1 };
  int iWin[]{1, 2, 0};
  while (p < end) {
    // char space char \n
    // last one has no \n
    char hePlayed = *p - 'A';
    p += 2; // space
    char weMust = *p - 'X';
    p += 2;
    switch (weMust) {
      case 0: // lose 1 - 0, 2 - 1, 0 - 2
        score += scoresForRPS[iLose[hePlayed]];
        break;
      case 1:
        score += 3 + scoresForRPS[hePlayed];
        break;
      case 2:
        score += 6 + scoresForRPS[iWin[hePlayed]];
    }
  }

  printf("Score is %d\n", score);
}