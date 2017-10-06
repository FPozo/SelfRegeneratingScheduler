/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Main.c                                                                                                             *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include "Synthesizer.h"
#include "IOInterface.h"
#include <sys/time.h>

struct timeval start_time_write, end_time_write;              // Solver time

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    one_shot_scheduling("/Users/fpo01/OneDrive - Mälardalens högskola/Software/Self-Regenerating Scheduler/Networks/auto.xml",
                        NULL);
    gettimeofday(&start_time_write, NULL);
    write_schedule_xml("/Users/fpo01/OneDrive - Mälardalens högskola/Software/Self-Regenerating Scheduler/Networks/schedule.xml");
    gettimeofday(&end_time_write, NULL);
    printf("Time to write xml in ms => %f\n", time_diff(start_time_write, end_time_write));
    
    return 0;
}
