#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <termios.h>

static struct termios old, current;

void initTermios() {
  tcgetattr(0, &old);
  current = old;
  current.c_lflag &= ~ICANON;
  current.c_lflag &= ~ECHO;
  tcsetattr(0, TCSANOW, &current);
}

void resetTermios(void) {
  tcsetattr(0, TCSANOW, &old);
}

char getch() {
  char ch;
  initTermios();
  ch = getchar();
  resetTermios();
  return ch;
}

struct trainer {
  char input;
  char output[1024];
  int mode;
  int menu;
  int powder_counter;
  int is_appearing;
  int is_searching;
  int is_escaping;
};

struct pokemon {
  char name[50];
  int type;
  int escape_rate;
  int capture_rate;
  int pokedollar;
  int ap;
  int is_shiny;
  int is_locked;
};

struct pokedex {
  struct pokemon *list[7];
  int size;
};

struct item_stock {
  int powder;
  int pokeball;
  int berry;
  int pokedollar;
};

struct trainer trainer;
struct pokedex pokedex;
struct item_stock item_stock;

struct pokemon *random_pokemon;
struct item_stock *shop_stock;
int random_pokemon_id, shop_stock_id;

int add_pokemon_pokedex(struct pokemon *pokemon) {
  if (pokedex.size == 7) return 0;
  pokedex.list[pokedex.size++] = pokemon;
  return 1;
}

void remove_pokemon_pokedex(int i) {
  free(pokedex.list[i]);
  for (int num = i; num < pokedex.size-1; num++) {
    pokedex.list[num] = pokedex.list[num+1];
  }
  pokedex.size--;
}

void *print_routine(void *arg) {
  pthread_detach(pthread_self());
  memset(trainer.output, 0, 1024);
  while(1) {
    if (strlen(trainer.output) > 0) {
      printf("%s", trainer.output);
      memset(trainer.output, 0, 1024);
    }
  }
}

void *scan_routine(void *arg) {
  pthread_detach(pthread_self());
  while(1) {
    char buffer = getch();
    trainer.input = buffer;
  }
}

void *powder_routine(void *arg) {
  pthread_detach(pthread_self());
  trainer.powder_counter++;
  sleep(10);
  trainer.powder_counter--;
}

void *pokemon_escape_routine(void *arg) {
  pthread_detach(pthread_self());
  while(trainer.mode == 1) {
    sleep(20);
    if (trainer.mode == 0) break;
    int num = rand()%100;
    if (num < random_pokemon->escape_rate) trainer.is_escaping = 1;
  }
}

void *search_pokemon_routine(void *arg) {
  sleep(10);
  int num = rand()%100;
  if (num < 100) {
    random_pokemon->is_locked = 1;
    trainer.is_appearing = 1;
    trainer.is_searching = 0;
  }
}

void *pokemon_routine(void *arg) {
  pthread_detach(pthread_self());
  int i = *(int *)arg;
  while(pokedex.list[i]->is_locked) {
    sleep(10);
    if (trainer.mode == 0) {
      pokedex.list[i]->ap -=10;
      if (pokedex.list[i]->ap == 0) {
        int num = rand()%100;
        if (num < 10) {
          pokedex.list[i]->ap = 50;
        } else {
          remove_pokemon_pokedex(i);
          break;
        }
      }
    }
  }
}

