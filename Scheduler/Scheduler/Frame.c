/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame.c                                                                                                            *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Frame.h                                                                                             *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Frame.h"
#include <stdlib.h>

                                                    /* VARIABLES */

                                                /* AUXILIAR FUNCTIONS */

/**
 For the given Offsets Linked List, adds the offsets if is new.
 If it not new and already present, returns the pointer to the found one

 @param offset_pt offset linked list root
 @param link link to find or add
 @return the offset pointer to the offset created or found
 */
Offset * add_new_offset(Offset *offset_pt, int link) {
 
    // While there are offsets to search and the link is no the same, we keep searching until the last offset
    while (offset_pt->next_offset_pt != NULL && offset_pt->link != link) {
        offset_pt = offset_pt->next_offset_pt;
    }
    
    // If the link is not in the linked list, we create a new offset and add it, if not we just return the found offset
    if (offset_pt->link != link) {
        offset_pt->link = link;
        offset_pt->offset = 0;
        offset_pt->next_offset_pt = malloc(sizeof(Offset));     // We create the next offset as empty
        offset_pt->next_offset_pt->next_offset_pt = NULL;
        offset_pt->next_offset_pt->link = -1;                   // Just in case to control the link value
    }
    
    return offset_pt;
}

                                                    /* FUNCTIONS */

/**
 Init all the values of the frame to avoid unwanted values when malloc
 */
int init_frame(Frame *frame_pt) {
    
    frame_pt->deadline = -1;
    frame_pt->period = -1;
    frame_pt->size = -1;
    frame_pt->num_paths = -1;
    frame_pt->num_splits = -1;
    frame_pt->path_array_ls = NULL;
    frame_pt->split_array_ls = NULL;
    frame_pt->offset_ls = NULL;
    frame_pt->offset_hash = NULL;
    return 0;
}

/**
 Init the size of the array of hashes to speed up offsets allocation
 */
int init_hash(Frame *frame_pt, int num_links) {
    
    if (frame_pt == NULL) {
        return -1;
    }
    
    frame_pt->offset_hash = malloc(sizeof(Offset *) * num_links);
    for (int i = 0; i < num_links; i++) {
        frame_pt->offset_hash[i] = NULL;
    }
    return 0;
}

/**
 Get the period of the given frame
 */
long long int get_period(Frame *frame_pt) {
    
    return frame_pt->period;
}

/**
 Set the period of the given frame
 */
void set_period(Frame *frame_pt, long long int period) {
    
    frame_pt->period = period;
}

/**
 Get the deadline of the given frame
 */
long long int get_deadline(Frame *frame_pt) {
    
    return frame_pt->deadline;
}

/**
 Set the deadline of the given frame
 */
void set_deadline(Frame *frame_pt, long long int deadline) {
    
    frame_pt->deadline = deadline;
}

/**
 Get the size in bytes of the given frame
 */
int get_size(Frame *frame_pt) {
    
    return frame_pt->size;
}

/**
 Set the size of the given frame
 */
void set_size(Frame *frame_pt, int size) {
    
    frame_pt->size = size;
}

/**
 Get the end to end delay of the given frame
 */
long long int get_end_to_end_delay(Frame *frame_pt) {
    
    return frame_pt->end_to_end_delay;
}

/**
 Set the end to end delay of the given frame
 */
void set_end_to_end_delay(Frame *frame_pt, long long int delay) {
    
    frame_pt->end_to_end_delay = delay;
}

/**
 Get the number of instances of the offset
 */
int get_number_instances(Offset *offset_pt) {
    
    return offset_pt->num_instances;
}

/**
 Set the number of instances of the given frame
 */
void set_instances(Offset *offset_pt, int instances) {
    
    offset_pt->num_instances = instances;
}

/**
 Get the number of replicas of the offset
 */
int get_number_replicas(Offset *offset_pt) {
    
    return offset_pt->num_replicas;
}

/**
 Set the number of replicas of the given frame
 */
void set_replicas(Offset *offset_pt, int replicas) {
    
    offset_pt->num_replicas = replicas;
}

/**
 Gets the number of timeslots that the offset needs to be transmitted
 */
long long int get_timeslot_size(Offset *offset_pt) {
    
    return offset_pt->timeslots;
}

/**
 Set the number of timeslots that the offset needs to be transmitted
 */
void set_timeslot_size(Offset *offset_pt, int size) {
    
    offset_pt->timeslots = size;
}

/**
 Get the path root of the given frame and path number
 */
Path * get_path_root(Frame *frame_pt, int path_id) {
    
    return &frame_pt->path_array_ls[path_id];
}

/**
 Get the next path pointer if exists, NULL if it is the last one
 */
Path * get_next_path(Path *path_pt) {
    
    return path_pt->next_path_pt;
}

/**
 Checks if it is the last path of the paths linked list
 */
int is_last_path(Path *path_pt) {
    
    if (path_pt->next_path_pt == NULL) {
        return 1;
    }
    return 0;
}

/**
 Get the offset root of the given frame
 */
Offset * get_offset_root(Frame *frame_pt) {
    
    return frame_pt->offset_ls;
}

/**
 Get the next offset pointer if exists, NULL if is the last one
 */
