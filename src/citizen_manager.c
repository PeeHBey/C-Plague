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

int main(){
    map_t* m;
    int shmd;
    /* To receive fifo data from epidemic_sim */
    int f_epidemic_sim_r;
    char* fifo_epidemic_sim_r = "/tmp/fifo_citizen_manager_w";
    char buf_epid_r[MAX_BUF];
    /* To send data to epidemic_sim */
    int f_epidemic_sim_w;
    char* fifo_epidemic_sim_w = "/tmp/fifo_epidemic_sim_r";
    /* Fifo for the end */
    int f_end;
    char* fifo_end = "/tmp/fifo_end";
    char buf_end[MAX_BUF];

    shmd = shm_open("/plague", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if(shmd == -1){
        errno = 1;
        perror("Shared memory creation failed");
        return 0;
    }

    if(ftruncate(shmd,sizeof(map_t)) == -1){
        errno = 1;
        perror("Allocation of memory failed");
        return 0;
    }

    m = mmap(NULL, sizeof(map_t), PROT_READ | PROT_WRITE, MAP_SHARED, shmd, 0);

    printf("Waiting for map to be created...\n");
    do {
        f_epidemic_sim_r = open(fifo_epidemic_sim_r, O_RDONLY);
        read(f_epidemic_sim_r, buf_epid_r, MAX_BUF);
        close(f_epidemic_sim_r);
    } while(strcmp(buf_epid_r, "map_created"));
    //while(!m->is_map_created);
    printf("Done\n");

    printf("\nInitializing the population...\n");
    init_population(m);
    printf("Done\n");

    printf("\nSignal transmitted to epidemic_sim\n\nWaiting for the simulation to be finished...\n");
    //m->is_population_initialized = 1;
    mkfifo(fifo_epidemic_sim_w, 0666);
    f_epidemic_sim_w = open(fifo_epidemic_sim_w, O_WRONLY);
    write(f_epidemic_sim_w, "population_initialized", sizeof("population_initialized"));
    close(f_epidemic_sim_w);

    //while(!m->notify_stop_citizen_manager);
    do {
        f_end = open(fifo_end, O_RDONLY);
        read(f_end, buf_end, MAX_BUF);
        close(f_end);
    } while(strcmp(buf_end, "end"));

    unlink(fifo_epidemic_sim_w);
    return 0;
}