/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network.c                                                                                                          *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Network.h                                                                                           *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Network.h"
#include <stdlib.h>

                                                    /* VARIABLES */


int num_frames;                     // Number of frames in the network
int num_links;                      // Number of links in the network
Frame *frames;                      // Array with all the frames in the network
Link *links;                        // Array with all the links in the network
long long int hyperperiod;          // Hyperperiod of the network schedule in ns
int hop_delay;                      // Time to wait to relay a frame after being received
long long int protocol_period;      // Period that we save for the protocol
long long int protocol_time;        // Time saved in every period for the protocol
long long int time_between_frames;  // Time between transmissions between frames

                                                /* AUXILIAR FUNCTIONS */

                                                    /* FUNCTIONS */

/**
 Get the number of frames in the network
 */
int get_number_frames(void) {
    
    return num_frames;
}

/**
 Set the number of frames in the network
 */
void set_number_frames(int number_frames) {
    
    num_frames = number_frames;
    frames = malloc(sizeof(Frame) * num_frames);        // Init the array of frames now that we now the number
}

/**
 Get the frame pointer given the frame id
 */
Frame * get_frame(int frame_id) {
    
    return &frames[frame_id];
}

/**
 Set the number of links in the network
 */
void set_number_links(int number_links) {
    
    num_links = number_links;
    links = malloc(sizeof(Link) * number_links);        // Init the array of links now that we now the number
}

/**
 Get the hop delay of the switches in the network
 */
int get_hop_delay(void) {
    
    return hop_delay;
}

/**
 Sets the hop delay of the switches in the network
 */
void set_hop_delay(int hp) {

    hop_delay = hp;
}

/**
 Sets the hyperperiod of the network schedule
 */
void set_hyper_period(long long int hyper_period) {
    
    hyperperiod = hyper_period;
}

/**
 Sets the parameters of the protocol
 */
void set_protocol_parameters(long long int period, long long int time) {
    
    protocol_period = period;
    protocol_time = time;
}

/**
 Get the time between frames
 */
long long int get_time_between_frames(void) {
    
    return time_between_frames;
}

/**
 Set the time between frames
 */
void set_time_between_frames(long long int time) {
    
    time_between_frames = time;
}

/**
 Add a link information to the link array
 */
int add_link(int link_id, int speed, LinkType link_type) {
    
    if (link_id >= num_links) {                       // If there are more links than it is supossed
        return -1;
    }
    set_link(&links[link_id], speed, link_type);      // Save the values
    return 0;
}

/**
 Adds to the given index frame the general information of the period, deadline and size in the frame array
 */
int add_frame_information(int frame_id, long long int period, long long int deadline, int size, long long int delay) {
    
    // Check if it fits in the frame array
    if (frame_id >= num_frames) {
        printf("There are more frames that the stated in the general information of the network file\n");
        return -1;
    }
    
    // Save all the information
    set_period(&frames[frame_id], period);
    set_deadline(&frames[frame_id], deadline);
    set_size(&frames[frame_id], size);
    set_end_to_end_delay(&frames[frame_id], delay);
    
    // Init also the hash array to accelerate links search
    init_hash(&frames[frame_id], num_links);
    
    return 0;
}

/**
 Add the number of paths to the frame
 */
int add_num_paths(int frame_id, int num_paths) {
    
    if (frame_id >= num_frames) {
        return -1;
    }
    
    set_num_paths(&frames[frame_id], num_paths);
    return 0;
}

/**
 Add a new path to the given frame identifier
 */
int add_frame_path(int frame_id, int path_id, int *path, int len_path) {
    
    if (frame_id > num_frames) {
        return -1;
    }
    
    add_path(&frames[frame_id], path_id, path, len_path);
    return 0;
}

/**
 Add the number of splits to the frame
 */
int add_num_splits(int frame_id, int num_splits) {
    
    if (frame_id >= num_frames) {
        return -1;
    }
    
    set_num_splits(&frames[frame_id], num_splits);
    return 0;
}

/**
 Add a new split to the given frame identifier
 */
int add_frame_split(int frame_id, int split_id, int *split, int len_split) {
    
    if (frame_id > num_frames) {
        return -1;
    }
    
    add_split(&frames[frame_id], split_id, split, len_split);
    return 0;
}

/**
 Init to reserve bandwitch for the protocol creating a fake frame.
 The fake frame has the period of the protocol, the timesize slot of the protocol and the same number of paths as links
 that include only one link in each path
 */
void initialize_protocol(void) {
    
    int path[1];
    
    // If there are protocol, add an extra fake frame
    if (protocol_period != 0) {
        num_frames++;
        frames = realloc(frames, sizeof(Frame) * num_frames);   // Allocate space for the new fake frame
        add_frame_information(num_frames - 1, protocol_period, protocol_period, 0, protocol_time + 1);
        add_num_paths(num_frames - 1, num_links);               // As many paths as links
        for (int i = 0; i < num_links; i++) {
            path[0] = i;
            add_path(&frames[num_frames - 1], i, path , 1);
        }
    }
    
}

/**
 Init all the needed variables in the network to start the scheduling, such as frame appearances, instances and similar
 */
