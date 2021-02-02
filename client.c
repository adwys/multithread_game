#include "include.h"
#include <signal.h> // kill
#include <errno.h> // errno, ESRCH
#include "server.h"

int main(int argc, char **argv)
{
    del();
//    sem_t* sem = sem_open("/msg_signal", 0);
//    err(sem == SEM_FAILED, "sem_open");
//
//    int fd = shm_open("/msg_data", O_RDWR, 0600);
//    err(fd == -1, "shm_open");
//
//    struct data_t* pdata = (struct data_t*)mmap(NULL, sizeof(struct data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
//    err(pdata == NULL, "mmap");
//
//    srand(time(NULL));
//    int my_id = rand() % 1000;
//    printf("Identyfikator sesji: %d; pdata=%p......\n", my_id, pdata);
//    while(1) {
//    move(pdata,my_id,sem);
//    if(pdata->payload == 'q')break;
//    sem_post(sem);
////    printf("odpowiedz servera: %s\n",pdata->map);
//    }
//    sem_close(sem);
//    munmap(pdata, sizeof(struct data_t));
//    close(fd);
    return 0;
}