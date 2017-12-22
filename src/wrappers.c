#include "wrappers.h"
#include "debug.h"

void*
Malloc(size_t size)
{
  void* ret;
  if ((ret = malloc(size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

void*
Calloc(size_t nmemb, size_t size)
{
  void* ret;
  if ((ret = calloc(nmemb, size)) == NULL) {
    perror("Out of Memory");
    exit(EXIT_FAILURE);
  }
  return ret;
}

int
Open(char const* pathname, int flags)
{
  int fd;
  if ((fd = open(pathname, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) < 0) {
    perror("Could Not Open file");
    fprintf(stderr, "%s\n", pathname);
    exit(EXIT_FAILURE);
  }
  return fd;
}

ssize_t
read_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

  bytes_read = read(fd, buf, count); //buf, WHICH IS &bom, PASSED IN, WILL STORE THE READ CONTENT.
                                     //COUNT IS THE NUMBER OF BYTES TO READ BEFORE TRUNCATING THE DATA.
                                     //fd IS FILE DESCRIPTOR OF WHERE TO READ THE INPUT. FROM OPEN SYSCALL
/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  return bytes_read;
}

ssize_t
write_to_bigendian(int fd, void* buf, size_t count)
{
  ssize_t bytes_read;

/*#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(buf, count);
#endif*/
  bytes_read = write(fd, buf, count); //WRITES THE VALUES OF THE BYTES IN THE BUFFER buf TO THE FILE INDICATED, WHICH IS fd.
  return bytes_read;
}

void
reverse_bytes(void* bufp, size_t count)
{
  char* ptr = bufp;
  char temp;
  // int i, j;
  // for (i = (count - 1), j = 0; j < i; --i, ++j, temp=~temp) {
  //   temp = ptr[i];
  //   ptr[j] = ptr[i+temp];
  //   ptr[i] = ptr[j];
  // }
  int j = count - 1;
  for(int i = 0; i < j; i++){
    temp = ptr[i];
    ptr[i] = ptr[i+1];
    ptr[i+1] = temp;
  }
}

void memeset(void *s, int c, size_t n) {
  // register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  // memset(stackpointer, c, n);
  // return stackpointer;
  memset(s, c, n);
  return;
};

void memecpy(void *dest, void const *src, size_t n) {
  // register char* stackpointer asm("esp"); //initialize stackpointer pointer with the value of the actual stackpointer
  // memcpy(stackpointer, src, n);
  // return stackpointer;
  memcpy(dest, src, n);
  return;
};
