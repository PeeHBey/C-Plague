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

#include "map.h"
#include <errno.h>

void create_tile(unsigned int row, unsigned int col, unsigned int type, tile_t* tile) {

    tile->row = row;
    tile->col = col;
    tile->type = type;
    tile->occupation = 0;
    tile->contamination = 0;
    tile->number_doctor = 0;
    tile->number_firefighter = 0;

    if( !type ) {
        tile->max_capacity = WASTELAND_CAPACITY;
    } else if( type == 1 ){
        tile->max_capacity = HOUSE_CAPACITY;
    } else if( type == 2 ) {
        tile->max_capacity = HOSPITAL_CAPACITY;
    } else if( type == 3 ){
        tile->max_capacity = BARRACK_CAPACITY;
    }
}

void generate_house_positions(map_t* map){
    int row_rand;
    int col_rand;
    int counter;

    if (NUMBER_OF_HOUSES > MAP_SIZE_ROW * MAP_SIZE_COL - 3) {
        errno = 1;
        perror("Too many houses\n");
    }

    counter = 0;
    srand(time(NULL));

    while( counter < NUMBER_OF_HOUSES ){
        row_rand = rand() % MAP_SIZE_ROW;
        col_rand = rand() % MAP_SIZE_COL;

        if( !map->matrix[row_rand][col_rand].type){
            map->matrix[row_rand][col_rand].type = 1;
            map->matrix[row_rand][col_rand].max_capacity = HOUSE_CAPACITY;
            counter++;
        }
    }

}

void generate_infected_wasteland(map_t* map){
    int wasteland_to_infect;
    int infected_x;
    int infected_y;
    int i;
    double infection_rate;

    wasteland_to_infect = round(INITIAL_PERCENTAGE_INFECTED_WASTELAND * ((MAP_SIZE_ROW * MAP_SIZE_COL) - (NUMBER_OF_HOUSES + 3)));

    for(i = 0; i < wasteland_to_infect; i++) {
        do {
            infected_x = rand() % MAP_SIZE_ROW;
            infected_y = rand() % MAP_SIZE_COL;
        } while(map->matrix[infected_x][infected_y].type || map->matrix[infected_x][infected_y].contamination);

        infection_rate = rand()%21;
        map->matrix[infected_x][infected_y].contamination = (infection_rate+20)/100;
    }
}

void create_map(map_t* m){
    int i;
    int j;

    m->row = MAP_SIZE_ROW;
    m->col = MAP_SIZE_COL;

    m->start_turn = 0;
    m->notify_timer_end = 0;
    m->notify_press_agency_end = 0;

    m->citizen_count = 0;
    m->doctor_count = 0;
    m->firefighter_count = 0;
    m->journalist_count = 0;

    m->alive_counter = POPULATION;
    m->sick_counter = 0;
    m->dead_counter = 0;
    m->burnt_counter = 0;

    for(i = 0; i < MAP_SIZE_ROW; i++){
        for(j = 0; j < MAP_SIZE_COL; j++){
            create_tile(i, j, 0, &m->matrix[i][j]);
        }
    }

    m->matrix[HOSPITAL_ROW][HOSPITAL_COL].type = 2;
    m->matrix[HOSPITAL_ROW][HOSPITAL_COL].max_capacity = HOSPITAL_CAPACITY;

    m->matrix[BARRACK_1_ROW][BARRACK_1_COL].type = 3;
    m->matrix[BARRACK_1_ROW][BARRACK_1_COL].max_capacity = BARRACK_CAPACITY;

    m->matrix[BARRACK_2_ROW][BARRACK_2_COL].type = 3;
    m->matrix[BARRACK_2_ROW][BARRACK_2_COL].max_capacity = BARRACK_CAPACITY;

    generate_house_positions(m);

    generate_infected_wasteland(m);
}

void signal_start_turn(map_t* map, int turn_counter){
    unsigned int i;

    for(i = 0; i < NB_CITIZEN; i++){
        map->citizens[i].data.start_turn = turn_counter;
    }
    for(i = 0; i < NB_DOCTOR; i++){
        map->doctors[i].data.start_turn = turn_counter;
    }
    for(i = 0; i < NB_FIREFIGHTER; i++){
        map->firefighters[i].data.start_turn = turn_counter;
    }
    for(i = 0; i < NB_JOURNALIST; i++){
        map->journalists[i].data.start_turn = turn_counter;
    }
}

