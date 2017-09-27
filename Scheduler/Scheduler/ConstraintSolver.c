/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  ConstraintSolver.c                                                                                                 *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in ConstraintSolver.h                                                                                  *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "ConstraintSolver.h"

                                                    /* VARIABLES */

context_t *logical_context;             // Yices context where the constraints are saved to be solved
model_t *schedule_model;                // Model where to save the solution that yices find when the context is SAT
ctx_config_t *context_configuration;    // Configuration of the context to synthesize schedules faster

                                                /* AUXILIAR FUNCTIONS */

/**
 Returns 1 if there is any number that is shared between the two given intervals

 @param min1 minimum long long int of the first interval
 @param max1 maximum long long int of the first interval
 @param min2 minimum long long int of the second interval
 @param max2 maximum long long int of the second interval
 @return 1 if they share any number between both intervals, 0 otherwise
 */
int share_time_interval(long long int min1, long long int max1, long long int min2, long long int max2) {
    
    // if the first interval starts before and the second interval starts before the first ends
    // or if the second interval starts before and the first interval starts before the second ends
    if ((min1 <= min2 && min2 <= max1) || (min2 <= min1 && min1 <= max2)) {
        return 1;
    }
    return 0;
}

/**
 Returns 1 if it is possible for both offsets to collide in a transmission dependening on theirs allowed transmission
 time

 @param frame1_pt pointer of the frame 1
 @param offset1_pt pointer of the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param frame2_pt pointer of the frame 2
 @param offset2_pt pointer of the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @return 1 if it is possible for the times to collide, 0 otherwise
 */
int offsets_share_interval(Frame *frame1_pt, Offset *offset1_pt, int instance1, int replica1, Frame *frame2_pt,
                           Offset *offset2_pt, int instance2, int replica2) {
    
    long long int period1, period2;         // Periods of the given offsets
    long long int min1, max1, min2, max2;   // Time intervals of both offsets
    
    period1 = get_period(frame1_pt);
    period2 = get_period(frame2_pt);
    min1 = (period1 * instance1) + 1;       // Minimum is the period * the number of instance, +1 to avoid 0
    max1 = period1 * (instance1 + 1);       // Maximum is the period * (next instance) - 1, +1 to avoid 0
    min2 = (period2 * instance2) + 1;
    max2 = period2 * (instance2 + 1);
    
    // Compare with function
    if (share_time_interval(min1, max1, min2, max2) == 1) {
        return 1;
    }
    
    return 0;
}

/**
 Initialize the solver yices to be able to synthesize schedules
 */
void initialize_yices2_solver(void) {
    
    yices_init();
    context_configuration = yices_new_config();
    yices_default_config_for_logic(context_configuration, "QF_LIA");    // Faster for integer schedule synthesis
    logical_context = yices_new_context(context_configuration);     // Create the context where to add the constraints
    yices_free_config(context_configuration);                       // Free memory
    
}

/**
 Init the constraint variable into the offset

 @param offset_pt pointer of the offset
 @param instance of the offset
 @param replica of the offset
 @param name given to the variable
 @param csolver constraint solver used
 */
void init_variable(Offset *offset_pt, int instance, int replica, char *name, Solver csolver) {
    
    switch (csolver) {
        case yices2:
            set_yices_offset(offset_pt, instance, replica, yices_new_uninterpreted_term(yices_int_type()), name);
            break;
            
        default:
            break;
    }
}

