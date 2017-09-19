/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  IOInterface.c                                                                                                      *
 *  Self-Regenerating Scheduler                                                                                        *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 15/09/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Description in IOInterface.h                                                                                       *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "IOInterface.h"
#include "Network.h"
#include <string.h>

                                                /* VARIABLES */

                                            /* AUXILIAR FUNCTIONS */

/**
 Read the general information of the network and add its into the Network variables

 @param file XML network document file name
 @return 1 if all the information was found and could be saved, 0 otherwise
 */
int read_network_parameters(xmlDoc *file) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context;
    xmlXPathObjectPtr result;
    
    // Search the number of frames in the network and save it
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/GeneralInformation/NumberFrames", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, NumberFrames\n");
        return -1;
    }
    value = xmlNodeListGetString(file, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_number_frames(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    
    // Search the number of links in the network and save it
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/GeneralInformation/NumberLinks", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no NumberLinks\n");
        return -1;
    }
    value = xmlNodeListGetString(file, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_number_links(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    
    // Search the minimum time in switch in the network and save it
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/GeneralInformation/MinimumTimeSwitch", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no MinimumTimeSwitch\n");
        return -1;
    }
    value = xmlNodeListGetString(file, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_hop_delay(atoi((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    
    // Search the hyperperiod in the network and save it
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/GeneralInformation/HyperPeriod", context);
    if (result->nodesetval->nodeTab == NULL) {
        printf("The Network xml file is wrongly constructed, no HyperPeriod\n");
        return -1;
    }
    value = xmlNodeListGetString(file, result->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
    set_hyper_period(atoll((const char*) value));
    // Free xml objects
    xmlFree(value);
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    
    return 0;
}

/**
 Read the information of all the links in the network file and saves it into the network links array
 
 @param file XML network document file name
 @return 0 if read correctly, -1 otherwise
 */
int read_links(xmlDoc *file) {

    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_link = NULL;
    xmlXPathObjectPtr result, result_link = NULL;
    
    // Init variables to save the value of the link found
    int speed;
    LinkType link_type;
    
    // Seach on the xml tree where the links are stored
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/NetworkDescription/Links/Link", context);
    
    // Iterate over all links on the network file
    for (int i = 0; i < result->nodesetval->nodeNr; i++) {
        context_link = xmlXPathNewContext(file);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[i], context_link);   // Set the context to the actual link
        
        // Search the category of the current link
        value = xmlGetProp(result->nodesetval->nodeTab[i], (xmlChar*) "category");
        if (xmlStrcmp(value, (xmlChar*) "Wired") == 0) {
            link_type = wired;
        } else if (xmlStrcmp(value, (xmlChar*) "Wireless") == 0) {
            link_type = wireless;
        } else {
            printf("The link has a unknown category\n");
            return -1;
        }
        xmlFree(value);
        
        // Search the speed of the current link
        result_link = xmlXPathEvalExpression((xmlChar*) "Speed", context_link);
        value = xmlNodeListGetString(file, result_link->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        speed = atoi((const char*) value);
        
        // Free xml objects
        xmlFree(value);
        xmlXPathFreeObject(result_link);
        xmlXPathFreeContext(context_link);
        
        // Save the current link information into the network
        add_link(i, speed, link_type);
        
    }
    // Free xml objects
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    
    return 0;
}

/**
 Read the paths of a given frame, and store it into the paths linked lists of the frame

 @param frame_id index of the frame
 @param file network xml file
 @param context_frame xml context of the frame where the paths are being read
 */
void read_paths(int frame_id, xmlDoc *file, xmlXPathContextPtr context_frame) {
    
    // Init xml variabels needed to search information in the file
    xmlChar *value;
    xmlXPathObjectPtr result_frame;
    
    // Auxiliar variables to convert the path string to array of integers
    char *link_char;
    int link_char_it;
    int *path_array;
    
    // Search on the frame tree all paths
    result_frame = xmlXPathEvalExpression((xmlChar*) "Paths/Path", context_frame);
    
    // Init the linked lists of the paths for the current frame
    add_num_paths(frame_id, result_frame->nodesetval->nodeNr);
    
    // Iterate over all paths of the frame
    for (int i = 0; i < result_frame->nodesetval->nodeNr; i++) {
        
        value = xmlNodeListGetString(file, result_frame->nodesetval->nodeTab[i]->xmlChildrenNode, 1);
        
        // Parse the string into an array and save it into the frame
        link_char = strtok((char*) value, ";");
        link_char_it = 0;
        path_array = malloc(sizeof(int));
        while (link_char != NULL) {
            path_array[link_char_it] = atoi(link_char);
            link_char = strtok(NULL, ";");
            link_char_it++;
            path_array = realloc(path_array, sizeof(int) * (link_char_it + 1));     // Allocate memory for next link
        }
        add_frame_path(frame_id, i, path_array, link_char_it);          // Add the found path to the frame
        
        xmlFree(value);
    }
    
    // Free xml structures
    xmlXPathFreeObject(result_frame);
    
}

/**
 Read the splits of a given frame, and store it into the paths linked lists of the frame
 
 @param frame_id index of the frame
 @param file network xml file
 @param context_frame xml context of the frame where the paths are being read
 */
void read_splits(int frame_id, xmlDoc *file, xmlXPathContextPtr context_frame) {
    
    // Init xml variabels needed to search information in the file
    xmlChar *value;
    xmlXPathObjectPtr result_frame;
    
    // Auxiliar variables to convert the path string to array of integers
    char *link_char;
    int link_char_it;
    int *split_array;
    
    // Search on the frame tree all paths
    result_frame = xmlXPathEvalExpression((xmlChar*) "Splits/Split", context_frame);
    
    // Init the linked lists of the paths for the current frame
    add_num_splits(frame_id, result_frame->nodesetval->nodeNr);
    
    // Iterate over all paths of the frame
    for (int i = 0; i < result_frame->nodesetval->nodeNr; i++) {
        
        value = xmlNodeListGetString(file, result_frame->nodesetval->nodeTab[i]->xmlChildrenNode, 1);
        
        // Parse the string into an array and save it into the frame
        link_char = strtok((char*) value, ";");
        link_char_it = 0;
        split_array = malloc(sizeof(int));
        while (link_char != NULL) {
            split_array[link_char_it] = atoi(link_char);
            link_char = strtok(NULL, ";");
            link_char_it++;
            split_array = realloc(split_array, sizeof(int) * (link_char_it + 1));   // Allocate memory for next link
        }
        add_frame_split(frame_id, i, split_array, link_char_it);                    // Add the found path to the frame
        
        xmlFree(value);
    }
    
    // Free xml structures
    xmlXPathFreeObject(result_frame);
    
}

/**
 Read the frames in the network xml file and save it into the network array of frames

 @param file name of the network xml file
 @return 0 if everything correct, -1 otherwise
 */
int read_frames(xmlDoc *file) {
    
    // Init xml variables needed to search information in the file
    xmlChar *value;
    xmlXPathContextPtr context, context_frame = NULL;
    xmlXPathObjectPtr result, result_frame = NULL;
    
    // Init variables for the general information of a frame
    long long int period, deadline;
    int size;
    
    // Search on the xml tree where the frames are stored
    context = xmlXPathNewContext(file);
    result = xmlXPathEvalExpression((xmlChar*) "/Network/TrafficInformation/Frames/Frame", context);
    
    // Iterate over all frames on the network file
    for (int i = 0; i < result->nodesetval->nodeNr; i++) {
        
        // Set the context to the actual frame
        context_frame = xmlXPathNewContext(file);
        xmlXPathSetContextNode(result->nodesetval->nodeTab[i], context_frame);
        
        // Search the period of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Period", context_frame);
        value = xmlNodeListGetString(file, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        period = atoll((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the deadline of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Deadline", context_frame);
        value = xmlNodeListGetString(file, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        deadline = atoll((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Search the size of the current frame
        result_frame = xmlXPathEvalExpression((xmlChar*) "Size", context_frame);
        value = xmlNodeListGetString(file, result_frame->nodesetval->nodeTab[0]->xmlChildrenNode, 1);
        size = atoi((const char*) value);
        // Free xml structures
        xmlFree(value);
        xmlXPathFreeObject(result_frame);
        
        // Add the information to the frame
        add_frame_information(i, period, deadline, size);
        
        // Search and store all the paths and splits (Important to call after adding information due to dependencies)
        read_paths(i, file, context_frame);
        read_splits(i, file, context_frame);
        
    }
    
    // Free xml structures
    xmlXPathFreeObject(result);
    xmlXPathFreeContext(context);
    xmlXPathFreeContext(context_frame);
        
    return 0;
}

                                                /* FUNCTIONS */

/**
 Reads the given network xml file and parse everything into the network variables.
 It starts reading the general information of the network.
 Then continues with the important information from the network components description (links and its speeds).
 It ends with the information of each frame
 */
int parse_network_xml(char *namefile) {
    
    xmlDoc *file_network;       // Variable where all the xml document will be saved
    
    // Open the xml file if it exists;
    file_network = xmlReadFile(namefile, NULL, 0);
    if (file_network == NULL) {
        fprintf(stderr, "The xml information of the network file does not exist\n");
        return -1;
    }
    
    // Parse everything and save it into internal memory
    if (read_network_parameters(file_network) == -1) {
        xmlFreeDoc(file_network);
        xmlCleanupParser();
        return -1;
    }
    if (read_links(file_network) == -1) {
        xmlFreeDoc(file_network);
        xmlCleanupParser();
        return -1;
    }
    if (read_frames(file_network) == -1) {
        xmlFreeDoc(file_network);
        xmlCleanupParser();
        return -1;
    }
    
    // Cleanup of the xml library
    xmlFreeDoc(file_network);
    xmlCleanupParser();
    return 0;
}
