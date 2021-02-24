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

#ifndef PLAGUE_FIREFIGHTER_H
#define PLAGUE_FIREFIGHTER_H

void create_firefighter(firefighter_t* firefighter, unsigned int initial_row, unsigned int initial_col);
void init_firefighter(map_t * map, arg_firefighter_t* arg_firefighter, pthread_t* thread_citizen, int* thread_count);
void* manage_life_firefighter(void* inhabitant);
void random_move_firefighter(arg_firefighter_t* arg_firefighter);
void update_infection_firefighter(arg_firefighter_t* arg_firefighter);
void trigger_disease_firefighter(arg_firefighter_t* arg_firefighter);
void disease_progression_firefighter(arg_firefighter_t* arg_firefighter);
void spread_disease_firefighter(arg_firefighter_t* arg_firefighter);
void burn_corpses(arg_firefighter_t* arg_firefighter);
void reduce_infection(arg_firefighter_t*);

#endif //PLAGUE_FIREFIGHTER_H
