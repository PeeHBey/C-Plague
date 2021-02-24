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

void create_doctor(doctor_t* doctor, unsigned int initial_row, unsigned int initial_col) {
    doctor->data.pos_row = initial_row;
    doctor->data.pos_col = initial_col;
    doctor->data.has_moved = 0;
    doctor->data.alive = 1;
    doctor->data.sane = 1;
    doctor->data.present = 1;
    doctor->data.contamination = 0.0;
    doctor->data.days_infected = 0;
    doctor->data.start_turn = -1;

    if(initial_row == HOSPITAL_ROW && initial_col == HOSPITAL_COL) {
        doctor->medical_kit = 10;
    } else {
        doctor->medical_kit = 5;
    }

    doctor->healed_today = 0;
    doctor->days_out = 0;
}

void init_doctor(map_t * map, arg_doctor_t* arg_doctor, pthread_t* thread_citizen, int* thread_count){
    int counter_doctor = 0;

    int row_rand;
    int col_rand;
    int i;

    arg_doctor = malloc(NB_DOCTOR * sizeof(arg_doctor_t));

    for(i = 0; i < NB_DOCTOR; i++){
        arg_doctor[i].map = map;
    }

    if(NB_DOCTOR >= 1){
        create_doctor(&(map->doctors[0]), HOSPITAL_ROW, HOSPITAL_COL);
        map->doctor_count++;

        arg_doctor[0].doctor = &(map->doctors[0]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_doctor, (void *) &arg_doctor[0]);
        map->matrix[HOSPITAL_ROW][HOSPITAL_COL].occupation++;
        map->matrix[HOSPITAL_ROW][HOSPITAL_COL].number_doctor++;
        counter_doctor++;
        (*thread_count)++;
    }
    while (counter_doctor < NB_DOCTOR) {
        do {
            row_rand = rand() % MAP_SIZE_ROW;
            col_rand = rand() % MAP_SIZE_COL;
        }while(map->matrix[row_rand][col_rand].occupation > map->matrix[row_rand][col_rand].max_capacity
               || (map->matrix[row_rand][col_rand].type == 3 && !map->matrix[row_rand][col_rand].number_firefighter)
               || (map->matrix[row_rand][col_rand].occupation >= map->matrix[row_rand][col_rand].max_capacity));

        create_doctor(&(map->doctors[counter_doctor]), row_rand, col_rand);
        map->doctor_count++;

        arg_doctor[counter_doctor].doctor = &(map->doctors[counter_doctor]);

        pthread_create(&thread_citizen[*thread_count], NULL, manage_life_doctor, (void *) &arg_doctor[counter_doctor]);
        map->matrix[row_rand][col_rand].occupation++;
        map->matrix[row_rand][col_rand].number_doctor++;
        counter_doctor++;
        (*thread_count)++;
    }

}

void* manage_life_doctor(void* inhabitant) {
    arg_doctor_t* arg_doctor;
    unsigned int current_turn = 0;

    arg_doctor = (arg_doctor_t*)inhabitant;

    while(current_turn < arg_doctor->map->number_of_turns){
        if (arg_doctor->doctor->data.start_turn == current_turn) {
            if (arg_doctor->doctor->data.alive) {
                arg_doctor->doctor->healed_today = 0;
                random_move_doctor(arg_doctor);
                cure(arg_doctor);
                update_infection_doctor(arg_doctor);
                trigger_disease_doctor(arg_doctor);
                disease_progression_doctor(arg_doctor);
                spread_disease_doctor(arg_doctor);
            } else {
                spread_disease_doctor(arg_doctor);
            }
            current_turn++;
        }
    }

    return inhabitant;
}

void random_move_doctor(arg_doctor_t* arg_doctor) {
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

    old_row = arg_doctor->doctor->data.pos_row;
    old_col = arg_doctor->doctor->data.pos_col;

    if (move < MOVE_PROBABILITY || (arg_doctor->map->matrix[old_row][old_col].type == 3 && !arg_doctor->map->matrix[old_row][old_col].number_firefighter)) {
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
                || arg_doctor->map->matrix[new_row][new_col].occupation + 1 > arg_doctor->map->matrix[new_row][new_row].max_capacity
                || (arg_doctor->map->matrix[new_row][new_col].type == 3 && (!arg_doctor->map->matrix[new_row][new_col].number_firefighter))
                || (arg_doctor->map->matrix[new_row][new_col].type == 2 && arg_doctor->doctor->days_out < 2));

        arg_doctor->map->matrix[old_row][old_col].occupation--;
        arg_doctor->map->matrix[new_row][new_col].occupation++;

        arg_doctor->map->matrix[old_row][old_col].number_doctor--;
        arg_doctor->map->matrix[new_row][new_col].number_doctor++;

        arg_doctor->doctor->data.pos_row += row_rand;
        arg_doctor->doctor->data.pos_col += col_rand;
        arg_doctor->doctor->data.has_moved = 1;

        row = arg_doctor->doctor->data.pos_row;
        col = arg_doctor->doctor->data.pos_col;

        if (arg_doctor->map->matrix[row][col].type == 2) {
            arg_doctor->map->matrix[row][col].contamination += arg_doctor->doctor->data.contamination * 0.1 * 0.25;
            arg_doctor->doctor->days_out = 0;
            arg_doctor->doctor->medical_kit += 10;

        } else if (arg_doctor->map->matrix[row][col].type != 3) {
            arg_doctor->map->matrix[row][col].contamination += arg_doctor->doctor->data.contamination * 0.1;
        }

        /* To cap the contamination */
        if(arg_doctor->map->matrix[row][col].contamination > 1){
            arg_doctor->map->matrix[row][col].contamination = 1;
        }
    } else {
        arg_doctor->doctor->data.has_moved = 0;
    }

    row = arg_doctor->doctor->data.pos_row;
    col = arg_doctor->doctor->data.pos_col;

    if(arg_doctor->map->matrix[row][col].type != 2){ arg_doctor->doctor->days_out++; }
}

