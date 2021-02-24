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

#ifndef PLAGUE_DOCTOR_H
#define PLAGUE_DOCTOR_H

void create_doctor(doctor_t* doctor, unsigned int initial_row, unsigned int initial_col);
void init_doctor(map_t * map, arg_doctor_t* arg_doctor, pthread_t* thread_citizen, int* thread_count);
void* manage_life_doctor(void* inhabitant);
void random_move_doctor(arg_doctor_t* arg_doctor);
void update_infection_doctor(arg_doctor_t* arg_doctor);
void trigger_disease_doctor(arg_doctor_t* arg_doctor);
void disease_progression_doctor(arg_doctor_t* arg_doctor);
void spread_disease_doctor(arg_doctor_t* arg_doctor);
void cure(arg_doctor_t* arg_doctor);

#endif //PLAGUE_DOCTOR_H
