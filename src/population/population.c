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
 * @version 1.0 2019/12/22
 */

#include "population.h"

void init_population(map_t * map) {
    arg_firefighter_t* arg_firefighter = NULL;
    arg_doctor_t* arg_doctor = NULL;
    arg_citizen_t* arg_citizen = NULL;
    arg_journalist_t* arg_journalist = NULL;

    pthread_t thread_citizen[POPULATION];
    int thread_count = 0;

    srand(time(NULL));

    init_firefighter(map, arg_firefighter, thread_citizen, &thread_count);
    printf("\tFirefighters initialized!\n");

    init_citizen(map, arg_citizen, thread_citizen, &thread_count);
    printf("\tCitizens initialized!\n");

    init_journalist(map, arg_journalist, thread_citizen, &thread_count);
    printf("\tJournalists initialized!\n");

    init_doctor(map, arg_doctor, thread_citizen, &thread_count);
    printf("\tDoctors initialized!\n");
}