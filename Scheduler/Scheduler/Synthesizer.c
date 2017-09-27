/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Synthesizer.c                                                                                                      *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Synthesizer.h                                                                                       *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Synthesizer.h"
#include "Network.h"
#include "IOInterface.h"
#include "ConstraintSolver.h"

                                                    /* VARIABLES */

                                                /* AUXILIAR FUNCTIONS */

                                                    /* FUNCTIONS */

/**
 Produces the schedule solving all constraints in one call to the SMT Solver for a given network.
 It inits the solver and the network.
 It starts creating all the constraints (one variable for each transmission offset), then adds constraints relating
 different offsets. At the end solves the logical context and the model obtained is the solver.
 It creates an xml file with the output schedule.
 It also creates different constraint files for every switch in the network containing specific constraints for each
 switch
 */
int one_shot_scheduling(char *network_file, char *param_file) {
    
    // Variables
    Solver csolver = yices2;            // State the constraint solver we want to use
    
    // Read the network file, parse it into internal memory, and prepare the network and solver
    parse_network_xml(network_file);
    initialize_network();               // Prepare the network variables to start scheduling
    initialize_solver(csolver);         // Prepare the constraint solver to start scheduling
    
    // Create all the offset variables with the allowed ranges of transmissions
    if (create_offset_variables(csolver) == -1) {
        printf("There was a problem creating and Initializing constraint variables\n");
        return -1;
    }
    // Make sure that frames are not transmitted at the same time at the same link
    if (contention_free(csolver) == -1) {
        printf("There was a problem making the contention free constraints");
        return -1;
    }
    // Make all the frame to be path dependent, they should follow an order
    if (frame_path_dependent(csolver) == -1) {
        printf("There was a a problem making the frames to be path dependent\n");
        return -1;
    }
    // Generate end to end delays constraings for all the frames
    if (frame_end_to_end_delay(csolver) == -1) {
        printf("There was a problem making the end to end delay of the frames\n");
        return -1;
    }
    // Solve the logical context and get the schedule if it exist
    if (check_solver(csolver) == -1) {
        printf("The constraints were unsatisfiable, no schedule was found\n");
        return -1;
    }
    // Save the values obtained by the solver
    save_offsets(csolver);
    
    // Check if the scheduled done is correct
    if (check_schedule_correctness() == -1) {
        printf("The schedule is not correct\n");
        return -1;
    }
    
    return 0;
}