Offset * get_next_offset(Offset *offset_pt) {
    
    return offset_pt->next_offset_pt;
}

/**
 Checks if it is the last offset of the offsets linked list
 */
int is_last_offset(Offset *offset_pt) {
    
    if (offset_pt->next_offset_pt == NULL) {        // It is the last offset of the linked list
        return 1;
    }
    return 0;
}

/**
 Get the link of the given offset
 */
int get_offset_link(Offset *offset_pt) {
    
    return offset_pt->link;
}

/**
 Get the number of paths in the given frame
 */
int get_num_paths(Frame *frame_pt) {
    
    return frame_pt->num_paths;
}

/**
 Set the number of paths in the given frame and allocate memory to store the roots of the linked lists
 */
void set_num_paths(Frame *frame_pt, int num_paths) {
    
    frame_pt->num_paths = num_paths;
    frame_pt->path_array_ls = malloc(sizeof(Path) * num_paths);     // Allocate memory for all the paths roots
    frame_pt->offset_ls = malloc(sizeof(Offset));                   // We can start allocating for the offsets too
    frame_pt->offset_ls->next_offset_pt = NULL;                     // Just in case
    
}

/**
 Add a new link of the path to the given path linked list pointer.
 We also use this function to add new offsets and link them to the new path and the hash acceleration table
 */
int add_path(Frame *frame_pt, int path_id, int *path, int len_path) {
    
    Path *path_it;      // Iterator over the path
    
    if (frame_pt == NULL) {
        return -1;
    }
    
    // For all the links in the path, add it to the new path linked list
    path_it = &frame_pt->path_array_ls[path_id];
    for (int i = 0; i < len_path; i++) {
        // Add the new path at the end of the path linked list
        path_it->link = path[i];
        path_it->next_path_pt = malloc(sizeof(Path));       // Allocate memory for the next path
        path_it->next_path_pt->next_path_pt = NULL;         // Mark as empty
        path_it->next_path_pt->link = -1;
        
        // Link the path offset pointer to the offset in the linked list with the same link id
        path_it->offset_pt = add_new_offset(frame_pt->offset_ls, path[i]);
        
        // Add also the offset link to the hash accelerator
        frame_pt->offset_hash[path_it->link] = path_it->offset_pt;
        
        path_it = path_it->next_path_pt;                    // Point now to the next path
    }

    return 0;
}

/**
 Set the number of splits in the given frame and allocate memory to store the roots of the linked lists
 */
void set_num_splits(Frame *frame_pt, int num_splits) {
    
    frame_pt->num_splits = num_splits;
    frame_pt->split_array_ls = malloc(sizeof(Split) * num_splits);  // Allocate memory for all the splits roots
}

/**
 Add a new link of the path to the given split linked list pointer.
 */
int add_split(Frame *frame_pt, int split_id, int *split, int split_len) {
    
    Split *split_it;    // Iterator over the split
    
    if (frame_pt == NULL) {
        return -1;
    }
    
    // For all the links in the split, add it to the new split linked list
    split_it = &frame_pt->split_array_ls[split_id];
    for (int i = 0; i < split_len; i++) {
        split_it->link = split[i];
        split_it->next_split_pt = malloc(sizeof(Split));        // Allocate memory for the next split
        split_it = split_it->next_split_pt;                     // Point to the next split
        split_it->next_split_pt = NULL;                         // Mark it as empty
        split_it->link = -1;
    }
    
    return 0;
}

/**
 Set a transmission time to the offset of the given Offset
 */
void set_offset(Offset *offset_pt, int instance, int replica, long long int value) {
    
    offset_pt->offset[instance][replica] = value;
}

/**
 Get the Offset pointer from the given path pointer
 */
Offset * get_offset_from_path(Path *path_pt) {
    
    return path_pt->offset_pt;
}

/**
 Get yices offset of the given instance and replica
 */
term_t get_yices_offset(Offset *offset_pt, int instance, int replica) {
    
    return offset_pt->y_offset[instance][replica];
}

/**
 Set a yices variable constraint of the given Offset
 */
void set_yices_offset(Offset *offset_pt, int instance, int replica, term_t constraint, char* name) {
    
    offset_pt->y_offset[instance][replica] = constraint;
    yices_set_term_name(offset_pt->y_offset[instance][replica], name);
}

/**
 Allocates the memory needed and prepare all variables for the used to be ready to be used
 */
void prepare_offset(Offset *offset_pt) {
    
    // Dynamically allocate an array for the offsets of size [num_instances][num_replicas + 1]
    offset_pt->offset = malloc(sizeof(long long int *) * offset_pt->num_instances);
    offset_pt->y_offset = malloc(sizeof(term_t *) * offset_pt->num_instances);
    for (int i = 0; i < offset_pt->num_instances; i++) {
        offset_pt->offset[i] = malloc(sizeof(long long int) * (offset_pt->num_replicas + 1));
        offset_pt->y_offset[i] = malloc(sizeof(term_t) * (offset_pt->num_replicas + 1));
    }
}

/**
 Get the Offset pointer of a frame with the given link.
 This function is O(1) using a hash table and tries to avoid to find the offset iterating the whole offset linked list
 */
Offset * get_frame_offset_by_link(Frame *frame_pt, int link) {
    
    return frame_pt->offset_hash[link];
}