void update_wasteland_contamination(map_t* map){
    int i;
    int j;
    int k;
    int l;
    double augment;
    double diff;

    for(i = 0; i < MAP_SIZE_ROW; i++){
        for(j = 0; j < MAP_SIZE_COL; j++){
            if(!map->matrix[i][j].type){
                /* Explore the 8 around tiles */
                for(k = -1; k < 2; k++){
                    for(l = -1; l < 2; l++){
                        if(i + k >= 0 && i + k < MAP_SIZE_ROW
                            && j + l >= 0 && j + l < MAP_SIZE_COL
                            && !map->matrix[i+k][j+l].type
                            && map->matrix[i+k][j+l].contamination < map->matrix[i][j].contamination // This test forbid the case k=0 and l=0
                            && rand()%100 < WASTELAND_NEIGHBOR_CONTAMINATION_PERCENTAGE)
                        {
                            diff = map->matrix[i][j].contamination - map->matrix[i+k][j+l].contamination;
                            augment = diff * (rand()%19 + 1)/100;
                            map->matrix[i+k][j+l].contamination += augment;
                        }
                    }
                }
            }
        }
    }
}

double calculate_average_city_contamination(map_t* map){
    int i;
    int j;
    double sum = 0.0;

    for(i = 0; i < MAP_SIZE_ROW; i++){
        for(j = 0; j < MAP_SIZE_COL; j++){
            sum += map->matrix[i][j].contamination;
        }
    }
    return sum/MAP_SIZE_ROW/MAP_SIZE_COL;
}

int** calculate_alive_citizens_matrix(map_t* map){
    int** alive_matrix;
    int i;
    int row;
    int col;

    alive_matrix = (int**)malloc(MAP_SIZE_ROW * sizeof(int*));
    for(i = 0; i < MAP_SIZE_ROW; i++){
        alive_matrix[i] = (int*)calloc(MAP_SIZE_COL, sizeof(int));
    }

    for(i = 0; i < NB_CITIZEN; i++){
        if(map->citizens[i].data.alive){
            row = map->citizens[i].data.pos_row;
            col = map->citizens[i].data.pos_col;
            alive_matrix[row][col]++;
        }
    }
    for(i = 0; i < NB_DOCTOR; i++){
        if(map->doctors[i].data.alive){
            row = map->doctors[i].data.pos_row;
            col = map->doctors[i].data.pos_col;
            alive_matrix[row][col]++;
        }
    }
    for(i = 0; i < NB_FIREFIGHTER; i++){
        if(map->firefighters[i].data.alive){
            row = map->firefighters[i].data.pos_row;
            col = map->firefighters[i].data.pos_col;
            alive_matrix[row][col]++;
        }
    }
    for(i = 0; i < NB_JOURNALIST; i++){
        if(map->journalists[i].data.alive){
            row = map->journalists[i].data.pos_row;
            col = map->journalists[i].data.pos_col;
            alive_matrix[row][col]++;
        }
    }

    return alive_matrix;
}

int calculate_number_alive_citizens(map_t* map){
    int i;
    unsigned int number_alive = 0;

    for(i = 0; i < NB_CITIZEN; i++){
        if(map->citizens[i].data.alive){ number_alive++; }
    }

    return number_alive;
}

int calculate_number_alive_doctors(map_t* map) {
    int i;
    unsigned int number_alive = 0;

    for (i = 0; i < NB_DOCTOR; i++) {
        if (map->doctors[i].data.alive) { number_alive++; }
    }

    return number_alive;
}

int calculate_number_alive_firefighters(map_t* map) {
    int i;
    unsigned int number_alive = 0;

    for(i = 0; i < NB_FIREFIGHTER; i++){
        if(map->firefighters[i].data.alive){ number_alive++; }
    }

    return number_alive;
}

int calculate_number_alive_journalists(map_t* map) {
    int i;
    unsigned int number_alive = 0;

    for(i = 0; i < NB_JOURNALIST; i++){
        if(map->journalists[i].data.alive){ number_alive++; }
    }

    return number_alive;
}