void update_infection_doctor(arg_doctor_t* arg_doctor) {
    double augment;
    int row;
    int col;

    row = arg_doctor->doctor->data.pos_row;
    col = arg_doctor->doctor->data.pos_col;

    if (arg_doctor->doctor->data.has_moved) {
        augment = CONTAMINATION_PROGRESSION_MVMT * arg_doctor->map->matrix[row][col].contamination;
    } else {
        augment = CONTAMINATION_PROGRESSION_REST * arg_doctor->map->matrix[row][col].contamination;
    }

    if (arg_doctor->map->matrix[row][col].type == 3) {
        arg_doctor->doctor->data.contamination -= 0.2;
    } else if (arg_doctor->map->matrix[row][col].type == 2) {
        arg_doctor->doctor->data.contamination += HOSPITAL_REDUCTION * augment; /* if the citizen is on an hospital, the infection progresses slower */
    } else {
        arg_doctor->doctor->data.contamination += augment;
    }

    /* To keep the contamination between 0 and 1 */
    if(arg_doctor->doctor->data.contamination > 1){
        arg_doctor->doctor->data.contamination = 1;
    } else if(arg_doctor->doctor->data.contamination < 0){
        arg_doctor->doctor->data.contamination = 0;
    }
}

void trigger_disease_doctor(arg_doctor_t* arg_doctor) {
    double r;
    if (arg_doctor->doctor->data.sane) {
        r = rand()%1000000;
        if (r/1000000 < arg_doctor->doctor->data.contamination) {
            arg_doctor->doctor->data.sane = 0;
            arg_doctor->map->alive_counter--;
            arg_doctor->map->sick_counter++;
        }
    }
}

void disease_progression_doctor(arg_doctor_t* arg_doctor) {
    int days;
    double r;
    unsigned int x;
    unsigned int y;
    double coeff;

    x = arg_doctor->doctor->data.pos_row;
    y = arg_doctor->doctor->data.pos_col;

    if (!arg_doctor->doctor->data.sane) {
        arg_doctor->doctor->data.days_infected++;
    }

    days = arg_doctor->doctor->data.days_infected;

    coeff = 2.5;

    if (days >= 5) {
        if (arg_doctor->map->matrix[x][y].type == 2) { /* being in an hospital divides the risk of death by 4*/
            coeff = coeff / 4;
        }

        r = 0.1 * (rand()%1000);
        if (r < (days-5) * coeff) {
            arg_doctor->doctor->data.alive = 0;
            arg_doctor->map->sick_counter--;
            arg_doctor->map->dead_counter++;
        }
    }
}

void spread_disease_doctor(arg_doctor_t* arg_doctor) {

    unsigned int x;
    unsigned int y;
    int i;
    double r;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;

    x = arg_doctor->doctor->data.pos_row; /* this block is here to simplify the lecture of the code */
    y = arg_doctor->doctor->data.pos_col;
    citizens = arg_doctor->map->citizens;
    journalists = arg_doctor->map->journalists;
    firefighters = arg_doctor->map->firefighters;
    doctors = arg_doctor->map->doctors;

    if (!arg_doctor->doctor->data.sane || (!arg_doctor->doctor->data.alive && arg_doctor->doctor->data.present)) {

        for(i=0;i<NB_CITIZEN;i++) {
            r = (double) (rand()%100)/100;
            if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && citizens[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                citizens[i].data.sane = 0;
                arg_doctor->map->alive_counter--;
                arg_doctor->map->sick_counter++;
            }
        }
        for(i=0;i<NB_JOURNALIST;i++) {
            r = (double) (rand()%100)/100;
            if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && journalists[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                journalists[i].data.sane = 0;
                arg_doctor->map->alive_counter--;
                arg_doctor->map->sick_counter++;
            }
        }
        for(i=0;i<NB_DOCTOR;i++) {
            r = (double) (rand()%100)/100;
            if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && doctors[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION) {
                doctors[i].data.sane = 0;
                arg_doctor->map->alive_counter--;
                arg_doctor->map->sick_counter++;
            }
        }
        for(i=0;i<NB_FIREFIGHTER;i++) {
            r = (double) (rand()%100)/100;
            if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && firefighters[i].data.sane && r < CORPSES_AND_SICK_TRANSMISSION * (1 - FIREFIGHTER_TRANSMISSION_RESISTANCE)) {
                firefighters[i].data.sane = 0;
                arg_doctor->map->alive_counter--;
                arg_doctor->map->sick_counter++;
            }
        }
    }
}

