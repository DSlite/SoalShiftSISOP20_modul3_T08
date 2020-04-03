#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/shm.h>
#include <termios.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#define PORT 8080

/* TERMIOS */
static struct termios old, current;

void initTermios()
{
  tcgetattr(0, &old);
  current = old;
  current.c_lflag &= ~ICANON;
  current.c_lflag &= ~ECHO;
  tcsetattr(0, TCSANOW, &current);
}

void resetTermios(void)
{
  tcsetattr(0, TCSANOW, &old);
}

char getch()
{
  char ch;
  initTermios();
  ch = getchar();
  resetTermios();
  return ch;
}

/* ROUTINE */

pthread_t printer;
pthread_t scanner;

int is_game = 0;

void *print_routine(void *arg) {
  int sock = *(int *)arg;
  char buffer[1024] = {0};
  while (1) {
    memset(buffer, 0, 1024);
    if(recv(sock, buffer, 1024, 0) > 1) {
      char buffer2[1024];
      strcpy(buffer2, buffer);
      char *token = strtok(buffer2, "\n");
      if (strcmp(token, "Uhuk Start") == 0) {
        is_game = 1;
        resetTermios();
        pthread_cancel(scanner);
      } else if (strcmp(token, "Uhuk Stop") == 0) {
        is_game = 0;
        resetTermios();
        pthread_cancel(scanner);
      } else {
        printf("%s", buffer);
      }
    }
  }
}

void *scan_routine(void *arg) {
  int sock = *(int *)arg;
  char c[1024];
  int i = 0;
  while(1) {
    if (!is_game) {
      do {
        char buff = getch();

        if (buff == 127 && i != 0) {
          printf("\e[D\e[K");
          c[i--] = 0;
        } else if (buff == 10 || !iscntrl(buff)) {
          c[i] = buff;
          printf("%c", c[i++]);
        }
      } while (c[i-1] != '\n' && c[i-1] != EOF);
      c[--i] = 0;
      i = 0;
      send(sock, c, 1024, 0);
    } else {
      char buff = getch();
      c[0] = buff;
      c[1] = 0;
      send(sock, c, 1024, 0);
    }
  }
}


void sighandler(int signum) {
  printf("\e[2J\e[?25h\e[?1049l");
  resetTermios();
  exit(1);
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, sighandler);
    printf("\e[?1049h");
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) {

        printf("\e[2J\e[?25h\e[?1049l");
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {

        printf("\e[2J\e[?25h\e[?1049l");
        printf("\nConnection Failed \n");
        return -1;
    }


    pthread_create(&printer, NULL, &print_routine, (void *)&sock);
    pthread_create(&scanner, NULL, &scan_routine, (void *)&sock);
    while(1) {
      if (pthread_join(scanner, NULL) == 0) {
        pthread_create(&scanner, NULL, &scan_routine, (void *)&sock);
      }
    }
    pthread_join(printer, NULL);

    printf("\e[2J\e[?25h\e[?1049l");

}
