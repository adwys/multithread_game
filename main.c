#include "server.h"
//TODO zamykanie servera po wyjsciu gracza/servera (DONE) bez obsługi ctrl+q
//TODO ruch po wiecej niz pustym polu (DONE)
//TODO nie dzaiala updatowanie mapy z graczem update_client (DONE)
//TODO bestia i zabijanie gracza
//TODO zbieranie coin oraz odkładanie ich w bazie (DONE)
//TODO board z informacjami (DONE)


int main(int argc, char **argv) {
//    del();
    initscr();
    noecho();
    cbreak();
//    refresh();
    getmaxyx(stdscr, win_y, win_x);
    board = newwin(50, 60, 0, 0);
    consola = newwin(win_y + 10, 30, 11, 0);
    pthread_t Server, Client;

    wprintw(consola, "hello pid:%d\n", getpid());
    wrefresh(consola);
    if (new_serv()) {
        wprintw(consola, "Starting server\n");
        pthread_create(&Server, NULL, &server_thr, NULL);
//        pthread_join(Server,NULL);
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