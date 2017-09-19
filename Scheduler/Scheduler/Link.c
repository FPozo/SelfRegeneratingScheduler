/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link.c                                                                                                             *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/08/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Link.h                                                                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Link.h"

                                                    /* VARIABLES */

                                                /* AUXILIAR FUNCTIONS */

                                                    /* FUNCTIONS */

/**
 Saves the space of the link for when a malloc is needed
 */
int init_link(Link *link_pt) {
    
    link_pt->speed = -1;
    link_pt->type = wired;
    return 0;
}


/**
 Sets the speed and the type of the link
 */
int set_link(Link *link_pt, int speed, LinkType type) {
    
    if (link_pt == NULL) {
        return -1;
    }
    
    link_pt->speed = speed;
    link_pt->type = type;
    return 0;
}

/**
 Gets the speed of the link
 */
int get_link_speed(Link *link_pt) {
    
    if (link_pt == NULL) {
        return -1;
    }
    
    return link_pt->speed;
}

/**
 Gets the type of the link (wired or wireless), 0 if the pointer given was not correct
 */
LinkType get_link_type(Link *link_pt) {
    
    if (link_pt == NULL) {
        return 0;
    }
    return link_pt->type;
}
