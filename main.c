#include "server.h"


int main(int argc,char ** argv){
//    del();
    initscr();
    noecho();
    cbreak();
//    refresh();
    getmaxyx(stdscr,y,x);
    board=newwin(20,30,0,0);
    consola = newwin(y+10,30,11,0);
    pthread_t Server,Client;

    wprintw(consola,"hello pid:%d\n",getpid());
    wrefresh(consola);
    if(new_serv()){
        wprintw(consola,"Starting server\n");
        pthread_create(&Server,NULL,&server_thr,NULL);
//        pthread_join(Server,NULL);
    }
    sleep(3);
    int canconn = -1;
    for(int i=0;i<3;i++){
        if(canconn == -1)
            canconn = shm_open(SHM_SERV,O_RDWR,0666);
        if(canconn != -1)break;
        sleep(1);
    }
    if(canconn == -1){
        wprintw(consola,"\n not connected shuting down");
        wrefresh(consola);
        sleep(1);
        return 1;
    }
    close(canconn);
    wprintw(consola,"\nconnected\n");
    wrefresh(consola);

    pthread_create(&Client,NULL,&client_thr,NULL);

    while (1);
    del();
    return 0;
}