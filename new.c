#include "simlib.h"
#include "simlibdefs.h"
#include <stdio.h>
#include <math.h>


// Possible Events
#define PERSON_ARRIVED_1 1
#define PERSON_ARRIVED_2 2
#define PERSON_ARRIVED_3 3
#define BUS_ARRIVED_1    4
#define BUS_ARRIVED_2    5
#define BUS_ARRIVED_3    6
#define BUS_DEPARTED_1           7  
#define BUS_DEPARTED_2           8  
#define BUS_DEPARTED_3           9 
#define UNLOAD_1         10 
#define UNLOAD_2         11 
#define UNLOAD_3         12 
#define LOAD_1           13 
#define LOAD_2           14 
#define LOAD_3           15
#define END_SIMULATION   16

// Streams Constants
#define STREAM_INTERARRIVAL_1 1
#define STREAM_INTERARRIVAL_2 2
#define STREAM_INTERARRIVAL_3 3
#define STREAM_UNLOAD         4
#define STREAM_LOAD           5
#define STREAM_DESTINATION    6

// Event logic gate
#define IS_PERSON_ARRIVED_EVENT(x) (x>=PERSON_ARRIVED_1 && x<=PERSON_ARRIVED_3)
#define IS_BUS_ARRIVED_EVENT(x) (x>=BUS_ARRIVED_1 && x<=BUS_ARRIVED_3)
#define IS_BUS_DEPARTED_EVENT(x) (x>=BUS_DEPARTED_1 && x<=BUS_DEPARTED_3)
#define IS_UNLOAD_EVENT(x) (x>=UNLOAD_1 && x<=UNLOAD_3)
#define IS_LOAD_EVENT(x) (x>=LOAD_1 && x<=LOAD_3)
#define BUS_WAIT_TIME 5

// SAMPST Storage
#define PERSON_QUEUE_1 1
#define PERSON_QUEUE_2 2
#define PERSON_QUEUE_3 3
#define PASSENGER_BUS_1 4
#define PASSENGER_BUS_2 5
#define PASSENGER_BUS_3 6

// Transfer IDX
#define TRANSFER_TIME_IDX 1
#define TRANSFER_LOC_IDX 2

#define VARIABLE_NQ_1 1
#define VARIABLE_NQ_2 2
#define VARIABLE_NQ_3 3
#define VARIABLE_DELAY_1 4
#define VARIABLE_DELAY_2 5
#define VARIABLE_DELAY_3 6
#define VARIABLE_N_PASSENGER_BUS 7
#define VARIABLE_BUS_STOP_1 8
#define VARIABLE_BUS_STOP_2 9
#define VARIABLE_BUS_STOP_3 10
#define VARIABLE_BUS_LOOP 11
#define VARIABLE_PERSON_SYSTEM_1 12
#define VARIABLE_PERSON_SYSTEM_2 13
#define VARIABLE_PERSON_SYSTEM_3 14

#define MAX_BUS_SIZE 20

int total_time;
double bus_last_arrival = 0.0, bus_last_departure_3 = 0.0;
FILE *input, *output;

double mean_interval_arrival[4], 
unloading_min, unloading_max, 
loading_min, loading_max, 
destination_probability[3], arrival_time[4];

void init_model(void);
void person_is_arrived(int loc);
void bus_is_arrived(int loc);
void bus_is_departed(int loc);
void unload(int loc);
void load(int loc);
void summary(void);

int list_bus_size() {return list_size[PASSENGER_BUS_1] + list_size[PASSENGER_BUS_2] + list_size[PASSENGER_BUS_3]; }

