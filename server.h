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
#include <stdbool.h>
#define PAYLOAD_SIZE 1024
#define SHM_SERV "/msg_data"
#define SHM_CL "/cl_data"
#define COMMON_SEMAPHORE_NAME "/sem"
#define GAME_MAP_FILE "../map.txt"
WINDOW* board;
WINDOW* consola;
WINDOW* ledger;
int win_x,win_y;
sem_t* nc;
struct connection {
    pid_t pid;
    bool connected;
};


struct client_data_t {
    sem_t request;
    sem_t ready;
    int input;
    int num;
    int cl_id;
    int x,y;
    int cmp_x,cmp_y;
    int pid;
    unsigned int coins_in_backpack;
    unsigned int saved_coins;
    char curr_square;
    int deaths;
    bool campsite_known;
    bool in_bush;
    bool bot; // server ustawia bestie, trzeba jeszcze zrobic arg w treadzie kileta
    bool exit_request;
    char map[6][6];
};
struct server_data_t {
    sem_t cs;
    sem_t cl_wait;
    sem_t cl_ready;
    sem_t server_run;
    bool server_ready;
    int id;
    int pid;
    char payload;
    int clients;
    char map[26][55];
    struct client_data_t client[3];
};


static void err(int c, const char* msg) {
    if (!c)
        return;
    perror(msg);
    exit(1);
};
void display_ledger(struct client_data_t *cdata,int server_pid);
enum squares{wall,empty,bush,coin,treasure,big_treasure,camp};
enum squares check_place(int x,int y, struct server_data_t* pdata);
bool bush_wait(struct client_data_t* cdata);
bool square_action(struct client_data_t *cdata,struct server_data_t* pdata,int x,int y);
int change_cords(struct client_data_t *cdata,struct server_data_t *pdata);
int take_player_input(struct server_data_t* pdata,int cl);
int load_map(struct server_data_t* pdata);
int player_update(struct server_data_t** pdata,struct client_data_t* cdata);
int find_client(struct server_data_t* pdata,pid_t my_pid);
void client_map(struct server_data_t** pdata,int clinet);
int player_create(struct client_data_t *cdata,struct server_data_t* pdata, int cl );
int check_map(int x,int y,struct server_data_t *pdata);
void make_move(struct server_data_t* pdata, int my_id, sem_t* sem);
void take(struct server_data_t* pdata, sem_t* sem, int turn);
int new_serv();
void *server_thr();
void *client_thr();
void *player_input_thr(void * arg);
void del();
#endif