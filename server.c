//#include "include.h"
#include "server.h"


int new_serv(){
    int ret_val=1;
    int serv_exist = shm_open(SHM_SERV,O_RDWR,0600);
    if(serv_exist != -1){
        struct data_t* asd = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, serv_exist, 0);
//        wprintw(consola,"pid=",asd->pid);
//        wrefresh(consola);
        if(asd != (void*)-1 ) {
//            int x =kill(asd->pid,0)
            if(kill(asd->pid,0) == 0) {
                wprintw(consola, "pid servera=%d", asd->pid);
                ret_val = 0;
            }
        }
        else del();
        munmap(asd,sizeof(struct data_t));
        }
        close(serv_exist);

    return ret_val;
}
void del(){
    shm_unlink(SHM_SERV);
    sem_unlink(COMMON_SEMAPHORE_NAME);
}
void *server_thr(){

    sem_t* sem = sem_open(COMMON_SEMAPHORE_NAME, O_CREAT, 0600, 0);
    err(sem == SEM_FAILED, "sem_open");

    int fd = shm_open(SHM_SERV, O_CREAT | O_RDWR, 0600);
    err(fd == -1, "shm_open");

    ftruncate(fd, sizeof(struct data_t));
    struct data_t* pdata = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");
    pdata->pid = getpid();
    sem_init(&pdata->cs, 1, 1);
    sem_init(&pdata->take,1,1);
//    printf("Gotowe, czekam na klienta; pdata=%p...,pdata->pid\n", pdata);
    wprintw(consola,"Gotowe, czekam na klienta; pdata=%p...,pdata->pid\n", pdata);
    wrefresh(consola);
    while(1){
        take(pdata,sem,0);
        if(getchar() == 'q')break;
        wrefresh(consola);
    }

    return NULL;
}
void *client_thr(){
    wprintw(consola,"Client thread started\n");
    wrefresh(consola);
    sem_t* sem = sem_open(COMMON_SEMAPHORE_NAME, 0);
    err(sem == SEM_FAILED, "sem_open");

    int fd = shm_open(SHM_SERV, O_RDWR, 0600);
    err(fd == -1, "shm_open");

    struct data_t* pdata = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");

    srand(time(NULL));
    int my_id = rand() % 1000;
    wprintw(consola,"Identyfikator sesji: %d; pdata=%p......\nserver pid=%d\n", my_id, pdata,pdata->pid);
    wrefresh(consola);

    wprintw(board,"Wykonaj ruch");
    while(1) {
        make_move(pdata,my_id,sem);
        wrefresh(board);
        if(pdata->payload == 'q')break;
//        sem_post(sem);
//        printf("odpowiedz servera: \n");
    }
    sem_close(sem);
    munmap(pdata, sizeof(struct data_t));
    close(fd);
    return 0;
}

int check_map(int x,int y){

}

void client_map(){
//    MAP_FILE
}

void make_move(struct data_t* pdata, int my_id, sem_t* sem){


        char move = 'x';


        while(move != 'a' && move != 's' && move != 'd' && move != 'w' && move != 'q') {
            move = getchar();
        }
        sem_wait(&pdata->take);
        pdata->payload = move;
        pdata->id = my_id;
        sem_post(&pdata->cs);
        sem_post(sem);
}

void take(struct data_t* pdata,sem_t* sem,int turn){
    sem_wait(sem);
    sem_wait(&pdata->cs);
//    printf("turn: %d player_id: %03d: move:%c\n", turn, pdata->id, pdata->payload);
    wprintw(consola,"turn: %d player_id: %03d: move:%c\n", turn, pdata->id, pdata->payload);
    wrefresh(consola);
    sem_post(&pdata->take);
}

//int printl(WINDOW *win,const char* fmt, ...)
//{
//    va_list ap;
//    va_start(ap, fmt);
//
//    char dest[300];
//    vsnprintf(dest, 300, fmt, ap);
//    va_end(ap);
//
//    sem_wait(&nc);
//    wprintw(win,"%s\n",dest);
//    wrefresh(win);
//    sem_post(&nc);
//    return !ERR;
//}

//int main(int argc, char **argv)
//{
//    sem_t* sem = sem_open("/msg_signal", O_CREAT, 0600, 0);
//    err(sem == SEM_FAILED, "sem_open");
//
//    int fd = shm_open("/msg_data", O_CREAT | O_RDWR, 0600);
//    err(fd == -1, "shm_open");
//
//    ftruncate(fd, sizeof(struct data_t));
//    struct data_t* pdata = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//    err(pdata == NULL, "mmap");
//
//    sem_init(&pdata->cs, 1, 1); // shared, signaled
//    printf("Gotowe, czekam na klienta; pdata=%p...\n", pdata);
//
//    int turn = 0;
//    while (turn < 10) {
//        show_server_map("./map.txt");
//        take(pdata,sem,turn);
//        if(pdata->payload == 'q')break;
////        if(pdata->payload = 'a')strcpy(pdata->map, "UP");
////        sem_post(&pdata->cs);
////        sem_post(sem);
//        sem_wait(sem);
//        turn++;
//    }
//    munmap(pdata, sizeof(struct data_t));
//    close(fd);
//    shm_unlink("/msg_data");
//    sem_close(sem);
//    return 0;
//}
