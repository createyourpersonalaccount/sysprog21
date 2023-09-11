/* Use this file to modify the ioctl of the device driver. */
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define PROGRAM_NAME "modify_ioctl"

int main(int argc, char **argv) {
  int fd;
  if(argc < 2) {
    fprintf(stderr, "%s FILENAME\n", argv[0] ? argv[0] : PROGRAM_NAME);
    return EXIT_FAILURE;
  }
  if((fd = open(argv[1], 0)) == -1) {
    perror("open");
    return EXIT_FAILURE;
  }
  ioctl(fd, 3, '\xAB');
  close(fd);
  return 0;
}