void display_legend(){
    initscr();

    start_color();
    init_pair(1,COLOR_BLACK,75); // Light blue
    init_pair(2,COLOR_BLACK,33); // Blue
    init_pair(3,COLOR_BLACK,71); // Green
    init_pair(4,COLOR_BLACK,77); // Light green
    init_pair(5,COLOR_BLACK,221); // Yellow
    init_pair(6,COLOR_BLACK,208); // Orange
    init_pair(7,COLOR_BLACK,160); // Red
    init_pair(8,COLOR_WHITE,52); // Dark red

    // Displays the legend of the citizen map
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +1, DISPLAY_MAP_STARTING_COL +4, "Legend:");
    attron(COLOR_PAIR(5));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +2, DISPLAY_MAP_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(5));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +2, DISPLAY_MAP_STARTING_COL +10, "Wasteland");
    attron(COLOR_PAIR(3));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +3, DISPLAY_MAP_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(3));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +3, DISPLAY_MAP_STARTING_COL +10, "House");
    attron(COLOR_PAIR(2));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +4, DISPLAY_MAP_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(2));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +4, DISPLAY_MAP_STARTING_COL +10, "Hospital");
    attron(COLOR_PAIR(7));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +5, DISPLAY_MAP_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(7));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +5, DISPLAY_MAP_STARTING_COL +10, "Barrack");

    // Displays the legend of the contamination map
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +1, DISPLAY_CONTAMINATION_STARTING_COL +4, "Legend:");
    attron(COLOR_PAIR(1));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +2, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(1));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +2, DISPLAY_CONTAMINATION_STARTING_COL +10, "No infection");
    attron(COLOR_PAIR(2));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +3, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(2));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +3, DISPLAY_CONTAMINATION_STARTING_COL +10, "Very light infection");
    attron(COLOR_PAIR(3));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +4, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(3));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +4, DISPLAY_CONTAMINATION_STARTING_COL +10, "Light infection");
    attron(COLOR_PAIR(4));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +5, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(4));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +5, DISPLAY_CONTAMINATION_STARTING_COL +10, "Medium infection");
    attron(COLOR_PAIR(5));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +6, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(5));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +6, DISPLAY_CONTAMINATION_STARTING_COL +10, "Intermediate infection");
    attron(COLOR_PAIR(6));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +7, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(6));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +7, DISPLAY_CONTAMINATION_STARTING_COL +10, "High infection");
    attron(COLOR_PAIR(7));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +8, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(7));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +8, DISPLAY_CONTAMINATION_STARTING_COL +10, "Very High infection");
    attron(COLOR_PAIR(8));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +9, DISPLAY_CONTAMINATION_STARTING_COL +6, "   ");
    attroff(COLOR_PAIR(8));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +9, DISPLAY_CONTAMINATION_STARTING_COL +10, "Critical infection");
}

