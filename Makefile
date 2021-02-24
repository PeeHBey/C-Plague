BIN	= bin/
SRC	= src/
OBJ	= obj/
INC = include/
POP = population/
CC	= gcc
RM	= rm
CFLAGS = -Wall -Wextra -pedantic
LDFLAGS = -lrt -lpthread -lncurses -lm -g

all : $(BIN)epidemic_sim $(BIN)timer $(BIN)citizen_manager $(BIN)press_agency

$(BIN)% :
	$(CC) $^ -o $@ $(LDFLAGS)

$(BIN)epidemic_sim : $(OBJ)epidemic_sim.o $(OBJ)map.o $(OBJ)population.o $(OBJ)citizen.o $(OBJ)doctor.o $(OBJ)firefighter.o $(OBJ)journalist.o
$(BIN)timer : $(OBJ)timer.o
$(BIN)citizen_manager : $(OBJ)citizen_manager.o $(OBJ)population.o $(OBJ)citizen.o $(OBJ)doctor.o $(OBJ)firefighter.o $(OBJ)journalist.o $(OBJ)map.o
$(BIN)press_agency : $(OBJ)press_agency.o

$(OBJ)%.o :
	$(CC) -c $< -I$(INC) -o $@ $(CFLAGS)

$(OBJ)epidemic_sim.o : $(SRC)epidemic_sim.c
$(OBJ)map.o : $(SRC)map.c
$(OBJ)timer.o : $(SRC)timer.c
$(OBJ)citizen_manager.o : $(SRC)citizen_manager.c
$(OBJ)press_agency.o : $(SRC)press_agency.c

$(OBJ)population.o : $(SRC)$(POP)population.c
$(OBJ)citizen.o : $(SRC)$(POP)citizen.c
$(OBJ)doctor.o : $(SRC)$(POP)doctor.c
$(OBJ)firefighter.o : $(SRC)$(POP)firefighter.c
$(OBJ)journalist.o : $(SRC)$(POP)journalist.c


clean :
	$(RM) $(OBJ)*.o

distclean : clean
	$(RM) $(BIN)*
