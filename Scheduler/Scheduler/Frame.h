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
    long long int **offset;             // Matrix with the transmission times in ns
    term_t **y_offset;                  // Matrix with the yices 2 variables containting the offsets
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
    long long int end_to_end_delay;     // Maximum end to end delay from a frame being sent to being received
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
 Get the period of the given frame

 @param frame_pt pointer of the frame to get the period
 @return long long int with the period
 */
long long int get_period(Frame *frame_pt);

/**
 Set the period of the given frame

 @param frame_pt pointer of the frame to add the period
 @param period long long int with the period in ns
 */
void set_period(Frame *frame_pt, long long int period);

/**
 Get the deadline of the given frame

 @param frame_pt pointer to the frame
 @return long long int with the deadline
 */
long long int get_deadline(Frame *frame_pt);

/**
 Set the deadline of the given frame
 
 @param frame_pt pointer of the frame to add the deadline
 @param deadline long long int with the deadline in ns
 */
void set_deadline(Frame *frame_pt, long long int deadline);

/**
 Get the size in bytes of the given frame

 @param frame_pt pointer of the frame
 @return integer with the size of the frame in bytes
 */
int get_size(Frame *frame_pt);

/**
 Set the size of the given frame
 
 @param frame_pt pointer of the frame to add the size
 @param size long long int with the size in bytes
 */
void set_size(Frame *frame_pt, int size);

/**
 Get the end to end delay of the given frame

 @param frame_pt pointer to the frame
 @return long long int with the end to end delay of the frame
 */
long long int get_end_to_end_delay(Frame *frame_pt);

/**
 Set the end to end delay of the given frame

 @param frame_pt pointer to the frame
 @param delay long long int with the desired end to end delay
 */
void set_end_to_end_delay(Frame *frame_pt, long long int delay);

/**
 Get the number of instances of the offset

 @param offset_pt pointer to the offset
 @return number of instances
 */
int get_number_instances(Offset *offset_pt);

/**
 Set the number of instances of the given frame

 @param offset_pt pointer to the offset
 @param instances number of instances to set
 */
void set_instances(Offset *offset_pt, int instances);

/**
 Get the number of replicas of the offset

 @param offset_pt offset pointer
 @return number of replicas
 */
int get_number_replicas(Offset *offset_pt);

/**
 Set the number of replicas of the given frame

 @param offset_pt pointer to the offset
 @param replicas number of replicas to set
 */
void set_replicas(Offset *offset_pt, int replicas);

/**
 Gets the number of timeslots that the offset needs to be transmitted

 @param offset_pt pointer to the offset
 @return size in timeslots (ns)
 */
long long int get_timeslot_size(Offset *offset_pt);

/**
 Set the number of timeslots that the offset needs to be transmitted

 @param offset_pt pointer of the offset
 @param  size in timeslots (ns)
 */
void set_timeslot_size(Offset *offset_pt, int size);

/**
 Get the path root of the given frame and path number

 @param frame_pt pointer to the frame
 @param path_id integer identifying the number of the path
 @return path pointer to the root of the paths linked list
 */
Path * get_path_root(Frame *frame_pt, int path_id);

/**
 Get the next path pointer if exists, NULL if it is the last one

 @param path_pt pointer to the path
 @return path pointer to the root of the path linked list
 */
Path * get_next_path(Path *path_pt);

/**
 Checks if it is the last path of the paths linked list

 @param path_pt pointer to the path
 @return 1 if it is the last path, 0 if not, -1 if a problem was found
 */
int is_last_path(Path *path_pt);

/**
 Get the offset root of the given frame

 @param frame_pt pointer to the frame
 @return offset pointer to the root of the offsets linked list
 */
Offset * get_offset_root(Frame *frame_pt);

/**
 Get the next offset pointer if exists, NULL if it is the last one
 
 @param offset_pt pointer to the offset
 @return next offset pointer, or null if is the last offset
 */
Offset * get_next_offset(Offset *offset_pt);

/**
 Checks if it is the last offset of the offsets linked list

 @param offset_pt pointer to the offset
 @return 1 if is the last offset, 0 if not, -1 if a problem was found
 */
int is_last_offset(Offset *offset_pt);

/**
 Get the link of the given offset

 @param offset_pt pointer of the offset
 @return the identifier of the link
 */
int get_offset_link(Offset *offset_pt);

/**
 Get the number of paths in the given frame

 @param frame_pt pointer to the frame
 @return integer with the number of paths
 */
int get_num_paths(Frame *frame_pt);

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

/**
 Set a transmission time to the offset of the given Offset

 @param offset_pt offset pointer
 @param instance number of instance in the offset
 @param replica number of replica in the offset
 @param value transmission time in ns
 */
void set_offset(Offset *offset_pt, int instance, int replica, long long int value);

/**
 Get the Offset pointer from the given path pointer

 @param path_pt pointer of the path
 @return offset pointer of the path
 */
Offset * get_offset_from_path(Path *path_pt);

/**
 Get yices offset of the given instance and replica
 
 @param offset_pt pointer to the offset
 @param instance number
 @param replica number
 @return term_t offset constraint of yices
 */
term_t get_yices_offset(Offset *offset_pt, int instance, int replica);

/**
 Set a yices variable constraint of the given Offset

 @param offset_pt offset pointer
 @param instance number of instance in the offset
 @param replica number of replica in the offset
 @param constraint yices constraint
 @param name of the constraint variable
 */
void set_yices_offset(Offset *offset_pt, int instance, int replica, term_t constraint, char* name);

/**
 Allocates the memory needed and prepare all variables for the used to be ready to be used

 @param offset_pt pointer of the offset
 */
void prepare_offset(Offset *offset_pt);

/**
 Get the Offset pointer of a frame with the given link.
 This function is O(1) using a hash table and tries to avoid to find the offset iterating the whole offset linked list

 @param frame_pt pointer to the frame
 @param link identifier of the link being search
 @return pointer to the offset of the frame that has the link given in the parameters, NULL if not in the linked list
 */
Offset * get_frame_offset_by_link(Frame *frame_pt, int link);