/**
 Adds into the solver the constraints to limit transmission time range
 offset[instance][replica] = (min, max]

 @param offset_pt pointer to the offset
 @param instance of the offset
 @param replica of the offset
 @param min minimum transmission time in ns
 @param max maximum transmission time in ns
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int set_offset_range(Offset *offset_pt, int instance, int replica, long long int min, long long int max,
                     Solver csolver) {
    
    // Auxiliar variables to store constraints
    term_t y_integer;
    term_t y_formula;
    
    switch (csolver) {
        case yices2:
            
            // Set the minimum transmission time. Note that in Yices, we invert the schedule due to yices2 being weird
            y_integer = yices_int64(-min);
            y_formula = yices_arith_lt_atom(get_yices_offset(offset_pt, 0, 0), y_integer);
            //yices_pp_term(stdout, y_formula, 80, 1, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error asserting minimum transmission time in yices2\n");
                return -1;
            }
            
            // Set the maximum transmission time. Also inverted because is yices2 is weird
            y_integer = yices_int64(-max);
            y_formula = yices_arith_geq_atom(get_yices_offset(offset_pt, 0, 0), y_integer);
            //yices_pp_term(stdout, y_formula, 80, 1, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error asserting maximum transmission time in yices2\n");
                return -1;
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

/**
 Adds into the solver a constraint to set the distance between two offsets
 offset1[instance][replica] = offset2[instance][replica] + distance

 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance long long int with the distance between the two offsets in ns
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int set_fixed_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                       int replica2, long long int distance, Solver csolver) {
    
    // Auxiliar variables to store constraints
    term_t y_integer;
    term_t y_add;
    term_t y_formula;
    
    switch (csolver) {
        case yices2:
            // Set the distance between both offsets
            y_integer = yices_int64(distance);
            // Set the distance to the offset 1, yices_sub because schedule is inverted
            y_add = yices_sub(get_yices_offset(offset1_pt, instance1, replica1), y_integer);
            // Equal the distance with offset 2
            y_formula = yices_arith_eq_atom(get_yices_offset(offset2_pt, instance2, replica2), y_add);
            //yices_pp_term(stdout, y_formula, 80, 1, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error asserting distance between two offsets in yices2\n");
                return -1;
            }
            break;
            
        default:
            break;
    }
    
    return 0;
}

/**
 Adds into the solver a constraint to set the minimum distance between two offsets
 offset1[instance][replica] + distance < offset2[instance][replica]

 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance int with the distance between the two offsets in ns
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int set_minimum_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                         int replica2, long long int distance, Solver csolver) {
    
    // Auxiliar variabels to store constraints
    term_t y_integer;
    term_t y_add;
    term_t y_formula;
    
    switch (csolver) {
        case yices2:
            // Set the minimum distance between both offset
            y_integer = yices_int64(distance);
            // Set the distance to the offset 1, yices_sub because schedule is inverted
            y_add = yices_sub(get_yices_offset(offset1_pt, instance1, replica1), y_integer);
            // Less than with the offset2 (greather or equal because schedule is inverted)
            y_formula = yices_arith_geq_atom(y_add, get_yices_offset(offset2_pt, instance2, replica2));
            //yices_pp_term(stdout, y_formula, 80, 1, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error asserting minimum distance between two offsets in yices2\n");
                return -1;
            }
            break;
            
        default:
            break;
    }
    return 0;
}

/**
 Adds into the solver a constraint to set the maximum distance between two offsets
 offset1[instance][replica] + distance >= offset2[instance][replica]
 
 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance int with the distance between the two offsets in ns
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int set_maximum_distance(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                         int replica2, long long int distance, Solver csolver) {
    
    // Auxiliar variabels to store constraints
    term_t y_integer;
    term_t y_add;
    term_t y_formula;
    
    switch (csolver) {
        case yices2:
            // Set the maximum distance between both offset
            y_integer = yices_int64(distance);
            // Set the distance to the offset 1, yices_sub because schedule is inverted
            y_add = yices_sub(get_yices_offset(offset1_pt, instance1, replica1), y_integer);
            // Greater or equal with the offset2 (less than because schedule is inverted)
            y_formula = yices_arith_lt_atom(y_add, get_yices_offset(offset2_pt, instance2, replica2));
            //yices_pp_term(stdout, y_formula, 80, 1, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error asserting maximum distance between two offsets in yices2\n");
                return -1;
            }
            break;
            
        default:
            break;
    }
    return 0;
}

/**
 Avoids that the two given offsets share any transmission time
 offset1[instance][replica] + distance1 < offset2[instance][replica]
 OR
 offset2[instance][replica] + distance2 >= offset1[instance][replica]

 @param offset1_pt pointer to the offset 1
 @param instance1 of the offset 1
 @param replica1 of the offset 1
 @param offset2_pt pointer to the offset 2
 @param instance2 of the offset 2
 @param replica2 of the offset 2
 @param distance1 long long int with the distance the first offset can go
 @param distance2 long long int with the distance the second offset can go
 @param csolver constraint solver used
 @return 0 if everything went ok, -1 if something failed
 */
