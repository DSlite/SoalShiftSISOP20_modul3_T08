#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#define PORT 8080

struct player {
  char name[1000];
  int idsock;
  int health;
  int is_game;
  int is_auth;
  int is_ready;
  char input[1024];
};

struct player player_data[2];

void resetScreen(int i) {
  char buffer[1024];
  sprintf(buffer, "\e[H\e[2J");
  send(player_data[i].idsock, buffer, 1024, 0);
}

void prompt(int prompt_no, int i) {
  char buffer[1024];
  if (prompt_no == 0) {
    sprintf(buffer, "\e[30;47mTap Tap Berhadiah\e[m\n");
  }
  if (prompt_no == 1) {
    sprintf(buffer,
      "1. Login\n"
      "2. Register\n"
      "Choices: \e[s\n\e[u");
  }
  if (prompt_no == 11) {
    sprintf(buffer, "Username: \e[s\n\e[u");
  }
  if (prompt_no == 12) {
    sprintf(buffer, "Password: \e[s\n\e[u");
  }
  if (prompt_no == 101) {
    sprintf(buffer, "Login Gagal... (Press Anything to Continue)\n");
  }
  if (prompt_no == 121) {
    sprintf(buffer, "Register Berhasil... (Press Anything to Continue)\n");
  }
  if (prompt_no == 20) {
    sprintf(buffer, "Username: %s\n", player_data[i].name);
  }
  if (prompt_no == 2) {
    sprintf(buffer,
      "1. Find Match\n"
      "2. Logout\n"
      "Choices: \e[s\n\e[u");
  }
  if (prompt_no == 310) {
    sprintf(buffer, "\e[H\e[2JFinding Match.\n");
  }
  if (prompt_no == 311) {
    sprintf(buffer, "\e[H\e[2JFinding Match..\n");
  }
  if (prompt_no == 312) {
    sprintf(buffer, "\e[H\e[2JFinding Match...\n");
  }
  if (prompt_no == 32) {
    sprintf(buffer, "\e[H\e[2JMatch Found (Press Anything to Start)\n");
  }
  if (prompt_no == 401) {
    sprintf(buffer, "\e[H\e[2KGame start in 3...\n");
  }
  if (prompt_no == 402) {
    sprintf(buffer, "\e[H\e[2KGame start in 2...\n");
  }
  if (prompt_no == 403) {
    sprintf(buffer, "\e[H\e[2KGame start in 1...\n");
  }
  if (prompt_no == 410) {
    sprintf(buffer, "\e[s\e[H\e[2KYour Health: %d\n\e[u", player_data[i].health);
  }
  if (prompt_no == 411) {
    sprintf(buffer, "HIT!\n");
  }
  if (prompt_no == 412) {
    sprintf(buffer, "\e[3;1H");
  }
  if (prompt_no == 421) {
    sprintf(buffer, "Congratulation You Win! (Press Anything to Continue)\n");
  }
  if (prompt_no == 422) {
    sprintf(buffer, "Sorry You Lose... (Press Anything to Continue)");
  }
  if (prompt_no == 989) {
    sprintf(buffer, "\e[?25l");
  }
  if (prompt_no == 988) {
    sprintf(buffer, "\e[?25h");
  }
  if (prompt_no == 999) {
    sprintf(buffer, "Uhuk Start\n");
  }
  if (prompt_no == 998) {
    sprintf(buffer, "Uhuk Stop\n");
  }

  send(player_data[i].idsock, buffer, 1024, 0);

}

char *get_data(char buff[], int i) {
  memset(player_data[i].input, 0, 1024);
  while(strlen(player_data[i].input) == 0);
  strcpy(buff, player_data[i].input);
  memset(player_data[i].input, 0, 1024);
  // printf("From player_data[%d]: %s\n", i, buff);
  return buff;
}

int auth(char username[], char password[]) {
  FILE *fp = fopen("akun.txt", "r");
  int is_auth = 0;
  char buffer[1024];
  while (fgets(buffer, 1024, fp) != NULL && is_auth == 0) {
    char f_username[1024], f_password[1024];
    char *token = strtok(buffer, ".");
    strcpy(f_username, token);
    token = strtok(NULL, "\n");
    strcpy(f_password, token);
    if (strcmp(username, f_username) == 0 && strcmp(password, f_password) == 0)
      is_auth = 1;
  }
  fclose(fp);
  return is_auth;
}