int main() {
    input = fopen("file.in", "r");
    output = fopen("file.out", "w");

    fscanf(input, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %d", 
            &mean_interval_arrival[1], &mean_interval_arrival[2], &mean_interval_arrival[3], 
            &unloading_min, &unloading_max,
            &loading_min, &loading_max, 
            &destination_probability[1], &destination_probability[2], 
            &total_time
        );

    /* Write report heading and input parameters. */

    fprintf(output, "Car-rental system using simlib\n\n");
    fprintf(output, "Mean interarrival time at location 1%11.3f minutes\n\n", mean_interval_arrival[0]);
    fprintf(output, "Mean interarrival time at location 2%11.3f minutes\n\n", mean_interval_arrival[1]);
    fprintf(output, "Mean interarrival time at location 3%11.3f minutes\n\n", mean_interval_arrival[2]);
    fprintf(output, "Unloading time is distributed uniformly between %5.3f minutes and %5.3f minutes\n\n", unloading_min, unloading_max);
    fprintf(output, "Loading time is distributed uniformly between %5.3f minutes and %5.3f minutes\n\n", loading_min, loading_max);
    fprintf(output, "Person want to go to terminal 1 with probability %5.3f\n\n", destination_probability[0]);
    fprintf(output, "Person want to go to terminal 2 with probability %5.3f\n\n", destination_probability[1] - destination_probability[0]);
    fprintf(output, "\nTime simulation%28d     minutes\n\n", total_time);

    init_simlib();
    maxatr = 4;
    init_model();

    while (sim_time < total_time) {
        timing();
        if (IS_LOAD_EVENT(next_event_type)) {
            load(next_event_type - LOAD_1 + 1);
        } else if (IS_UNLOAD_EVENT(next_event_type)) {
            unload(next_event_type - UNLOAD_1 + 1);
        } else if (IS_BUS_DEPARTED_EVENT(next_event_type)) {
            bus_is_departed(next_event_type - BUS_DEPARTED_1 + 1);
        } else if (IS_BUS_ARRIVED_EVENT(next_event_type)) {
            bus_is_arrived(next_event_type - BUS_ARRIVED_1 + 1);
        } else if (IS_PERSON_ARRIVED_EVENT(next_event_type)) {
            person_is_arrived(next_event_type - PERSON_ARRIVED_1 + 1);
        }
    }

    summary();
    return 0;
}

void init_model(void) {

    // Schedule People Arrivals in every location
    for (int i = 0; i < 3; i++) {
        event_schedule(sim_time + expon(60.0 * 60.0 / 14.0, STREAM_INTERARRIVAL_1 + i), PERSON_ARRIVED_1 + i);
    }

    // As per problem statement, the bus is initially at location 3
    event_schedule(sim_time, BUS_ARRIVED_3); 

    // End of simulation
    event_schedule(sim_time + total_time, END_SIMULATION);

    sampst(0.0, 0);
    timest(0.0, 0);
}

void person_is_arrived(int loc) {
    transfer[TRANSFER_TIME_IDX] = sim_time;

    switch(loc) {
        case 1:
            transfer[TRANSFER_LOC_IDX] = 3;

            list_file(LAST, PERSON_QUEUE_1);
            timest(list_size[PERSON_QUEUE_1], VARIABLE_NQ_1);
            event_schedule(sim_time + expon(60.0 * 60.0 / 14.0, STREAM_INTERARRIVAL_1), PERSON_ARRIVED_1);
            break;
        case 2:
            transfer[TRANSFER_LOC_IDX] = 3;

            list_file(LAST, PERSON_QUEUE_2);
            timest(list_size[PERSON_QUEUE_2], VARIABLE_NQ_2);
            event_schedule(sim_time + expon(60.0 * 60.0 / 10.0, STREAM_INTERARRIVAL_2), PERSON_ARRIVED_2);
            break;
        case 3:
            if (uniform(0, 1, STREAM_DESTINATION) < destination_probability[1])
                transfer[TRANSFER_LOC_IDX] = 1;
            else
                transfer[TRANSFER_LOC_IDX] = 2;

            list_file(LAST, PERSON_QUEUE_3);
            timest(list_size[PERSON_QUEUE_3], VARIABLE_NQ_3);
            event_schedule(sim_time + expon(60.0 * 60.0 / 24.0, STREAM_INTERARRIVAL_3), PERSON_ARRIVED_3);
            break;
        default:
            // Handle invalid loc
            break;
    }
}