int avoid_intersection(Offset *offset1_pt, int instance1, int replica1, Offset *offset2_pt, int instance2,
                       int replica2, long long int distance1, long long int distance2, Solver csolver) {
    
    // Auxiliar variables for yices
    term_t y_integer;
    term_t y_add;
    term_t y_less;
    term_t y_greater;
    term_t y_formula;
    
    switch (csolver) {
        case yices2:
            y_integer = yices_int64(distance1);
            y_add = yices_sub(get_yices_offset(offset1_pt, instance1, replica1), y_integer);
            y_less = yices_arith_gt_atom(y_add, get_yices_offset(offset2_pt, instance2, replica2));
            y_integer = yices_int64(distance2);
            y_add = yices_sub(get_yices_offset(offset2_pt, instance2, replica2), y_integer);
            y_greater = yices_arith_lt_atom(get_yices_offset(offset1_pt, instance1, replica1), y_add);
            y_formula = yices_or2(y_less, y_greater);
            //yices_pp_term(stdout, y_formula, 12000, 10000, 0);     // Printing of the formula, remove after debugging
            if (yices_assert_formula(logical_context, y_formula) == -1) {
                printf("Error avoiding intersection between two offsets in yices2\n");
                return -1;
            }
            break;
            
        default:
            break;
    }
    return 0;
}

/**
 From the solver, get the value of the offset and return it converted to long long int

 @param offset_pt pointer to the offset
 @param instance of the offset
 @param replica of the offset
  @param csolver constraint solver used
 @return long long integer of the value obtained by the solver
 */
long long int get_solver_offset(Offset *offset_pt, int instance, int replica, Solver csolver) {
    
    long long int value;
    
    switch (csolver) {
        case yices2:
            yices_get_int64_value(schedule_model, get_yices_offset(offset_pt, instance, replica), &value);
            return -value;      // The schedule is inverted in yices, so we invert the value
            break;
            
        default:
            break;
    }
}

                                                    /* FUNCTIONS */

/**
 Initialize the given solver to start the scheduling process
 */
void initialize_solver(Solver s) {
    
    switch (s) {
        case yices2:
            initialize_yices2_solver();
            break;
            
        default:
            break;
    }
}

/**
 Creates the offset variables for all frames in the network, then adds them into the logical context
 */
int create_offset_variables(Solver csolver) {
    
    Offset *offset_it;                  // Iterator to move through the offsets of the offsets linked list
    Frame *frame_pt;                    // Pointer to a frame of the network
    char name[50];                      // String to store the name of a offset variables
    long long int period;               // Stores the period of a frame
    long long int distance;             // To calculate the distance between instance 0, replica 0 and others
    long long int transmission_time;    // Time for a frame needed to be transmitted in a specific link
    long long int maximum_time;         // Maximum time allowed to start the transmission of an offset
    
    // For all the given frames, look for all its offsets
    for (int i = 0; i < get_number_frames(); i++) {
        frame_pt = get_frame(i);
        period = get_period(frame_pt);
        offset_it = get_offset_root(frame_pt);      // Get the offset root of the frame to iterate over all offsets
        while (!is_last_offset(offset_it)) {
            
            // Get the number of replicas and instances of the offsets, as there is a variable for each one
            transmission_time = get_timeslot_size(offset_it);   // Get also the transmission time, needed to set range
            // Iterate over all replicas and instances
            for (int instance = 0; instance < get_number_instances(offset_it); instance++) {
                // <= Because there exist a replica 0
                for (int replica = 0; replica <= get_number_replicas(offset_it); replica++) {
                    // The name of a variable is O_frameid_instance_replica_link (O => Offset)
                    sprintf(name, "O_%d_%d_%d_%d", i, instance, replica, get_offset_link(offset_it));
                    init_variable(offset_it, instance, replica, name, csolver);
                    
                    // When the instance and replica is no 0 and 0 set the distance to intance and replica 0 and 0
                    if (instance != 0 || replica != 0) {
                        // Set the instances and replicas > 1 to be related to the instance 0 replica 0
                        distance = period * instance;
                        if (set_fixed_distance(offset_it, 0, 0, offset_it, instance, replica, distance,
                                               csolver) == -1) {
                            printf("Error when setting the distance between different instances and replicas\n");
                            return -1;
                        }
                    }
                }
            }
            // Set the minimum and maximum transmission time for the offset, note that we only do it for the
            // instance 0, replica 0, as the time between different instances and replicas are related to the 0, 0
            // We need to extract the transmission time of the offset to the deadline to allow it to finish before
            maximum_time = get_deadline(frame_pt) - transmission_time;
            if (set_offset_range(offset_it, 0, 0, 0, maximum_time, csolver) == -1) {
                printf("Error when setting the offset range creating the offset variable\n");
                return -1;
            }
            
            offset_it = get_next_offset(offset_it);
        }
    }
    
    return 0;
}

