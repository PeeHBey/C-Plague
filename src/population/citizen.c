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

#include "population.h"

void create_citizen(citizen_t* citizen, unsigned int initial_row, unsigned int initial_col) {
    citizen->data.pos_row = initial_row;
    citizen->data.pos_col = initial_col;
    citizen->data.has_moved = 0;
    citizen->data.alive = 1;
    citizen->data.sane = 1;
    citizen->data.present = 1;
    citizen->data.contamination = 0.0;
    citizen->data.days_infected = 0;
    citizen->data.start_turn = -1;
}

void init_citizen(map_t * map, arg_citizen_t* arg_citizen, pthread_t* thread_citizen, int* thread_count){
    int counter_citizen = 0;

    int row_rand;
    int col_rand;
    int i;

    arg_citizen = malloc(NB_CITIZEN * sizeof(arg_citizen_t));

    for(i = 0; i < NB_CITIZEN; i++){
        arg_citizen[i].map = map;
    }
    while (counter_citizen < NB_CITIZEN) {
        do{
            row_rand = rand() % MAP_SIZE_ROW;
            col_rand = rand() % MAP_SIZE_COL;
        }while( (map->matrix[row_rand][col_rand].type == 3 && !map->matrix[row_rand][col_rand].number_firefighter) ||
                (map->matrix[row_rand][col_rand].occupation >= map->matrix[row_rand][col_rand].max_capacity));

        create_citizen(&(map->citizens[counter_citizen]), row_rand, col_rand);
        map->citizen_count++;

        arg_citizen[counter_citizen].citizen = &(map->citizens[counter_citizen]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_citizen, (void *) &arg_citizen[counter_citizen]);
        map->matrix[row_rand][col_rand].occupation++;
        counter_citizen++;
        (*thread_count)++;
    }
}

void* manage_life_citizen(void* inhabitant) {
    arg_citizen_t* arg_citizen;
    unsigned int current_turn = 0;

    arg_citizen = (arg_citizen_t*)inhabitant;

    while(current_turn < arg_citizen->map->number_of_turns){
        if (arg_citizen->citizen->data.start_turn == current_turn) {
            if (arg_citizen->citizen->data.alive) {
                random_move_citizen(arg_citizen);
                update_infection_citizen(arg_citizen);
                trigger_disease_citizen(arg_citizen);
                disease_progression_citizen(arg_citizen);
                spread_disease_citizen(arg_citizen);
            } else {
                spread_disease_citizen(arg_citizen);
            }
            current_turn++;
        }
    }

    return inhabitant;
}

void random_move_citizen(arg_citizen_t* arg_citizen) {
    int move;
    int row_rand;
    int col_rand;
    int old_row;
    int old_col;
    int new_row;
    int new_col;
    int row;
    int col;

    move = rand() % 100;

    old_row = arg_citizen->citizen->data.pos_row;
    old_col = arg_citizen->citizen->data.pos_col;
    if (move < MOVE_PROBABILITY || (arg_citizen->map->matrix[old_row][old_col].type == 3 && !arg_citizen->map->matrix[old_row][old_col].number_firefighter)) {
        do {
            row_rand = rand() % 3 - 1;
            col_rand = rand() % 3 - 1;

            new_row = old_row + row_rand;
            new_col = old_col + col_rand;
        }while ((!row_rand && !col_rand)
                    || new_row > MAP_SIZE_ROW - 1
                    || new_row < 0
                    || new_col > MAP_SIZE_COL - 1
                    || new_col < 0
                    || arg_citizen->map->matrix[new_row][new_col].occupation + 1 > arg_citizen->map->matrix[new_row][new_row].max_capacity
                    || (arg_citizen->map->matrix[new_row][new_col].type == 2  && arg_citizen->citizen->data.sane)
                    || (arg_citizen->map->matrix[new_row][new_col].type == 3 && (!arg_citizen->map->matrix[new_row][new_col].number_firefighter)));

        arg_citizen->map->matrix[old_row][old_col].occupation--;
        arg_citizen->map->matrix[new_row][new_col].occupation++;

        arg_citizen->citizen->data.pos_row += row_rand;
        arg_citizen->citizen->data.pos_col += col_rand;

        arg_citizen->citizen->data.has_moved = 1;

        row = arg_citizen->citizen->data.pos_row;
        col = arg_citizen->citizen->data.pos_col;

        if (arg_citizen->map->matrix[row][col].type == 2) {
            arg_citizen->map->matrix[row][col].contamination += arg_citizen->citizen->data.contamination * 0.1 * 0.25;
        } else if (arg_citizen->map->matrix[row][col].type != 3) {
            arg_citizen->map->matrix[row][col].contamination += arg_citizen->citizen->data.contamination * 0.1;
        } else {
            arg_citizen->citizen->data.contamination -= 0.2;
        }

        /* To cap the contamination */
        if(arg_citizen->map->matrix[row][col].contamination > 1){
            arg_citizen->map->matrix[row][col].contamination = 1;
        }
    } else {
        arg_citizen->citizen->data.has_moved = 0;
    }
}

