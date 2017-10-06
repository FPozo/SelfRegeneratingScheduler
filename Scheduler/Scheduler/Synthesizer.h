/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Synthesizer.h                                                                                                      *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information and algorithgms to synthesize the Schedule.                                  *
 *  Different algorithms to build the schedule and store it in a sortered array of all frames in the network. Such     *
 *  array will the ordered by the order in which the frames will be scheduled (in iterative and segmented approaches)  *
 *  It also contains auxiliar structures to save characteristics of the frames and the network in order to perfom the  *
 *  scheduling.                                                                                                        *
 *  The main approaches used are one-shot, incremental and segmented. Every technique has a different                  *
 *  performance. The better performance, the worse "quality" the final schedule will be.                               *
 *  Also numerous optimizations can be applied to different techniques to try to modify the "quality" of the schedules *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Synthesizer_h
#define Synthesizer_h

#include <stdio.h>

#endif /* Synthesizer_h */

                                                /* STRUCT DEFINITIONS */

                                                /* AUXILIAR FUNCTIONS */

/**
 Calculates the time difference in ms
 
 @param x starting time
 @param y ending time
 @return time difference in ms
 */
double time_diff(struct timeval x , struct timeval y);

                                                /* CODE DEFINITIONS */

/**
 Produces the schedule solving all constraints in one call to the SMT Solver for a given network.
 It inits the solver and the network.
 It starts creating all the constraints (one variable for each transmission offset), then adds constraints relating
 different offsets. At the end solves the logical context and the model obtained is the solver.
 It creates an xml file with the output schedule.
 It also creates different constraint files for every switch in the network containing specific constraints for each
 switch

 @param network_file name of the file with the description of the network
 @param param_file name of the file with the parameters for the synthesizer
 @return 0 if the schedule was found, -1 if not found or so problem happened
 */
int one_shot_scheduling(char *network_file, char *param_file);
