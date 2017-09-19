/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  IOInterface.h                                                                                                      *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Package that contains the input and outpus interface functions for the xml packages                                *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef IOInterface_h
#define IOInterface_h

#include <stdio.h>
#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>
#include <libxml2/libxml/xpath.h>
#include <libxml2/libxml/xmlstring.h>
#include <libxml2/libxml/globals.h>
#include <libxml2/libxml/xmlwriter.h>

#endif /* IOInterface_h */

                                                /* STRUCT DEFINITIONS */

                                                /* CODE DEFINITIONS */

/**
 Reads the information of the network in the xml file and parses its information into the internal variables
 
 @param namefile name of the xml input file
 @return 0 if correctly read, -1 otherwise
 */
int parse_network_xml(char *namefile);
