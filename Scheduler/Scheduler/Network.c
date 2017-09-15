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

                                                    /* VARIABLES */


int num_frames;                 // Number of frames in the network
int num_links;                  // Number of links in the network
Frame *frames;                  // Array with all the frames in the network
Link *links;                    // Array with all the links in the network
long long int hyperperiod;      // Hyperperiod of the network schedule in ns
long long int hop_delay;        // Time to wait to relay a frame after being received

                                                    /* FUNCTIONS */
