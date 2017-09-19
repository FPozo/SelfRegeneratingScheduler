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


int num_frames;                 // Number of frames in the network
int num_links;                  // Number of links in the network
Frame *frames;                  // Array with all the frames in the network
Link *links;                    // Array with all the links in the network
long long int hyperperiod;      // Hyperperiod of the network schedule in ns
int hop_delay;                  // Time to wait to relay a frame after being received

                                                /* AUXILIAR FUNCTIONS */

                                                    /* FUNCTIONS */

/**
 Set the number of frames in the network
 */
void set_number_frames(int number_frames) {
    
    num_frames = number_frames;
    frames = malloc(sizeof(Frame) * num_frames);        // Init the array of frames now that we now the number
}

/**
 Set the number of links in the network
 */
void set_number_links(int number_links) {
    
    num_links = number_links;
    links = malloc(sizeof(Link) * number_links);        // Init the array of links now that we now the number
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
int add_frame_information(int frame_id, long long int period, long long int deadline, int size) {
    
    // Check if it fits in the frame array
    if (frame_id >= num_frames) {
        printf("There are more frames that the stated in the general information of the network file\n");
        return -1;
    }
    
    // Save all the information
    set_period(&frames[frame_id], period);
    set_deadline(&frames[frame_id], deadline);
    set_size(&frames[frame_id], size);
    
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
