#include "server.h"

int main(int argc, char **argv) {

    initscr();
    noecho();
    cbreak();
    getmaxyx(stdscr, win_y, win_x);
    board = newwin(50, 60, 0, 0);
    consola = newwin(win_y + 10, 30, 11, 0);
    pthread_t Server, Client;

    wprintw(consola, "hello pid:%d\n", getpid());
    wrefresh(consola);
    if (new_serv()) {
        wprintw(consola, "Starting server\n");
        pthread_create(&Server, NULL, &server_thr, NULL);
    }
    sleep(3);
    int canconn=-1;
    struct connection conn;
    conn.connected = false;
    for (int i = 0; i < 3; i++) {
        if (canconn == -1)
            canconn = shm_open(SHM_SERV, O_RDWR, 0666);
        if (canconn != -1)break;
        sleep(1);
    }
    if (canconn == -1) {
        wprintw(consola, "\n not connected shuting down");
        wrefresh(consola);
        sleep(1);
        return 1;
    }
    close(canconn);
    wprintw(consola, "\nconnected\n");
    wrefresh(consola);

    pthread_create(&Client, NULL, &client_thr, NULL);
    pthread_join(Client,NULL);
    wclear(board);
    wclear(consola);
    del();
    return 0;
}