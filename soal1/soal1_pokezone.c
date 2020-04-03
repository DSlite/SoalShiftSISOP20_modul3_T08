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

struct stock {
  int powder;
  int pokeball;
  int berry;
  int pokedollar;
};

struct pokemon *random_pokemon;
struct stock *shop_stock;
int random_pokemon_id, shop_stock_id;

void *update_random_pokemon(void *arg) {
  pthread_detach(pthread_self());

  char *normal_list[] = {"Bulbasaur", "Charmander", "Squirtle", "Rattata", "Caterpie"};
  char *rare_list[] = {"Pikachu", "Eevee", "Jigglypuff", "Snorlax", "Dragonite"};
  char *legendary_list[] = {"Mew", "Mewtwo", "Moltres", "Zapdos", "Articuno"};

  while(1) {
    while(!random_pokemon->is_locked)
      random_pokemon->is_shiny = 0;
      random_pokemon->ap = 100;

      int num = rand()%100;
      if (num < 80) {
        num = rand()%5;
        strcpy(random_pokemon->name, normal_list[num]);
        random_pokemon->type = 1;
        random_pokemon->escape_rate = 5;
        random_pokemon->capture_rate = 70;
        random_pokemon->pokedollar = 80;
      } else if (num < 95) {
        num = rand()%5;
        strcpy(random_pokemon->name, rare_list[num]);
        random_pokemon->type = 2;
        random_pokemon->escape_rate = 10;
        random_pokemon->capture_rate = 50;
        random_pokemon->pokedollar = 100;
      } else {
        num = rand()%5;
        strcpy(random_pokemon->name, legendary_list[num]);
        random_pokemon->type = 3;
        random_pokemon->escape_rate = 10;
        random_pokemon->capture_rate = 50;
        random_pokemon->pokedollar = 100;
      }

      num = rand()%8000;
      if (num == 1337) {
        random_pokemon->is_shiny = 1;
      }

      sleep(1);
    }
  }
}

void *update_shop_stock(void *arg) {
  pthread_detach(pthread_self());

  shop_stock->powder = 100;
  shop_stock->pokeball = 100;
  shop_stock->berry = 100;

  while(1) {
    sleep(10);
    shop_stock->powder += 10;
    shop_stock->pokeball += 10;
    shop_stock->berry += 10;
    if (shop_stock->powder > 200) shop_stock->powder = 200;
    if (shop_stock->pokeball > 200) shop_stock->pokeball = 200;
    if (shop_stock->berry > 200) shop_stock->berry = 200;
  }
}


void sighandler(int signum) {
  shmdt(random_pokemon);
  shmctl(random_pokemon_id, IPC_RMID, NULL);
  shmdt(shop_stock);
  shmctl(shop_stock_id, IPC_RMID, NULL);
  printf("\e[H\e[2J\e[m\e[?1049l");
  exit(1);
}

int main() {
  srand(time(NULL));
  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);

  random_pokemon_id = shmget(1337, sizeof(struct pokemon), IPC_CREAT | 0666);
  random_pokemon = shmat(random_pokemon_id, NULL, 0);
  memset(random_pokemon, 0, sizeof(struct pokemon));
  random_pokemon->is_locked = 0;
  shop_stock_id = shmget(7331, sizeof(struct stock), IPC_CREAT | 0666);
  shop_stock = shmat(shop_stock_id, NULL, 0);
  memset(shop_stock, 0, sizeof(struct stock));

  pthread_t random_pokemon_thread, shop_stock_thread;
  pthread_create(&random_pokemon_thread, NULL, &update_random_pokemon, NULL);
  pthread_create(&shop_stock_thread, NULL, &update_shop_stock, NULL);

  printf("\e[?1049h");
  while(1) {
    printf("\e[H\e[2J");
    printf("--\e[31mPoke\e[37mZone\e[m--\n");
    printf("------------\n");
    printf("1.Shutdown\n");
    printf("Choice: ");
    char buffer[100];
    scanf("%s", buffer);
    for (int i = 0; buffer[i]; i++) {
      buffer[i] = tolower(buffer[i]);
    }
    if (strcmp(buffer, "1") == 0 || strcmp(buffer, "shutdown") == 0) {
      pid_t child_id = fork();
      if (child_id == 0) {
        child_id = fork();
        if (child_id == 0) {
          char *argv[] = {"pkill", "soal1_traizone", NULL};
          execv("/usr/bin/pkill", argv);
        }
        char *argv[] = {"pkill", "soal1_pokezone", NULL};
        execv("/usr/bin/pkill", argv);
      }
    }
  }
}
