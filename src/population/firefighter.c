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

void create_firefighter(firefighter_t* firefighter, unsigned int initial_row, unsigned int initial_col) {
    firefighter->data.pos_row = initial_row;
    firefighter->data.pos_col = initial_col;
    firefighter->data.has_moved = 0;
    firefighter->data.alive = 1;
    firefighter->data.sane = 1;
    firefighter->data.present = 1;
    firefighter->data.contamination = 0.0;
    firefighter->data.days_infected = 0;
    firefighter->data.start_turn = -1;

    if((initial_row == BARRACK_1_ROW && initial_col == BARRACK_1_COL) || (initial_row == BARRACK_2_ROW && initial_col == BARRACK_2_COL)) {
        firefighter->sprayer_level = 10;
    } else {
        firefighter->sprayer_level = 5;
    }
}

void init_firefighter(map_t * map, arg_firefighter_t* arg_firefighter, pthread_t* thread_citizen, int* thread_count){
    int counter_firefighter = 0;

    int row_rand;
    int col_rand;
    int i;

    arg_firefighter = malloc(NB_FIREFIGHTER * sizeof(arg_firefighter_t));

    for(i = 0; i < NB_FIREFIGHTER; i++){
        arg_firefighter[i].map = map;
    }

    if (NB_FIREFIGHTER >= 1) {
        create_firefighter(&(map->firefighters[counter_firefighter]),BARRACK_1_ROW, BARRACK_1_COL);
        map->firefighter_count++;

        arg_firefighter[0].firefighter = &(map->firefighters[counter_firefighter]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_firefighter, (void *) &arg_firefighter[0]);
        map->matrix[BARRACK_1_ROW][BARRACK_1_COL].occupation++;
        map->matrix[BARRACK_1_ROW][BARRACK_1_COL].number_firefighter++;
        counter_firefighter++;
        (*thread_count)++;
    }
    if (NB_FIREFIGHTER >= 2){
        create_firefighter(&(map->firefighters[counter_firefighter]),BARRACK_2_ROW, BARRACK_2_COL);
        map->firefighter_count++;

        arg_firefighter[1].firefighter = &(map->firefighters[counter_firefighter]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_firefighter, (void *) &arg_firefighter[1]);
        map->matrix[BARRACK_2_ROW][BARRACK_2_COL].occupation++;
        map->matrix[BARRACK_2_ROW][BARRACK_2_COL].number_firefighter++;
        counter_firefighter++;
        (*thread_count)++;
    }
    while (counter_firefighter < NB_FIREFIGHTER) {
        do{
            row_rand = rand() % MAP_SIZE_ROW;
            col_rand = rand() % MAP_SIZE_COL;
        } while(map->matrix[row_rand][col_rand].occupation >= map->matrix[row_rand][col_rand].max_capacity);
        create_firefighter(&(map->firefighters[counter_firefighter]),row_rand, col_rand);

        map->firefighter_count++;

        arg_firefighter[counter_firefighter].firefighter = &(map->firefighters[counter_firefighter]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_firefighter, (void *) &arg_firefighter[counter_firefighter]);
        map->matrix[row_rand][col_rand].occupation++;
        map->matrix[row_rand][col_rand].number_firefighter++;
        counter_firefighter++;
        (*thread_count)++;
    }
}

void* manage_life_firefighter(void* inhabitant) {
    arg_firefighter_t* arg_firefighter;
    unsigned int current_turn = 0;

    arg_firefighter = (arg_firefighter_t*)inhabitant;

    while(current_turn < arg_firefighter->map->number_of_turns){
        if (arg_firefighter->firefighter->data.start_turn == current_turn) {
            if (arg_firefighter->firefighter->data.alive) {
                random_move_firefighter(arg_firefighter);
                reduce_infection(arg_firefighter);
                burn_corpses(arg_firefighter);
                update_infection_firefighter(arg_firefighter);
                trigger_disease_firefighter(arg_firefighter);
                disease_progression_firefighter(arg_firefighter);
                spread_disease_firefighter(arg_firefighter);
            } else {
                spread_disease_firefighter(arg_firefighter);
            }
            current_turn++;
        }
    }

    return inhabitant;
}

