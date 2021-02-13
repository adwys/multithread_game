//#include "include.h"
#include "server.h"


int new_serv(){
    int ret_val=1;
    int serv_exist = shm_open(SHM_SERV,O_RDWR,0600);
    if(serv_exist != -1){
        struct server_data_t* asd = (struct server_data_t*)mmap(NULL, sizeof(struct server_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, serv_exist, 0);
        if(asd != (void*)-1 ) {
            if(kill(asd->pid,0) == 0) {
                wprintw(consola, "pid servera=%d", asd->pid);
                ret_val = 0;
            }
        }
        else del();
        munmap(asd,sizeof(struct server_data_t));
    }
    close(serv_exist);

    return ret_val;
}
void del(){
    shm_unlink(SHM_SERV);
    shm_unlink(SHM_CL);
    sem_unlink(COMMON_SEMAPHORE_NAME);
}
void *server_thr(){

    sem_t* sem = sem_open(COMMON_SEMAPHORE_NAME, O_CREAT, 0600, 0);
    err(sem == SEM_FAILED, "sem_open");

    int fd = shm_open(SHM_SERV, O_CREAT | O_RDWR, 0600);
    err(fd == -1, "shm_open");

    ftruncate(fd, sizeof(struct server_data_t));
    struct server_data_t* pdata = (struct server_data_t*)mmap(NULL, sizeof(struct server_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");
    pdata->pid = getpid();
    sem_init(&pdata->cs, 1, 0);
    sem_init(&pdata->cl_wait,1,0);
    sem_init(&pdata->cl_ready,1,0);
    sem_init(&pdata->server_run,1,0);
    pdata->server_ready=false;
    load_map(pdata);
    for(int i=0;i<3;i++){
        pdata->client[i].pid=0;
    }

    pdata->clients=0;
    // Waiting for clinet to connect
    wprintw(consola,"Gotowe, czekam na klienta\n");
    wrefresh(consola);
    int curr = 0,i;
    while(1){
        sem_post(&pdata->cl_wait);
        sem_wait(&pdata->cl_ready);

        if(curr<pdata->clients) {
            wprintw(consola, "server dolaczyl gracz o pid=%d\n", pdata->client[curr].pid);
            wrefresh(consola);
            curr++;
        }
        if(curr >= 2)break;
    }
    //BEAST THREAD
    pthread_t beast_thr;
    sem_post(&pdata->cl_wait);
    pthread_create(&beast_thr,NULL,client_thr,NULL);
    sem_wait(&pdata->cl_ready);
    wprintw(consola,"Game Start\n");
    wrefresh(consola);
    for(int i=0;i<3;i++){
        sem_init(&pdata->client[i].request,1,0);
    }
    for(int x=0;x<3;x++)sem_post(&pdata->server_run);
    pdata->server_ready=true;
    while(pdata->server_ready){
        for(int i=0;i<3;i++){
            sem_post(&pdata->client[i].request);
            sleep(0.5);
            if(pdata->client[i].input == 'a' || pdata->client[i].input == 'w' || pdata->client[i].input == 's' || pdata->client[i].input == 'd'){
                change_cords(&pdata->client[i],pdata);
                player_update(&pdata,&pdata->client[i]);
                if(i!=2)client_map(&pdata,i);
            }

        }
        if(pdata->client[0].exit_request)pdata->server_ready=false;
        if(pdata->client[1].exit_request)pdata->server_ready=false;
        sleep(1);
    }

    return NULL;
}
int find_player_byxy(int x,int y,struct server_data_t *pdata){
    for(int i=0;i<2;i++){
        if(pdata->client[i].x == x && pdata->client[i].y == y)return i;
    }
    return -1;
}
void kill_player(struct client_data_t *cdata){
    //11 24
    cdata->x = 24;
    cdata->y = 11;
    cdata->deaths++;
    cdata->coins_in_backpack=0;
    cdata->curr_square='A';
}
bool square_action(struct client_data_t *cdata,struct server_data_t* pdata,int x,int y){
    if(check_place(cdata->x+x,cdata->y+y,pdata) == empty) {
        return true;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == player) {
        if(cdata->bot){
            int player_cl = find_player_byxy(cdata->x,cdata->y,pdata);
            if(player_cl!=-1)kill_player(&pdata->client[player_cl]);
            return true;
        }
        return false;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == beast) {
        kill_player(cdata);
        return false;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == coin){
        cdata->coins_in_backpack+=1;
        return true;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == treasure){
        cdata->coins_in_backpack+=5;
        return true;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == big_treasure){
        cdata->coins_in_backpack+=15;
        return true;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == bush){
        cdata->in_bush=true;
        return true;
    }
    if(check_place(cdata->x+x,cdata->y+y,pdata) == camp){
        cdata->saved_coins=cdata->coins_in_backpack;
        cdata->coins_in_backpack=0;
        cdata->campsite_known=true;
        cdata->cmp_x=cdata->x+x;
        cdata->cmp_y=cdata->y+y;
        return true;
    }
//    wprintw(consola,"square_action error");
//    wrefresh(consola);
    return false;
}
int change_cords(struct client_data_t *cdata, struct server_data_t* pdata){
    if(cdata->input == 'a'){
        if(square_action(cdata,pdata,-1,0)){
            cdata->x--;
        }
    }
    else if(cdata->input == 'd'){
        if(square_action(cdata,pdata,+1,0)){
            cdata->x++;
        }
    }
    else if(cdata->input == 'w'){
        if(square_action(cdata,pdata,0,-1)){
            cdata->y--;
        }
    }
    else if(cdata->input == 's'){
        if(square_action(cdata,pdata,0,+1)){
            cdata->y++;
        }
    }
    cdata->input = ' ';
    return 0;
}
int load_map(struct server_data_t* pdata){
    FILE *f = fopen("../map.txt","r");
    if(f== NULL){
        f = fopen("map.txt","r");
    }
    char map[26][55] = {0};
    char buff[1024];
    for(int i=0;i<26;i++){
        fgets(pdata->map[i],54,f);
    }
    fclose(f);
    return 0;
}

int player_update(struct server_data_t** pdata,struct client_data_t* cdata){
//    if(!check_map(cdata->x,cdata->y,*pdata))return -1;
    for(int i=0;i<26;i++){
        for(int j=0;j<55;j++){
            if((*pdata)->map[i][j] == cdata->look){
                if(cdata->curr_square == 'c' || cdata->curr_square == 't' || cdata->curr_square == 'T')cdata->curr_square=' ';
                (*pdata)->map[i][j] = cdata->curr_square;
            }
        }
    }
    cdata->curr_square = (*pdata)->map[cdata->y][cdata->x];
    (*pdata)->map[cdata->y][cdata->x] = cdata->look;

    return 0;
}
int player_create(struct client_data_t* cdata,struct server_data_t* pdata,int cl){
    srand(time(0));
    int my_id = rand() % 1000;
    int player_x,player_y;
    do{
        player_x = rand() % 50;
        player_y = rand() % 25;
    }while(!check_map(player_x,player_y,pdata));
    cdata->curr_square=' ';
    cdata->in_bush=false;
    cdata->look = cdata->num + '0';
    if(cdata->bot)cdata->look = '*';
    cdata->campsite_known=false;
    cdata->cmp_x=0;
    cdata->cmp_y=0;
    cdata->deaths=0;
    cdata->coins_in_backpack=0;
    cdata->saved_coins=0;
    cdata->x=player_x;
    cdata->y=player_y;
    cdata->num=cl;
    (cdata)->pid=getpid();
    (cdata)->cl_id=my_id;

}

int find_client(struct server_data_t* pdata,pid_t my_pid){

    for(int i=0;i<3;i++){
        if(pdata->client[i].pid == my_pid)return i;
    }
    return -1;
}
void display_ledger(struct client_data_t *cdata,int server_pid){
    wprintw(ledger,"Server's PID: %d\n",server_pid);
    wprintw(ledger,"Campsite X/Y: ");
    if(cdata->campsite_known)wprintw(ledger,"%d %d",cdata->cmp_x,cdata->cmp_y);
    wprintw(ledger,"\n");
    wprintw(ledger,"Player \n");
    wprintw(ledger,"NUMBER: %d \n",cdata->num);
    wprintw(ledger,"CURR X/Y: %d/%d \n",cdata->x,cdata->y);
    wprintw(ledger,"Deaths: %d \n",cdata->deaths);
    wprintw(ledger,"Coins in backpack: %d \n",cdata->coins_in_backpack);
    wprintw(ledger,"Coins saved: %d \n",cdata->saved_coins);
    wrefresh(ledger);
}

void *client_thr(){
    wprintw(consola,"Client thread started\n");
    wrefresh(consola);
    sem_t* sem = sem_open(COMMON_SEMAPHORE_NAME, 0);
    err(sem == SEM_FAILED, "sem_open");
    int fd = shm_open(SHM_SERV, O_RDWR, 0600);
    err(fd == -1, "shm_open");

    struct server_data_t* pdata = (struct server_data_t*)mmap(NULL, sizeof(struct server_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    err(pdata == NULL, "mmap");

    //create client data
    sem_wait(&pdata->cl_wait);
    pdata->client[pdata->clients].bot= pdata->clients==2 ? true:false;
    bool bot_thr = pdata->client[pdata->clients].bot;
    player_create(&pdata->client[pdata->clients],pdata,pdata->clients);
    pdata->clients++;
    sem_post(&pdata->cl_ready);
    wprintw(consola,"Dolaczanie zakonczone\n");
    wrefresh(consola);
    while(!pdata->server_ready); // czekanie na rozpoczecie gry
    int cl = find_client(pdata,getpid());
    if(bot_thr)cl = 2;
    if(cl == -1){
        wprintw(board,"ERROR");
        wrefresh(board);
        return NULL;
    }
    player_update(&pdata,&pdata->client[cl]);
    if(!pdata->client[cl].bot)client_map(&pdata,cl);
//    wprintw(consola,"gracz nr=%d pos x=%d y=%d\n",cl,pdata->client[cl].x,pdata->client[cl].y);
//    wrefresh(consola);
    pdata->client[cl].exit_request=false;
    pthread_t cl_input;
    if(!pdata->client[cl].bot)pthread_create(&cl_input,NULL,&player_input_thr,&pdata->client[cl]);
//    sem_wait(&pdata->server_run);
    if(!pdata->client[cl].bot)ledger = newwin(100, 60, 0, 40);
    srand(time(0));
    while(pdata->server_ready) {
        if(bot_thr){
            int beast_mov = rand() % 100;
            if(beast_mov<25){
                pdata->client[cl].input='a';
            }
            else if(beast_mov<50){
                pdata->client[cl].input='d';
            }
            else if(beast_mov<75){
                pdata->client[cl].input='w';
            }
            else{
                pdata->client[cl].input='s';
            }
            struct timespec time;
            clock_gettime(CLOCK_REALTIME, &time);
            time.tv_sec += 60;
            sem_timedwait(&pdata->client[2].request, &time);
            if(pdata->client[2].in_bush){
                sleep(1);
                pdata->client[2].in_bush=false;
            }
//            sleep(1);
        }
        else {
            for (int i = 0; i < 5; i++) {
                wprintw(board, "%s\n", pdata->client[cl].map[i]);
                wrefresh(board);
            }
            wclear(board);
            display_ledger(&pdata->client[cl], pdata->pid);
            wclear(ledger);
        }
        sleep(1);
    }
    wprintw(consola,"EXIT");
    wrefresh(consola);
    sem_close(sem);
    munmap(pdata, sizeof(struct server_data_t));
    close(fd);
    wclear(board);
    wclear(consola);
//    pthread_exit(NULL);
    return NULL;
}
bool bush_wait(struct client_data_t* cdata){ // NIE UZYWAC
    if(cdata->input == 'a'){
        if(cdata->map[cdata->y][cdata->x-1] == '#')return true;
    }
    if(cdata->input == 'w'){
        if(cdata->map[cdata->y-1][cdata->x] == '#')return true;
    }
    if(cdata->input == 's'){
        if(cdata->map[cdata->y+1][cdata->x] == '#')return true;
    }
    if(cdata->input == 'd') {
        if(cdata->map[cdata->y][cdata->x+1] == '#')return true;
    }
    return false;
}

void *player_input_thr(void * arg){
    wprintw(consola,"Make a move:");
    wrefresh(consola);
    struct client_data_t* cdata = (struct client_data_t*)arg;
    int move = 'h';
    while(move != 'q'){
        move = getch();
        cdata->input = move;
//        sem_post(&cdata->ready); // NIE POTRZEBNE
        struct timespec time;
        clock_gettime(CLOCK_REALTIME, &time);
        time.tv_sec += 60;
        sem_timedwait(&cdata->request, &time);
        if(cdata->in_bush){
            sleep(1);
            cdata->in_bush=false;
        }
    }
    cdata->exit_request=true;
    pthread_exit(NULL);
}
// NIE uzywane
int take_player_input(struct server_data_t* pdata,int cl){
    struct timespec time;
    clock_gettime(CLOCK_REALTIME,&time);
    time.tv_sec+=60;
    sem_timedwait(&pdata->client[cl].request,&time);

}
enum squares check_place(int x,int y, struct server_data_t* pdata){
    if(check_map(x,y,pdata))return empty;
    if(pdata->map[y][x] == '#')return bush;
    if(pdata->map[y][x] == 'A')return camp;
    if(pdata->map[y][x] == 'c')return coin;
    if(pdata->map[y][x] == 't')return treasure;
    if(pdata->map[y][x] == 'T')return big_treasure;
    if(pdata->map[y][x] == '1' || pdata->map[y][x] == '0')return player;
    if(pdata->map[y][x] == '*')return beast;

}

int check_map(int x,int y,struct server_data_t* pdata){ // NIE UZYWAC

    if(pdata->map[y][x] == ' ')return 1;
    return 0;
}

void client_map(struct server_data_t** pdata,int clinet){

    //  / trzeba tez zrobic w  koncu input graczy / bestie i reszte gry
    int x = (*pdata)->client[clinet].x>=5 ? (*pdata)->client[clinet].x-3:0;
    int y = (*pdata)->client[clinet].y>=5 ? (*pdata)->client[clinet].y-3:0;

    for(int i=0;i<5;i++){
        for(int j=0;j<5;j++){
            (*pdata)->client[clinet].map[i][j] = (*pdata)->map[y+i][x+j];
        }
        (*pdata)->client[clinet].map[i][5] = '\0';
    }

}

