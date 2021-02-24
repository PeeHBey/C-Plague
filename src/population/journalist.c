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

void create_journalist(journalist_t* journalist, unsigned int initial_row, unsigned int initial_col) {
    journalist->data.pos_row = initial_row;
    journalist->data.pos_col = initial_col;
    journalist->data.has_moved = 0;
    journalist->data.alive = 1;
    journalist->data.sane = 1;
    journalist->data.present = 1;
    journalist->data.contamination = 0.0;
    journalist->data.days_infected = 0;
    journalist->data.start_turn = -1;
}

void init_journalist(map_t * map, arg_journalist_t* arg_journalist, pthread_t* thread_citizen, int* thread_count){
    int counter_journalist = 0;

    int row_rand;
    int col_rand;
    int i;

    arg_journalist = malloc(NB_JOURNALIST * sizeof(arg_journalist_t));

    for(i = 0; i < NB_JOURNALIST; i++){
        arg_journalist[i].map = map;
    }

    while (counter_journalist < NB_JOURNALIST) {
        do{
            row_rand = rand() % MAP_SIZE_ROW;
            col_rand = rand() % MAP_SIZE_COL;
        }while((map->matrix[row_rand][col_rand].type == 3 && !map->matrix[row_rand][col_rand].number_firefighter) ||
               (map->matrix[row_rand][col_rand].occupation >= map->matrix[row_rand][col_rand].max_capacity));

        create_journalist(&(map->journalists[counter_journalist]), row_rand, col_rand);
        map->journalist_count++;

        arg_journalist[counter_journalist].journalist = &(map->journalists[counter_journalist]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_journalist, (void *) &arg_journalist[counter_journalist]);
        map->matrix[row_rand][col_rand].occupation++;
        counter_journalist++;
        (*thread_count)++;
    }
}

void* manage_life_journalist(void* inhabitant) {
    arg_journalist_t* arg_journalist;
    unsigned int current_turn = 0;

    arg_journalist = (arg_journalist_t*)inhabitant;

    while(current_turn < arg_journalist->map->number_of_turns){
        if (arg_journalist->journalist->data.start_turn == current_turn) {
            if (arg_journalist->journalist->data.alive) {
                random_move_journalist(arg_journalist);
                update_infection_journalist(arg_journalist);
                trigger_disease_journalist(arg_journalist);
                disease_progression_journalist(arg_journalist);
                spread_disease_journalist(arg_journalist);
                communication_with_press_agency(arg_journalist);
            } else {
                spread_disease_journalist(arg_journalist);
            }
            current_turn++;
        }
    }
    return inhabitant;
}

void random_move_journalist(arg_journalist_t* arg_journalist) {
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

    old_row = arg_journalist->journalist->data.pos_row;
    old_col = arg_journalist->journalist->data.pos_col;
    if (move < MOVE_PROBABILITY || (arg_journalist->map->matrix[old_row][old_col].type == 3 && !arg_journalist->map->matrix[old_row][old_col].number_firefighter)) {
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
                || arg_journalist->map->matrix[new_row][new_col].occupation + 1 > arg_journalist->map->matrix[new_row][new_row].max_capacity
                || (arg_journalist->map->matrix[new_row][new_col].type == 2 && arg_journalist->journalist->data.sane)
                || (arg_journalist->map->matrix[new_row][new_col].type == 3 && (!arg_journalist->map->matrix[new_row][new_col].number_firefighter)));

        arg_journalist->map->matrix[old_row][old_col].occupation--;
        arg_journalist->map->matrix[new_row][new_col].occupation++;

        arg_journalist->journalist->data.pos_row += row_rand;
        arg_journalist->journalist->data.pos_col += col_rand;
        arg_journalist->journalist->data.has_moved = 1;

        row = arg_journalist->journalist->data.pos_row;
        col = arg_journalist->journalist->data.pos_col;

        if (arg_journalist->map->matrix[row][col].type == 2) {
            arg_journalist->map->matrix[row][col].contamination += arg_journalist->journalist->data.contamination * 0.1 * 0.25;
        } else if (arg_journalist->map->matrix[row][col].type != 3) {
            arg_journalist->map->matrix[row][col].contamination += arg_journalist->journalist->data.contamination * 0.1;
        } else {
            arg_journalist->journalist->data.contamination -= 0.2;
        }

        /* To cap the contamination */
        if(arg_journalist->map->matrix[row][col].contamination > 1){
            arg_journalist->map->matrix[row][col].contamination = 1;
        }
    } else {
        arg_journalist->journalist->data.has_moved = 0;
    }
}