void initialize_network(void) {
    
    int instances;
    int time;
    Offset *offset_it;          // Iterator to go through all offsets
    
    initialize_protocol();
    
    // For all frames, init the offset to -1, and set the appearances and the replicas depending on its period and
    // if they are wired or wireless link transmissions, also time for transmission
    for (int i = 0; i < num_frames; i++) {
        
        // Calculate the number of appearances Hyperperiod/period of the frame
        instances = (int)(hyperperiod / get_period(&frames[i]));
        offset_it = get_offset_root(&frames[i]);
        while (!is_last_offset(offset_it)) {
            
            // Set the number of instances, replicas and the transmission time of the offset
            set_instances(offset_it, instances);
            if (links[get_offset_link(offset_it)].type == wired) {      // If it wired, there is not replicas
                set_replicas(offset_it, 0);
            }
            // Calculate the time to transmit as BytesFrame / Speed in MB/s * 10^6 (to get to ns)
            if (get_size(&frames[i]) == 0) {
                time = (int)protocol_time;
            } else {
                time = (get_size(&frames[i]) * 1000000) / get_link_speed(&links[get_offset_link(offset_it)]);
            }
            set_timeslot_size(offset_it, time);
            
            // At the end, we prepare the offset to be ready, which allocates the matrix of transmission times
            prepare_offset(offset_it);
            
            offset_it = get_next_offset(offset_it);     // Advance to the next offset
        }
    }
}

/**
 Check if the schedule stored is correct and satisfies all the constraints
 */
int check_schedule_correctness(void) {
    
    Path *path_it, *next_path_pt;                 // Iterator for the paths linked list of a frame
    Offset *offset_pt, *next_offset_pt, *offset_it, *other_offset_it;
    long long int offset1, offset2, offset3 = 0;
    
    // For all the frames in the network
    for (int i = 0; i < num_frames; i++) {
        
        // For all the offsets of the frame
        offset_it = get_offset_root(&frames[i]);
        while (!is_last_offset(offset_it)) {
            
            // Check if the transmission times satisfy deadlines
            offset1 = get_offset(offset_it, 0, 0) + get_timeslot_size(offset_it);
            if (offset1 > get_deadline(&frames[i])) {
                printf("Error, the offset frames do not satisfy deadlines");
                return -1;
            }
            
            // Check that the transmission times are periodic
            for (int instace = 0; instace < get_number_instances(offset_it); instace++) {
                if (instace > 0) {
                    offset1 = get_offset(offset_it, instace - 1, 0);
                    offset2 = get_offset(offset_it, instace, 0);
                    if ((offset1 + get_period(&frames[i])) != offset2) {
                        printf("Error, the frames instances are not periodic\n");
                        return -1;
                    }
                }
            }
            
            // Check if this frames collide with another frames in the same time and link
            for (int j = 0; j < i; j++) {
                other_offset_it = get_frame_offset_by_link(&frames[j], get_offset_link(offset_it));
                if (other_offset_it != NULL) {  // If both frames share a link compare times
                    for (int instance = 0; instance < get_number_instances(offset_it); instance++) {
                        for (int other_instance = 0; other_instance < get_number_instances(other_offset_it);
                             other_instance++) {
                            offset1 = get_offset(offset_it, instance, 0);
                            offset2 = get_offset(other_offset_it, other_instance, 0);
                            if (((offset1 <= (offset2 + get_timeslot_size(other_offset_it))) &&
                                ((offset1 + get_timeslot_size(offset_it)) >= offset2)) ||
                                ((offset2 <= (offset1 + get_timeslot_size(offset_it))) &&
                                ((offset2 + get_timeslot_size(other_offset_it)) >= offset1))) {
                                printf("Error, frames are colliding\n");
                                return -1;
                            }
                        }
                    }
                }
            }
            
            offset_it = get_next_offset(offset_it);
        }
        
        // For all the paths in the network
        for (int j = 0; j < get_num_paths(&frames[i]); j++) {
            path_it = get_path_root(&frames[i], j);
            offset_pt = get_offset_from_path(path_it);
            offset3 = get_offset(offset_pt, 0, 0);
            while (!is_last_path(path_it)) {
                
                // Check if the paths are correctly formed
                offset1 = get_offset(offset_pt, 0, 0);
                offset_pt = get_offset_from_path(path_it);
                next_path_pt = get_next_path(path_it);
                if (!is_last_path(next_path_pt)) {
                    next_offset_pt = get_offset_from_path(next_path_pt);
                    offset2 = get_offset(next_offset_pt, 0, 0);
                    if (offset2 < (offset1 + hop_delay)) {
                        printf("Error checking the path dependent constraint\n");
                        return -1;
                    }
                } else {    // If it is the last, then we can check the end to end delay of the path
                    offset2 = get_offset(offset_pt, 0, 0);
                    if (((offset2 + get_timeslot_size(offset_pt)) - offset3) > get_end_to_end_delay(&frames[i])) {
                        printf("Error checking the end to end delay constraint\n");
                        return -1;
                    }
                    
                }
                path_it = get_next_path(path_it);
            }
        }
        
    }
    
    return 0;
}

/**
 Tells if the protocol for bandwitch allocation is active or not
 */
int is_protocol_active(void) {
    
    if (protocol_period == 0) {
        return 0;
    }
    return 1;
}