void random_move_firefighter(arg_firefighter_t* arg_firefighter) {
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

    if (move < MOVE_PROBABILITY) {
        old_row = arg_firefighter->firefighter->data.pos_row;
        old_col = arg_firefighter->firefighter->data.pos_col;
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
                || arg_firefighter->map->matrix[new_row][new_col].occupation + 1 > arg_firefighter->map->matrix[new_row][new_row].max_capacity);

        arg_firefighter->map->matrix[old_row][old_col].occupation--;
        arg_firefighter->map->matrix[new_row][new_col].occupation++;

        arg_firefighter->map->matrix[old_row][old_col].number_firefighter--;
        arg_firefighter->map->matrix[new_row][new_col].number_firefighter++;

        arg_firefighter->firefighter->data.pos_row += row_rand;
        arg_firefighter->firefighter->data.pos_col += col_rand;
        arg_firefighter->firefighter->data.has_moved = 1;

        row = arg_firefighter->firefighter->data.pos_row;
        col = arg_firefighter->firefighter->data.pos_col;

        if(arg_firefighter->map->matrix[row][col].type == 2){
            arg_firefighter->map->matrix[row][col].contamination += arg_firefighter->firefighter->data.contamination * 0.1 * 0.25;
        } else if(arg_firefighter->map->matrix[row][col].type != 3){
            arg_firefighter->map->matrix[row][col].contamination += arg_firefighter->firefighter->data.contamination * 0.1;
        } else {
            arg_firefighter->firefighter->sprayer_level = 10;
        }

        /* To cap the contamination */
        if(arg_firefighter->map->matrix[row][col].contamination > 1){
            arg_firefighter->map->matrix[row][col].contamination = 1;
        }
    } else{
        arg_firefighter->firefighter->data.has_moved = 0;
    }
}

void update_infection_firefighter(arg_firefighter_t* arg_firefighter) {
    double augment;
    int row;
    int col;

    row = arg_firefighter->firefighter->data.pos_row;
    col = arg_firefighter->firefighter->data.pos_col;

    if (arg_firefighter->firefighter->data.has_moved) {
        augment = CONTAMINATION_PROGRESSION_MVMT * FIREFIGHTER_RESISTANCE *
                  arg_firefighter->map->matrix[row][col].contamination;
    } else {
        augment = CONTAMINATION_PROGRESSION_REST * FIREFIGHTER_RESISTANCE *
                  arg_firefighter->map->matrix[row][col].contamination;
    }

    if (arg_firefighter->map->matrix[row][col].type == 3) {
        arg_firefighter->firefighter->data.contamination -= 0.2;
    } else if (arg_firefighter->map->matrix[row][col].type == 2) {
        arg_firefighter->firefighter->data.contamination += HOSPITAL_REDUCTION * augment; /* if the citizen is on an hospital, the infection progresses slower */
    } else {
        arg_firefighter->firefighter->data.contamination += augment;
    }

    /* To keep the contamination between 0 and 1 */
    if (arg_firefighter->firefighter->data.contamination > 1) {
        arg_firefighter->firefighter->data.contamination = 1;
    } else if (arg_firefighter->firefighter->data.contamination < 0) {
        arg_firefighter->firefighter->data.contamination = 0;
    }
}

void trigger_disease_firefighter(arg_firefighter_t* arg_firefighter) {
    double r;
    if (arg_firefighter->firefighter->data.sane) {
        r = rand()%1000000;
        if (r/1000000 < arg_firefighter->firefighter->data.contamination) {
            arg_firefighter->firefighter->data.sane = 0;
            arg_firefighter->map->alive_counter--;
            arg_firefighter->map->sick_counter++;
        }
    }
}

