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


#ifndef PLAGUE_POPULATION_H
#define PLAGUE_POPULATION_H

#include "map.h"
#include "data.h"

#include "population/citizen.h"
#include "population/doctor.h"
#include "population/firefighter.h"
#include "population/journalist.h"

void init_population(map_t * map);

#endif //PLAGUE_POPULATION_H