/**
 Assures that no frames are allowed to be transmitted at the same time in the same link
 */
int contention_free(Solver csolver) {
    
    Offset *offset_it;                  // Iterator to move through the offsets of the offsets linked list
    Offset *previous_offset_it;         // Iterator to move through the previous offsets of the offsets linked list
    Frame *frame_pt;                    // Pointer to a frame of the network
    Frame *previous_frame_pt;           // Pointer to the previous added constraint frame of the network
    int link;                           // Link identifier
    
    // For all the given frames
    for (int i = 0; i < get_number_frames(); i++) {
        frame_pt = get_frame(i);
        offset_it = get_offset_root(frame_pt);      // Get the offset root of the frame to iterate over all offsets
        while (!is_last_offset(offset_it)) {
            link = get_offset_link(offset_it);      // Get the link of the offset to compare with the previous frame
            for (int instance = 0; instance < get_number_instances(offset_it); instance++) {
                // <= Because there exist a replica 0
                for (int replica = 0; replica <= get_number_replicas(offset_it); replica++) {
                    
                    // For all frames that were previously iterated (so we save ourselves from duplicities)
                    for (int j = 0; j < i; j++) {
                        previous_frame_pt = get_frame(j);
                        previous_offset_it = get_frame_offset_by_link(previous_frame_pt, link);
                        // If the previous frame has an offset with the same link, continue
                        if (previous_offset_it != NULL) {
                            for (int previous_instance = 0;
                                 previous_instance < get_number_instances(previous_offset_it); previous_instance++) {
                                for (int previous_replica = 0;
                                     previous_replica <= get_number_replicas(previous_offset_it);
                                     previous_replica++) {
                                    
                                    if (offsets_share_interval(frame_pt, offset_it, instance, replica,
                                                               previous_frame_pt, previous_offset_it, previous_instance,
                                                               previous_replica)) {
                                        // Add the constraint to avoid the offsets to collide
                                        if (avoid_intersection(offset_it, instance, replica, previous_offset_it,
                                                               previous_instance, previous_replica,
                                                               get_timeslot_size(offset_it),
                                                               get_timeslot_size(previous_offset_it), csolver) == -1) {
                                            printf("Error when doing contention free constraints\n");
                                            return -1;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            offset_it = get_next_offset(offset_it);
        }
    }
    return 0;
}

/**
 Assures that all frames follow their parth in the correct order
 */
int frame_path_dependent(Solver csolver) {
    
    Path *path_it;                      // Iterator to move through the paths of the paths linked list
    Offset *offset_pt;                  // Offset pointer of the path
    Offset *next_path_offset_pt;        // Offset pointer to the next link of the path
    Frame *frame_pt;                    // Pointer to a frame of the network
    int hop_delay = get_hop_delay();    // Minimum time that a frame has to wait in a switch to be relayed
    int distance;                       // Minimum distance between both consecutive links in a path
    
    // For all the given frames
    for (int i = 0; i < get_number_frames(); i++) {
        frame_pt = get_frame(i);
        // For all the paths of the frame, go path by path
        for (int path_id = 0; path_id < get_num_paths(frame_pt); path_id++) {
            path_it = get_path_root(frame_pt, path_id);
            while (!is_last_path(path_it)) {                // While it is not the last link of the path
                offset_pt = get_offset_from_path(path_it);  // Get the offset of the path and calculate the min distance
                distance = hop_delay + (int)get_timeslot_size(offset_pt) + 1;
                path_it = get_next_path(path_it);           // Get the next link on the path
                if (!is_last_path(path_it)) {               // If it is not the last one, add the constraint
                    next_path_offset_pt = get_offset_from_path(path_it);
                    if (set_minimum_distance(offset_pt, 0, 0, next_path_offset_pt, 0, 0, distance, csolver) == -1) {
                        printf("Error when doing path dependent constraints\n");
                        return -1;
                    }
                }
            }
        }
    }
    return 0;
}

/**
 Assures that all frames follow their end to end delay in all paths from the first transmission to the last
 */
int frame_end_to_end_delay(Solver cssolver) {
    
    Path *path_it;                      // Iterator to move through the paths of the paths linked list
    Path *last_path = NULL;             // Last link on a path
    Offset *first_offset_pt;            // Offset pointer of the path
    Offset *last_offset_pt;             // Offset pointer of the path
    Frame *frame_pt;                    // Pointer to a frame of the network
    long long int distance;             // Maximum end to end delay distance
    long long int delay;                // End to end delay of the frame
    
    // For all the given frames
    for (int i = 0; i < get_number_frames(); i++) {
        frame_pt = get_frame(i);
        delay = get_end_to_end_delay(frame_pt);
        // For all the paths of the frame, go path by path
        for (int path_id = 0; path_id < get_num_paths(frame_pt); path_id++) {
            path_it = get_path_root(frame_pt, path_id);
            first_offset_pt = get_offset_from_path(path_it);
            
            // Get the last link of the path
            while (!is_last_path(path_it)) {                // While it is not the last link of the path
                last_path = path_it;                        // Save the pointer in case is the last link of the path
                path_it = get_next_path(path_it);
            }
            
            // Get the last link offset and calculate the distance as the delay - the time to transmit last link
            last_offset_pt = get_offset_from_path(last_path);
            distance = delay - get_timeslot_size(last_offset_pt);
            if (set_maximum_distance(first_offset_pt, 0, 0, last_offset_pt, 0, 0, distance, cssolver) == -1) {
                printf("Error when doing end to end delay constraints\n");
                return -1;
            }
        }
    }
    return 0;
}

/**
 Check the constraint solver and returns the status of it, if everything went well, it creates the schedule model
 */
int check_solver(Solver csolver) {
    
    switch (csolver) {
        case yices2:
            // If we found schedule, create the model
            if (yices_check_context(logical_context, NULL) == STATUS_SAT) {
                schedule_model = yices_get_model(logical_context, 1);       // Get the model with the schedule
                //yices_pp_model(stdout, schedule_model, 80, 1000, 1);        // Debug print model
                return 1;
            }
            return -1;
            break;
            
        default:
            break;
    }
}

/**
 Get the values obtained in the constraint solver and saves them into the offset variables as long long integers
 */
void save_offsets(Solver csolver) {
    
    Offset *offset_it;                  // Iterator to move through the offsets of the offsets linked list
    Frame *frame_pt;                    // Pointer to a frame of the network
    long long int transmission_time;    // Transmission time of a offset
    
    // For all the given frames, look for all its offsets
    for (int i = 0; i < get_number_frames(); i++) {
        frame_pt = get_frame(i);
        offset_it = get_offset_root(frame_pt);      // Get the offset root of the frame to iterate over all offsets
        while (!is_last_offset(offset_it)) {
            // Iterate over all replicas and instances
            for (int instance = 0; instance < get_number_instances(offset_it); instance++) {
                // <= Because there exist a replica 0
                for (int replica = 0; replica <= get_number_replicas(offset_it); replica++) {
                    
                    // Get the value from the solver and save it as long long integer
                    transmission_time = get_solver_offset(offset_it, instance, replica, csolver);
                    set_offset(offset_it, instance, replica, transmission_time);
                }
            }
            offset_it = get_next_offset(offset_it);
        }
    }
}
