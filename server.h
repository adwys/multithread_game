#ifndef SERVER_H
#define SERVER_H
#include <stdlib.h>
#include <locale.h>
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <stdarg.h>
#include <wchar.h>
#include <stdio.h>
#include <semaphore.h> //sem*
#include <sys/mman.h> // mmap, munmap, shm_open, shm_unlink
#include <fcntl.h> // O_*
#include <stdlib.h> // exit
#include <unistd.h> // close, ftruncate
#include <string.h> // strcasecmp
#include <time.h> // time
#include <sys/stat.h>
#include <stdio.h>
#include <signal.h>
#include <curses.h>
#define PAYLOAD_SIZE 1024
#define SHM_SERV "/msg_data"
#define COMMON_SEMAPHORE_NAME "/sem"
#define COMMON_MAP_FILE "/map.txt"
WINDOW* board;
WINDOW* consola;
int x,y;
sem_t* nc;
struct data_t {
    sem_t cs; // sekcja krytyczna
    int id;
    int pid;
    char payload;
    char map[5][5];
};

static void err(int c, const char* msg) {
    if (!c)
        return;
    perror(msg);
    exit(1);
};
int printl(WINDOW *win,const char* fmt, ...);
int new_serv();
void *server_thr();
void *client_thr();
void del();
#endif