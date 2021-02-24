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
 * @version 1.0 2019/12/09
 */

#ifndef PLAGUE_DATA_H
#define PLAGUE_DATA_H

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>

/** For the map */
#define MAP_SIZE_ROW 7
#define MAP_SIZE_COL 7
#define HOSPITAL_ROW MAP_SIZE_ROW/2
#define HOSPITAL_COL MAP_SIZE_COL/2
#define BARRACK_1_ROW 0
#define BARRACK_1_COL MAP_SIZE_COL-1
#define BARRACK_2_ROW MAP_SIZE_ROW-1
#define BARRACK_2_COL 0

#define NUMBER_OF_HOUSES 12

#define WASTELAND_CAPACITY 16
#define HOUSE_CAPACITY 6
#define HOSPITAL_CAPACITY 12
#define BARRACK_CAPACITY 8

#define MOVE_PROBABILITY 60

/** For the display with Ncurses*/
#define DISPLAY_MAP_STARTING_ROW 8
#define DISPLAY_MAP_STARTING_COL 6
#define DISPLAY_CONTAMINATION_STARTING_ROW DISPLAY_MAP_STARTING_ROW
#define DISPLAY_CONTAMINATION_STARTING_COL 2*DISPLAY_MAP_STARTING_COL + 4*MAP_SIZE_COL

/** For the contamination */
#define CONTAMINATION_PROGRESSION_MVMT 0.02
#define CONTAMINATION_PROGRESSION_REST 0.05
#define FIREFIGHTER_RESISTANCE 0.1
#define FIREFIGHTER_CONTAMINATION_MAX_REDUCTION 0.2
#define FIREFIGHTER_MAX_SPRAYER_USAGE 1
#define HOSPITAL_REDUCTION 0.25
#define CORPSES_AND_SICK_TRANSMISSION 0.1
#define FIREFIGHTER_TRANSMISSION_RESISTANCE 0.7

#define INITIAL_PERCENTAGE_INFECTED_WASTELAND 0.1 // from 0 to 1
#define WASTELAND_NEIGHBOR_CONTAMINATION_PERCENTAGE 15 // from 0 to 100

/** For the citizens */
#define NB_CITIZEN 25
#define NB_JOURNALIST 2
#define NB_DOCTOR 4
#define NB_FIREFIGHTER 6

#define POPULATION NB_CITIZEN + NB_JOURNALIST + NB_DOCTOR + NB_FIREFIGHTER

#define MIN(X,Y) (((X) < (Y)) ? (X) : (Y))
#define MAX(X,Y) (((X) > (Y)) ? (X) : (Y))

#define MAX_BUF 1024

typedef struct data{
    unsigned int pos_row;
    unsigned int pos_col;
    unsigned int has_moved;
    unsigned int alive; /* 0 dead, 1 alive */
    unsigned int sane; /* 0 sick, 1 sane */
    unsigned int present; /* 0 burnt, 1 present on the map (dead, sick or alive) */
    double contamination;
    unsigned int days_infected;
    unsigned int start_turn;
} data_t;

typedef struct citizen{
    data_t data;
} citizen_t;

typedef struct journalist{
    data_t data;
} journalist_t;

typedef struct doctor{
    data_t data;
    unsigned int medical_kit;
    unsigned int healed_today; /* 0 no, 1 yes*/
    unsigned int days_out;
} doctor_t;

typedef struct firefighter{
    data_t data;
    double sprayer_level;
} firefighter_t;

#endif //PLAGUE_DATA_H
