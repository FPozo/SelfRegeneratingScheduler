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

                                                    /* VARIABLES */

                                                    /* FUNCTIONS */

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
    return 1;
}