void disease_progression_firefighter(arg_firefighter_t* arg_firefighter) {

    int days;
    double r;
    unsigned int x;
    unsigned int y;
    double coeff;

    x = arg_firefighter->firefighter->data.pos_row;
    y = arg_firefighter->firefighter->data.pos_col;

    if (!arg_firefighter->firefighter->data.sane) {
        arg_firefighter->firefighter->data.days_infected++;
    }

    days = arg_firefighter->firefighter->data.days_infected;

    coeff = 5;

    if (days >= 5) {
        if (arg_firefighter->map->matrix[x][y].number_doctor > 0) { /* being on the same tile as a doctor divides risk of death by 2*/
            coeff = coeff / 2;
        }
        if (arg_firefighter->map->matrix[x][y].type == 2) { /* being in an hospital divides the risk of death by 4*/
            coeff = coeff / 4;
        }

        r = 0.1 * (rand()%1000);
        if (r < (days-5) * coeff) {
            arg_firefighter->firefighter->data.alive = 0;
            arg_firefighter->map->sick_counter--;
            arg_firefighter->map->dead_counter++;
        }
    }
}

void spread_disease_firefighter(arg_firefighter_t* arg_firefighter) {

    unsigned int x;
    unsigned int y;
    int i;
    double r;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_firefighter->firefighter->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_firefighter->firefighter->data.pos_col;
    citizens = arg_firefighter->map->citizens;
    journalists = arg_firefighter->map->journalists;
    firefighters = arg_firefighter->map->firefighters;
    doctors = arg_firefighter->map->doctors;

    if (!arg_firefighter->firefighter->data.sane || (!arg_firefighter->firefighter->data.alive && arg_firefighter->firefighter->data.present)) {

        for(i=0;i<NB_CITIZEN;i++) {
            r = (double) (rand()%100)/100;
            if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && citizens[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                citizens[i].data.sane = 0;
                arg_firefighter->map->alive_counter--;
                arg_firefighter->map->sick_counter++;
            }
        }
        for(i=0;i<NB_JOURNALIST;i++) {
            r = (double) (rand()%100)/100;
            if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && journalists[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                journalists[i].data.sane = 0;
                arg_firefighter->map->alive_counter--;
                arg_firefighter->map->sick_counter++;
            }
        }
        for(i=0;i<NB_DOCTOR;i++) {
            r = (double) (rand()%100)/100;
            if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && doctors[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                doctors[i].data.sane = 0;
                arg_firefighter->map->alive_counter--;
                arg_firefighter->map->sick_counter++;
            }
        }
        for(i=0;i<NB_FIREFIGHTER;i++) {
            r = (double) (rand()%100)/100;
            if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && firefighters[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION * (1 - FIREFIGHTER_TRANSMISSION_RESISTANCE)) {
                firefighters[i].data.sane = 0;
                arg_firefighter->map->alive_counter--;
                arg_firefighter->map->sick_counter++;
            }
        }
    }
}