void *print_search_routine (void *arg) {
  pthread_detach(pthread_self());
  while(trainer.is_searching) {
    while(strlen(trainer.output) > 0);
    if (trainer.is_searching) {
      sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching.   (Press to Cancel)\n\e[u");
    } else {
      break;
    }
    sleep(1);
    while(strlen(trainer.output) > 0);
    if (trainer.is_searching) {
      sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching..  (Press to Cancel)\n\e[u");
    } else {
      break;
    }
    sleep(1);
    while(strlen(trainer.output) > 0);
    if (trainer.is_searching) {
      sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching... (Press to Cancel)\n\e[u");
    } else {
      break;
    }
    sleep(1);
  }
}

void sighandler(int signum) {
  shmdt(random_pokemon);
  shmctl(random_pokemon_id, IPC_RMID, NULL);
  shmdt(shop_stock);
  shmctl(shop_stock_id, IPC_RMID, NULL);
  printf("\e[H\e[2J\e[m\e[25h\e[?1049l");
  resetTermios();
  exit(1);
}

int main() {
  srand(time(NULL));
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);

  random_pokemon_id = shmget(1337, sizeof(struct pokemon), IPC_CREAT | 0666);
  random_pokemon = shmat(random_pokemon_id, NULL, 0);
  shop_stock_id = shmget(7331, sizeof(struct item_stock), IPC_CREAT | 0666);
  shop_stock = shmat(shop_stock_id, NULL, 0);

  trainer.mode = 0;
  trainer.menu = 1;
  trainer.powder_counter = 0;
  trainer.is_searching = 0;
  trainer.is_appearing = 0;
  trainer.is_escaping = 0;
  pthread_t tid[2];
  printf("\e[?1049h");
  pthread_create(&tid[0], NULL, &print_routine, NULL);
  pthread_create(&tid[1], NULL, &scan_routine, NULL);

  time_t searching;
  pthread_t search_tid;

  while(1) {
    while(strlen(trainer.output) == 0);
    sprintf(trainer.output, "\e[H\e[2J");
    if (trainer.mode == 0) {
      if (trainer.menu == 1) {

        // STATIC PART

        while(strlen(trainer.output) > 0);
        sprintf(trainer.output,
          "--\e[31mPoke\e[37mZone\e[m--\n"
          "------------\n");

        while(strlen(trainer.output) > 0);
        if (trainer.is_searching) {
          sprintf(trainer.output, "\n");
        } else {
          sprintf(trainer.output, "1. Cari Pokemon\n");
        }

        while(strlen(trainer.output) > 0);
        sprintf(trainer.output,
          "2. Pokedex\n"
          "3. Shop\n"
          "Choice: \e[s\n\e[u");

        // DYNAMIC PART w/ Searching

        if (trainer.is_searching) {
          // searching = time(NULL);
          pthread_create(&search_tid, NULL, &print_search_routine, NULL);
          while(trainer.is_searching) {
            // DYNAMIC OUTPUT
            // time_t diff = (time(NULL) - searching)%3;
            // while(strlen(trainer.output) > 0);
            // if (diff == 0) {
            //   sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching.   (Press to Cancel)\e[u");
            // } else if (diff == 1) {
            //   sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching..  (Press to Cancel)\e[u");
            // } else if (diff == 2) {
            //   sprintf(trainer.output, "\e[s\e[3;1H\e[2K1. Searching... (Press to Cancel)\e[u");
            // }

            // DYNAMIC INPUT
            if (trainer.input != 0) {
              char buffer = trainer.input;
              trainer.input = 0;
              if (buffer == '1') {
                pthread_cancel(search_tid);
                trainer.is_searching = 0;
              } else if (buffer == '2') {
                trainer.menu = 2;
              } else if (buffer == '3') {
                trainer.menu = 3;
              }
            }

          }
        } else {
          while(trainer.input == 0);
          char buffer = trainer.input;
          trainer.input = 0;
          if (buffer == '1') {
            trainer.is_searching = 1;
            pthread_create(&search_tid, NULL, &search_pokemon_routine, NULL);
          } else if (buffer == '2') {
            trainer.menu = 2;
          } else if (buffer == '3') {
            trainer.menu = 3;
          }
        }

      } else if (trainer.menu == 2) {
        // BELOM BELOM BELOM BELOM
        while(strlen(trainer.output) != 0);
        sprintf(trainer.output,
          "--\e[31mPoke\e[37mDex\e[m--\n"
          "-----------\n");

        if (trainer.is_searching) {
          while(trainer.is_searching) {
            // for (int i = 0; i < )
          }
        } else {

        }
      }
    }
  }
}
