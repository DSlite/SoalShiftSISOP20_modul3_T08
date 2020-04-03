#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int matrix[4][5];
long hasil[4][5];

struct args {
  int i;
  int j;
};

void *factorial(void* arg) {
  int i = ((struct args*)arg)->i;
  int j = ((struct args*)arg)->j;
  long hasilEl = 1;
  for (int n = 1; n <= matrix[i][j]; n++) hasilEl += (long)n;
  hasil[i][j] = hasilEl;
}

int main() {
  key_t key = 1337;
  int *value;
  int shmid = shmget(key, 80, IPC_CREAT | 0666);
  value = shmat(shmid, NULL, 0);

  int* p = (int *)value;
  memcpy(matrix, p, 80);

  shmdt(value);
  shmctl(shmid, IPC_RMID, NULL);

  pthread_t tid[4][5];

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      struct args *index = (struct args *)malloc(sizeof(struct args));
      index->i = i;
      index->j = j;
      pthread_create(&tid[i][j], NULL, &factorial, (void *)index);
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
      printf("%20ld", hasil[i][j]);
    }
    printf("\n");
  }

}