void cure(arg_doctor_t* arg_doctor) {
    int i;
    unsigned int on_hospital;
    unsigned int x;
    unsigned int y;
    citizen_t* citizens;
    journalist_t* journalists;
    firefighter_t* firefighters;
    doctor_t* doctors;
    double min_contamination;
    int most_infected_index;
    unsigned int most_infected_type; /* 0 citizen | 1 journalist | 2 doctor | 3 firefighter */

    min_contamination = 0;
    most_infected_type = -1;
    most_infected_index = -1;

    x = arg_doctor->doctor->data.pos_row;
    y = arg_doctor->doctor->data.pos_col;
    citizens = arg_doctor->map->citizens;
    journalists = arg_doctor->map->journalists;
    firefighters = arg_doctor->map->firefighters;
    doctors = arg_doctor->map->doctors;

    if (x == HOSPITAL_ROW && y == HOSPITAL_COL) {
        on_hospital = 1;
    }

    if(arg_doctor->doctor->medical_kit > 0 && !arg_doctor->doctor->healed_today) {
        if (!arg_doctor->doctor->data.sane) {
            if(arg_doctor->doctor->data.days_infected < 10) {
                arg_doctor->doctor->data.sane = 1;
                arg_doctor->doctor->data.days_infected = 0;
                arg_doctor->map->sick_counter--;
                arg_doctor->map->alive_counter++;
                if (!on_hospital) {
                    arg_doctor->doctor->medical_kit--;
                }
            }
        } else { /* if the doctor is sane, he can heal the most infected person on his tile */
            for(i=0;i<NB_CITIZEN;i++) {
                if(citizens[i].data.pos_row == x && citizens[i].data.pos_col == y && !citizens[i].data.sane && citizens[i].data.alive && citizens[i].data.contamination > min_contamination) {
                    min_contamination = citizens[i].data.contamination;
                    most_infected_index = i;
                    most_infected_type = 0;
                }
            }
            for(i=0;i<NB_JOURNALIST;i++) {
                if(journalists[i].data.pos_row == x && journalists[i].data.pos_col == y && !journalists[i].data.sane && journalists[i].data.alive && journalists[i].data.contamination > min_contamination) {
                    min_contamination = journalists[i].data.contamination;
                    most_infected_index = i;
                    most_infected_type = 1;
                }
            }
            for(i=0;i<NB_DOCTOR;i++) {
                if(doctors[i].data.pos_row == x && doctors[i].data.pos_col == y && !doctors[i].data.sane && doctors[i].data.alive && doctors[i].data.contamination > min_contamination) {
                    min_contamination = doctors[i].data.contamination;
                    most_infected_index = i;
                    most_infected_type = 2;
                }
            }
            for(i=0;i<NB_FIREFIGHTER;i++) {
                if(firefighters[i].data.pos_row == x && firefighters[i].data.pos_col == y && !firefighters[i].data.sane && firefighters[i].data.alive && firefighters[i].data.contamination > min_contamination) {
                    min_contamination = firefighters[i].data.contamination;
                    most_infected_index = i;
                    most_infected_type = 3;
                }
            }

            switch (most_infected_type) { /* maybe refactor this later (DRY) */
                case -1:
                    break;
                case 0:
                    citizens[most_infected_index].data.sane = 1;
                    citizens[most_infected_index].data.days_infected = 0;
                    arg_doctor->map->sick_counter--;
                    arg_doctor->map->alive_counter++;
                    if (!on_hospital) {
                        arg_doctor->doctor->medical_kit--;
                    }
                    arg_doctor->doctor->healed_today = 1;
                    break;
                case 1:
                    journalists[most_infected_index].data.sane = 1;
                    journalists[most_infected_index].data.days_infected = 0;
                    arg_doctor->map->sick_counter--;
                    arg_doctor->map->alive_counter++;
                    if (!on_hospital) {
                        arg_doctor->doctor->medical_kit--;
                    }
                    arg_doctor->doctor->healed_today = 1;
                    break;
                case 2:
                    doctors[most_infected_index].data.sane = 1;
                    doctors[most_infected_index].data.days_infected = 0;
                    arg_doctor->map->sick_counter--;
                    arg_doctor->map->alive_counter++;
                    if (!on_hospital) {
                        arg_doctor->doctor->medical_kit--;
                    }
                    arg_doctor->doctor->healed_today = 1;
                    break;
                case 3:
                    firefighters[most_infected_index].data.sane = 1;
                    firefighters[most_infected_index].data.days_infected = 0;
                    arg_doctor->map->sick_counter--;
                    arg_doctor->map->alive_counter++;
                    if (!on_hospital) {
                        arg_doctor->doctor->medical_kit--;
                    }
                    arg_doctor->doctor->healed_today = 1;
                    break;
            }
        }
    }
}

