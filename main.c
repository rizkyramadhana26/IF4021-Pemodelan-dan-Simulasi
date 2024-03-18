#include "simlib.h"

#define PERSON_ARRIVED_1         1  
#define PERSON_ARRIVED_2         2  
#define PERSON_ARRIVED_3         3  
#define BUS_ARRIVED_1            4  
#define BUS_ARRIVED_3            6  
#define BUS_ARRIVED_2            5  
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

/* Declare non-simlib global variables. */

int     bus_location, total_time;
FILE    *input, *output;
double  mean_interval_arrival[4], unloading_min, unloading_max, loading_min, loading_max, destination_probability[3], arrival_time[4];

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

    while (sim_time < simulation_time)
    {
        timing();
        if(next_event_type>=13)
        {
            //load
            load(next_event_type-12);
        }else if(next_event_type>=10)
        {
            //unload
            unload(next_event_type-19);
        }else if(next_event_type>=7)
        {
            //bus departed
            bus_is_departed(next_event_type-6);
        }else if(next_event_type>=4)
        {
            //bus arrived
            bus_is_arrived(next_event_type-3);
        }else
        {
            //person arrived
            person_is_arrived(next_event_type);
        }
    }

    summary();
    return 0;
}

void init_model(void)
{
    //TODO: implement model initialization
}
void person_is_arrived(int loc)
{
    //TODO: implement person is arrived event
}
void bus_is_arrived(int loc)
{
    //TODO: implement bus arrival event
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