#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pty.h>
#include <utmp.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>


#define _SC(a) if ((a) == -1) {perror(NULL); exit(202);}

void getmessage(int fd, char *buffer, const int bufsize, int *count) {
  time_t starttime;
  struct timeval timeout;
  fd_set rfds;
  int sizeavail, tmpint;

  sizeavail = bufsize - 3;
  *count = 0;
  *count = read(fd, buffer, 1);

  if (*count < 1) {
    exit(255);
  }
  
  sizeavail -= *count;
  starttime = time(NULL);
  
  while ((time(NULL) - starttime < 5) &&
	 sizeavail) {
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    timeout.tv_usec = 0;
    timeout.tv_sec = 5 - (time(NULL) - starttime);
    if (timeout.tv_sec < 1) {
      timeout.tv_sec = 1;
    }
    if (select(fd + 1, &rfds, NULL, NULL, &timeout)) {
      tmpint = read(fd, buffer + (*count), sizeavail);
      if (tmpint < 1) {
	exit(255);
      }
      *count += tmpint;
      sizeavail -= tmpint;
    } else {
      break;
    }
  }
}
    
int main(int argc, char *argv[]) {
  int masterfd, slavefd;
  char ptyname[1024];
  char buffer[10240];
  struct utmp utentry;
  int count;

  _SC(openpty(&masterfd, &slavefd, ptyname, NULL, NULL));
  printf("Allocated PTY at %s\n", ptyname);
  _SC(dup2(slavefd, 0));
  printf("Done dup2\n");
  strncpy(utentry.ut_user, getpwuid(getuid())->pw_name, UT_NAMESIZE);
  utentry.ut_user[UT_NAMESIZE-1] = 0;
  strncpy(utentry.ut_host, "N/A", UT_HOSTSIZE);
  utentry.ut_host[UT_HOSTSIZE-1] = 0;
  gettimeofday(&(utentry.ut_tv), NULL);
  login(&utentry);
  printf("Done login\n");
  
  while (1) {
    getmessage(masterfd, buffer, sizeof(buffer), &count);
    printf("----- START OF MESSAGE -----\n");
    fflush(stdout);
    write(1, buffer, count);
    printf("----- END OF MESSAGE -----\n");

  }
  return 0;
}