void display_map(map_t* map, unsigned int turn_number){
    unsigned int i;
    unsigned int j;
    int** alive_matrix;

    alive_matrix = calculate_alive_citizens_matrix(map);

    initscr();

    mvprintw(0,0, "Turn n°%d",turn_number);
    mvprintw(DISPLAY_MAP_STARTING_ROW-3, MAP_SIZE_COL*2 ,"Citizen Map");
    mvprintw(DISPLAY_MAP_STARTING_ROW-3, DISPLAY_CONTAMINATION_STARTING_COL ,"Contamination Map");

    // Declaration of used colors
    start_color();
    init_pair(1,COLOR_BLACK,75); // Light blue
    init_pair(2,COLOR_BLACK,33); // Blue
    init_pair(3,COLOR_BLACK,71); // Green
    init_pair(4,COLOR_BLACK,77); // Light green
    init_pair(5,COLOR_BLACK,221); // Yellow
    init_pair(6,COLOR_BLACK,208); // Orange
    init_pair(7,COLOR_BLACK,160); // Red
    init_pair(8,COLOR_WHITE,52); // Dark red

    display_legend();

    // Displays the index of columns
    for(j = 0; j < MAP_SIZE_COL; j++){
        mvprintw(DISPLAY_MAP_STARTING_ROW-1, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "%3d", j+1);
        mvprintw(DISPLAY_CONTAMINATION_STARTING_ROW-1, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "%3d ", j+1);
    }

    for(i = 0; i < MAP_SIZE_ROW; i++){
        mvprintw(i + DISPLAY_MAP_STARTING_ROW, DISPLAY_MAP_STARTING_COL-1, "%3d ", i+1); // Displays the index of rows
        mvprintw(i + DISPLAY_MAP_STARTING_ROW, DISPLAY_CONTAMINATION_STARTING_COL-4, "%3d ", i+1);
        for(j = 0; j < MAP_SIZE_COL; j++){
            // First we display the map (with the type of each tile and the number of citizens)
            if(!map->matrix[i][j].type){
                attron(COLOR_PAIR(5));
                if(alive_matrix[i][j]){ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "%3d", alive_matrix[i][j]);}
                else{ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "   "); }
                attroff(COLOR_PAIR(5));
            } else if(map->matrix[i][j].type == 1){
                attron(COLOR_PAIR(3));
                if(alive_matrix[i][j]){ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "%3d", alive_matrix[i][j]);}
                else{ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "   "); }
                attroff(COLOR_PAIR(3));
            } else if(map->matrix[i][j].type == 2){
                attron(COLOR_PAIR(2));
                if(alive_matrix[i][j]){ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "%3d", alive_matrix[i][j]);}
                else{ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "   "); }
                attroff(COLOR_PAIR(2));
            } else if(map->matrix[i][j].type == 3){
                attron(COLOR_PAIR(7));
                if(alive_matrix[i][j]){ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "%3d", alive_matrix[i][j]);}
                else{ mvprintw(i + DISPLAY_MAP_STARTING_ROW, (j+1)*3 + DISPLAY_MAP_STARTING_COL, "   "); }
                attroff(COLOR_PAIR(7));
            }

            // Then we display the contamination map
            if(!map->matrix[i][j].contamination){
                attron(COLOR_PAIR(1));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(1));
            } else if(map->matrix[i][j].contamination <= 0.0025){
                attron(COLOR_PAIR(2));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(2));
            } else if(map->matrix[i][j].contamination <= 0.01){
                attron(COLOR_PAIR(3));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(3));
            } else if(map->matrix[i][j].contamination <= 0.025){
                attron(COLOR_PAIR(4));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(4));
            } else if(map->matrix[i][j].contamination <= 0.05){
                attron(COLOR_PAIR(5));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(5));
            } else if(map->matrix[i][j].contamination <= 0.2){
                attron(COLOR_PAIR(6));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(6));
            } else if(map->matrix[i][j].contamination <= 0.4){
                attron(COLOR_PAIR(7));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(7));
            } else{
                attron(COLOR_PAIR(8));
                mvprintw(i + DISPLAY_CONTAMINATION_STARTING_ROW, 3*j + DISPLAY_CONTAMINATION_STARTING_COL, "   ");
                attroff(COLOR_PAIR(8));
            }
        }
    }

    // Calculate the number of alive citizens
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +12, 4, "Number of citizens still alive: %4d", calculate_number_alive_citizens(map));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +13, 4, "Number of journalists still alive: %4d", calculate_number_alive_journalists(map));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +14, 4, "Number of doctors still alive: %4d", calculate_number_alive_doctors(map));
    mvprintw(DISPLAY_MAP_STARTING_ROW + MAP_SIZE_ROW +15, 4, "Number of firefighters still alive: %4d", calculate_number_alive_firefighters(map));

    mvprintw(LINES-2, COLS/2, "Press a key to continue");
    mvprintw(LINES-1, COLS-1, " "); // Just not to display the entered input of the getch
    getch();
    //endwin();
}

