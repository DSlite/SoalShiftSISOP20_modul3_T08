# Shift 3 SISOP 2020 - T08
Penyelesaian Soal Shift 3 Sistem Operasi 2020\
Kelompok T08
  * I Made Dindra Setyadharma (05311840000008)
  * Muhammad Irsyad Ali (05311840000041)

---
## Table of Contents
* [Soal 1](#soal-1)
* [Soal 2](#soal-2)
* [Soal 3](#soal-3)
* [Soal 4](#soal-4)
  * [Soal 4.a.](#soal-4a)
  * [Soal 4.b.](#soal-4b)
  * [Soal 4.c.](#soal-4c)

---

## Soal 1
Source Code : [soal1](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/tree/master/soal1)

**Deskripsi:**
Membuat game text-based yang menyerupai Pokemon GO dengan menggunakan IPC-shared memory, thread, dan fork-exec dengan ketentuan yang berlaku

**Asumsi Soal:**
Program akan dijalankan terlebih dahulu melalui pokezone untuk membuat random pokemon tiap detik dan menyimpannya dalam shared memory, mengupdate shop stock setiap 10 detik dan menyimpan shop stock di shared memory, dan menyediakan fitur untuk men-*shutdown* pokezone dan traizone. Pada traizone akan terdapat 2 mode yaitu mode normal, dan mode capture. Pada mode normal akan terdiri dari menu menu yang dapat dituju, seperti pokedex dan shop, dan satu menu khusus untuk melakukan find pokemon yang dimana akan menjalankan thread untuk mencari pokemon. Ketika pokemon ketemu, maka akan membaca data pada shared memory dan akan memunculkan tampilannya dan akan masuk ke dalam capture mode. Dalam capture mode akan terdapat thread untuk menghitung kemungkinan pokemon kabur. Capture mode memiliki 3 menu yaitu untuk menangkap pokemon, menggunakan item atau keluar. Pada pokedex mode normal dapat melihat pokemon yang dimiliki, dan pada shop dapat membeli item sesuai dengan stock yang terdapat pada shared memory.

*Catatan:*
* *Untuk soal1_traizone.c belum terselesaikan*
* *Jadi untuk soal1_pokezone.c telah selesai secara teori*

### soal1_pokezone.c

```c
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
```

* `#include <stdio.h>` Library fungsi input output (e.g. printf(), scanf())
* `#include <sys/types.h>` Library tipe data khusus (e.g. pid_t)
* `#include <stdlib.h>` Library untuk fungsi umum  (e.g. exit())
* `#include <string.h>` Library untuk manipulasi string (e.g. strcpy())
* `#include <unistd.h>` Library untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <sys/ipc.h>` Library untuk menggunakan system call IPC pada linux
* `#include <sys/shm.h>` Library untuk menggunakan system call shared memory pada linux (e.g. shmat(), shmget())
* `#include <pthread.h>` Library untuk operasi thread (e.g. pthread_create(), pthread_exit())
* `#include <ctype.h>` Library untuk fngsi tipe (e.g. tolower())
* `#include <signal.h>` Library untuk mengatur fungsi signal yang datang (e.g. signal())

#### Routine dan Fungsi

***struct pokemon* & *struct stock***

```c
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
```

* Sebelum program utama dijalankan, akan didefinisikan `struct pokemon` untuk random pokemon dengan attribute:
  * `name` menyimpan nama pokemon.
  * `type` menyimpan tipe pokemon (normal/rare/legendary).
  * `escape_rate` menyimpan escape_rate dari pokemon.
  * `capture_rate` menyimpan capture_rate dari pokemon.
  * `pokedollar` menyimpan harga jual pokemon.
  * `ap` menyimpan AP dari pokemon.
  * `is_shiny` menyimpan status apakah pokemon shiny atau bukan.
  * `is_locked` menyimpan status apakah pokemon sedang tidak boleh diganti atau tidak.
* Dan juga didefinisikan `struct stock` untuk shop stock dengan attribute:
  * `powder` menyimpan stock lullaby powder.
  * `pokeball` menyimpan stock pokeball.
  * `berry` menyimpan stock berry.
  * `pokedollar` menyimpan pokedollar yang dimiliki.
* Lalu akan diset `*random_pokemon` dan `*shop_stock` yang nantinya akan digunakan untuk menyimpan random pokemon dan shop stock yang akan terdapat pada shared memory dengan id `random_pokemon_id` dan `shop_stock_id`.

***Routine update_random_pokemon***

```c
void *update_random_pokemon(void *arg) {
  pthread_detach(pthread_self());

  char *normal_list[] = {"Bulbasaur", "Charmander", "Squirtle", "Rattata", "Caterpie"};
  char *rare_list[] = {"Pikachu", "Eevee", "Jigglypuff", "Snorlax", "Dragonite"};
  char *legendary_list[] = {"Mew", "Mewtwo", "Moltres", "Zapdos", "Articuno"};

  while(1) {
    while(!random_pokemon->is_locked) {
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
```

* Pertama, thread akan di `pthread_detach()` agar program utama tidak perlu melakukan `pthread_join()`.
* Lalu Meng-assign variable `normal_list`, `rare_list`, `legendary_list` sesuai dengan nama pokemon masing-masing.
* Lalu akan dilakukan looping yang dimana tiap looping `.is_locked` pada pokemon yang terdapat di shared memory harus bernilai `0` (tidak digunakan pada capture mode traizone). Jika tidak di-lock, maka akan disetting terlebih dahulu `.is_shiny` menjadi `0` dan `.ap` menjadi `100`.
* Lalu dicari bilangan acak menggunakan `rand()` dari 0-99. Lalu untuk yang bernilai dibawah `80` (80%) akan mendapatkan normal pokemon dan masing-masing attributenya diset. Untuk yang bernilai `80 - 94` (15%) akan mendapatkan rare pokemon dan masing-masing attributenya diset. Untuk yang bernilai `95 - 99` (5%) akan mendapatkan legendary pokemon dan masing-masing attributenya diset.
* Lalu terakhir akan dicari bilangan random kembali, jika bilangan tersebut merupakan `1337` (1/8000), maka `.is_shiny` pokemon tersebut akan diset 1. Lalu thread akan sleep selama 1 detik.

***Routine update_shop_stock***

```c
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
```

* Pertama, thread akan di `pthread_detach()` agar program utama tidak perlu melakukan `pthread_join()`.
* Lalu akan diset stock `powder`, `pokeball`, dan `berry` awal (`100`).
* Lalu akan diloop setiap 10 detik untuk menambahkan stock `powder`, `pokeball`, dan `berry` sebanyak 10.
* Lalu masing-masing stock akan di cek apakah melebihi `200`, jika iya maka set menjadi `200`.

***Fungsi sighandler()***

```c
void sighandler(int signum) {
  shmdt(random_pokemon);
  shmctl(random_pokemon_id, IPC_RMID, NULL);
  shmdt(shop_stock);
  shmctl(shop_stock_id, IPC_RMID, NULL);
  printf("\e[H\e[2J\e[m\e[?1049l");
  exit(1);
}
```

* Fungsi diatas untuk menghandle signal `SIGTERM` dan signal `SIGINT`.
* Ketika signal tersebut didapat, maka akan melakukan `shmdt` untuk masing-masing thread dan melakukan `shmctl` untuk masing-masing thread agar ketika shared memory sudah tidak ada yang mengakses akan dihapus datanya.
* Lalu print escape sequence (Mengembalikan kursor -> Menghapus seluruh isi terminal -> Mengembalikan warna yang digunakan -> Mengembalikan buffer terminal menjadi buffer utama) untuk mengembalikan tampilan terminal.

#### Main Program

```c
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
```

* Pada `main()`, program akan men-set *seed* untuk fungsi *`rand()`* menjadi `time(NULL)`.
* Lalu akan diset handling untuk signal `SIGTERM` dan `SIGINT` dengan fungsi `signal()`.
* Lalu akan didapatkan random_pokemon_id dan pointer ke shared memory tersebut dengan menggunakan `shmget` dan `shmat`, dan diset menjadi 0 untuk masing-masing nilai pada shared memory random_pokemon.
* Dan shop_stock_id dan pointer ke shared memory tersebut akan didapat menggunakan cara yang sama.
* Lalu akan dibuat thread untuk menjalankan routine ***`update_random_pokemon`*** dan routine ***`update_shop_stock`***.

```c
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

```

* Lalu program utama akan menampilkan prompt untuk menampilkan menu pada pokezone.
* Menu yang terdapat pada pokezone hanya menu shutdown.
* Jika user menginputkan `"shutdown"`, maka akan dilakukan fork() untuk melakukan `pkill` terhadap proses dengan nama `soal1_traizone` dan `pkill` terhadap proses dengan nama `soal1_pokezone`,

**Kesulitan:**
Waktu dan kompleksitas soal.

**ScreenShot:**\
![Output](https://user-images.githubusercontent.com/17781660/79044698-be3a7d00-7c30-11ea-9bea-5f517ba9226b.png)

## Soal 2
Source Code : [soal2](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/tree/master/soal2)

**Deskripsi:**
Soal meminta kami untuk membuat game dalam bahasa c menggunakan socket dan thread. Yang dimana nantinya terdapat 2 client yang akan battle menggunakan tombol spasi sampai salah satu player *healthnya* habis. Pada client juga terdapat sistem untuk melakukan login dan register. Dan pada server terdapat pengecekan untuk login, dan register.

**Asumsi Soal:**
Pada kodingan kami, nantinya pada client.c akan terdapat 2 thread utama, yaitu satu thread untuk scan input dari player dan mengirimkannya ke socket, dan satu thread lagi untuk menunggu input dari socket dan menampilkannya dalam terminal. Dan disini nantinya client akan menggunakan beberapa fungsi untuk mengatur terminal. Lalu pada server akan melakukan logic gamenya, dan mengatur input dari masing-masing client dengan thread, dan mengatur output masing-masing client menggunakan thread.

**Pembahasan:**

### client.c

**Include Library & Define**

```c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <termios.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#define PORT 8080
```

* `#include <stdio.h>` Library fungsi input output (e.g. printf(), scanf())
* `#include <sys/socket.h>` Library untuk membuat socket
* `#include <sys/types.h>` Library tipe data khusus (e.g. pid_t)
* `#include <stdlib.h>` Library untuk fungsi umum (e.g. exit())
* `#include <netinet/in.h>` Library untuk melakukan koneksi socket
* `#include <string.h>` Library untuk manipulasi string (e.g. strcpy())
* `#include <arpa/inet.h>` Library untuk melakukan koneksi socket
* `#include <unistd.h>` Library untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <termios.h>` Library untuk mengatur terminal (e.g. tcsetattr())
* `#include <pthread.h>` Library untuk operasi thread (e.g. pthread_create(), pthread_exit())
* `#include <ctype.h>` Library untuk fungsi tipe (e.g. tolower())
* `#include <signal.h>` Library untuk mengatur signal yang datang (e.g. signal())
* `#define PORT 8080` Pendefinisian port dari socket

Untuk client, pertama akan didefinisikan global variable dan beberapa fungsi untuk memudahkan fungsi termios. Termios sendiri digunakan untuk mengganti input mode agar ketika user mengetik satu karakter, karakter tersebut langsung dikirim ke socket tanpa harus menunggu ***ENTER (\n)***.

#### Termios

**Global Variable & Fungsi *initTermios*, *resetTermios*, dan *getch***

```c
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
```

* Variable `static struct termios old` untuk menyimpan settingan awal dari terminal
* Variable `static struct termios current` dapat diubah untuk mengatur settingan terminal
* `initTermios()`
  * Fungsi `initTermios()` akan mendapatkan settingan terminal dan disimpan dalam variable `old` menggunakan fungsi `tcgetattr()`.
  * Lalu settingan tersebut diassign juga ke dalam variable `current`.
  * Lalu kita set mode pada `current` menjadi **Tidak Kanonikal** (Input yang didapat langsung dibaca tanpa menunggu newline). Dan juga mode pada `current` menjadi **Tanpa Echo** (Input yang diketik tidak dimunculkan kedalam STDOUT).
  * Lalu settingan `current` di set pada terminal menggunakan `tcsetattr()`.
* Fungsi `resetTermios()` akan mengubah settingan terminal kembali menjadi normal.
* `getch()`
  * Pada fungsi `getch()`, nanti akan melakukan `initTermios()` agar input modenya berubah. Lalu menggunakan fungsi `getchar()` untuk mendapatkan 1 karakter yang diinputkan. Dan terakhir akan dijalankan `resetTermios` untuk mengembalikan settingan terminal.

Lalu akan didefinisikan global variable dan beberapa routine yang akan dijalankan oleh threade,

#### Thread dan Routine

**Global Variable**

```c
pthread_t printer;
pthread_t scanner;

int input_mode = 0;
```

* Variable `printer` dan `scanner` untuk menyimpan thread_id dari kedua routine yang akan dijalankan.
* `input_mode` untuk setting input mode (apakah menunggu buffer sampai newline atau menggunakan termios). Untuk `input_mode` bernilai 0 berarti menggunakan input mode biasa dan bernilai 1 berarti menggunakan input mode dengan termios.

***print_routine***

```c
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
        input_mode = 1;
        resetTermios();
        pthread_cancel(scanner);
      } else if (strcmp(token, "Uhuk Stop") == 0) {
        input_mode = 0;
        resetTermios();
        pthread_cancel(scanner);
      } else {
        printf("%s", buffer);
      }
    }
  }
}
```

* Pada `print_routine()`, `arg` nantinya akan diinputkan variable `socket` yang digunakan oleh fungsi utama. Sehingga akan diassign kedalam variable `sock`.
* Lalu akan dilakukan looping untuk melakukan print ke terminal ketika ada data yang didapat dari `sock`.
  * Untuk mengecek, akan melakukan `recv` terhadap variable `sock` yang diberikan, dan data yang didapat harus lebih besar dari 1 byte.
  * Jika iya, maka akan dicek terlebih dahulu apakah string yang dikirimkan merupakan flag `"Uhuk Start"` atau `"Uhuk Stop"`. Flag `"Uhuk Start"` digunakan untuk mengganti `input_mode` menjadi 1 (satu karakter input). dan Flag `"Uhuk Stop"` digunakan untuk mengganti `input_mode` menjadi 0 (input biasa). Jika kedua flag tersebut jalan, maka thread `scanner` akan di `pthread_cancel()` agar fungsinya berhenti, dan nantinya akan direset oleh program utama.
  * Lalu jika bukan input string yang didapat selain itu, maka akan langsung ditampilkan menggunakan `printf()`.


***scan_routine***

```c
void *scan_routine(void *arg) {
  int sock = *(int *)arg;
  char c[1024];
  int i = 0;
  while(1) {
    if (!input_mode) {
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
```

* Untuk `scan_routine()`, `arg` yang diinputkan sama dengan `print_routine` yaitu `socket` dari program utama.
* Lalu looping untuk melakukan scanning terhadap user input.
* Jika `input_mode == 0`, maka karakter akan diinputkan sampai newline. Disini kami memanipulasi agar input seolah-olah terlihat normal dengan mengatur input yang diterima oleh user. Contoh jika input yang didapat `127` **(BACKSPACE)**, maka proses yang akan kita lakukan yaitu menggeser kursor kekiri, lalu menghapus semua karakter yang didepannya, dan juga pada buffer `c` akan dihapus satu input terakhir. Dan jika input karakter biasa atau `10` **(ENTER)**, maka akan dimasukkan kedalam buffer `c` dan diprint biasa. Dan terakhir buffer `c` akan dikirimkan ke server menggunakan `send()` dengan socket `sock`.
* Jika `input_mode == 1`, maka karakter yang diinputkan didapat menggunakan fungsi `getch()` yang telah didefinisikan. Dan karakter tersebut akan langsung dikirim ke server.

#### Main Program

```c
void sighandler(int signum) {
  printf("\e[2J\e[?25h\e[?1049l");
  resetTermios();
  exit(1);
}

int main(int argc, char const *argv[]) {
    signal(SIGINT, sighandler);
    printf("\e[?1049h");
```

* Sebelum masuk ke `main()`, ada satu fungsi lagi yang kami definisikan, yaitu fungsi `sighandler()`. Fungsi `sighandler()` akan dijalankan ketika terjadi program utama menerima signal `SIGINT`. `sighandler()` sendiri berfungsi untuk mengubah terminal kembali menjadi normal.
  * `\e[2J` merupakan escape sequence untuk menghapus isi terminal dan mengembalikan posisi kursor ke posisi 1,1 (pojok kiri atas).
  * `\e[?25h` merupakan escape sequence untuk memunculkan posisi kursor.
  * `\e[?1049l` merupakan escape sequence untuk mengembalikan terminal menjadi buffer utamanya.
* Lalu pada main akan dijalankan fungsi `signal()` untuk menghandle signal `SIGINT`. lalu akan di di-print kedalam terminal escape sequence `"\e[?1049h"` untuk mengubah buffer terminal menjadi ***alternate-buffer***.

```c
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
```

* Operasi untuk membuat program utama terkoneksi dengan socket.
* Socket file descriptor akan dimasukkan kedalam variable `sock` ketika proses `connect()` berhasil berjalan.

```c
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
```

* Lalu dibuat 2 thread untuk menjalankan routine ***`print_routine`*** dan ***`scan_routine`***.
* Loop akan dilakukan untuk melakukan join terhadap routine `scan_routine`, jika telah selesai (di-cancel), maka akan dijalankan ulang.

### server.c

**Include Library & Define**

```c
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
```

* Untuk `server.c`, library yang digunakan sama dengan library pada `client.c`.

***Struct player***
```c
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
```

Definisi `struct player` untuk memudahkan pengaturan data.
* `player.name[]` variable nama player.
* `player.idsock` variable socket yang digunakan untuk player tersebut.
* `player.health` variable health dari player.
* `player.is_game` variable status apakah player sedang bermain.
* `player.is_auth` variable status apakah player sudah terotentikasi.
* `player.is_ready` variable status apakah player siap untuk bermain.
* `player.input[]` variable untuk menyimpan input dari user.
Lalu kami membuat global variable `player_data` berupa array dari `struct player`

#### Fungsi-Fungsi

**Fungsi *resetScreen***

```c
void resetScreen(int i) {
  char buffer[1024];
  sprintf(buffer, "\e[H\e[2J");
  send(player_data[i].idsock, buffer, 1024, 0);
}
```

* Reset screen digunakan untuk menghapus isi terminal menjadi kosong. Escape sequence `"\e[H"` digunakan untuk meng-reset posisi kursor menjadi (1, 1). Lalu escape sequence "\e[2J" digunakan untuk menghapus seluruh isi terminal. Data tersebut dikirimkan melalui `.idsock` dan dikirimkan ke client agar dijalankan oleh routine pada client.

**Fungsi *prompt***

```c
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
    sprintf(buffer, "\e[H\e[2JFinding Match.   (Press Anything to Cancel)\n");
  }
  if (prompt_no == 311) {
    sprintf(buffer, "\e[H\e[2JFinding Match..  (Press Anything to Cancel)\n");
  }
  if (prompt_no == 312) {
    sprintf(buffer, "\e[H\e[2JFinding Match... (Press Anything to Cancel)\n");
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
```

Fungsi ini digunakan untuk mengirimkan ***prompt*** kepada user. Disini berisi list dari prompt-prompt yang dapat dikirimkan ke user. Selain itu terdapat 4 prompt khusus untuk mengatur terminal client.
* `prompt_no == 989` untuk menghilangkan kursor pada terminal.
* `prompt_no == 988` untuk memunculkan kursor pada terminal.
* `prompt_no == 999` untuk mengganti `input_mode` menjadi `1` pada client.
* `prompt_no == 998` untuk mengganti `input_mode` menjadi `2` pada client.

**Fungsi *get_data***

```c
char *get_data(char buff[], int i) {
  memset(player_data[i].input, 0, 1024);
  while(strlen(player_data[i].input) == 0);
  strcpy(buff, player_data[i].input);
  memset(player_data[i].input, 0, 1024);
  // printf("From player_data[%d]: %s\n", i, buff);
  return buff;
}
```

Fungsi ini digunakan untuk menunggu input dari client pada `player_data[i].input` dan mengembalikannya pada variable `buffer`. `player_data[i].input` akan diatur oleh sebuah thread untuk mendapatkan inputnya dari socket.

**Fungsi *auth***

```c
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
```

Fungsi ini digunakan untuk melakukan otentikasi dari username dan password yang diinputkan dalam parameter, dan meng-return apakah username tersebut terotentikasi apa tidak.
* Pertama file `"akun.txt"` akan dibuka dengan operasi `read`, variable `is_auth` diset 0 karena awalnya masih belum terotentikasi.
* Lalu looping untuk membaca tiap baris `"akun.txt"`, masing-masing loop akan dibaca username dan password yang ada pada tiap baris dengan delimiter `"."`, dan setelah itu akan dicek apakah username dan password yang diinputkan sama dengan baris yang ada di dalam `"akun.txt"`, jika terdapat kesamaan, maka akan langsung terotentikasi.

**Fungsi *reg***

```c
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
```

Fungsi ini digunakan untuk melakuken *register*.
* Pertama file `"akun.txt"` akan dibuka dengan operasi `append`. Lalu username dan password akan langsung diinput kedalam file tersebut.
* Lalu file akan dibuka lagi tetapi dengan operasi `read`. Lalu masing masing username dan password yang ada dalam `"akun.txt"` akan di-print kedalam terminal server.

#### Routine

***server_main_routine***

```c
void *server_main_routine(void *arg) {
  int i = *(int *)arg - 1;

  char buffer[1024];
  while(1) {
    resetScreen(i);
```

Routine utama yang akan dijalankan ketika ada client yang terkoneksi.
* variable yang akan dimasukkan kedalam `arg` merupakan id dari `player_data[]` yang didapat oleh user.
* Lalu akan dilakukan looping dari gamenya, pertama akan menjalankan fungsi `resetScreen()`.

```c
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
```

Kode program diatas akan dijalankan jika client baru masuk dan masih belum terotentikasi.
* Routine akan mengirimkan `prompt()` yang sesuai lalu mendapatkan data dan menyimpannya kedalam variable `buffer`.
* Lalu `buffer` akan di `tolower()` untuk mengubah menjadi lowercase.
* Jika user menginputkan `"1"` atau `"login"` maka, akan di `prompt()` untuk mendapatkan `username` beserta `password`nya. Setelah itu akan dijalankan fungsi `auth()` dan hasilnya akan dimasukkan kedalam `player_data[i].is_auth`.  Lalu variable `player_data[i].is_auth` akan dicek apakah berhasil login atau tidak. Jika gagal maka akan dimunculkan `prompt()` gagal dan jika berhasil maka `player_data[i].name` akan diganti menjadi `username`. Dan pada terminal server akan meng-print status dari otentikasi tersebut.
* Jika user menginputkan `"2"` atau `"register"` maka, akan di `prompt()` seperti bagian `"login"` untuk mendapatkan `username` beserta `password`nya. Setelah itu akan dijalankan fungsi `reg()` dan memunculkan `prompt()` register berhasil.

```c
} else if (player_data[i].is_ready == 0) {
  prompt(0, i);
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
```

Kode program diatas akan dijalankan jika user sudah terotentikasi namun masih belum ingin melakukan `"Find Match"`.
* Program akan mengirmkan `prompt()` yang sesuai, lalu menunggu input client dengan fungsi `get_data()` dan hasilnya di lowercase.
* Jika client ingin `"Find Match"`, maka variable `player_data[i].is_ready` di set menjadi `1`.
* Jika client ingin `"Logout"`, maka variable `player_data[i].is_auth` di set menjadi `0` dan `player_data[i].name` akan dikosongkan.

```c
    } else if (player_data[i].is_game == 0) {

      prompt(989, i);
      prompt(999, i);
      time_t now = time(NULL);
      while(player_data[!i].is_ready == 0) {
        prompt(310+((time(NULL)-now)%3), i);
        if (strlen(player_data[i].input) > 0) {
          player_data[i].is_ready = 0;
          memset(player_data[i].input, 0, 1000);
          prompt(988, i);
          break;
        }
      }
      prompt(998, i);

      if (player_data[i].is_ready) {
        prompt(999, i);
        prompt(32, i);
        get_data(buffer, i);
        prompt(998, i);
        player_data[i].is_game = 1;
      }
```

Kode program diatas akan dijalankan jika user melakukan `"Find Match"` namun masih belum menemukan lawan.
* Program akan menghilangkan kursor dengan mengubah `input_mode` client dengan `prompt()` `989` dan `999`.
* Lalu program akan menunggu sampai status `.is_ready` lawan `1`. Jika belum akan menampilkan `prompt` tiap detiknya dan mengecek apakah client menginputkan sesuatu. Jika user menginputkan sesuatu, maka user ingin meng-cancel, sehingga status `.is_ready` player diubah menjadi `0`, kursor akan dikembalikan dengan `prompt()` `988` dan loop di-*break*.
* Setelah loop selesai, `input_mode` client diubah kembali menggunakan `prompt` `998`.
* Lalu dicek apakah status `.is_ready` player masih `1`, jika iya maka akan ditampilkan `prompt` untuk memulai match, dan status `is_game` player akan di-set menjadi `1`.

```c
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
```

Kode program diatas dijalankan ketika status `.is_game` player adalah `1`.
* Program akan menunggu sampai status `.is_game` lawan menjadi `1`.
* Lalu akan menampilkan `prompt()` yang sesuai, `410` untuk menampilkan `.health` dari player pada posisi print (1, 1). `412` digunakan untuk mengarahkan kursor ke posisi (3, 1).
* Lalu looping sampai salah satu player mencapai 0. Tiap loop akan mengecek `.input` dari player dan akan dicek apakah yang diinputkan merupakan `" "`. Jika iya, maka `.health` musuh akan dikurangi 10, akan dikirim `prompt()` `410` ke player musuh, dan `411` ke player. `411` digunakan untuk memunculkan `"HIT!"`. `.input` akan diset ulang.
* Jika telah selesai, maka status `.is_game` dan `.is_ready` akan diset menjadi `0` kembali.
* Lalu akan di `prompt()` apakah player pemenangnya atau bukan. dan terakhir `.health` player akan diset kembali menjadi 100.

***server_scan_routine***

```c
void *server_scan_routine(void *arg) {
  int i = *(int *)arg - 1;
  char buffer[1024];
  while(1) {
    if (recv(player_data[i].idsock, buffer, 1024, 0) > 0) {
      strcpy(player_data[i].input, buffer);
    }
  }
}

```

Routine diatas digunakan untuk mengupdate `.input` untuk masing-masing player dengan user input yang didapat dari socket pada `.idsock` player. Disini menggunakan fungsi `recv` seperti pada client dan `strcpy` untuk meng-copy kedalam `.input` player.

#### Main Program

```c
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
```

* Pertama, server akan membuat file `"akun.txt"` jika file tersebut belum ada.
* Lalu proses berikutnya adalah setting socket agar melakukan `listen()` kepada 2 socket saja.

```c
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
```

* Lalu akan dibuat `socket_thread` untuk membuat thread ***server_main_routine*** dan ***server_scan_routine*** untuk masing-masing socket.
* Lalu setiap kali socket diaccept, akan dimasukkan kedalam `player_data[i].idsock` yang sesuai. Lalu akan diset, status `.is_game`, `.is_auth`, `.is_ready`, dan `.health` masing-masing player.
* Lalu dibuat 2 thread untuk masing-masing socket untuk menjalankan routine ***`server_main_routine`*** dan ***`server_scan_routine`***.
* Terakhir, masing-masing thread akan dijoin.

**Kesulitan:**
Mengatur Game Setting.

**ScreenShot:**\
![Output](https://user-images.githubusercontent.com/17781660/79044790-64868280-7c31-11ea-8878-4abbad13f10f.png)
![Output](https://user-images.githubusercontent.com/17781660/79044792-651f1900-7c31-11ea-9951-a8f586b50b6a.png)
![Output](https://user-images.githubusercontent.com/17781660/79044793-66504600-7c31-11ea-9d39-c01f6becf6f9.png)
![Output](https://user-images.githubusercontent.com/17781660/79044794-67817300-7c31-11ea-9cb3-9d308e8a552a.png)
![Output](https://user-images.githubusercontent.com/17781660/79044795-68b2a000-7c31-11ea-88cf-2d3479e65474.png)
![Output](https://user-images.githubusercontent.com/17781660/79044796-694b3680-7c31-11ea-9da7-4cc0bf268aaa.png)

## Soal 3
Source Code : [soal3](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/blob/master/soal3/soal3.c)

**Deskripsi:**
Soal meminta kami untuk membuat sebuah program dari C untuk mengkategorikan file. Program ini akan memindahkan
file sesuai ekstensinya (tidak case sensitive. JPG dan jpg adalah sama) ke dalam folder sesuai ekstensinya
yang folder hasilnya terdapat di working directory ketika program kategori tersebut dijalankan. Terdapat 3
arguman yang dapat di inputkan yaitu **(-f)**, **(*)** dan **(-d)**. Dengan ketentuan sebagai berikut:

    * (-f) :
                 *  user bisa menambahkan argumen file yang bisa dikategorikan sebanyak yang user inginkan
                 *  Pada program kategori tersebut, folder jpg,c,zip tidak dibuat secara manual,
                    melainkan melalui program c. Semisal ada file yang tidak memiliki ekstensi,
                    maka dia akan disimpan dalam folder “Unknown”.

    * (-d) :
                 *  user hanya bisa menginputkan 1 directory saja.
                 *  Hasilnya perintah di atas adalah mengkategorikan file di /path/to/directory dan
                    hasilnya akan disimpan di working directory di mana program C tersebut
                    berjalan (hasil kategori filenya bukan di /path/to/directory).
                 *  Program ini tidak rekursif.
                 *  Setiap 1 file yang dikategorikan dioperasikan oleh 1 thread

    * (*) :
                 *  mengkategorikan seluruh file yang ada di working directory

**Asumsi Soal:**
Soal meminta kami untuk membuat program c yang mampu mengkategorikan file secara tidak rekursif dengan beberapa
parameter yang sudah ditentukan. Karena diminta untuk adanya thread pada setiap file yang akan di kategorikan, kami
mengasumsikan bahwa thread akan menjalankan routinenya pada setiap absolut path yang diambil dari sebuah variabel yang menyimpannya terlebih dahulu dan bukan langsung dari dari sebuah fungsi yang me-return absolut path

**Pembahasan:**

``` c
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include <pthread.h>
#include <errno.h>
```

* `#include <sys/types.h>` Library tipe data khusus (e.g. pid_t)
* `#include <sys/stat.h>` Library untuk pendeklarasian fungsi stat() dan semacamnya (e.g.fstat() and lstat())
* `#include <stdio.h>` Library untuk fungsi input-output (e.g. printf(), sprintf())
* `#include <stdlib.h>` Library untuk fungsi umum (e.g. exit(), atoi())
* `#include <unistd.h>` Llibrary untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <string.h>` Library untuk pendefinisian berbagai fungsi untuk manipulasi array karakter (e.g. strtok())
* `#include <ctype.h>` Library untuk pendefinisian berbagai fungsi untuk karakter handling(e.g.tolower())
* `#include <dirent.h>` Library untuk merepresentasikan directory stream & struct dirent(e.g. struct dirent *entry)
* `#include <pthread.h>` Library untuk operasi thread (e.g. pthread_create(), ptrhead_exit() )
* `#include <errno.h>` Library untuk error handling (e.g. errno)


Pertama kami melakukan pendefinisian 3 fungsi dan 1 routine(thread) untuk program ini yaitu: `getFileName`,
`getExtension`, `dirChecking` dan `routine`.

**Fungsi *getFileName***
``` c
char *getFileName(char *fName, char buff[]) {
  char *token = strtok(fName, "/");
  while (token != NULL) {
    sprintf(buff, "%s", token);
    token = strtok(NULL, "/");
  }
}
```
* Fungsi ini didefinisikan menggunakan dua parameter yaitu `*fname` sebagai pointernya dan `buff[]` untuk
store hasil dari fungsi ini sendiri, dan akan mereturn file name yang masih beserta ekstensinya.
  * Selanjutnya nama dari file akan diambil menggunakan fungsi **strtok()** untuk memecah string dengan
  dengan delimiter `/` dan akan disimpan di dalam `*token`
  * Lalu **while** loop akan berjalan selama token belum habis dan file name yang sudah diambil akan di
  print kedalam `buffer`.
  * Fungsi **strtok()** akan dijalankan lagi dengan parameter pertama = **NULL** untuk mencari token
  selanjutnya hingga akhir dari input.


**Fungsi *getExtension***
``` c
char *getExtension(char *fName, char buff[]) {
  char buffFileName[1337];
  char *token = strtok(fName, "/");
  while (token != NULL) {
    sprintf(buffFileName, "%s", token);
    token = strtok(NULL, "/");
  }
```
* Fungsi ini didefinisikan menggunakan dua parameter yaitu `*fname` sebagai pointernya dan `buff[]` untuk store hasil dari fungsi ini sendiri, dan akan mereturn ekstensi dari sebuah file.
* Selanjutnya Fungsi akan melakukan hal yang sama persis seperti fungsi getFile name yang nantinya akan menghasilkan nama file yang masih beserta ekstensinya

``` c
 int count = 0;
  token = strtok(buffFileName, ".");
  while (token != NULL) {
    count++;
    sprintf(buff, "%s", token);
    token = strtok(NULL, ".");
  }
```
* Disini file name yang masih beserta ekstensinya akan dipecah kembali menggunakan fungsi **strtok()** dengan delimiter `.` sebagai pemisah antara nama file dengan ekstensi dan disimpan kedalam `token*`
* Karna yang akan pertama di return oleh fungsi **strtok()** adalah `token` pertma/kata pertama seblum delimiter maka **while** loop akan berjalan selama `token` belum habis atau belum sampai ekstensinya dan `counter` akan di increment sebagai variabel untuk checking
* Ekstensi yang sudah didapat akan di print ke dalam `buffer`.

``` c
 if (count <= 1) {
    strcpy(buff, "unknown");
  }

  return buff;
```
* Pengecekan untuk jumlah `counter` yang kurang atau kondisi dimana file tidak ada ekstensi atau file memiliki karakter "." diawal maupun diakhir string.
* Untuk file yang tidak memiliki ekstensi, `buffer` akan berisi `unknown`

**Fungsi *directory Checking***
``` c
 void dirChecking(char buff[]) {
  DIR *dr = opendir(buff);
  if (ENOENT == errno) {
    mkdir(buff, 0775);
    closedir(dr);
  }
}
```
* Fungsi didefinisikan menggunakan satu parameter yaitu `buff[]` untuk menyimpan hasil dari fungsi ini sendiri
* Disini pembuatan directory baru akan dilakukan jika ada sebuah error yang dihasilakan oleh fungsi **opendir()**, lalu **if** akan melakukan error handling.
* Fungsi ini melakukan pembuatan directory baru menggunakan fungsi **mkdir()** dengan nama yang di return `buffer` dan permission `0775` atau `read and execute` lalu akan di tutup kembali.

***Routine***
``` c
void *routine(void* arg) {
  char buffExt[100];
  char buffFileName[1337];
  char buffFrom[1337];
  char buffTo[1337];
  char cwd[1337];
  getcwd(cwd, sizeof(cwd));
  strcpy(buffFrom, (char *) arg);
}
```
* Pada Routine ini kami mendefinisikan lima `buffer` yang masih masingnya akan menghandle: `ext` `fileName`
`path input` `path to` dan `cwd`
* Dimana `buffer` untuk `cwd` akan langsung diisi dengan fungsi **getcwd()** yang mereturn
current working directory beserta sizenya dan untuk `from` atau argumen path yang diinpukan user
akan diambil menggunakan **(char *)arg**

``` c
  if (access(buffFrom, F_OK) == -1) {
    printf("File %s tidak ada\n", buffFrom);
    pthread_exit(0);
  }
  DIR* dir = opendir(buffFrom);
  if (dir) {
    printf("file %s berupa folder\n", buffFrom);
    pthread_exit(0);
  }
  closedir(dir);
```
* Disini program akan melakukan mengecek eksistensi dan bentuk dari file yang diinputkan oleh user
dari argumen yang diinputkan user menggunakan fungsi:
    * **access()** dengan source `buffFrom` & `F_OK` sebagai `amode` untuk eksistensi. Jika argumen yang diinputkan
      tidak sesuai makan ditampilkan error message dan `thread` akan diselesaikan menggunakan **pthread_exit(0)**
    * Pengecekan bentuk file yang diinputkan adalah dengan menggunakan **dir** yang dimana jika terbuka sebagai
      directory maka akan ditampilkan error message dan `thread` akan diselesaikan menggunakan **pthread_exit(0)**

``` c
  getFileName(buffFrom, buffFileName);
  strcpy(buffFrom, (char *) arg);

  getExtension(buffFrom, buffExt);
  for (int i = 0; i < sizeof(buffExt); i++) {
    buffExt[i] = tolower(buffExt[i]);
  }
  strcpy(buffFrom, (char *) arg);
```
* Selanjutnya kami memanggil fungsi **getFilename()** dengan filename yang akan masuk ke `buffer`
baru `buffFilename`
* Lalu kami memanggil fungsi **getExtension()** untuk mengambil setiap ext dari filename yang ada di `buffFrom`
dan merubah tiap extension dari file yang ada menjadi `lowercase` menggunakan **for** loop degam fungsi **tolower()** dan `i` sebagai `counter`nya.

``` c
  dirChecking(buffExt);

  sprintf(buffTo, "%s/%s/%s", cwd, buffExt, buffFileName);
  rename(buffFrom, buffTo);

  pthread_exit(0);
```
* Selanjutnya fungsi **dirChecking()** akan dipanggil yang akan membuat directory baru untuk setiap ekstensi didalam `buffExt` yang belum memiliki directory
* Lalu `buffTo` akan diisi dengan value setiap `buffer` yang sudah di set sebelumnya dengan urutan `cwd`,`buffExt`
dan `buffFilename` menggunakan **sprintf()**. Kemudian file name yang ada di `buffFrom` `(const char *old_filename)`
akan di **rename()** dengan urutan dari `buffTo` `(const char *new_filename)` yang sudah di set.


***main()***
``` c
int main(int argc, char *argv[]) {
  if (argc == 1) {
    printf("Argument kurang\n");
    exit(1);
  }
  if (strcmp(argv[1], "-f") != 0 && strcmp(argv[1], "*") != 0 && strcmp(argv[1], "-d")) {
    printf("Argument tidak ada\n");
    exit(1);
  }
```
### note input
*note:  * input untuk argumen `-f`  > 2
        * input untuk argumen `*`   = 2
        * input untuk argumen `-d`  = 3
* Pada main, kami menggunakan dua parameter yaitu `argc` & `*argv[]` untuk jumlah argumen & pointer ke masing masing
argumen tersebut karna akan dibutuhkan beberapa pengecekan untuk argumen yang diinputkan.
* Pertama program akan melakukan pengecekan jumlah pada **if()** pertama yang akan menampilkan error message jika
jumlah input tidak sesuai dengan [note input](#note-input).
* Kedua program akan melakukan pengecekan character argumen yang diinputkan dengan menggunakkan fungsi **strcmpr()**
dengan pointer ke masing masing argumennya menggunakan ***argv[]** sebagai parameter pertama
dan `-f` , `*` serta `-d`sebagi parameter kedua. Dan akan menampilakan error message jika argumen yang diinputkan
tidak sesuai

``` c
if (strcmp(argv[1], "-f") == 0) {
    if (argc <= 2) {
      printf("Argument salah\n");
      exit(1);
    }

    pthread_t tid[argc-2];
    for (int i = 2; i < argc; i++) {
      pthread_create(&tid[i-2], NULL, &routine, (void *)argv[i]);
    }
    for (int i = 2; i < argc; i++) {
      pthread_join(tid[i-2], NULL);
    }
    exit(0);
  }
```
* Disini program akan mengecek banyak argumen jika yang diinputkan adalah `-f` sekaligus membuat thread dengan
men-set `thread id` terlebih dahulu dengan nilai `jumlah_argumen - 2` disini **for()** loop akan berjalan sebanyak
`i` kali, dimana `i` adalah jumlah argumen, **for()** loop akan membuat thread menggunakan **pthread_create()**
untuk setiap argumennya dengan `tid` `i - 2` dimana `i` tadi akan di increment setiap parameter **for()** terpenuhi
* Jumlah argumen untuk penggunaan `-f` harus sesuai dengan [note input](#note-input), jika tidak maka ditampilkan
error message dan program akan ditutup dengan **exit(1)**
* Lalu `thread` akan diajalankan dengan `routine` kepada `(void *)argv[i])`
* Selanjutnya program akan men-join setiap `thread` yang sudah dibuat dengan **pthread_join()**

``` c
 char *directory;
  if (strcmp(argv[1], "*") == 0) {
    if (argc != 2) {
      printf("Argument salah\n");
      exit(1);
    }
    char buff[1337];
    getcwd(buff, sizeof(buff));
    directory = buff;
  }
```
* Disini program akan mengecek banyak argumen jika yang diinputkan adalah `*`. Jumlah argumen untuk penggunaan `*`
harus sesuai dengan [note input](#note-input), jika tidak maka ditampilkan error message dan program akan ditutup
dengan **exit(1)**
* Jika hanya **if()** pertama yang terpenuhi(argumen benar) maka program akan menset `cwd` beserta sizenya
menggunakan **getcwd()** kedalam `buffer` baru yang nantinya akan dimasukkan ke variabel `directory`

``` c
if (strcmp(argv[1], "-d") == 0) {
    if (argc != 3) {
      printf("Argument salah\n");
      exit(1);
    }
    DIR* dir = opendir(argv[2]);
    if (dir) {
      directory = argv[2];
    } else if (ENOENT == errno) {
      printf("Directory tidak ada\n");
      exit(1);
    }
    closedir(dir);
  }
```
* Disini program akan mengecek banyak argumen jika yang diinputkan adalah `-d`. Jumlah argumen untuk penggunaan `-d`
harus sesuai dengan [note input](#note-input), jika tidak maka ditampilkan error message dan program akan ditutup
dengan **exit(1)**
* Jika hanya **if()** pertama yang terpenuhi(argumen benar) maka program akan membuka directory sesuai dengan
argumen kedua yang menggunakan **opendir()** dan memasukan nama directory tersebut kedalam variabel dir, namun
jika directory tidak terbuka maka akan ada error handling di dalam **else if()** yang akan menampilkan error
message dan exit prgram dengan **exit(1)**
* Contoh untuk kondisi **else if()** disini adalah file yang tidak memiliki ekstensi, jadi variabel `dir` berisi
`NULL` dan directory tidak terbuka.

``` c
int file_count = 0;
  DIR* dir = opendir(directory);
  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      file_count++;
    }
  }
  closedir(dir);
```
* Disini kami membuat file counter untuk setiap file yang ada dalam suatu directory, bagian ini adalah handler untuk
argumen `-d` dan `*`
* Pertama-tama directory akan dibuka dengan **opendir()** dan di set ke dalam variabel `dir` untuk nantinya di cek
* `struct dirent *entry;` pendefinisian struct `dirent` untuk penggunaan fungsi **readdir()**
* **while** disini adalah untuk pengecekan tiap filenya di dalam `dir` yang sudah dibuka dengan menggunakan
`entry->d_type` yang dimana itu adalah field yang berisi indikasi tipe file dan `DT_REG` merupakan macro constant
dari `d_type` yang berarti tipe file reguler, untuk setiap file reguler yang ditemukan maka nilai `counter` akan di
increment, **while()** akan berjalan sampai tiap file yang ada di dalam directory habis.

```c
pthread_t tid[file_count];
  char buff[file_count][1337];
  int iter = 0;

  dir = opendir(directory);
  while ((entry = readdir(dir)) != NULL) {
    if (entry->d_type == DT_REG) {
      sprintf(buff[iter], "%s/%s", directory, entry->d_name);
      iter++;
    }
  }
  closedir(dir);
```
* Sebelumnya kami mendefinisikan sebuah `buffer` untuk menyimpan absolut path dan satu variabel untuk iterasi
* Lalu directory dibuka dengan **opendir()** untuk nantinya pengecekan tiap file didalamnya dilakukan
* Disini kami men-set `tid` sejumlah nilai `counter` (banyak file reguler) di setiap directory untuk thread yang
akan dibuat
* `sprintf(buff[iter], "%s/%s", directory, entry->d_name);` disini akan memasukan absolut path dari setiap file
reguler itu sendiri, kondisi ini akan berjalan selama file dari sebuah directory belum habis sesuai parameter **while** loop.
* Size dari `buffer`(iter) akan di increment untuk setiap absolut path yang masuk ke dalam `buffer` tersebut

``` c
  for (int i = 0; i < file_count; i++) {
    char  *test = (char*)buff[i];
    printf("%s\n", test);
    pthread_create(&tid[i], NULL, &routine, (void *)test);
  }

  for (int i = 0; i < file_count; i++) {
    pthread_join(tid[i], NULL);
  }
```
* **for()** loop disini akan berjalan sebanyak jumlah file reguler yang sudah tersimpan di `buffer` sebelumnya.
* `*test` disini berfungsi untuk menyimpan terlebih dahulu absolut path dari setiap file sebelum dijalankan di
dalam thread. Jadi thread disini tidak langusng mengambil argumen keempatnya dari `buffer`
* Pembuatan thread menggunakan `pthread_create(&tid[i], NULL, &routine, (void *)test)` terlihat disini argumen
keempatnya kami mengguanakan `test` yang tadi sudah menyimpan dahulu absolut path dari setiap file
* **for()** loop kedua akan men-join setiap thread yang sudah dibuat

**Kesulitan:**
Manipulasi String.

**ScreenShot**\

**Contoh input argumen `-f`:**\
![Output](https://user-images.githubusercontent.com/17781660/79044913-5be27c00-7c32-11ea-9b34-1a42ed570bf5.png)

**Contoh input argumen `-d`:**\
![Output](https://user-images.githubusercontent.com/17781660/79045005-f8a51980-7c32-11ea-8f6c-2cca6a20ae6f.png)

**Contoh input argumen `*`:**\
![Output](https://user-images.githubusercontent.com/17781660/79045099-7701bb80-7c33-11ea-80af-a8249d1595ef.png)

## Soal 4
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/tree/master/soal4)

**Deskripsi:**
Norland mendapati ada sebuah tiga teka-teki yang tertulis di tiga pilar berbeda. Untuk dapat mengambil batu mulia
di suatu pilar, Ia harus memecahkan teka-teki yang ada di pilar tersebut. Norland menghampiri setiap pilar secara
bergantian.

### Soal 4.a.
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/blob/master/soal4/soal4a.c)

**Deskripsi:**
Pada teka teki untuk pilar pertama Norland diminta untuk :
*  Membuat program C dengan nama "4a.c", yang berisi program untuk melakukan perkalian matriks. Ukuran matriks
pertama adalah 4x2, dan matriks kedua 2x5. Isi dari matriks didefinisikan di dalam kodingan. Matriks nantinya akan
berisi angka 1-20 (tidak perlu dibuat filter angka).
* Dan menampilkan matriks hasil perkaliannya ke layar.

**Asumsis Soal:**
Kami mengasumsikan untuk perkalian matriks ini bahwa akan ada variabel yang bisa mengindikasikan baris pertama dan
kolom pertama pada pengulangan pertama yang dijadikan sebagai counter sesuai dengan syarat perkalian matriks

**Pembahasan:**
```c
#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
```
* `#include <stdio.h>` Library untuk fungsi input-output (e.g. printf(), sprintf())
* `#include <pthread.h>` Library untuk operasi thread (e.g. pthread_create(), ptrhead_exit() )
* `#include <sys/ipc.h>` Library digunakan untuk tiga mekanisme interprocess communication (IPC)(e.g. semaphore)
* `#include <sys/shm.h>` Library untuk mendefinisikan symbolic constants structure seperti(SHM_RDONLY,SHMLBA)
* `#include <stdlib.h>` Library untuk fungsi umum (e.g. exit(), atoi())
* `#include <unistd.h>` Llibrary untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <string.h>` Library untuk pendefinisian berbagai fungsi untuk manipulasi array karakter (e.g. strtok())


```c
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
```
* Disini kami melakukan pendefinisian tiga matriks yaitu `matA`(matriks 4*2), `matB`(matriks 2*5) dan `matC`
(matriks 4*5 sebagai matriks hasil perkalian)
* Value dari matriks di set dengan range int 1-20 sesuai soal

``` c
struct args {
  int i;
  int j;
};
```
* Lalu kami mendefinisikan `struct` dengan member `i` sebgai baris dan `j` sebagai kolom

***fungsi kali***
``` c
void *kali(void* arg) {
  int i = ((struct args*)arg)->i;
  int j = ((struct args*)arg)->j;

  for (int k = 0; k < 2; k++) {
    matC[i][j] += matA[i][k] * matB[k][j];
  }
}
```
* Fungsi pengkalian disini mengguanakan `arg` sebagai argumennya dan `((struct args*)arg)->i`akan memasukan
baris dari tiap matriks kedalam variabel `i` serta `int j = ((struct args*)arg)->j` akan memasukan
kolom dari tiap matriks kedalam variabel `j`
* Pada **for()** loop , kami membuat sebuah variabel `k` yang menjadi nilai kesamaan ordo matriks( 2 kolom
di matriks pertama dan 2 baris di matriks kedua) yang akan mengulang sebanyak 2 kali
* Perkalian dilakukan dengan mengalikan setiap baris di `matirksA` dengan setiap kolom di `matriksB`, jadi `k`
disini mengindikasikan baris pertama dan kolom pertama pada pengulangan pertama, dan setrusnya
* Lalu hasil perkalian akan ditambahkan dan dimasukan ke `matriksC`. Pada case ini ordo matriks hasil adalah (4*5),
karena ordo matriks hasil perkalian dua buah matriks adalah jumlah baris pertama dikali jumlah kolom ke dua.

``` c
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
```
* Pada **main()**, pertama-tama kami mendefinisikan `tid` dari `thread` dengan jumlah ordo matriks hasil, dan
sebuah `struct` yang berisi atribut `index`
* **for()** pertama adalah sebagai looping untuk indikasi baris dan **for()** kedua sebagai loopinh indikasi kolom
yang setiap indikasi baris dan kolom tersbeut akan diset ke `i` dan `j`
* Disini `thread` akan dibuat dengan **pthread_create(&tid[i][j], NULL, &kali, (void *)index)** dan berjalan dengan
`tid` `i` dan `j` yang di increment setiap perulangannya
* `thread` akan menjalankan fungsi `kali` sebagai routine dengan atribut `index` sebagai variabel yang digunakan
* Selanjutnya kami men-join setiap `thread` yang sudah dibuat dengan **pthread_join(tid[i][j], NULL)**

``` c
printf("Matriks :\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      printf("%4d", matC[i][j]);
    }
    printf("\n");
  }
```
* Lalu kami melakukan printout dengan **printf("%4d", matC[i][j])** untuk setiap baris dan kolom menggunakan
counter `i` dan `j` pada dua **for()** loops yang masing masingnya untuk counter baris dan kolom yang kan
diincrement sebanyak jumlah baris(4) dan kolom(5) pada matriks hasil

``` c
key_t key = 1337;
  int *value;

  int shmid = shmget(key, sizeof(matC), IPC_CREAT | 0666);
  value = shmat(shmid, NULL, 0);

  int* p = (int *)value;

  memcpy(p, matC, 80);

  shmdt(value);
```
* Disini kami membuat shared memory untuk `matriksC` sesuai dengan template pembuatan shared memory yang ada
pada modul, karena nanti `matriksC `akan digunakan untuk acuan dari soal 4.b

**Kesulitan:**
Tidak ada.

**Screenshot:**\
![Output](https://user-images.githubusercontent.com/17781660/79045196-060ed380-7c34-11ea-912e-db9043f27dd2.png)

### Soal 4.b.
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/blob/master/soal4/soal4b.c)

**Deskripsi:**
Pada teka teki untuk pilar kedua Norland diminta untuk :
* membuatlah program C kedua dengan nama "4b.c". Program ini akan mengambil variabel hasil perkalian matriks dari
program "4a.c" (program sebelumnya), dan tampilkan hasil matriks tersebut ke layar.
* Setelah ditampilkan, berikutnya untuk setiap angka dari matriks tersebut, carilah nilai penjumlahannya, dan
tampilkan hasilnya ke layar dengan format seperti matriks.
*note*: Mengguankaan shared memory dan mengguanakan thread dalam perhitungan penjumlahan


**Asumsi Soal:**
Kami mengasumsikan bahwa akan dibuat matriks yang akan digunakan sebagai shared memory dan akan dibuat
thread untuk setiap nilai dari matriks hasil yang akan berjalan dengan rutin penjumlahan

**Pembahasan:**

```c
#include <stdio.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
```
* `#include <stdio.h>` Library untuk fungsi input-output (e.g. printf(), sprintf())
* `#include <pthread.h>` Library untuk operasi thread (e.g. pthread_create(), ptrhead_exit() )
* `#include <sys/ipc.h>` Library digunakan untuk tiga mekanisme interprocess communication (IPC)(e.g. semaphore)
* `#include <sys/shm.h>` Library untuk mendefinisikan symbolic constants structure seperti(SHM_RDONLY,SHMLBA)
* `#include <stdlib.h>` Library untuk fungsi umum (e.g. exit(), atoi())
* `#include <unistd.h>` Llibrary untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <string.h>` Library untuk pendefinisian berbagai fungsi untuk manipulasi array karakter (e.g. strtok())

``` c
int matrix[4][5];
int hasil[4][5];

struct args {
  int i;
  int j;
};
```
* Pertama kami membuat array dan matriks dengan ordo sesuai dengan output matriks soal 3
* Mendefinisikan `struct` dengan atribut `i` dan `j` yang nanti digunakan sebagai baris dan kolom

***fungsi Penjumlahan***
``` c
void *factorial(void* arg) {
  int i = ((struct args*)arg)->i;
  int j = ((struct args*)arg)->j;
  long hasilEl = 0;
  for (int n = 1; n <= matrix[i][j]; n++) hasilEl += n;
  hasil[i][j] = hasilEl;
}
```
* Pendefinisian fungsi `penjumalah` dengan pembuatan `struct` yang akan menset atribut `i` dan `j` nya keadalam
variable `i` dan `j`
* **For()** loops disini akan berjalan dengan counter `n` yang akan di increment untuk setiap baris dan kolom dari
matriks, dan akan menjumlahkan counter kedalam `hasilEl` untuk setiap perulangannya

```c
int main() {
  key_t key = 1337;
  int *value;
  int shmid = shmget(key, 80, IPC_CREAT | 0666);
  value = shmat(shmid, NULL, 0);

  int* p = (int *)value;
  memcpy(matrix, p, 80);

  shmdt(value);
  shmctl(shmid, IPC_RMID, NULL);
```
* Pada **main()**, pertama-tama kami akan membuat shared memory untuk `matrix`, sesuai dengan template pembuatan shared memory yang ada pada modul

``` c
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
```
* **for()** loop pertama adalah sebagai looping untuk indikasi baris dan **for()** kedua sebagai loopinh indikasi
kolom yang setiap indikasi baris dan kolom tersbeut akan diset ke `i` dan `j`
* Disini `thread` akan dibuat dengan **pthread_create(&tid[i][j], NULL, &kali, (void *)index)** dan berjalan dengan
`tid` `i` dan `j` yang di increment setiap perulangannya
* `thread` akan menjalankan fungsi `faktorial` sebagai routine dengan atribut `index` sebagai variabel yang
digunakan
* Selanjutnya kami men-join setiap `thread` yang sudah dibuat dengan **pthread_join(tid[i][j], NULL)**

``` c
  printf("Matriks :\n");
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 5; j++) {
      printf("%4d", hasil[i][j]);
    }
    printf("\n");
  }
}
```
* Disini program akan menampilkan setiap baris dan kolom dari matriks `hasil[i][j]` menggunakan **for()** loops yang
dengan counter `i` untuk baris dan `j` untuk kolom, menggunakan **printf("%20ld", hasil[i][j])** dan
menampilkan hasil penjumlahan dari setiap matriks hasil, `%4d` disini adalah untuk banyak karakter int yang akan
di print dari matriks `hasil[i][j]`

**Kesulitan:**
Tidak ada.

**ScreenShot:**\
![Output](https://user-images.githubusercontent.com/17781660/79045198-07400080-7c34-11ea-9440-18a353abe03b.png)

### Soal 4.c.
Source Code : [source](https://github.com/DSlite/SoalShiftSISOP20_modul3_T08/blob/master/soal4/soal4c.c)

**Deskripsi:**
Pada teka teki untuk pilar ketiga Norland diminta untuk :
* Memebuat program C ketiga dengan nama "4c.c".
* Pada program ini, Norland diminta mengetahui jumlah file dan folder di direktori saat ini dengan command "ls | wc
-l"
*note*: menggunakan IPC Pipes

**Asumsi Soal:**
Disini kami mengasumsikan untuk melakukan fork dimana parent process akan menjadi `write end`  dari pipe dan child
akan menjadi `read end` dari output parrent proccess

**Pembahasan:**
```c
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/wait.h>
```
* `#include <stdio.h>` Library untuk fungsi input-output (e.g. printf(), sprintf())
* `#include <stdlib.h>` Library untuk fungsi umum (e.g. exit(), atoi())
* `#include <unistd.h>` Llibrary untuk melakukan system call kepada kernel linux (e.g. fork())
* `#include <sys/types.h>` Library tipe data khusus (e.g. pid_t)
* `#include <string.h>` Library untuk pendefinisian berbagai fungsi untuk manipulasi array karakter (e.g. strtok())
* `#include<sys/wait.h> `Library untuk pendefinisian symbolic constants untuk penggunaan waitpid(): (e.g. WNOHANG)

```c
int main() {
  int fd[2];

  pid_t pid;

  pipe(fd);
```
* Pertama kami men-set satu `pid`untuk thread dengan **pid_t** dan satu file descriptor  yaitu `fd[2]` dimana
fungsinya adalah

``` c
 pid = fork();
  if (pid == 0) {
    dup2(fd[1], 1);
    close(fd[0]);
    char *argv[] = {"ls", NULL};
    execv("/bin/ls", argv);
  }
  while(wait(NULL) > 0);
```
* Disini akan dilakukan **fork()** dan untuk parent proccesnya, dia akan membuat copy `fd[1]` yang berfungsi
sebgai `write` end dari `pipe`
* Selanjutnya `ls` akan dijalankan pertama untuk menampilkan semua `dir` dan `file` yang ada dan disimpan dalam
pointer `*argv` yang nantinya akan di `execv` pada `/bin/ls` sehingga parrent procces hanya berjalan sekali
* Child Proccess dibuat menunggu hingga parrent selesai menggunakan **while(wait(NULL) > 0)**

``` c
dup2(fd[0], 0);
  close(fd[1]);
  char *argv[] = {"wc", "-l", NULL};
  execv("/usr/bin/wc", argv);
}
```
* Disini **dup2** dijalankan kembali dengan `fd[0]` yang artinya akan menjadi sebgai input atau `read` end dari pipe
baru akan dilakukan perintah `wc` `-l` yang berfungsi untuk menampilkan seluruh jumlah file dan folder yang ada
pada direcotry, dan akan di set terlebih dahulu kedalam pointer `*argv[]` selanjutnya
* `wc``-l` yang ada di dalam pointer `*argv[]` tadi akan di execv menggunakan **execv("/usr/bin/wc", argv)**
sehingga child proccess hanya berjalan sekali

**Kesulitan:**
Tidak ada.

**Screenshot:**\
![Output](https://user-images.githubusercontent.com/17781660/79045199-08712d80-7c34-11ea-81c7-51797ac7465d.png)
