/*
 * ENSICAEN
 * 6 Boulevard Maréchal Juin
 * F-14050 Caen Cedex
 *
 * This file is owned by ENSICAEN.
 * No portion of this document may be reproduced, copied
 * or revised without written permission of the authors.
 */

/**
 * @file
 * @brief
 *
 */

/**
 * @author Paul Bala <paul.bala@ecole.ensicaen.fr>
 * @author Antoine Villemagne <antoine.villemagne@ecole.ensicaen.fr>
 * @version 1.0 2019/11/25
 */

#include "map.h"
#include "population.h"
#include <errno.h>

int main(int argc, char* argv[]) {
    map_t* m;
    int shmd;
    int timer_value;
    int f_timer;
    char* fifo_timer = "/tmp/fifo_timer";

    if (argc!=2) {
        errno = 1;
        perror("\033[1;31mTurn duration missing !\033[0m");
        return 0;
    }

    timer_value = atoi(argv[1]);
    if (timer_value > 5 || timer_value < 1) {
        errno = 1;
        perror("\033[1;31mTurn duration must be between 1 and 5\033[0m");
        return 0;
    }

    shmd = shm_open("/plague", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);

    if(shmd == -1){
        errno = 1;
        perror("\033[1;31mShared memory creation failed\033[0m");
        return 0;
    }

    if(ftruncate(shmd,sizeof(map_t)) == -1){
        errno = 1;
        perror("\033[1;31mAllocation of memory failed\033[0m");
        return 0;
    }

    m = mmap(NULL, sizeof(map_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmd, 0);

    /* Creation of the fifo */
    mkfifo(fifo_timer, 0666);

    printf("The timer is running with a %ds delay...\n", timer_value);

    while(!m->notify_timer_end) {
        f_timer = open(fifo_timer, O_WRONLY);
        write(f_timer, "start_turn", sizeof("start_turn"));
        close(f_timer);
        sleep(timer_value);
    }

    unlink(fifo_timer);

    return 1;
}