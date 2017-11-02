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
    
    if (one_shot_scheduling((char*) argv[1], NULL) != -1) {
        write_schedule_xml((char*) argv[2]);
    }
    return 0;
}
