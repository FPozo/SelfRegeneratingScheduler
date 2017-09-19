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
    struct Split *next_split_pt;        // Pointer to the next link in the split
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
    Split *split_array_ls;              // Array of root pointers to the split linked list (size is num_splits)
    int num_splits;                     // Number of splits in the frame
    Offset *offset_ls;                  // Pointer to the roof of the offsets linked list
    Offset **offset_hash;               // Array that stores the offsets with index the link identifier (to accelerate)
}Frame;

                                                /* CODE DEFINITIONS */


/**
 Init all the values of the frame to avoid unwanted values when malloc

 @param frame_pt Pointer to the frame to init
 @return 0 if init is correct, -1 otherwise
 */
int init_frame(Frame *frame_pt);

/**
 Init the size of the array of hashes to speed up offsets allocation

 @param frame_pt pointer to the frame
 @param num_links number of links in the network = size of the array
 @return 0 if done correctly, -1 otherwise
 */
int init_hash(Frame *frame_pt, int num_links);

/**
 Set the period of the given frame

 @param frame_pt pointer of the frame to add the period
 @param period long long int with the period in ns
 */
void set_period(Frame *frame_pt, long long int period);

/**
 Set the deadline of the given frame
 
 @param frame_pt pointer of the frame to add the deadline
 @param deadline long long int with the deadline in ns
 */
void set_deadline(Frame *frame_pt, long long int deadline);

/**
 Set the size of the given frame
 
 @param frame_pt pointer of the frame to add the size
 @param size long long int with the size in bytes
 */
void set_size(Frame *frame_pt, int size);

/**
 Set the number of paths in the given frame and allocate memory to store the roots of the linked lists

 @param frame_pt pointer to the frame
 @param num_paths number of paths that will have the frame
 */
void set_num_paths(Frame *frame_pt, int num_paths);

/**
 Add a new link of the path to the given path linked list pointer.
 We also use this function to add new offsets and link them to the new path and the hash acceleration table

 @param frame_pt pointer to the frame which path is being added
 @param path_id identifier of the path to add
 @param path array with the links of the path
 @param path_len number of links in the given path
 @return 0 if done correctly, -1 otherwise
 */
int add_path(Frame *frame_pt, int path_id, int *path, int path_len);

/**
 Set the number of splits in the given frame and allocate memory to store the roots of the linked lists
 
 @param frame_pt pointer to the frame
 @param num_splits number of splits that will have the frame
 */
void set_num_splits(Frame *frame_pt, int num_splits);

/**
 Add a new link of the path to the given split linked list pointer.
 
 @param frame_pt pointer to the frame which path is being added
 @param split_id identifier of the split to add
 @param split array with the links of the split
 @param split_len number of links in the given split
 @return 0 if done correctly, -1 otherwise
 */
int add_split(Frame *frame_pt, int split_id, int *split, int split_len);
