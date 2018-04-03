//
// Created by k.leyfer on 12.10.2017.
//

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>

#include "exec_utils.h"
#include "../dirty/common/logging.h"

static void set_timer(long timeout_ms)
{
  struct itimerval new_t;

  new_t.it_value.tv_sec = timeout_ms / 1000;
  new_t.it_value.tv_usec = (timeout_ms % 1000) * 1000;
  new_t.it_interval.tv_sec = 0;
  new_t.it_interval.tv_usec = 0;

  setitimer(ITIMER_REAL, &new_t, NULL);
}

static int
run_child_process_with_timeout_args(const char* path, const char** args, int* in_fd, int* out_fd,
                                    long timeout_ms)
{
  int child_in_pipe[2];
  pipe(child_in_pipe);
  int child_out_pipe[2];
  pipe(child_out_pipe);

  pid_t pid = fork();
  if (pid == 0)
  {
    if (timeout_ms > 0)
    {
      set_timer(timeout_ms);
    }

    close(child_in_pipe[1]);
    close(child_out_pipe[0]);

    dup2(child_in_pipe[0], STDIN_FILENO);
    dup2(child_out_pipe[1], STDOUT_FILENO);
    dup2(child_out_pipe[1], STDERR_FILENO);
    if (execv(path, args) == -1)
    {
      LOGV("Unable to exec %s!", path);
      exit(EXIT_FAILURE);
    }
  }
  else if (pid == -1)
  {
    LOGV("Unable to fork: %s!", strerror(errno));
    close(child_in_pipe[0]);
    close(child_in_pipe[1]);
    close(child_out_pipe[0]);
    close(child_out_pipe[1]);
    return -1;
  }

  close(child_in_pipe[0]);
  close(child_out_pipe[1]);

  *in_fd = child_in_pipe[1];
  *out_fd = child_out_pipe[0];

  return pid;
}

int run_child_process(const char* path, const char** args, int* in_fd, int* out_fd)
{
  return run_child_process_with_timeout_args(path, args, in_fd, out_fd, 0);
}

int run_child_process_with_timeout(const char* path, const char** args, int* in_fd, int* out_fd,
                                   long timeout_ms)
{
  return run_child_process_with_timeout_args(path, args, in_fd, out_fd, timeout_ms);
}
