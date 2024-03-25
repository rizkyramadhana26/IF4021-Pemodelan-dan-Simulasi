#include "simlib.h"
#include "simlibdefs.h"
#include <stdio.h>
#include <math.h>

#define TIME_IDX                 1
#define LOC_IDX                  2
#define PROB_IDX                 3

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
#define QUEUE_1                  1   //
#define QUEUE_2                  2   //
#define QUEUE_3                  3   //
#define SERVER                   4  
#define SAMPST_DELAYS_1          1  //untuk menghitung lama waktu antrean
#define SAMPST_DELAYS_2          2  
#define SAMPST_DELAYS_3          3  
#define SAMPST_SERVER_1          4  //untuk menghitung durasi bus berhenti
#define SAMPST_SERVER_2          5  
#define SAMPST_SERVER_3          6  
#define SAMPST_LOOP_1            7  //untuk menghitung durasi loop
#define SAMPST_LOOP_2            8  
#define SAMPST_LOOP_3            9

#define SAMPST_PERSON_1          10 //untuk menghitung total durasi di sistem, dikelompokkan berdasar lokasi tujuan
#define SAMPST_PERSON_2          11
#define SAMPST_PERSON_3          12

#define SAMPST_QUEUE_1           13  // Panjang queue 1
#define SAMPST_QUEUE_2           14
#define SAMPST_QUEUE_3           15  
#define SAMPST_SERVER            16  // Panjang server

#define STREAM_INTER_ARRIVAL_1       1  
#define STREAM_INTER_ARRIVAL_2       2  
#define STREAM_INTER_ARRIVAL_3       3  
#define STREAM_UNLOADING       4  
#define STREAM_LOADING         5  
#define STREAM_DESTINATION     6

#define IS_PERSON_ARRIVED_EVENT(x) (x>=PERSON_ARRIVED_1 && x<=PERSON_ARRIVED_3)
#define IS_BUS_ARRIVED_EVENT(x) (x>=BUS_ARRIVED_1 && x<=BUS_ARRIVED_3)
#define IS_BUS_DEPARTED_EVENT(x) (x>=BUS_DEPARTED_1 && x<=BUS_DEPARTED_3)
#define IS_UNLOAD_EVENT(x) (x>=UNLOAD_1 && x<=UNLOAD_3)
#define IS_LOAD_EVENT(x) (x>=LOAD_1 && x<=LOAD_3)
#define BUS_WAIT_TIME 5

int     bus_location, total_time;
FILE* input, * output;

// CHANGED FROM SIZE 4 -> 3 FOR mean_interval_arrival & arrival_time
// DESTINATION_PROBABILITY[3] IS STILL ORIGINAL FROM REFERENCE
double  mean_interval_arrival[3], unloading_min, unloading_max, loading_min, loading_max, destination_probability[2], arrival_time[3];

void init_model(void);
void person_is_arrived(int loc);
void bus_is_arrived(int loc);
void bus_is_departed(int loc);
void unload(int loc);
void load(int loc);
void summary(void);

