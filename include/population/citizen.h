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

#ifndef PLAGUE_CITIZEN_H
#define PLAGUE_CITIZEN_H

void create_citizen(citizen_t* citizen, unsigned int initial_row, unsigned int initial_col);
void init_citizen(map_t * map, arg_citizen_t* arg_citizen, pthread_t* thread_citizen, int* thread_count);
void* manage_life_citizen(void* inhabitant);
void random_move_citizen(arg_citizen_t* arg_citizen);
void update_infection_citizen(arg_citizen_t* arg_citizen);
void trigger_disease_citizen(arg_citizen_t* arg_citizen);
void disease_progression_citizen(arg_citizen_t* arg_citizen);
void spread_disease_citizen(arg_citizen_t* arg_citizen);
#endif //PLAGUE_CITIZEN_H
