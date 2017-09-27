/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network.h                                                                                                          *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information of the network.                                                              *
 *  A network has the information of all the frames and its dependencies (Application constraints of different types). *
 *  All frames are saved in an array.                                                                                  *
 *  Additions of new relations between frames are supposed to be added here, while the behavior is on the schedule,    *
 *  as done with the application constraints, period and deadlines.                                                    *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Network_h
#define Network_h

#include <stdio.h>
#include "Frame.h"

#endif /* Network_h */

                                                /* STRUCT DEFINITIONS */


                                                /* CODE DEFINITIONS */

/**
 Get the number of frames in the network

 @return number of frames in the network
 */
int get_number_frames(void);

/**
 Set the number of frames in the network

 @param number_frames integer with the number of frames in the network
 */
void set_number_frames(int number_frames);

/**
 Get the frame pointer given the frame id

 @param frame_id integer with the frame identifier
 @return pointer of the frame
 */
Frame * get_frame(int frame_id);

/**
 Set the number of links in the network

 @param number_links integer with the number of links in the network
 */
void set_number_links(int number_links);

/**
 Get the hop delay of the switches in the network

 @return integer with the hop delay
 */
int get_hop_delay(void);

/**
 Sets the hop delay of the switches in the network
 
 @param hp integer with the hope delay of all switches in the network
 */
void set_hop_delay(int hp);

/**
 Sets the hyperperiod of the network schedule

 @param hyper_period hyperperiod of the network in ns
 */
void set_hyper_period(long long int hyper_period);

/**
 Add a link information to the link array

 @param link_id index of the link
 @param speed integer with the speed of the link in MB/s
 @param link_type type of the link (wired or wireless)
 @return 0 if added correctly, -1 if out of index
 */
int add_link(int link_id, int speed, LinkType link_type);

/**
 Adds to the given index frame the general information of the period, deadline and size in the frame array

 @param frame_id index of the frame to add information from the frame array
 @param period long long int of the period in ns
 @param deadline long long int of the deadline in ns
 @param size int of the size in bytes
 @param delay long long int of the end to end delay in ns
 @return 0 if done correctly, -1 if index out of array of frames
 */
int add_frame_information(int frame_id, long long int period, long long int deadline, int size, long long int delay);

/**
 Add the number of paths to the frame

 @param frame_id index of the frame
 @param num_paths integer with the number of paths that the frame will have
 @return 0 if correct, -1 otherwise
 */
int add_num_paths(int frame_id, int num_paths);

/**
 Add a new path to the given frame identifier

 @param frame_id identifier of the frame
 @param path_id identifier of the number of the path in the frame
 @param path array of integers with the information of the links of the path to add
 @param len_path number of links in the path to add
 @return 0 if everything correct, -1 otherwise
 */
int add_frame_path(int frame_id, int path_id, int *path, int len_path);

/**
 Add the number of splits to the frame
 
 @param frame_id frame index
 @param num_splits integer with the number of splits that the frame will have
 @return 0 if correct, -1 otherwise
 */
int add_num_splits(int frame_id, int num_splits);

/**
 Add a new split to the given frame identifier
 
 @param frame_id identifier of the frame
 @param split_id identifier of the number of the split in the frame
 @param split array of integers with the information of the links of the split to add
 @param len_split number of links in the split to add
 @return 0 if everything correct, -1 otherwise
 */
int add_frame_split(int frame_id, int split_id, int *split, int len_split);

/**
 Init all the needed variables in the network to start the scheduling, such as frame appearances, instances and similar
 */
void initialize_network(void);

/**
 Check if the schedule stored is correct and satisfies all the constraints

 @return 0 if correct, -1 otherwise
 */
int check_schedule_correctness(void);