/** The display function without Ncurses*/
/*void display_map(map_t* map){
    unsigned int i;
    unsigned int j;
    unsigned int k;
    int total_occupation = 0;
    unsigned int number_alive = 0;

    // To print the title of each map
    printf("\t    ");
    for(i = 0; i < MAP_SIZE_COL/2-1; i++){
        printf("    ");
    }
    printf("Map of citizens");
    for(i = 0; i < MAP_SIZE_COL/2-1; i++){
        printf("    ");
    }
    printf("\t\t ");
    for(i = 0; i < MAP_SIZE_COL/2-1; i++){
        printf("         ");
    }
    printf("Map of infection\n");

    // To print the index of column of each map
    printf("\t    ");
    for(i = 0; i < MAP_SIZE_COL; i++){
        printf("%3d ",i+1);
    }
    printf("\t\t ");
    for(i = 0; i < MAP_SIZE_COL; i++){
        printf("%8d ",i+1);
    }

    // To print the map itself
    printf("\n");
    for(i = 0; i < MAP_SIZE_ROW; i++){
        // To print the index of row
        printf("\t%3d ",i+1);

        // To print the citizen map
        for(j = 0; j < MAP_SIZE_COL; j++) {
            if(!map->matrix[i][j].type){
                printf("\033[7;33m%3d \033[0m", map->matrix[i][j].occupation);
            } else if(map->matrix[i][j].type == 1){
                printf("\033[7;32m%3d \033[0m", map->matrix[i][j].occupation);
            } else if(map->matrix[i][j].type == 2){
                printf("\033[7;34m%3d \033[0m", map->matrix[i][j].occupation);
            } else if(map->matrix[i][j].type == 3){
                printf("\033[7;31m%3d \033[0m", map->matrix[i][j].occupation);
            }
            total_occupation += map->matrix[i][j].occupation;
        }

        printf("\t\t%3d ",i+1);
        // To print the infection map
        for (k = 0; k < MAP_SIZE_COL ; k++) {
            if(!map->matrix[i][k].contamination){
                printf("\033[0;30;48;5;75m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.0025){
                printf("\033[0;30;48;5;33m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.01){
                printf("\033[0;30;48;5;71m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.025){
                printf("\033[0;30;48;5;77m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.05){
                printf("\033[0;30;48;5;221m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.2){
                printf("\033[0;30;48;5;208m%6f \033[0m", map->matrix[i][k].contamination);
            } else if(map->matrix[i][k].contamination < 0.5){
                printf("\033[0;30;48;5;160m%6f \033[0m", map->matrix[i][k].contamination);
            } else{
                printf("\033[48;5;52m%6f \033[0m", map->matrix[i][k].contamination);
            }
        }
        printf("\n");
    }
    printf("\nTotal occupation : %d\n", total_occupation);

    // To display the number of alive citizens
    for(i = 0; i < NB_CITIZEN; i++){ if(map->citizens[i]->data.alive){ number_alive++;} }
    for(i = 0; i < NB_DOCTOR; i++){ if(map->doctors[i]->data.alive){ number_alive++;} }
    for(i = 0; i < NB_FIREFIGHTER; i++){ if(map->firefighters[i]->data.alive){ number_alive++;} }
    for(i = 0; i < NB_JOURNALIST; i++){ if(map->journalists[i]->data.alive){ number_alive++;} }

    printf("Number of alive citizens : %d\n", number_alive);

    // To display the legend
    printf("\n\n\033[4mLegend of the citizen map:\033[0m\n");
    printf("\t\033[7;33m   \033[0m : Wasteland\n");
    printf("\t\033[7;32m   \033[0m : House\n");
    printf("\t\033[7;34m   \033[0m : Hospital\n");
    printf("\t\033[7;31m   \033[0m : Barrack\n");


    printf("\n\n\033[4mLegend of the infection map:\033[0m\n");
    printf("\t\033[0;30;48;5;75m   \033[0m : No infection\n");
    printf("\t\033[0;30;48;5;33m   \033[0m : Very light infection\n");
    printf("\t\033[0;30;48;5;71m   \033[0m : Light infection\n");
    printf("\t\033[0;30;48;5;77m   \033[0m : Medium infection\n");
    printf("\t\033[0;30;48;5;221m   \033[0m : Intermediate infection\n");
    printf("\t\033[0;30;48;5;208m   \033[0m : High infection\n");
    printf("\t\033[0;30;48;5;160m   \033[0m : Very high infection\n");
    printf("\t\033[0;30;48;5;52m   \033[0m : Critical infection\n");
}*/