void reduce_infection(arg_firefighter_t* arg_firefighter) {
    unsigned int x;
    unsigned int y;
    int i;
    double sprayer_amout_to_use;
    double sprayer_used_today;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_firefighter->firefighter->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_firefighter->firefighter->data.pos_col;
    citizens = arg_firefighter->map->citizens;
    journalists = arg_firefighter->map->journalists;
    firefighters = arg_firefighter->map->firefighters;
    doctors = arg_firefighter->map->doctors;

    sprayer_used_today = 0;

    if (arg_firefighter->firefighter->sprayer_level > 0) {
        for (i = 0; i < NB_CITIZEN; i++) {
            if (citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && citizens[i].data.alive && citizens[i].data.contamination > 0) {
                sprayer_amout_to_use = MIN(MIN(MIN(citizens[i].data.contamination*0.2,FIREFIGHTER_CONTAMINATION_MAX_REDUCTION),arg_firefighter->firefighter->sprayer_level),FIREFIGHTER_MAX_SPRAYER_USAGE - sprayer_used_today);
                citizens[i].data.contamination -= sprayer_amout_to_use;
                arg_firefighter->firefighter->sprayer_level -= sprayer_amout_to_use;
                sprayer_used_today += sprayer_amout_to_use;
            }
        }
        for (i = 0; i < NB_JOURNALIST; i++) {
            if (journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && journalists[i].data.alive && journalists[i].data.contamination > 0) {
                sprayer_amout_to_use = MIN(MIN(MIN(journalists[i].data.contamination*0.2,FIREFIGHTER_CONTAMINATION_MAX_REDUCTION),arg_firefighter->firefighter->sprayer_level),FIREFIGHTER_MAX_SPRAYER_USAGE - sprayer_used_today);
                journalists[i].data.contamination -= sprayer_amout_to_use;
                arg_firefighter->firefighter->sprayer_level -= sprayer_amout_to_use;
                sprayer_used_today += sprayer_amout_to_use;
            }
        }
        for (i = 0; i < NB_DOCTOR; i++) {
            if (doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && doctors[i].data.alive && doctors[i].data.contamination > 0) {
                sprayer_amout_to_use = MIN(MIN(MIN(doctors[i].data.contamination*0.2,FIREFIGHTER_CONTAMINATION_MAX_REDUCTION),arg_firefighter->firefighter->sprayer_level),FIREFIGHTER_MAX_SPRAYER_USAGE - sprayer_used_today);
                doctors[i].data.contamination -= sprayer_amout_to_use;
                arg_firefighter->firefighter->sprayer_level -= sprayer_amout_to_use;
                sprayer_used_today += sprayer_amout_to_use;
            }
        }
        for (i = 0; i < NB_FIREFIGHTER; i++) {
            if (firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && firefighters[i].data.alive && firefighters[i].data.contamination > 0) {
                sprayer_amout_to_use = MIN(MIN(MIN(firefighters[i].data.contamination*0.2,FIREFIGHTER_CONTAMINATION_MAX_REDUCTION),arg_firefighter->firefighter->sprayer_level),FIREFIGHTER_MAX_SPRAYER_USAGE - sprayer_used_today);
                firefighters[i].data.contamination -= sprayer_amout_to_use;
                arg_firefighter->firefighter->sprayer_level -= sprayer_amout_to_use;
                sprayer_used_today += sprayer_amout_to_use;
            }
        }
        sprayer_amout_to_use = MIN(MIN(MIN(arg_firefighter->map->matrix[x][y].contamination*0.2,FIREFIGHTER_CONTAMINATION_MAX_REDUCTION),arg_firefighter->firefighter->sprayer_level),FIREFIGHTER_MAX_SPRAYER_USAGE - sprayer_used_today);
        arg_firefighter->map->matrix[x][y].contamination -= sprayer_amout_to_use;
        arg_firefighter->firefighter->sprayer_level -= sprayer_amout_to_use;
        sprayer_used_today += sprayer_amout_to_use;
    }
}

void burn_corpses(arg_firefighter_t* arg_firefighter) {
    unsigned int x;
    unsigned int y;
    int i;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_firefighter->firefighter->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_firefighter->firefighter->data.pos_col;
    citizens = arg_firefighter->map->citizens;
    journalists = arg_firefighter->map->journalists;
    firefighters = arg_firefighter->map->firefighters;
    doctors = arg_firefighter->map->doctors;

    for(i=0;i<NB_CITIZEN;i++) {
        if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && !citizens[i].data.alive && citizens[i].data.present) {
            citizens[i].data.present = 0;
            arg_firefighter->map->dead_counter--;
            arg_firefighter->map->burnt_counter++;
        }
    }
    for(i=0;i<NB_JOURNALIST;i++) {
        if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && !journalists[i].data.alive && journalists[i].data.present) {
            journalists[i].data.present = 0;
            arg_firefighter->map->dead_counter--;
            arg_firefighter->map->burnt_counter++;
        }
    }
    for(i=0;i<NB_DOCTOR;i++) {
        if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && !doctors[i].data.alive && doctors[i].data.present) {
            doctors[i].data.present = 0;
            arg_firefighter->map->dead_counter--;
            arg_firefighter->map->burnt_counter++;
        }
    }
    for(i=0;i<NB_FIREFIGHTER;i++) {
        if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && !firefighters[i].data.alive && firefighters[i].data.present) {
            firefighters[i].data.present = 0;
            arg_firefighter->map->dead_counter--;
            arg_firefighter->map->burnt_counter++;
        }
    }
}