int main() {

    input = fopen("file.in", "r");
    output = fopen("file.out", "w");

    fscanf(input, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %d", &mean_interval_arrival[0], &mean_interval_arrival[1], &mean_interval_arrival[2], &unloading_min, &unloading_max, &loading_min, &loading_max, &destination_probability[0], &destination_probability[1], &total_time);

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
    list_rank[SERVER] = 1;

    // for each location schedule the first people arrivals 
    // with random time_difference utilizing mean_interval_arrival
    for (int i = 0; i < 3; i++) {
        event_schedule(sim_time + expon(mean_interval_arrival[i], STREAM_INTER_ARRIVAL_1+ i), PERSON_ARRIVED_1 + i);
    }

    bus_location = 0;

    // As per problem statement, the bus is initially at location 3
    event_schedule(sim_time, BUS_ARRIVED_3);
}
void person_is_arrived(int loc) {
    // schedule the next person arrival with random time_difference utilizing mean_interval_arrival
    event_schedule(sim_time + expon(mean_interval_arrival[loc - 1], STREAM_INTER_ARRIVAL_1+ loc - 1), PERSON_ARRIVED_1 + loc - 1);

    transfer[TIME_IDX] = sim_time;
    if (loc == 3)
    {
        double rand = uniform(0, 1, STREAM_DESTINATION);
        if (rand < destination_probability[0]){
            transfer[LOC_IDX] = 1;
        } else{
            transfer[LOC_IDX] = 2;
        }
    } else{
        transfer[LOC_IDX] = 3;
    }
    list_file(LAST, QUEUE_1 + loc - 1);
    sampst(list_size[QUEUE_1 + loc - 1], SAMPST_QUEUE_1 + loc - 1);
}
void bus_is_arrived(int loc) {
    bus_location = loc;
    // ARRIVAL TIME BIS SEBELUMNYA, INI UNTUK MENJAWAB SOAL E
    sampst(sim_time - arrival_time[loc - 1], SAMPST_LOOP_1 + loc - 1); // UNTUK MENGHITUNG DURASI LOOP< TAPI NANTI PAKE YG 3 AJA

    // NIH ARRIVAL TIME NYA DITIMPANYA DISINI
    arrival_time[loc - 1] = sim_time;

    // schedule unloading event
    event_schedule(sim_time, UNLOAD_1 + loc - 1);
}
void bus_is_departed(int loc) {
    /* Unset bus location and register server delay. */
    bus_location = 0;
    // ARRIVAL TIME NYA BUS YANG INI
    sampst(sim_time - arrival_time[loc - 1], SAMPST_SERVER_1 + loc - 1); //ini buat jawab yang D, menghitung durasi bus berhenti

    /* Schedule bus arrival at next location. */
    if (loc == 1) {
        event_schedule(sim_time + 2, BUS_ARRIVED_2);
    } else if (loc == 2) {
        event_schedule(sim_time + 9, BUS_ARRIVED_3);
    } else {
        event_schedule(sim_time + 9, BUS_ARRIVED_1);
    }
}
void unload(int loc) {
    /* Check to see whether bus is not empty. */
    if (list_size[SERVER] != 0)
    {
        double total_duration = 0;
        for(int i=0;i<list_size[SERVER];i++){
            list_remove(FIRST, SERVER);
            if(transfer[LOC_IDX]==loc){
                //kalo tujuan di sini, gaperlu list_file
                double duration = uniform(unloading_min, unloading_max, STREAM_UNLOADING);
                total_duration += duration;
                sampst(sim_time + total_duration - transfer[TIME_IDX], SAMPST_PERSON_1+loc-1); //untuk menjawab soal f
                sampst(list_size[SERVER], SAMPST_SERVER);
            }else{
                list_file(LAST, SERVER);
            }
        }
        event_schedule(sim_time+total_duration, LOAD_1 + loc-1);       
    }
    else
    {
        /* Bus is empty, so schedule loading. */
        event_schedule(sim_time, LOAD_1 + loc - 1);
    }
}
void load(int loc) {
    double total_duration = 0.0;
    int i = 0;
    
    while((list_size[SERVER]<20) && (i<list_size[QUEUE_1 + loc -1])){
        double duration = uniform(loading_min, loading_max, STREAM_LOADING);
        total_duration += duration;
        
        // update lama waktu orang di antrian
        sampst(sim_time + total_duration - transfer[TIME_IDX], SAMPST_DELAYS_1+loc-1);

        list_remove(FIRST, QUEUE_1 + loc -1);
        list_file(LAST, SERVER);
        i++;

        // Update panjang antrian
        sampst(list_size[QUEUE_1 + loc - 1], SAMPST_QUEUE_1 + loc - 1);
        sampst(list_size[SERVER], SAMPST_SERVER);
    }

    double duration_to_departure = 0 >= 5-(sim_time + total_duration - arrival_time[loc-1]) ? 0 : 5-(sim_time + total_duration - arrival_time[loc-1]);
    event_schedule(sim_time+total_duration+duration_to_departure, BUS_DEPARTED_1 + loc -1);
}
void summary(void) {
    /* Get and write out estimates of desired measures of performance. */
    fprintf(output, "\n(a) Average and maximum number in each queue\n");
    fprintf(output, "\n    Queue at location 1 (1), queue at location 2 (2) and queue at location 3 (3):\n");
    out_sampst(output, SAMPST_QUEUE_1, SAMPST_QUEUE_3);
    fprintf(output, "\n(b) Average and maximum delay in each queue\n");
    fprintf(output, "\n    Queue at location 1 (1), queue at location 2 (2) and queue at location 3 (3):\n");
    out_sampst(output, SAMPST_DELAYS_1, SAMPST_DELAYS_3);
    fprintf(output, "\n(c) Average and maximum number on the bus\n");
    fprintf(output, "\n    Bus (4):\n");
    out_sampst(output, SAMPST_SERVER, SAMPST_SERVER);
    fprintf(output, "\n(d) Average, maximum, and minimum time the bus stopped at each location\n");
    fprintf(output, "\n    Stop at location 1 (4), stop at location 2 (5) and stop at location 3 (6):\n");
    out_sampst(output, SAMPST_SERVER_1, SAMPST_SERVER_3);
    fprintf(output, "\n(e) Average, maximum, and minimum time for the bus to make a loop for each location\n");                                                            fprintf(output, "\n Loop for location 1 (7), loop for location 2 (8) and loop for location 3 (9):\n");
    out_sampst(output, SAMPST_LOOP_3, SAMPST_LOOP_3);
    fprintf(output, "\n(f) Average, maximum, and minimum time a person is in the system by arrival location\n");
    fprintf(output, "\n    Person in location 1 (10), Person in location 2 (11), Person in location 3 (12):\n");
    out_sampst(output, SAMPST_PERSON_1, SAMPST_PERSON_3);
}