void bus_is_arrived(int loc) {
    bus_last_arrival = sim_time;
    
    switch(loc) {
        case 1:
            if (list_size[PASSENGER_BUS_1] > 0)
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_1);
            else if (list_size[PERSON_QUEUE_1] > 0 && list_bus_size() < MAX_BUS_SIZE)
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_1);
            else
                event_schedule(sim_time + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_1);
            break;
        case 2:
            if (list_size[PASSENGER_BUS_2] > 0)
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_2);
            else if (list_size[PERSON_QUEUE_2] > 0 && list_bus_size() < MAX_BUS_SIZE)
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_2);
            else
                event_schedule(sim_time + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_2);
            break;
        case 3:
            if (list_size[PASSENGER_BUS_3] > 0)
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_3);
            else if (list_size[PERSON_QUEUE_3] > 0 && list_bus_size() < MAX_BUS_SIZE)
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_3);
            else
                event_schedule(sim_time + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_3);
            break;
        default:
            // Handle invalid loc
            break;
    }
}

void bus_is_departed(int loc) {
    double departure_time = sim_time - bus_last_arrival;
    
    switch(loc) {
        case 1:
            if (departure_time < (BUS_WAIT_TIME * 60.0)) {
                event_schedule(bus_last_arrival + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_1);
            } else {
                sampst(departure_time, VARIABLE_BUS_STOP_1);
                event_schedule(sim_time + 1.0 / 30.0 * 60.0 * 60.0, BUS_ARRIVED_2);
            }
            break;
        case 2:
            if (departure_time < (BUS_WAIT_TIME * 60.0)) {
                event_schedule(bus_last_arrival + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_2);
            } else {
                sampst(departure_time, VARIABLE_BUS_STOP_2);
                event_schedule(sim_time + 4.5 / 30.0 * 60.0 * 60.0, BUS_ARRIVED_3);
            }
            break;
        case 3:
            if (departure_time < (BUS_WAIT_TIME * 60.0)) {
                event_schedule(bus_last_arrival + BUS_WAIT_TIME * 60.0, BUS_DEPARTED_3);
            } else {
                sampst((sim_time - bus_last_departure_3), VARIABLE_BUS_LOOP);
                bus_last_departure_3 = sim_time;
                sampst(departure_time, VARIABLE_BUS_STOP_3);
                event_schedule(sim_time + 4.5 / 30.0 * 60.0 * 60.0, BUS_ARRIVED_1);
            }
            break;
        default:
            // Handle invalid loc
            break;
    }
}


void unload(int loc) {
    switch(loc) {
        case 1:
            list_remove(FIRST, PASSENGER_BUS_1);
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            sampst(sim_time - transfer[TRANSFER_TIME_IDX], VARIABLE_PERSON_SYSTEM_1);

            if (list_size[PASSENGER_BUS_1] > 0) {
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_1);
            } else if (list_size[PERSON_QUEUE_1] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_1);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_1);
            }
            break;
        case 2:
            list_remove(FIRST, PASSENGER_BUS_2);
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            sampst(sim_time - transfer[TRANSFER_TIME_IDX], VARIABLE_PERSON_SYSTEM_2);

            if (list_size[PASSENGER_BUS_2] > 0) {
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_2);
            } else if (list_size[PERSON_QUEUE_2] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_2);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_2);
            }
            break;
        case 3:
            list_remove(FIRST, PASSENGER_BUS_3);
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            sampst(sim_time - transfer[TRANSFER_TIME_IDX], VARIABLE_PERSON_SYSTEM_3);

            if (list_size[PASSENGER_BUS_3] > 0) {
                event_schedule(sim_time + uniform(16, 24, STREAM_UNLOAD), UNLOAD_3);
            } else if (list_size[PERSON_QUEUE_3] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_3);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_3);
            }
            break;
        default:
            // Handle invalid loc
            break;
    }
}