void update_infection_journalist(arg_journalist_t* arg_journalist) {
    double augment;
    int row;
    int col;

    row = arg_journalist->journalist->data.pos_row;
    col = arg_journalist->journalist->data.pos_col;

    if (arg_journalist->journalist->data.has_moved) {
        augment = CONTAMINATION_PROGRESSION_MVMT * arg_journalist->map->matrix[row][col].contamination;
    } else {
        augment = CONTAMINATION_PROGRESSION_REST * arg_journalist->map->matrix[row][col].contamination;
    }

    if(arg_journalist->map->matrix[row][col].type == 3) {
        arg_journalist->journalist->data.contamination -= 0.2;
    } else if (arg_journalist->map->matrix[row][col].type == 2) {
            arg_journalist->journalist->data.contamination += HOSPITAL_REDUCTION * augment; /* if the citizen is on an hospital, the infection progresses slower */
        } else {
        arg_journalist->journalist->data.contamination += augment;
    }

    /* To keep the contamination between 0 and 1 */
    if(arg_journalist->journalist->data.contamination > 1){
        arg_journalist->journalist->data.contamination = 1;
    } else if(arg_journalist->journalist->data.contamination < 0){
        arg_journalist->journalist->data.contamination = 0;
    }
}

void trigger_disease_journalist(arg_journalist_t* arg_journalist) {
    double r;
    if (arg_journalist->journalist->data.sane) {
        r = rand()%1000000;
        if (r/1000000 < arg_journalist->journalist->data.contamination) {
            arg_journalist->journalist->data.sane = 0;
            arg_journalist->map->alive_counter--;
            arg_journalist->map->sick_counter++;
        }
    }
}

void disease_progression_journalist(arg_journalist_t* arg_journalist) {

    int days;
    double r;
    unsigned int x;
    unsigned int y;
    double coeff;

    x = arg_journalist->journalist->data.pos_row;
    y = arg_journalist->journalist->data.pos_col;

    if (!arg_journalist->journalist->data.sane) {
        arg_journalist->journalist->data.days_infected++;
    }

    days = arg_journalist->journalist->data.days_infected;

    coeff = 5;

    if (days >= 5) {
        if (arg_journalist->map->matrix[x][y].number_doctor > 0) { /* being on the same tile as a doctor divides risk of death by 2*/
            coeff = coeff / 2;
        }
        if (arg_journalist->map->matrix[x][y].type == 2) { /* being in an hospital divides the risk of death by 4*/
            coeff = coeff / 4;
        }

        r = 0.1 * (rand()%1000);
        if (r < (days-5) * coeff) {
            arg_journalist->journalist->data.alive = 0;
            communication_with_press_agency(arg_journalist);
            arg_journalist->map->sick_counter--;
            arg_journalist->map->dead_counter++;
        }
    }
}

void spread_disease_journalist(arg_journalist_t* arg_journalist) {

    unsigned int x;
    unsigned int y;
    int i;
    double r;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_journalist->journalist->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_journalist->journalist->data.pos_col;
    citizens = arg_journalist->map->citizens;
    journalists = arg_journalist->map->journalists;
    firefighters = arg_journalist->map->firefighters;
    doctors = arg_journalist->map->doctors;

    if (!arg_journalist->journalist->data.sane || (!arg_journalist->journalist->data.alive && arg_journalist->journalist->data.present)) {

        for(i=0;i<NB_CITIZEN;i++) {
            r = (double) (rand()%100)/100;
            if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && citizens[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                citizens[i].data.sane = 0;
                arg_journalist->map->alive_counter--;
                arg_journalist->map->sick_counter++;
            }
        }
        for(i=0;i<NB_JOURNALIST;i++) {
            r = (double) (rand()%100)/100;
            if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && journalists[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                journalists[i].data.sane = 0;
                arg_journalist->map->alive_counter--;
                arg_journalist->map->sick_counter++;
            }
        }
        for(i=0;i<NB_DOCTOR;i++) {
            r = (double) (rand()%100)/100;
            if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && doctors[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                doctors[i].data.sane = 0;
                arg_journalist->map->alive_counter--;
                arg_journalist->map->sick_counter++;
            }
        }
        for(i=0;i<NB_FIREFIGHTER;i++) {
            r = (double) (rand()%100)/100;
            if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && firefighters[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION * (1 - FIREFIGHTER_TRANSMISSION_RESISTANCE)) {
                firefighters[i].data.sane = 0;
                arg_journalist->map->alive_counter--;
                arg_journalist->map->sick_counter++;
            }
        }
    }
}

void communication_with_press_agency(arg_journalist_t* arg_journalist){
    key_t key;
    int message_id;
    message_t message;

    key = ftok("progfile",65);
    message_id = msgget(key, 0666 | IPC_CREAT);
    message.message_type = 1;

    if(arg_journalist->journalist->data.alive) {
        message.journalist_contamination = arg_journalist->journalist->data.contamination;
        message.number_of_dead = arg_journalist->map->dead_counter;
        message.number_of_infected_citizens = arg_journalist->map->sick_counter;
        message.average_contamination_city = calculate_average_city_contamination(arg_journalist->map);
        message.death = 0;
    } else {
        message.death = 1;
    }

    msgsnd(message_id, &message, sizeof(message), 0);
}