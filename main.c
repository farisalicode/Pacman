#include "game.h"



int main() 
{
    sem_init(&start_game,0,1);
    sem_wait(&start_game);
    sem_init(&end_game,0,1);
    sem_wait(&end_game);
    pthread_t ui_thread_id = -1;
    pthread_create(&ui_thread_id,NULL,ui,NULL);
    pthread_t game_engine_thread_id = -1;
    pthread_create(&game_engine_thread_id,NULL,game_engine,NULL);
    pthread_exit(NULL);
    return 0;
}