void load(int loc) {
    switch(loc) {
        case 1:
            list_remove(FIRST, PERSON_QUEUE_1);
            timest(list_size[PERSON_QUEUE_1], VARIABLE_NQ_1);
            sampst((sim_time - transfer[TRANSFER_TIME_IDX]), VARIABLE_DELAY_1);
            list_file(LAST, PASSENGER_BUS_3);
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            if (list_size[PERSON_QUEUE_1] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_1);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_1);
            }
            break;
        case 2:
            list_remove(FIRST, PERSON_QUEUE_2);
            timest(list_size[PERSON_QUEUE_2], VARIABLE_NQ_2);
            sampst((sim_time - transfer[TRANSFER_TIME_IDX]), VARIABLE_DELAY_2);
            list_file(LAST, PASSENGER_BUS_3);
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            if (list_size[PERSON_QUEUE_2] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_2);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_2);
            }
            break;
        case 3:
            list_remove(FIRST, PERSON_QUEUE_3);
            timest(list_size[PERSON_QUEUE_3], VARIABLE_NQ_3);
            sampst((sim_time - transfer[TRANSFER_TIME_IDX]), VARIABLE_DELAY_3);
            if(transfer[TRANSFER_LOC_IDX] == 1){
                list_file(LAST, PASSENGER_BUS_1);
            } else {
                list_file(LAST, PASSENGER_BUS_2);
            }
            timest(list_bus_size(), VARIABLE_N_PASSENGER_BUS);
            if (list_size[PERSON_QUEUE_3] > 0 && list_bus_size() < MAX_BUS_SIZE) {
                event_schedule(sim_time + uniform(15, 25, STREAM_LOAD), LOAD_3);
            } else {
                event_schedule(sim_time, BUS_DEPARTED_3);
            }
            break;
        default:
            // Handle invalid loc
            break;
    }
}

void summary(void) {
  timest(0,-VARIABLE_NQ_1);
  fprintf(output, "(a) Average and maximum number in each queue \n     1. Terminal 1:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[2]);
  timest(0,-VARIABLE_NQ_2);
  fprintf(output, "     2. Terminal 2:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[2]);
  timest(0,-VARIABLE_NQ_3);
  fprintf(output, "     3. Car Rental:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[2]);

  sampst(0,-VARIABLE_DELAY_1);
  fprintf(output, "(b) Average and maximum delay in each queue \n     1. Terminal 1:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[3]);
  sampst(0,-VARIABLE_DELAY_2);
  fprintf(output, "     2. Terminal 2:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[3]);
  sampst(0,-VARIABLE_DELAY_3);
  fprintf(output, "     3. Car Rental:\n       -Average: %.3lf\n       -Maximum: %.3lf\n", transfer[1], transfer[3]);
  
  fprintf(output, "(c) Average and maximum number on the bus \n");
  timest(0,-VARIABLE_N_PASSENGER_BUS);
  fprintf(output, "     -Average: %.3lf\n     -Maximum: %.3lf\n", transfer[1], transfer[2]);
  
  sampst(0,-VARIABLE_BUS_STOP_1);
  fprintf(output, "(d) Average, maximum, and minimum time bus stopped at each location \n     1. Terminal 1:\n       -Average: %.3lf\n       -Maximum: %.3lf\n       -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
  sampst(0,-VARIABLE_BUS_STOP_2);
  fprintf(output, "     2. Terminal 2:\n       -Average: %.3lf\n       -Maximum: %.3lf\n       -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
  sampst(0,-VARIABLE_BUS_STOP_3);
  fprintf(output, "     3. Car Rental:\n       -Average: %.3lf\n       -Maximum: %.3lf\n       -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);

  sampst(0,-VARIABLE_BUS_LOOP);
  fprintf(output, "(e) Average, maximum, and minimum time bus making a loop \n     -Average: %.3lf\n     -Maximum: %.3lf\n     -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
  
  sampst(0,-VARIABLE_PERSON_SYSTEM_1);
  fprintf(output, "(f) Average, maximum, and minimum time person in the system \n     1. Terminal 1:\n      -Average: %.3lf\n     -Maximum: %.3lf\n     -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
  sampst(0,-VARIABLE_PERSON_SYSTEM_2);
  fprintf(output, "     2. Terminal 2:\n       -Average: %.3lf\n       -Maximum: %.3lf\n       -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
  sampst(0,-VARIABLE_PERSON_SYSTEM_3);
  fprintf(output, "     3. Car Rental:\n       -Average: %.3lf\n       -Maximum: %.3lf\n       -Minimum: %.3lf\n", transfer[1], transfer[3], transfer[4]);
}