void reg(char username[], char password[]) {
  FILE *fp = fopen("akun.txt", "a");
  fprintf(fp, "%s.%s\n", username, password);
  fclose(fp);

  char buffer[1024];
  fp = fopen("akun.txt", "r");
  printf("[username].[password]\n");
  while(fgets(buffer, 1024, fp) != NULL) {
    printf("%s", buffer);
  }
  fclose(fp);
}

void *server_main_routine(void *arg) {
  int i = *(int *)arg - 1;

  char buffer[1024];
  while(1) {
    resetScreen(i);
    if (player_data[i].is_auth == 0) {
      prompt(0, i);
      prompt(1, i);

      get_data(buffer, i);
      for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
      }

      if (strcmp(buffer, "1") == 0 || strcmp(buffer, "login") == 0) {
        char username[1024];
        char password[1024];
        prompt(11, i);
        get_data(username, i);
        prompt(12, i);
        get_data(password, i);
        player_data[i].is_auth = auth(username, password);
        if (player_data[i].is_auth == 0) {
          printf("Auth Failed\n");
          prompt(999, i);
          prompt(101, i);
          get_data(buffer, i);
          prompt(998, i);
        }
        if (player_data[i].is_auth == 1) {
          strcpy(player_data[i].name, username);
          printf("Auth Success\n");
        }
      } else if (strcmp(buffer, "2") == 0 || strcmp(buffer, "register") == 0) {
        char username[1024];
        char password[1024];
        prompt(11, i);
        get_data(username, i);
        prompt(12, i);
        get_data(password, i);
        reg(username, password);
        prompt(999, i);
        prompt(121, i);
        get_data(buffer, i);
        prompt(998, i);
      }

    } else if (player_data[i].is_ready == 0) {
      prompt(20, i);
      prompt(2, i);

      get_data(buffer, i);
      for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
      }

      if (strcmp(buffer, "1") == 0 || strcmp(buffer, "find match") == 0) {
        player_data[i].is_ready = 1;
      } else if (strcmp(buffer, "2") == 0 || strcmp(buffer, "logout") == 0) {
        player_data[i].is_auth = 0;
        memset(player_data[i].name, 0, 1000);
      }

    } else if (player_data[i].is_game == 0) {

      prompt(989, i);
      time_t now = time(NULL);
      while(player_data[!i].is_ready == 0) {
        prompt(310+((time(NULL)-now)%3), i);
        if (strlen(player_data[i].input) > 0) {
          player_data[i].is_ready = 0;
          memset(player_data[i].input, 0, 1000);
          break;
        }
      }

      if (player_data[i].is_ready) {
        prompt(999, i);
        prompt(32, i);
        get_data(buffer, i);
        prompt(998, i);
        player_data[i].is_game = 1;
      }
    } else {
      while(player_data[!i].is_game == 0);
      prompt(410, i);
      prompt(412, i);

      prompt(999, i);
      while(player_data[i].health > 0 && player_data[!i].health > 0) {
        if (strlen(player_data[i].input) > 0) {
          if (strcmp(player_data[i].input, " ") == 0) {
            player_data[!i].health -= 10;
            prompt(410, !i);
            prompt(411, i);
          }
          memset(player_data[i].input, 0, 1000);
        }
      }

      player_data[i].is_game = 0;
      player_data[i].is_ready = 0;

      if (player_data[i].health > 0) {
        prompt(421, i);
      } else {
        prompt(422, i);
      }
      get_data(buffer, i);
      prompt(998, i);
      prompt(988, i);

      player_data[i].health = 100;

    }
  }
}

void *server_scan_routine(void *arg) {
  int i = *(int *)arg - 1;
  char buffer[1024];
  while(1) {
    if (recv(player_data[i].idsock, buffer, 1024, 0) > 0) {
      strcpy(player_data[i].input, buffer);
    }
  }
}


int main(int argc, char const *argv[]) {
    FILE *fp = fopen("akun.txt", "a");
    fclose(fp);

    int server_fd, valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    pthread_t socket_threads[2][2];


    for (int i = 0; i < 2; i++) {
      if (i == 0) printf("\e[30;47mTap Tap Berhadiah\e[m\n");
      player_data[i].idsock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
      player_data[i].is_game = 0;
      player_data[i].is_auth = 0;
      player_data[i].is_ready = 0;
      player_data[i].health = 100;
      pthread_create(&socket_threads[i][0], NULL, &server_main_routine, (void *)&i);
      pthread_create(&socket_threads[i][1], NULL, &server_scan_routine, (void *)&i);
    }


    for (int i = 0; i < 2; i++) {
      pthread_join(socket_threads[i][0], NULL);
      pthread_join(socket_threads[i][1], NULL);
    }

}
