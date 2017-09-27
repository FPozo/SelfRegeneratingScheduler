/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  ConstraintSolver.h                                                                                                 *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the constraints that are solved by the solver                                                *
 *  We plan to implement many different solvers, such as Z3, ILP, Gurobi, etc. But for now we only implement Yices 2   *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef ConstraintSolver_h
#define ConstraintSolver_h

#include <stdio.h>
#include "Network.h"
#include <yices.h>

#endif /* ConstraintSolver_h */

                                                /* STRUCT DEFINITIONS */

/**
 Avaliable solvers
 */
typedef enum Solver{
    yices2
}Solver;

                                                /* CODE DEFINITIONS */

/**
 Initialize the given solver to start the scheduling process

 @param s solver willed to be used and initialize
 */
void initialize_solver(Solver s);

/**
 Creates the offset variables for all frames in the network, then adds them into the logical context

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, -1 if there was a problem creating the offset variables
 */
int create_offset_variables(Solver csolver);

/**
 Assures that no frames are allowed to be transmitted at the same time in the same link

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, -1 if there was a problem creating the contention free constraints
 */
int contention_free(Solver csolver);

/**
 Assures that all frames follow their path in the correct order

 @param csolver indicates which solver are we using
 @return 0 if everything was ok, -1 if there was a problem adding the path dependent the offset variables
 */
int frame_path_dependent(Solver csolver);

/**
 Assures that all frames follow their end to end delay in all paths from the first transmission to the last

 @param cssolver indicates which solver are we using
 @return 0 if everything was ok, -1 if there was a problem adding end to end daly constraints in the offsets
 */
int frame_end_to_end_delay(Solver cssolver);

/**
 Check the constraint solver and returns the status of it, if everything went well, it creates the schedule model

 @param csolver indicates which solver are we using
 @return 1 if the schedule was found, 0 otherwise
 */
int check_solver(Solver csolver);

/**
 Get the values obtained in the constraint solver and saves them into the offset variables as long long integers

 @param csolver indicates which solver are we using
 */
void save_offsets(Solver csolver);
