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
 * @file epidemic_sim.c
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
#include <signal.h>

int main(int arg_c, char* arg_v[]) {
    map_t* m;
    int shmd;
    int number_of_turns = -1;
    int current_turn = 0;
    FILE * f;
    pid_t pid;
    /* To read the fifo of the timer */
    int f_timer;
    char* fifo_timer = "/tmp/fifo_timer";
    char buf_timer[MAX_BUF];
    /* To write the fifo of citizen_manager */
    int f_citizen_manager_w;
    char* fifo_citizen_manager_w = "/tmp/fifo_citizen_manager_w";
    /* To write the fifo of citizen_manager */
    int f_citizen_manager_r;
    char* fifo_citizen_manager_r = "/tmp/fifo_epidemic_sim_r";
    char buf_citi_r[MAX_BUF];
    /* To signal the end to all process */
    int f_end;
    char* fifo_end = "/tmp/fifo_end";

    if(arg_c != 2){
        errno = 1;
        perror("\033[1;31mError: The arguments should be :\n\t$./main <number_of_turns>\n\033[0m");
        exit(1);
    } else{
        number_of_turns = atoi(arg_v[1]);
    }

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

    create_map(m);
    m->number_of_turns = number_of_turns;
    /* Creation of the fifo for citizen_manager to inform that the map is created */
    mkfifo(fifo_citizen_manager_w, 0666);
    f_citizen_manager_w = open(fifo_citizen_manager_w, O_WRONLY);
    write(f_citizen_manager_w, "map_created", sizeof("map_created"));
    close(f_citizen_manager_w);

    /* Wait the signal from citizen_manager*/
    printf("Waiting for population to be initialized...\n");
    do {
        f_citizen_manager_r = open(fifo_citizen_manager_r, O_RDONLY);
        read(f_citizen_manager_r, buf_citi_r, MAX_BUF);
        close(f_citizen_manager_r);
    } while(strcmp(buf_citi_r,"population_initialized"));
    //while(!m->is_population_initialized);
    printf("Done\n");

    f = fopen("evolution.txt","w"); /* empty the file if it's not */
    fprintf(f, "-1\t%d\t%d\t%d\t%d\n", m->alive_counter, m->sick_counter, m->dead_counter,m->burnt_counter);
    fprintf(f, "-1.00001\t%d\t%d\t%d\t%d\n", m->alive_counter, m->sick_counter, m->dead_counter,m->burnt_counter);
    fclose(f);

    display_map(m, 0);

    pid = fork();
    if(pid < 0) {
        perror("Error using fork (gnuplot)");
        return 0;
    }
    if (pid > 0) {
        for (current_turn = 0; current_turn < number_of_turns; current_turn++) {

            do{
                f_timer = open(fifo_timer, O_RDONLY);
                read(f_timer, buf_timer, MAX_BUF);
                close(f_timer);
            } while(strcmp(buf_timer, "start_turn"));

            //while (!m->start_turn) {}
            m->start_turn = 1;

            signal_start_turn(m, current_turn);

            update_wasteland_contamination(m);

            endwin();
            display_map(m, current_turn + 1);
            f = fopen("evolution.txt","a+");
            fprintf(f, "%d\t%d\t%d\t%d\t%d\n", current_turn, m->alive_counter, m->sick_counter, m->dead_counter,m->burnt_counter);
            fclose(f);

            m->start_turn = 0;
        }
        /* Signal the end to all process */
        mkfifo(fifo_end, 0666);
        f_end = open(fifo_end, O_WRONLY);
        write(f_end,"end", sizeof("end"));
        close(f_end);
        unlink(fifo_end);
        //Seems to be a problem here with the timer and press_agency, so we used the shared memory to signal the end
        m->notify_timer_end = 1;
        m->notify_press_agency_end = 1;

        endwin();

        kill(0,SIGKILL);

        printf("\n\n\033[1;92mThe simulation is over !\033[0m\n\n");

        unlink(fifo_citizen_manager_w);

        munmap(m, sizeof(map_t));
        close(shmd);
        shm_unlink("/plague");

        return 0;
    } else {
        for (current_turn = 0; current_turn < number_of_turns; current_turn++) {
            while (!m->start_turn) {}
            execlp("gnuplot", "gnuplot", "-persist", "config", NULL);
        }

    }
}