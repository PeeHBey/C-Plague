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

#include <stdio.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "population/journalist.h"
#include "map.h"
#include <math.h>

int main(){
    map_t* m;
    int shmd;
    unsigned int number_of_journalists_still_alive = NB_JOURNALIST;
    key_t key;
    int message_id;
    message_t message_1;
    message_t message_2;

    key = ftok("progfile",65);
    message_id = msgget(key, 0666 | IPC_CREAT);

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

    while(!m->notify_press_agency_end){
        if (m->start_turn) {
            printf("\n\n\n\n###############################\n");
            printf("Press agency messages:\n\n");
            if (!number_of_journalists_still_alive) {
                printf("All journalist are dead\n");
                break;
            } else {
                msgrcv(message_id, &message_1, sizeof(message_1), 1, 0);
                if (message_1.death) {
                    printf("One journalist died!\n");
                    number_of_journalists_still_alive--;
                    sleep(1);
                } else {
                    printf("First journalist message:\n");
                    if (message_1.journalist_contamination > 0.8) {
                        printf("\tJournalist contamination :\t%f\n", message_1.journalist_contamination);
                    }
                    printf("\tNumber of dead :\t%d\n", (int) round(message_1.number_of_dead * 0.65));
                    printf("\tNumber of infected citizens :\t%d\n",
                           (int) round(message_1.number_of_infected_citizens * 0.9));
                    printf("\tAverage city contamination :\t%f\n", message_1.average_contamination_city * 0.9);
                }

                if (number_of_journalists_still_alive > 1) {
                    msgrcv(message_id, &message_2, sizeof(message_2), 1, 0);
                    printf("\nSecond journalist message:\n");
                    if (message_2.journalist_contamination > 0.8) {
                        printf("\tJournalist contamination :\t%f\n", message_2.journalist_contamination);
                    }
                    printf("\tNumber of dead :\t%d\n", (int) round(message_2.number_of_dead * 0.65));
                    printf("\tNumber of infected citizens :\t%d\n",
                           (int) round(message_2.number_of_infected_citizens * 0.9));
                    printf("\tAverage city contamination :\t%f\n", message_2.average_contamination_city * 0.9);
                }
            }
        }
    }

    msgctl(message_id, IPC_RMID, NULL);

    return 0;
}