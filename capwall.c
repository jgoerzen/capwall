#include <security/pam_appl.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pty.h>

#define _PC(a) pamerror = (a); if (pamerror != PAM_SUCCESS) { \
  printf("PAM error: %s\n", pam_strerror(pamh, pamerror)); \
  exit(200); };

#define _SC(a) if ((a) == -1) {perror(NULL); exit(202);}

int pamerror;

void startpam(pam_handle_t **pamh_p) {
  uid_t myuid = getuid();
  struct passwd *pwd = getpwuid(myuid);
  struct pam_conv conv;
  char *username = pwd->pw_name;
  
  pamerror = pam_start("capwall", username, &conv, pamh_p);
  printf("Started PAM\n");
  if (pamerror != PAM_SUCCESS) {
    printf("pam_start failed: %s\n", pam_strerror(*pamh_p, pamerror));
    exit(201);
  }
}

int main(int argc, char *argv[]) {
  pam_handle_t *pamh;
  int masterfd, slavefd;
  char ptyname[1024];
  char buffer[10240];
  int count;

  _SC(openpty(&masterfd, &slavefd, ptyname, NULL, NULL));
  printf("Allocated PTY at %s\n", ptyname);
  startpam(&pamh);

  _PC(pam_setcred(pamh, PAM_ESTABLISH_CRED));
  printf("Setted creds\n");
  _PC(pam_set_item(pamh, PAM_TTY, ptyname));
  printf("Setted item\n");
  _PC(pam_open_session(pamh, 0));
  printf("Opened session\n");
  
  while (1) {
    count = read(masterfd, buffer, sizeof(buffer) - 5);
    if (count < 1) {
      break;
    }
    buffer[count] = 0;
    printf("Received: %s", buffer);
  }
}


    
