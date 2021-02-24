#Small-project : epidemic simulation
###Author: Paul BALA and Antoine VILLEMAGNE

####Description:
This project is an epidemic simulation, split into 4 programs.
The first and main one is epidemic_sim, which creates a shared memory and the associated map and has the role of displaying various graphs representing the evolution of the epidemic during time.
Then citizen_manager implements the map with citizens, journalists, doctors and firefighters who all have different behaviors and are managed through threads. The program timer simulates the duration of a day, by sending a message to epidemic_sim at regular second intervals.
Finally the program press_agency displays various information given by the two journalists through message queues.
All those programs communicates through named pipes or signals.

####Installation:
To visualise the evolution of the contamination we use two technologies, GNUPLOT and Ncurses. If you have not installed them already you have to type:

To install Ncurses : sudo apt-get install libncurses5-dev libncursesw5-dev
To install GNUPLOT : sudo apt-get install gnuplot-x11

You also have to create two folders, bin and obj, for that type : '$mkdir bin' and '$mkdir obj'

Then to create all the executable programs, you only have to type '$make'

####Running:
To run the programs you have to type:

- For epidemic_sim : '$./bin/epidemic_sim <number_of_turns>', with number_of_turns higher than 0
- For citizen_manager : '$./bin/citizen_manager '
- For timer : '$./bin/timer <period>', with period between 1 and 5 (in seconds)
- For press_agency : '$./bin/press_agency' (Warning: it might be possible that this program does not stop at the end of the simulation. If it happens just press Ctrl+C to stop it)