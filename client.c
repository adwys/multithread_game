#include "include.h"
#include <signal.h> // kill
#include <errno.h> // errno, ESRCH
int main(int argc, char **argv)
{
    sem_t* sem = sem_open(COMMON_SEMAPHORE_NAME, 0);
    err(sem == SEM_FAILED, "sem_open");

    int fd = shm_open(COMMON_FILE_NAME, O_RDWR, 0600);
    err(fd == -1, "shm_open");
    struct data2_t* pdata = (struct data2_t*)mmap(NULL, sizeof(struct data2_t),
            PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");
    int result = kill(pdata->server_pid, 0);
    if (result == -1 && errno == ESRCH) {
        printf("Brak procesu serwera (%d)\n", pdata->server_pid);
        munmap(pdata, sizeof(struct data2_t));
        close(fd);
        sem_close(sem);
        return 1;
    }


    srand(time(NULL));
    int my_id = rand() % 1000;
    printf("Identyfikator sesji: %d; pdata=%p......\n", my_id, (void*)pdata);
    while(1) {
        char msg[PAYLOAD_SIZE];
        printf("MOVE: ");
        char p = getchar();
        if (!p) break;

        if (*msg)
            msg[strlen(msg) - 1] = '\x0';

        sem_wait(&pdata->cs);
        strcpy(pdata->payload, msg);
        pdata->id = my_id;
        sem_post(&pdata->cs);

        sem_post(sem);
    }
    sem_close(sem);
    munmap(pdata, sizeof(struct data2_t));
    close(fd);
    return 0;
}