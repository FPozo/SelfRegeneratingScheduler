/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame.h                                                                                                            *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/07/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information of a single frame in the network.                                            *
 *  A frame should contain the information of period, deadline, size and the tree path it is following, including      *
 *  in which switches it is spliting to multiple links.                                                                *
 *  It also contains the offset for every link. As it is meant to work with different solvers, it also contains its    *
 *  variables                                                                                                          *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Frame_h
#define Frame_h

#include <stdio.h>
#include <yices.h>
#include "Link.h"

#endif /* Frame_h */

                                                /* STRUCT DEFINITIONS */

/**
 Structure with information of an appearance of an offset because the period. It has also arrays for all the information 
 about its retransmissions
 */
typedef struct Offset {
    long long int *offset;              // Matrix with the transmission times in ns
    term_t *y_offset;                   // Matrix with the yices 2 variables containting the offsets
    int num_instances;                  // Number of instances of the offset (hyperperiod / period frame)
    int num_replicas;                   // Number of replicas of the offset (retransmissions due to wireless)
    int timeslots;                      // Number of ns to transmit in the link
    int link;                           // Identifier of the link where this offset is being transmitted
    struct Offset *next_offset_pt;      // Pointer to the next offset (no order in particular)
}Offset;


/**
 Linked list of paths of a frame, its root is the sender and the end is the receiver.
 For that reason the order in the linked list is important, as it defines the path that the frame has to follow
 */
typedef struct Path {
    Offset *offset_pt;                  // Pointer to the offset struct in the linked list of the specific link
    int link;                           // Identifier of the link in the path
    struct Path *next_path_pt;          // Pointer to the next link in the path
}Path;

/**
 Linked list of splits of a frame, a split is a point in the tree path where a switch relays the frame to multiple
 frames.
 */
typedef struct Split {
    Offset *offset_pt;                  // Pointer to the offset struct in the linked list of the specific link
    int link;                           // Identifier of the link in the split
    struct Split *next_path_pt;         // Pointer to the next link in the split
}Split;

/**
 Information about the frame.
 As a frame can have multiple paths and splits, we will save its linked list roots into an array.
 */
typedef struct Frame {
    int size;                           // Size of the frames in bytes
    long long int period;               // Period of the frame in ns
    long long int deadline;             // Deadline of the frame in ns
    Path *path_array_ls;                // Array of root pointers to the path linked list (size is num_paths)
    int num_paths;                      // Number of paths in the frame
    Path *split_array_ls;               // Array of root pointers to the split linked list (size is num_splits)
    int num_splits;                     // Number of splits in the frame
    Offset *offset_ls;                  // Pointer to the roof of the offsets linked list
    int *offset_hash;                   // Array that stores the offsets with index the link identifier (to accelerate)
}Frame;

                                                /* CODE DEFINITIONS */


/**
 Init all the values of the frame to avoid unwanted values when malloc

 @param frame_pt Pointer to the frame to init
 @return 1 if init is correct, 0 otherwise
 */
int init_frame(Frame *frame_pt);
