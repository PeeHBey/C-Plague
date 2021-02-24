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


#ifndef PLAGUE_MAP_H
#define PLAGUE_MAP_H

#include "data.h"
#include <ncurses.h>

typedef struct tile{
    unsigned int row;
    unsigned int col;
    unsigned int type; /*0 wasteland, 1 house, 2 hospital, 3 barrack*/
    unsigned int max_capacity;
    unsigned int occupation;
    double contamination;
    unsigned int number_doctor;
    unsigned int number_firefighter;
} tile_t;

typedef struct map {
    unsigned int row;
    unsigned int col;
    tile_t matrix[MAP_SIZE_ROW][MAP_SIZE_COL];

    unsigned int citizen_count;
    unsigned int doctor_count;
    unsigned int firefighter_count;
    unsigned int journalist_count;
    citizen_t citizens[NB_CITIZEN];
    doctor_t doctors[NB_DOCTOR];
    firefighter_t firefighters[NB_FIREFIGHTER];
    journalist_t journalists[NB_JOURNALIST];
    unsigned int alive_counter;
    unsigned int sick_counter;
    unsigned int dead_counter;
    unsigned int burnt_counter;

    unsigned int start_turn; /* usually at 0, the timer changes it to 1 each turn*/
    unsigned int number_of_turns;

    unsigned int notify_timer_end;
    unsigned int notify_press_agency_end;
} map_t;

typedef struct arg_citizen{
    citizen_t* citizen;
    map_t* map;
}arg_citizen_t;

typedef struct arg_doctor{
    doctor_t* doctor;
    map_t* map;
}arg_doctor_t;

typedef struct arg_firefighter{
    firefighter_t* firefighter;
    map_t* map;
}arg_firefighter_t;

typedef struct arg_journalist{
    journalist_t* journalist;
    map_t* map;
}arg_journalist_t;

void create_tile(unsigned int row, unsigned int col, unsigned int type, tile_t* tile);

tile_t** alloc_matrix();

void generate_house_positions(map_t* map);

void generate_infected_wasteland(map_t* map);

void create_map(map_t* map);

void signal_start_turn(map_t* map, int turn_counter);

void update_wasteland_contamination(map_t* map);

double calculate_average_city_contamination(map_t* map);

int** calculate_alive_citizens_matrix(map_t* map);

int calculate_number_alive_citizens(map_t* map);

int calculate_number_alive_doctors(map_t* map);

int calculate_number_alive_firefighters(map_t* map);

int calculate_number_alive_journalists(map_t* map);

void display_legend();

void display_map(map_t* map, unsigned int turn_number);

#endif //PLAGUE_MAP_H
