#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Evaluator Class                                                                                                    *
 *  Network Evaluator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 09/10/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *  Class with the information of the network and algorithms to evaluate how it will react to different link failures  *
 *  The network and the schedule is read, then a simulation to evaluate how the network actuates to different event is *
 *  done.                                                                                                              *
 *  For now, we can simulate the failure of a link, and how the self-regenerating protocol actuates to restore the     *
 *  correct functioning of the network regenerating the schedule.                                                      *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from NetworkGenerator.Network import Network
from NetworkGenerator.Link import LinkType
from Evaluator.FrameOffsets import FrameOffset, Offset
import xml.etree.ElementTree as Xml
import itertools
import os
import subprocess
import re


class Evaluator:
    """
    Evaluator class with the information of the whole network, including the schedule.
    It is able to simulate the regenerating protocol to create new constraint problems to find schedules that solve
    problems that want to be simulated
    """

    # Variable definitions #

    __network = None                # Information of the network
    __membership_network = None     # Network to simulate the membership process of the protocol
    __node_networks = []            # List with networks for each node of the membership network
    __frames = []                   # List with the information of all frames (including transmission times)
    __frames_membership = []        # List of frames of the membership to relate each frame index to the offset number
    __offsets_membership = []       # List of offsets of the membership
    __offsets_node_networks = []    # List of list of offsets of the node networks
    __frames_node_networks = []     # List of list of frames of the node networks
    __max_num_new_paths = 2         # Maximum number of new paths to select after link is broken
    __nodes_membership = []         # We save a list of the nodes and links of the membership to be able to match
    __links_membership = []         # Each index of the network membership with the original network index
    __nodes_node_networks = []      # Same as node membership, but one for each node (refers to membership not network!)
    __links_node_networks = []      # Same as link membership, but one for each node (refers to membership not network!)
    __link_broken_now = None

    # Private function definitions #

    def __init__(self):
        """
        Initialization of an empty evaluator
        """
        self.__network = Network()
        self.__membership_network = None
        self.__node_networks = []
        self.__frames = []
        self.__frames_membership = []
        self.__offsets_membership = []
        self.__offsets_node_networks = []
        self.__frames_node_networks = []
        self.__max_num_new_paths = 1
        self.__nodes_membership = []
        self.__links_membership = []
        self.__nodes_node_networks = []
        self.__links_node_networks = []
        self.__link_broken_now = None

    def __get_frames_and_offsets_link(self, link):
        """
        Get all the offsets that have the given link in all the frames
        :param link: identifier of the link
        :type link: int
        :return: list of offsets with the link same as the given one
        :rtype: list of Offset
        """
        offsets = []  # List where we will save all the offsets
        frames = []  # List with the indexes of frames of the offsets
        # For all frames
        for index_frame, frame in enumerate(self.__frames):
            offset = frame.get_offset_link(link)
            if offset is not None:  # Add offsets if we get one
                offsets.append(offset)
                frames.append(index_frame)
        return offsets, frames

    def __get_frames_and_offsets_link_membership(self, link):
        """
        Get all the offsets that have the given link in all the frames of the membership network
        :param link: identifier of the link
        :type link: int
        :return: list of offsets with the link same as the given one
        :rtype: list of Offset, list of Frames
        """
        offsets = []
        frames = []
        for index_frame, frame in enumerate(self.__frames_membership):
            offset = self.__offsets_membership[index_frame]
            if offset.get_link() == link:  # Add offsets if the offset has the same link as the given one
                offsets.append(offset)
                frames.append(index_frame)
        return offsets, frames

    @staticmethod
    def __get_unique_links_in_paths(paths):
        """
        Get a list of unique links in a list of paths
        :param paths: list of paths
        :type paths: list of list of int
        :return: list of unique links
        :rtype: list of int
        """
        links_membership = []
        for path in paths:
            for link in path:
                if link not in links_membership:
                    links_membership.append(link)
        return links_membership

    def __get_nodes_from_links(self, links, membership):
        """
        Get all the nodes from a list of links
        :param links: list of links
        :type links: list of int
        :param membership: true if to search in the membership network, false for normal network
        :type membership: Bool
        :return: list of nodes source and destination from the nodes
        :rtype: list of int
        """
        nodes = []
        for link in links:
            if membership:
                source, destination = self.__membership_network.get_link_source_destination(link)
            else:
                source, destination = self.__network.get_link_source_destination(link)
            # Check if the source and destination are already in the list
            if source not in nodes:
                nodes.append(source)
            if destination not in nodes:
                nodes.append(destination)
        return nodes

    def __get_nodes_from_links_membership(self, links):
        """
        Get all the nodes from a list of links
        :param links: list of links
        :type links: list of int
        :return: list of nodes source and destination from the nodes
        :rtype: list of int
        """
        nodes = []
        for link in links:
            source, destination = self.__membership_network.get_link_source_destination(link)
            # Check if the source and destination are already in the list
            if source not in nodes:
                nodes.append(source)
            if destination not in nodes:
                nodes.append(destination)
        return nodes

    def __create_membership_network(self, frames_affected, source, destination):
        """
        Given a disconnection from a source to a destination, create a membership network to simulate the protocol
        membership
        :param frames_affected: list of frames identifiers affected needed to add
        :type frames_affected: list of int
        :param source: source node of the disconnection
        :type source: int
        :param destination: destination node of the disconnection
        :type destination: int
        :return: nothing
        :rtype: None
        """
        paths = self.__network.get_all_simple_paths(source, destination)  # Get all other possible paths
        if len(paths) == 0:  # If no paths were found, we cannot recover
            raise Exception('We cannot recover from this link failure')
        paths.sort(key=len)  # Sort by size of paths
        if len(paths) > self.__max_num_new_paths:  # Select a max num of paths
            paths = paths[:self.__max_num_new_paths]

        # Create the membership network to be able to distribute all frames
        # Start getting all the links of the membership network
        self.__links_membership = self.__get_unique_links_in_paths(paths)
        # Get all the frames and offsets of the membership network
        self.__offsets_membership = []
        self.__frames_membership = []
        for link_index, link_membership in enumerate(self.__links_membership):
            offset, frame = self.__get_frames_and_offsets_link(link_membership)
            # Update the link of the offsets to match the membership network links
            new_off = []
            for _ in offset:
                new_off.append(Offset([], [], 0, link_index))
            self.__offsets_membership.extend(new_off)
            self.__frames_membership.extend(frame)
        # self.__unique_frames_membership = list(set(self.__frames_membership))

        # Create the membership network nodes and links and save the association with the real network ids
        self.__membership_network = Network()
        # We need the time between frames for some calculations, so we move it from network to membership network
        self.__membership_network.set_time_between_frames(self.__network.get_time_between_frames())
        self.__membership_network.set_min_time_switch(self.__network.get_min_time_switch())
        self.__membership_network.set_protocol(*self.__network.get_protocol())

        self.__nodes_membership = self.__get_nodes_from_links(self.__links_membership, False)
        # Add also the source and destination in case it did not exist before
        self.__nodes_membership.append(source)
        self.__nodes_membership.append(destination)
        self.__nodes_membership = list(set(self.__nodes_membership))
        # Creates all the nodes
        for _ in self.__nodes_membership:
            self.__membership_network.add_end_system()  # It does not matter as frames are artificially added
        # Create all the links connections from the links memberships
        for link in self.__links_membership:
            source_link, destination_link = self.__network.get_link_source_destination(link)
            speed, link_type = self.__network.get_link_stats(link)
            self.__membership_network.add_link(self.__nodes_membership.index(source_link),
                                               self.__nodes_membership.index(destination_link), link_type, speed)

        # Add the frames for the already scheduled links in the membership network, and create ranges with starting
        # and deadline to not miss path dependent and end to end frame constraints
        last_frame_index = -1
        for index_frame in self.__frames_membership:
            if index_frame == last_frame_index:
                last_frame_index = index_frame
            else:
                frame = self.__frames[index_frame]

                # Convert the path from the original network to the membership network. We must extract the links that
                # appear also in the membership network, and extract the source and destination of those
                # We have to take into account that a path can go and then return to the membership network links, that
                # is why we have consecutive links. If a path has links in the membership network that are not
                # separated, we consider them as two different paths!
                frame_paths = frame.get_paths()
                membership_paths = []
                for frame_path in frame_paths:          # For all the frame paths
                    membership_paths.append([])
                    consecutive_links = False
                    for path_link in frame_path:        # For all the links in the path
                        if path_link in self.__links_membership:    # If the link is in the membership network
                            if consecutive_links:
                                membership_paths[-1].append(self.__links_membership.index(path_link))
                            else:
                                membership_paths.append([self.__links_membership.index(path_link)])
                                consecutive_links = True
                        else:
                            consecutive_links = False
                membership_paths = list(filter(([]).__ne__, membership_paths))  # Remove empty paths
                # Remove duplicates
                membership_paths.sort()
                membership_paths = list(membership_paths for membership_paths, _ in itertools.groupby(membership_paths))

                if len(membership_paths) > 1:
                    print('We have a problem when calculating the ending time in the membership network!')
                # Extract source and destinations from the obtained paths
                source_node = self.__membership_network.get_link_source_destination(membership_paths[0][0])[0]
                destination_node = []
                for path in membership_paths:
                    # The destination node is the destination of the last link of the path
                    destination_node.append(self.__membership_network.get_link_source_destination(path[-1])[1])

                # starting = (frame.get_predecessor_path_receiving_time(offset.get_link(), 0) +
                #            self.__network.get_min_time_switch())
                # link_stats = self.__network.get_link_stats(offset.get_link())
                # Ending = next path receiving time - time needed to transmit the frame on the link
                # ending = int(frame.get_successor_path_transmission_time(offset.get_link(), 0) -
                #             ((frame.get_size() * 1000) / link_stats[0]))
                link = self.__links_membership[membership_paths[0][0]]
                starting = frame.get_predecessor_path_receiving_time(link, 0) + self.__network.get_min_time_switch()
                link = self.__links_membership[membership_paths[0][-1]]
                ending = int(frame.get_successor_path_transmission_time(link, 0))

                # Add the frame
                # We also do not need starting and ending transmission time as it can be checked from the original
                # network and it is different for every offset in the frame, which makes not possible to add them right
                # now (we only have one starting and ending time for the whole frame, not for every offset)
                self.__membership_network.add_custom_frame(source_node, destination_node, frame.get_period(), ending,
                                                           frame.get_size(), starting, frame.get_end_to_end())
                last_frame_index = index_frame

        # Add the new frames for the affected frames
        # Get the source and destination of the link, and shift it to the membership source and destination
        source = self.__nodes_membership.index(source)
        destination = self.__nodes_membership.index(destination)
        # Add the affected frames to the membership frame
        for index_frame in frames_affected:
            frame = self.__frames[index_frame]
            self.__membership_network.add_custom_frame(source, [destination], frame.get_period(), 0, frame.get_size(),
                                                       0, frame.get_end_to_end())

        self.__membership_network.generate_paths_frames()

        # As the new custom frames do not have offsets in the real network, we have to create fake ones and add
        # them in the offset membership list. Note this is only for the frames created due to the link failure, not
        # the ones that we added because the frames are transmitted in links that happen to be in the membership network
        for index_frame, new_frame in enumerate(frames_affected):
            # Get the path of the new frame (for now it only has one path)
            frame_to_check = self.__membership_network.get_num_frames() - len(frames_affected) + index_frame
            path = self.__membership_network.get_frame(frame_to_check).get_paths()[0]
            for link in path:       # For every link in the path, create the new offset
                self.__offsets_membership.append(Offset([], [], 0, link))   # Me only care about the link
                self.__frames_membership.append(new_frame)             # Register from which frame the offset comes

        # Create a new folder to contain the membership network file
        if not os.path.exists('Membership'):
            os.makedirs('Membership')
        for file in os.listdir('Membership'):
            file_path = os.path.join('Membership', file)
            if os.path.isfile(file_path):
                os.unlink(file_path)
        self.__membership_network.write_network_xml('Membership/membership_network.xml')

    def __create_single_node_networks(self, frames_affected):
        """
        From the membership network, it creates single node networks that can be solved by the scheduler to emulate
        what a node can obtain as schedule in the protocol
        :param frames_affected: list of frames identifiers affected needed to add
        :type frames_affected: list of int
        :return: nothing
        :rtype: None
        """
        # Check how many single node networks do we need (as many as different source nodes in the membership network)
        links = self.__membership_network.get_links_connection()
        source_nodes = []
        for link_index, link in enumerate(links):
            if link[0] not in source_nodes:     # If not in source nodes, we create a new network
                source_nodes.append(link[0])
                self.__links_node_networks.append([link_index])
                self.__node_networks.append(Network())

                # Add variables needed in the node networks
                self.__node_networks[-1].set_time_between_frames(self.__network.get_time_between_frames())
                self.__node_networks[-1].set_min_time_switch(self.__network.get_min_time_switch())
                self.__node_networks[-1].set_protocol(*self.__network.get_protocol())

            else:                               # Else, we add the link to that network
                self.__links_node_networks[source_nodes.index(link[0])].append(link_index)

        # Create a new folder to contain the node networks files
        if not os.path.exists('Node'):
            os.makedirs('Node')
        for file in os.listdir('Node'):
            file_path = os.path.join('Node', file)
            if os.path.isfile(file_path):
                os.unlink(file_path)

        # Get all the frames and offsets of the nodes networks. Recall that the offset comes from THE ORIGINAL NETWORK
        # while the frame just indicates which frame are we referring from THE MEMBERSHIP NETWORK
        for network_index, links_network in enumerate(self.__links_node_networks):    # For every node network
            self.__offsets_node_networks.append([])
            self.__frames_node_networks.append([])
            for link_index, link in enumerate(links_network):                       # For each link in the node network
                offset, frame = self.__get_frames_and_offsets_link_membership(link)
                new_off = []
                for _ in offset:
                    new_off.append(Offset([], [], 0, link_index))
                self.__offsets_node_networks[-1].extend(new_off)
                self.__frames_node_networks[-1].extend(frame)
            self.__nodes_node_networks.append(self.__get_nodes_from_links(links_network, True))

            # Create all the nodes
            for _ in self.__nodes_node_networks[network_index]:
                self.__node_networks[network_index].add_end_system()
            # Create all the links connections from the links memberships
            for link in links_network:
                source_node, destination_node = self.__membership_network.get_link_source_destination(link)
                speed, link_type = self.__membership_network.get_link_stats(link)
                self.__node_networks[network_index].add_link(self.__nodes_node_networks[network_index].index(
                                                                 source_node),
                                                             self.__nodes_node_networks[network_index].index(
                                                                 destination_node), link_type, speed)

            # Add the frames in the node network, and create ranges with starting and deadline to not miss path
            # dependent and end to frame constraints.
            for index_offset, offset in enumerate(self.__offsets_node_networks[network_index]):
                frame_network = self.__frames_membership[self.__frames_node_networks[network_index][index_offset]]
                frame = self.__frames[frame_network]

                offset_link = offset.get_link()
                # link = self.__links_node_networks[network_index].index(self.__links_membership.index(offset_link))
                source_node, \
                    destination_node = self.__node_networks[network_index].get_link_source_destination(offset_link)

                link = self.__links_membership[self.__links_node_networks[network_index][offset_link]]
                # If the frame in new (in the frame affected list) we have to calculate the starting and ending time
                # differently, splitting the time in the path equally for every link
                if frame_network in frames_affected:
                    pos_path = 0
                    len_path = 0
                    indices = [i for i, x in enumerate(self.__frames_membership) if x == frame_network]
                    for pos_index, index in enumerate(indices):
                        off_link = self.__offsets_membership[index].get_link()
                        link_node_network = self.__offsets_node_networks[network_index][index_offset].get_link()
                        link_membership = self.__links_node_networks[network_index][link_node_network]
                        if off_link == link_membership:
                            # Get the position and length and exit the loop
                            pos_path = pos_index
                            len_path = len(indices)
                            break

                    starting = frame.get_predecessor_path_receiving_time(self.__link_broken_now, 0) + \
                        self.__network.get_min_time_switch()
                    # ending = int(frame.get_successor_path_transmission_time(self.__link_broken_now, 0) -
                    #             ((frame.get_size() * 1000) / link_stats[0]))
                    ending = int(frame.get_successor_path_transmission_time(self.__link_broken_now, 0))
                    # Calculate the avaiable space
                    avaiable_space = ending - starting
                    # Be sure we take into account the end to end time constraint too!
                    if avaiable_space > frame.get_end_to_end():
                        first_link_path = frame.get_paths()[0][0]
                        avaiable_space = frame.get_end_to_end() - \
                            (frame.get_predecessor_path_receiving_time(self.__link_broken_now, 0) -
                             frame.get_predecessor_path_receiving_time(first_link_path, 0))

                    # Re-calculate the starting and ending transmission time with the avaiable space
                    starting += int((pos_path / len_path) * avaiable_space)
                    ending -= int(((len_path - (pos_path + 1)) / len_path) * avaiable_space)
                else:
                    starting = frame.get_predecessor_path_receiving_time(link, 0) + self.__network.get_min_time_switch()
                    # ending = int(frame.get_successor_path_transmission_time(link, 0) - ((frame.get_size() * 1000) /
                    #                                                                    link_stats[0]))
                    ending = int(frame.get_successor_path_transmission_time(link, 0))

                self.__node_networks[network_index].add_custom_frame(source_node, [destination_node],
                                                                     frame.get_period(), ending, frame.get_size(),
                                                                     starting, frame.get_end_to_end())

            self.__node_networks[network_index].generate_paths_frames()
            self.__node_networks[network_index].write_network_xml('Node/node_network' + str(network_index) + '.xml')

    def __schedule_single_node_networks(self):
        """
        Calls the scheduler to solve all the node networks created and stores the resulting schedule
        :return: nothing
        :rtype: None
        """
        # Create the folder to store the obtained schedules
        if not os.path.exists('Node/Schedules'):
            os.makedirs('Node/Schedules')
        for file in os.listdir('Node/Schedules'):
            file_path = os.path.join('Node/Schedules', file)
            if os.path.isfile(file_path):
                os.unlink(file_path)

        # For all the node networks created, solve them
        for file in os.listdir('Node'):
            # Find the id of the node network
            try:
                i = re.findall(r'\d+', file)
                i = int(i[0])
            except IndexError:
                pass
            network_path = os.path.join('Node', file)
            if network_path.split('.')[-1] == 'xml':    # Only schedule xml files (networks)
                schedule_path = 'Node/Schedules/node_schedule' + str(i) + '.xml'
                subprocess.call(['./Scheduler', network_path, schedule_path])
                if not os.path.isfile(schedule_path):
                    time_between_frames = self.__node_networks[i].get_time_between_frames()
                    min_time_switch = self.__node_networks[i].get_min_time_switch()
                    now_between = True
                    # Quick stupid search for good values to find the schedule
                    while not os.path.isfile(schedule_path):
                        if now_between:
                            time_between_frames = int(time_between_frames / 2)
                            self.__node_networks[i].set_time_between_frames(time_between_frames)
                            self.__node_networks[i].write_network_xml('Node/node_network' + str(i) + '.xml')
                        else:
                            min_time_switch = int(min_time_switch / 2)
                            self.__node_networks[i].set_min_time_switch(min_time_switch)
                            self.__node_networks[i].write_network_xml('Node/node_network' + str(i) + '.xml')
                        subprocess.call(['./Scheduler', network_path, schedule_path])
                        if time_between_frames == 0 and min_time_switch == 0 and not os.path.isfile(schedule_path):
                            raise Exception('Damn, Alpha Charlie Tango, we cannot recover that failure')

    def activate_protocol(self, link, time):
        """
        Activates the protocol to recover from a permanent link failure
        :param link: link identifier
        :type link: int
        :param time: time when the permanent failure occurs in nanoseconds
        :type time: int
        :return: time when it recovers from the failure
        :rtype: int
        """
        # Init values from old protocol activations
        self.__membership_network = None
        self.__node_networks = []
        self.__frames_membership = []
        self.__offsets_membership = []
        self.__offsets_node_networks = []
        self.__frames_node_networks = []
        self.__nodes_membership = []
        self.__links_membership = []
        self.__nodes_node_networks = []
        self.__links_node_networks = []
        self.__link_broken_now = None

        # Get all the frames that will be affected by the link failure
        offsets_affected, frames_affected = self.__get_frames_and_offsets_link(link)

        # Remove the affected link from the network and find all the simple paths to connect the now unconnected nodes
        source, destination = self.__network.get_link_source_destination(link)
        self.__network.remove_link(link)
        self.__link_broken_now = link

        # Create the membership network, first creating the frames that were not affected but are in the membership,
        # then add the frames affected
        self.__create_membership_network(frames_affected, source, destination)

        # Create the node networks for every source node in the membership network
        self.__create_single_node_networks(frames_affected)

        self.__schedule_single_node_networks()

    def __read_general_network_information(self, name):
        """
        Read general information about the network and save it
        :param name: path and name of the network xml
        :type name: str
        :return: nothing
        :rtype: None
        """
        # Open the file if exists
        try:
            tree = Xml.parse(name)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read general information variables that are needed for the evaluator
        self.__network.set_time_between_frames(int(root.find('GeneralInformation/TimeBetweenFrames').text))
        self.__network.set_protocol(int(root.find('GeneralInformation/PeriodProtocol').text),
                                    int(root.find('GeneralInformation/TimeProtocol').text))
        self.__network.set_min_time_switch(int(root.find('GeneralInformation/MinimumTimeSwitch').text))

    def __read_nodes(self, name):
        """
        Read the nodes of the xml file and save them into the network object
        :param name: path and name of the network xml
        :type name: str
        :return: nothing
        :rtype: None
        """
        # Open the file if exists
        try:
            tree = Xml.parse(name)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Find all the nodes
        nodes_xml = root.findall('NetworkDescription/Nodes/Node')

        # For all nodes, read which category they are and save them in the network
        for node_xml in nodes_xml:
            if node_xml.attrib['category'] == 'Switch':
                self.__network.add_switch()
            else:
                self.__network.add_end_system()

    def __read_links(self, name):
        """
        Read the links of the xml file and save them into the network object
        :param name: path and name of the network xml
        :type name: str
        :return: nothing
        :rtype: None
        """
        # Open the file if exists
        try:
            tree = Xml.parse(name)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Find all the links
        links_xml = root.findall('NetworkDescription/Links/Link')

        # For all links, read the information and save them in the network
        for link_xml in links_xml:
            if link_xml.attrib['category'] == 'Wired':
                link_type = LinkType.wired
            else:
                link_type = LinkType.wireless
            speed = int(link_xml.find('Speed').text)
            source = int(link_xml.find('Source').text)
            destination = int(link_xml.find('Destination').text)
            self.__network.add_link(source, destination, link_type, speed)

    def __read_frames(self, name):
        """
        Read the frames of the xml file and save them
        :param name: path and name of the schedule xml
        :type name: str
        :return: nothing
        :rtype: None
        """
        # Open the file if exists
        try:
            tree = Xml.parse(name)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Find all the frames
        frames_xml = root.findall('FramesTransmission/Frame')

        # For all frames, read its values and save them into the frame list
        for frame_xml in frames_xml:
            period = int(frame_xml.find('Period').text)
            starting = int(frame_xml.find('Starting').text)
            deadline = int(frame_xml.find('Deadline').text)
            size = int(frame_xml.find('Size').text)
            end_to_end = int(frame_xml.find('EndToEnd').text)
            self.__frames.append(FrameOffset(0, [0], period, deadline, size, starting, end_to_end))
            self.__frames[-1].clean_path()  # I do it because the list acts weird

            # Read and iterate over all the paths
            for path_xml in frame_xml.findall('Path'):
                path = []
                # Read and iterate over all the links in the path
                for link_xml in path_xml.findall('Link'):

                    # Take information of the link path
                    link = int(link_xml.find('LinkID').text)
                    path.append(link)
                    instances_xml = link_xml.findall('Instance')
                    num_instances = len(instances_xml)
                    offset_starting = []
                    offset_ending = []
                    # For all the instances, read and save its values
                    for instance_xml in instances_xml:
                        offset_starting.append(int(instance_xml.find('TransmissionTime').text))
                        offset_ending.append(int(instance_xml.find('EndingTime').text))
                    # Once all is saved, add the offset of the link
                    self.__frames[-1].add_offset(offset_starting, offset_ending, num_instances, link)

                # Add also information about the path, needed to define ranges in the "fake" network
                self.__frames[-1].set_new_path(path)

    def read_network_xml(self, name_network, name_schedule):
        """
        Read a network xml file and fills the network object
        :param name_network: path and name of the network xml
        :type name_network: str
        :param name_schedule: path and name of the schedule xml
        :type name_schedule: str
        :return: nothing
        :rtype: None
        """
        self.__read_general_network_information(name_network)  # Read general network information
        self.__read_nodes(name_network)  # Read all the nodes
        self.__read_links(name_network)  # Read all the links
        self.__read_frames(name_schedule)  # Read all the frames
