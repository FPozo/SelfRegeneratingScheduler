/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link.c                                                                                                             *
 *  Deterministic Ethernet Scheduler                                                                                   *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/08/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in Link.h                                                                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "Link.h"


int init_link(Link *link_pt) {
    
    link_pt->speed = -1;
    link_pt->type = wired;
    return 1;
}

int set_link(Link *link_pt, int speed, LinkType type) {
    
    if (link_pt == NULL) {
        return 0;
    }
    
    link_pt->speed = speed;
    link_pt->type = type;
    return 1;
}

int get_link_speed(Link *link_pt) {
    
    if (link_pt == NULL) {
        return 0;
    }
    
    return link_pt->speed;
}

LinkType get_link_type(Link *link_pt) {
    
    if (link_pt == NULL) {
        return 0;
    }
    return link_pt->type;
}
