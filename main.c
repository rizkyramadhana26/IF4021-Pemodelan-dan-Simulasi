#include "simlib.h"
#include "simlibdefs.h"
#include <stdio.h>
#include <math.h>


#define PERSON_ARRIVED_1         1  
#define PERSON_ARRIVED_2         2  
#define PERSON_ARRIVED_3         3  
#define BUS_ARRIVED_1            4  
#define BUS_ARRIVED_2            5  
#define BUS_ARRIVED_3            6  
#define BUS_DEPARTED_1           7  
#define BUS_DEPARTED_2           8  
#define BUS_DEPARTED_3           9  
#define UNLOAD_1                10 
#define UNLOAD_2                11 
#define UNLOAD_3                12 
#define LOAD_1                  13 
#define LOAD_2                  14 
#define LOAD_3                  15 
#define QUEUE_1                  1  
#define QUEUE_2                  2  
#define QUEUE_3                  3  
#define SERVER                   4  
#define SAMPST_DELAYS_1          1  
#define SAMPST_DELAYS_2          2  
#define SAMPST_DELAYS_3          3  
#define SAMPST_SERVER_1          4  
#define SAMPST_SERVER_2          5  
#define SAMPST_SERVER_3          6  
#define SAMPST_LOOP_1            7  
#define SAMPST_LOOP_2            8  
#define SAMPST_LOOP_3            9  
#define SAMPST_PERSON           10 
#define INTERVAL_ARRIVAL_1       1  
#define INTERVAL_ARRIVAL_2       2  
#define INTERVAL_ARRIVAL_3       3  
#define INTERVAL_UNLOADING       4  
#define INTERVAL_LOADING         5  
#define INTERVAL_DESTINATION     6

#define IS_PERSON_ARRIVED_EVENT(x) (x>=PERSON_ARRIVED_1 && x<=PERSON_ARRIVED_3)
#define IS_BUS_ARRIVED_EVENT(x) (x>=BUS_ARRIVED_1 && x<=BUS_ARRIVED_3)
#define IS_BUS_DEPARTED_EVENT(x) (x>=BUS_DEPARTED_1 && x<=BUS_DEPARTED_3)
#define IS_UNLOAD_EVENT(x) (x>=UNLOAD_1 && x<=UNLOAD_3)
#define IS_LOAD_EVENT(x) (x>=LOAD_1 && x<=LOAD_3)
#define BUS_WAIT_TIME 5

int     bus_location, total_time;
FILE    *input, *output;

// CHANGED FROM SIZE 4 -> 3 FOR mean_interval_arrival & arrival_time
// DESTINATION_PROBABILITY[3] IS STILL ORIGINAL FROM REFERENCE
double  mean_interval_arrival[3], unloading_min, unloading_max, loading_min, loading_max, destination_probability[3], arrival_time[3];

void init_model(void);
void person_is_arrived(int loc);
void bus_is_arrived(int loc);
void bus_is_departed(int loc);
void unload(int loc);
void load(int loc);
void summary(void);

int main() 
{

    input  = fopen("file.in",  "r");
    output = fopen("file.out", "w");

    //TODO: read input & determine file format

    init_simlib();
    maxatr = 4;
    init_model();

    while (sim_time < total_time)
    {
        timing();
        if(IS_LOAD_EVENT(next_event_type))
        {
            load(next_event_type - LOAD_1 + 1);
        }else if(IS_UNLOAD_EVENT(next_event_type))
        {
            unload(next_event_type - UNLOAD_1 + 1);
        }else if(IS_BUS_DEPARTED_EVENT(next_event_type))
        {
            bus_is_departed(next_event_type - BUS_DEPARTED_1 + 1);
        }else if(IS_BUS_ARRIVED_EVENT(next_event_type))
        {
            bus_is_arrived(next_event_type - BUS_ARRIVED_1 + 1);
        }else if(IS_PERSON_ARRIVED_EVENT(next_event_type))
        {
            person_is_arrived(next_event_type - PERSON_ARRIVED_1 + 1);
        }
    }

    summary();
    return 0;
}

void init_model(void)
{
    list_rank[SERVER] = 1;

    // for each location schedule the first people arrivals 
    // with random time_difference utilizing mean_interval_arrival
    for (int i = 0; i < 3; i++){
        event_schedule(sim_time + expon(mean_interval_arrival[i], INTERVAL_ARRIVAL_1 + i), PERSON_ARRIVED_1 + i);
    }

    bus_location = 0;

    // As per problem statement, the bus is initially at location 3
    event_schedule(sim_time, BUS_ARRIVED_3);
}
void person_is_arrived(int loc)
{
    // schedule the next person arrival with random time_difference utilizing mean_interval_arrival
    event_schedule(sim_time + expon(mean_interval_arrival[loc - 1], INTERVAL_ARRIVAL_1 + loc - 1), PERSON_ARRIVED_1 + loc - 1);

    transfer[1] = sim_time;
    list_file(LAST, QUEUE_1 + loc - 1);
}
void bus_is_arrived(int loc)
{
    bus_location = loc;
    sampst(sim_time - arrival_time[loc - 1], SAMPST_LOOP_1 + loc - 1);
    arrival_time[loc - 1] = sim_time;

    // schedule the next bus arrival 
    event_schedule(sim_time + BUS_WAIT_TIME, BUS_DEPARTED_1 + loc - 1);

    // schedule unloading event
    event_schedule(sim_time, UNLOAD_1 + loc - 1);
}
void bus_is_departed(int loc)
{
    //TODO: implement bus departure event
}
void unload(int loc)
{
    //TODO: implement unloading passenger event
}
void load(int loc)
{
    //TODO: implement loading passenger event
}
void summary(void)
{
    //TODO: implement generating summary
}