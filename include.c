#include "include.h"


//void msg(struct data_t* pdata, int my_id, sem_t* sem){
//    char msg[PAYLOAD_SIZE];
//    printf("Podaj tekst: ");
//    char* p = fgets(msg, PAYLOAD_SIZE, stdin);
//    if (*msg)
//        msg[strlen(msg) - 1] = '\x0';
//
//    sem_wait(&pdata->cs);
//    strcpy(pdata->payload, msg);
//    pdata->id = my_id;
//    sem_post(&pdata->cs);
//    sem_post(sem);
//}


void show_server_map(char * map){
    FILE *f = fopen(map,"r");
    if(f == NULL)return;
    char line[1024];
    while(!feof(f)){
        fgets(line,1024,f);
        printf("%s",line);
    }
    fclose(f);
}