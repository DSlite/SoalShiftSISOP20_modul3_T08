#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int matA[4][2] = {
  {1, 1},
  {2, 2},
  {1, 1},
  {2, 2}
};
int matB[2][5] = {
  {1, 4, 1, 2, 1},
  {1, 2, 1, 4, 1}
};
int matC[4][5];

struct args {
  int i;
  int j;
};

void *kali(void* arg) {
  int i = ((struct args*)arg)->i;
  int j = ((struct args*)arg)->j;

  for (int k = 0; k < 2; k++) {
    matC[i][j] += matA[i][k] * matB[k][j];
  }
}

int main() {

  pthread_t tid[4][5];

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      struct args *index = (struct args *)malloc(sizeof(struct args));
      index->i = i;
      index->j = j;
      pthread_create(&tid[i][j], NULL, &kali, (void *)index);
    }
  }

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      pthread_join(tid[i][j], NULL);
    }
  }

  printf("Matriks :\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      printf("%4d", matC[i][j]);
    }
    printf("\n");
  }

  key_t key = 1337;
  int *value;

  int shmid = shmget(key, sizeof(matC), IPC_CREAT | 0666);
  value = shmat(shmid, NULL, 0);

  int* p = (int *)value;

  memcpy(p, matC, 80);

  shmdt(value);
}
