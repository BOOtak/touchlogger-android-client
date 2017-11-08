//
// Created by k.leyfer on 12.10.2017.
//

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "su_utils.h"
#include "../dirty/common/logging.h"
#include "exec_utils.h"

static int on_error(int in_fd, int out_fd)
{
  close(in_fd);
  close(out_fd);
  return -1;
}

static int is_output_from_su(const char* output)
{
  int uid = -1, gid = -1;
  char* uid_pointer = strstr(output, "uid=");
  if (uid_pointer == NULL)
  {
    return -1;
  }

  sscanf(uid_pointer, "uid=%d", &uid);
  if (uid != 0)
  {
    return -1;
  }

  char* gid_pointer = strstr(output, "gid=");
  if (gid_pointer == NULL)
  {
    return -1;
  }

  sscanf(gid_pointer, "gid=%d", &gid);
  if (gid != 0)
  {
    return -1;
  }

  char* success_pointer = strstr(output, "success!");
  if (success_pointer == NULL)
  {
    return -1;
  }

  return 0;
}

int check_su_binary(const char* path)
{
  struct stat st;
  if (stat(path, &st) == -1)
  {
    LOGV("Unable to stat \"%s\": %s!", path, strerror(errno));
    return -1;
  }

  int child_in_fd = -1;
  int child_out_fd = -1;
  int child_timeout_ms = 30000; // In case user has SuperSU installed and should confirm root access
  const char* args[] = {path, 0};
  int pid = run_child_process_with_timeout(path, args, &child_in_fd, &child_out_fd,
                                           child_timeout_ms);
  if (pid == -1)
  {
    LOGV("Unable to run child process: %s!", strerror(errno));
    return -1;
  }

  const char* command =
      "id && echo success!\n"
          "echo exiting... && exit\n";

  ssize_t written = write(child_in_fd, command, strlen(command));

  int status = 0;
  if (waitpid(pid, &status, 0) == -1)
  {
    LOGV("Unable to wait for child pid: %s!", strerror(errno));
    return on_error(child_in_fd, child_out_fd);
  }

  if (!WIFEXITED(status) || (WEXITSTATUS(status) != 0))
  {
    LOGV("Abnormal child exit detected, abort.");
    return on_error(child_in_fd, child_out_fd);
  }

  if (written != strlen(command))
  {
    LOGV("Unable to write command to child process: %s!", strerror(errno));
    return on_error(child_in_fd, child_out_fd);
  }

  char buf[BUFSIZ];
  memset(buf, 0, BUFSIZ);
  ssize_t readed = read(child_out_fd, buf, BUFSIZ); // uid=0(root) gid=0(root) <.....> success!
  if (readed <= 0)
  {
    LOGV("Unable to read child output: %s!", strerror(errno));
    return on_error(child_in_fd, child_out_fd);
  }

  LOGV("Child output: %s", buf);

  close(child_in_fd);
  close(child_out_fd);
  return is_output_from_su(buf);
}

int check_suid_bit(const char* path)
{
  struct stat st;

  if (stat(path, &st) == -1)
  {
    LOGV("Unable to stat \"%s\": %s!", path, strerror(errno));
    return -1;
  }

  if ((st.st_mode & S_ISUID) == S_ISUID)
  {
    return 0;
  }

  return -1;
}
