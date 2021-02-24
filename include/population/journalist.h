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
 * @version 1.0 2019/12/23
 */

#ifndef PLAGUE_JOURNALIST_H
#define PLAGUE_JOURNALIST_H

#include <sys/ipc.h>
#include <sys/msg.h>
#include "map.h"

typedef struct mesg_buffer {
    long message_type;
    double journalist_contamination;
    int number_of_dead;
    int number_of_infected_citizens;
    double average_contamination_city;
    int death; // usually at 0, 1 if the journalist died
} message_t;

void create_journalist(journalist_t* journalist, unsigned int initial_row, unsigned int initial_col);
void init_journalist(map_t * map, arg_journalist_t* arg_journalist, pthread_t* thread_citizen, int* thread_count);
void* manage_life_journalist(void* inhabitant);
void random_move_journalist(arg_journalist_t* arg_journalist);
void update_infection_journalist(arg_journalist_t* arg_journalist);
void trigger_disease_journalist(arg_journalist_t* arg_journalist);
void disease_progression_journalist(arg_journalist_t* arg_journalist);
void spread_disease_journalist(arg_journalist_t* arg_journalist);
void communication_with_press_agency(arg_journalist_t* arg_journalist);
#endif //PLAGUE_JOURNALIST_H