void update_infection_citizen(arg_citizen_t* arg_citizen) {
    double augment;
    int row;
    int col;

    row = arg_citizen->citizen->data.pos_row;
    col = arg_citizen->citizen->data.pos_col;

    if(arg_citizen->citizen->data.has_moved){
        augment = CONTAMINATION_PROGRESSION_MVMT * arg_citizen->map->matrix[row][col].contamination;
    } else{
        augment = CONTAMINATION_PROGRESSION_REST * arg_citizen->map->matrix[row][col].contamination;
    }

    if (arg_citizen->map->matrix[row][col].type == 3) {
        arg_citizen->citizen->data.contamination -= 0.2;
    } else if(arg_citizen->map->matrix[row][col].type == 2){
            arg_citizen->citizen->data.contamination += HOSPITAL_REDUCTION * augment; /* if the citizen is on an hospital, the infection progresses slower */
    } else{
        arg_citizen->citizen->data.contamination += augment;
    }

    /* To cap the contamination */
    if(arg_citizen->citizen->data.contamination > 1){
        arg_citizen->citizen->data.contamination = 1;
    } else if(arg_citizen->citizen->data.contamination < 0){
        arg_citizen->citizen->data.contamination = 0;
    }
}

void trigger_disease_citizen(arg_citizen_t* arg_citizen) {
    double r;
    if (arg_citizen->citizen->data.sane) {
        r = rand()%1000000;
        if (r/1000000 < arg_citizen->citizen->data.contamination) {
            arg_citizen->citizen->data.sane = 0;
            arg_citizen->map->alive_counter--;
            arg_citizen->map->sick_counter++;
        }
    }
}

void disease_progression_citizen(arg_citizen_t* arg_citizen) {

    int days;
    double r;
    unsigned int x;
    unsigned int y;
    double coeff;

    x = arg_citizen->citizen->data.pos_row;
    y = arg_citizen->citizen->data.pos_col;

    if (!arg_citizen->citizen->data.sane) {
        arg_citizen->citizen->data.days_infected++;
    }

    days = arg_citizen->citizen->data.days_infected;

    coeff = 5;

    if (days >= 5) {
        if (arg_citizen->map->matrix[x][y].number_doctor > 0) { /* being on the same tile as a doctor divides risk of death by 2*/
            coeff = coeff / 2;
        }
        if (arg_citizen->map->matrix[x][y].type == 2) { /* being in an hospital divides the risk of death by 4*/
            coeff = coeff / 4;
        }

        r = 0.1 * (rand()%1000);
        if (r < (days-5) * coeff) {
            arg_citizen->citizen->data.alive = 0;
            arg_citizen->map->sick_counter--;
            arg_citizen->map->dead_counter++;
        }
    }
}

void spread_disease_citizen(arg_citizen_t* arg_citizen) {

    unsigned int x;
    unsigned int y;
    int i;
    double r;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_citizen->citizen->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_citizen->citizen->data.pos_col;
    citizens = arg_citizen->map->citizens;
    journalists = arg_citizen->map->journalists;
    firefighters = arg_citizen->map->firefighters;
    doctors = arg_citizen->map->doctors;

    if (!arg_citizen->citizen->data.sane || (!arg_citizen->citizen->data.alive && arg_citizen->citizen->data.present)) {
        for(i=0;i<NB_CITIZEN;i++) {
            r = (double) (rand()%100)/100;
            if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && citizens[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                citizens[i].data.sane = 0;
                arg_citizen->map->alive_counter--;
                arg_citizen->map->sick_counter++;
            }
        }
        for(i=0;i<NB_JOURNALIST;i++) {
            r = (double) (rand()%100)/100;
            if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && journalists[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                journalists[i].data.sane = 0;
                arg_citizen->map->alive_counter--;
                arg_citizen->map->sick_counter++;
            }
        }
        for(i=0;i<NB_DOCTOR;i++) {
            r = (double) (rand()%100)/100;
            if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && doctors[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                doctors[i].data.sane = 0;
                arg_citizen->map->alive_counter--;
                arg_citizen->map->sick_counter++;
            }
        }
        for(i=0;i<NB_FIREFIGHTER;i++) {
            r = (double) (rand()%100)/100;
            if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && firefighters[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION * (1 - FIREFIGHTER_TRANSMISSION_RESISTANCE)) {
                firefighters[i].data.sane = 0;
                arg_citizen->map->alive_counter--;
                arg_citizen->map->sick_counter++;
            }
        }
    }
}