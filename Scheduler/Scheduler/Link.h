/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link.h                                                                                                             *
 *  Deterministic Ethernet Scheduler                                                                                   *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the information of a single link in the network.                                             *
 *  A link containts the information of the speed of the link and the type (wired or wireless).                        *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef Link_h
#define Link_h

#include <stdio.h>

#endif /* Link_h */

                                                /* STRUCT DEFINITIONS */

/**
 Enumeration of different types of links
 */
typedef enum LinkType {
    wired,
    wireless
}LinkType;

/**
 Link of a network with its type and speed
 */
typedef struct Link {
    LinkType type;                      // Type of the link
    int speed;                          // Speed in MB/s of the link
}Link;

                                                /* CODE DEFINITIONS */


/**
 Saves the space of the link with a malloc

 @param link_pt pointer to the link
 @return 1 if successful, 0 otherwise
 */
int init_link(Link *link_pt);


/**
 Sets the speed and the type of the link

 @param link_pt pointer to the link to change
 @param speed desired speed
 @param type desired link type (wired or wireless)
 @return 1 if successful, 0 otherwise
 */
int set_link(Link *link_pt, int speed, LinkType type);


/**
 Gets the speed of the link

 @param link_pt pointer to the link to change
 @return the speed of the given link, 0 otherwise
 */
int get_link_speed(Link *link_pt);


/**
 Gets the type of the link (wired or wireless)

 @param link_pt pointer to the link to change
 @return the link type, 0 otherwise
 */
LinkType get_link_type(Link *link_pt);
