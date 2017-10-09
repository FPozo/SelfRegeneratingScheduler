"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Network Class                                                                                                      *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 01/10/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *  Class with the information of the network and algorithms to create them                                            *
 *  Networks are generated with a description language capable to describe any network that has no cycles.             *
 *  Different number of frames and types of frames frames (single, broadcast, etc) and dependencies and attributes of  *
 *  the network are also created.                                                                                      *
 *  As the number of parameters is large, standard configuration of parameters are also available.                     *
 *  It includes input and output for the xml files.                                                                    *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

import networkx as nx
import xml.etree.ElementTree as Xml
from xml.dom import minidom
from functools import reduce
from random import random, choice, shuffle, randint
from NetworkGenerator.Node import *
from NetworkGenerator.Link import *
from NetworkGenerator.Frame import *


# Auxiliary functions

def gcd(a, b):
    """Return greatest common divisor using Euclid's Algorithm."""
    while b:
        a, b = b, a % b
    return a


def lcm(a, b):
    """Return lowest common multiple."""
    return a * b // gcd(a, b)


def lcm_multiple(*args):
    """Return lcm of args."""
    return reduce(lcm, args)


class Network:
    """
    Network class with the information of the whole network, frames and dependencies on it and algorithms to construct
    them
    """

    # Variable definitions #

    __graph = None                  # Network Graph built with the NetworkX package
    __switches = []                 # List with all the switches identifiers in the network
    __end_systems = []              # List with all the end systems identifiers in the network
    __links = []                    # List with all the links identifiers in the network
    __links_objects = []            # List with all the links objects in the network (cannot be saved in the graph)
    __paths = []                    # Matrix with the number of end systems as index for x and y, it contains a list of
    # links to describe the path from end system x to end system y, None if x = y
    __frames = []                   # List with all the frames objects in the network
    __min_time_switch = None        # Minimum time for a frame to stay in the switch
    __period_protocol = None        # Period of the protocol to reserve bandwidth
    __time_protocol = None          # Time of the protocol to reserve bandwidth
    __time_between_frames = None    # Time the frames have to wait between transmissions
    __topology_description = []     # String with the information of the topology
    __link_description = []         # String with the information of the links
    __cyclic_description = []       # Matrix with the description of all the connections of all the nodes
    __nodes_description = []        # List with the types of nodes
    __num_frames = None             # Number of frames in the network
    __per_single_frames = None      # Percentage of frames that are sent to only one end system
    __per_local_frames = None       # Percentage of frames that are sent to end systems that have path length 2
    __per_multiple_frames = None    # Percentage of frames that are sent to multiple end systems, but not all
    __per_broadcast_frames = None   # Percentage of frames that are broadcast
    __periods = []                  # List with all the possible periods of the frames in ns
    __deadlines = []                # List with all the possible deadlines of the frames in ns
    __sizes = []                    # List with all the possible sizes of the frames in bytes
    __end_to_ends = []              # List with all the possible end to end constraints of the frames in ns
    __frames_percentages = []       # List with the percentages of frames that have the configurations based on the
    # index written in here, example, [0] = 0.1 implies 10% of the frames have periods = periods[0] and so on
    __hyper_period = None           # Hyper_period of the network
    __frame_instances = None        # Number of frame instances in the network (variables with transmission times)
    __utilization = None            # Utilization of the network
    __link_utilization = []         # Utilization of every link in the network
    __max_link_utilization = None   # Utilization of the link with maximum utilization

    # Standard function definitions #

    def __init__(self):
        """
        Initialization of an empty network
        """
        self.__graph = nx.Graph()
        self.__switches = []
        self.__end_systems = []
        self.__links = []
        self.__links_objects = []
        self.__paths = []
        self.__frames = []

    def create_network(self):
        """
        Create the network after reading the configuration file
        :return: nothing
        :rtype: None
        """
        # Cheeky trick to check if the network has been parsed before
        if self.__num_frames is None:
            raise Exception("No network has been parsed")

        self.__hyper_period = self.__calculate_hyper_period()

        # Start creating all the elements of the network
        self.__create_topology()
        # self.__generate_paths()   We updated this function
        self.__generate_frames()
        self.__add_frame_params()
        self.__generate_paths_frames()

        # Calculate some of the important parameters to know about the network
        # self.__utilization, self.__max_link_utilization, schedulable = self.__calculate_utilization()
        # Updated to do it in the frames generation

    # Private function definitions #

    def __add_switch(self):
        """
        Add a new switch into the network
        :return: None
        :rtype None
        """
        # Add into the Networkx graph a new node with type => object.switch node, id => switch number
        self.__graph.add_node(self.__graph.number_of_nodes(), type=Node(NodeType.switch), id=len(self.__switches))
        self.__switches.append(self.__graph.number_of_nodes() - 1)  # Save the identifier of Networkx

    def __add_end_system(self):
        """
        Add a new end system into the network
        :return: nothing
        :rtype: None
        """
        # Add into the Networkx graph a new node with type => object.end_system node, id => end system number
        self.__graph.add_node(self.__graph.number_of_nodes(), type=Node(NodeType.end_system), id=len(self.__switches))
        self.__end_systems.append(self.__graph.number_of_nodes() - 1)  # Save the identifier of Networkx

    def __add_link(self, source, destination, link_type, speed):
        """
        Adds a bi-directional link (two logical links) between a source node and a destination node
        :param source: node source
        :type source: int
        :param destination: node destination
        :type destination: int
        :param link_type: type of link (wired/wireless)
        :type link_type: LinkType
        :param speed: link speed
        :type speed: int
        :return: None
        :rtype: None
        """
        # Add into the Networkx graph a new link between two node with type => object.link, id => link number
        self.__graph.add_edge(source, destination, type=Link(speed=speed, link_type=link_type),
                              id=self.__graph.number_of_edges() - 1)
        self.__links.append([source, destination])  # Saves the same info in our link list with nodes
        # self.__links.append([destination, source])
        self.__links_objects.append(Link(speed=speed, link_type=link_type))  # Saves the object with same index
        # self.__links_objects.append(Link(speed=speed, link_type=link_type))

    def __add_link_information(self, links, num_links, branch, parent_node):
        """
        Reads the link information and check if is needed to add it to the collision domain, then creates the link
        :param links: links description
        :type links: list of str
        :param num_links: index of the number of links in the execution
        :type num_links: int
        :param branch: branch to add the link
        :type branch: int
        :param parent_node: parent node to link to
        :type parent_node: int
        :return: nothing
        :rtype: None
        """
        # Add the link type wired if is 'w' or wireless if is 'x'
        # IMPORTANT: Note that we take the link id from num_links (as they are not sorted in the
        # links array!)
        link_type = LinkType.wired if links[num_links + branch][0] == 'w' else LinkType.wireless
        speed = int(links[num_links + branch][1:])  # The speed is the rest of the string

        # Add the link with all the information
        self.__add_link(parent_node, self.__graph.number_of_nodes() - 1, link_type, speed)

    def __change_switch_to_end_system(self, switch):
        """
        Change an already introduced switch to an end system (needed for the create network function)
        :param switch: id of the switch
        :type switch: int
        :return: None
        :rtype: None
        """
        self.__graph.node[switch]['type'] = Node(NodeType.end_system)  # Update the information into the graph
        self.__graph.node[switch]['id'] = len(self.__end_systems)
        self.__end_systems.append(switch)  # Update the information into our lists
        self.__switches.remove(switch)

    def __create_cyclic_topology(self):
        """
        Creates a network with the new connection and nodes descriptions
        :return: nothing
        :rtype: None
        """
        # Start creating all nodes
        for node_type in self.__nodes_description:
            if node_type == 'switch':
                self.__add_switch()
            else:
                self.__add_end_system()

        # Start creating all connections
        for node_id, node in enumerate(self.__cyclic_description):
            for connection in node:
                link_type = LinkType.wired if connection[1][0] == 'w' else LinkType.wireless
                speed = int(connection[1][1:])  # The speed is the rest of the string
                self.__add_link(node_id, connection[0], link_type, speed)

    def __recursive_create_network(self, description, links, parent_node, num_calls, num_links):
        """
        Auxiliary recursive function for create network
        :param description: description of the network already parsed into integers
        :type description: list of int
        :param links: list with description of the links parameters in tuples
        :type links: list of str
        :param parent_node: number of the parent node of the actual call
        :type parent_node: int
        :param num_calls: number of calls that has been done to the function (to iterate the description string)
        :type num_calls: int
        :param num_links: number of links that has been done to the function (to map the links in the correct order as
        they are not correctly ordered in the links array for the perspective of the recursive function)
        :type num_links: int
        :return: the updated number of calls and the number of links to track correctly during the recursion
        :rtype: int, int
        """
        try:  # Try to catch wrongly formulated network descriptions

            # Create new leads as end systems and link them to the parent node
            if description[num_calls] < 0:
                # For all the new leafs, add the end system to the network and link it to the parent node
                for leaf in range(abs(description[num_calls])):
                    self.__add_end_system()
                    self.__add_link_information(links, num_links, leaf, parent_node)
                # Return subtracting the last links created by the last branch from the number of links, we want the
                # number of links when the branch is starting, no after (due to recursion)
                return num_links - int(description[num_calls]), num_calls

            # Finished branch, change switch parent node into an end system
            elif description[num_calls] == 0:
                self.__change_switch_to_end_system(parent_node)
                return num_links, num_calls  # Return as the branch is finished

            # Create new branches with switches
            elif description[num_calls] > 0:
                it_links = num_links  # Save the starting number of links to iterate
                last_call_link = 0  # Create variable to save the last link that was call (to return after)

                for branch in range(description[num_calls]):  # For all new branches, create the switch and link it
                    self.__add_switch()
                    new_parent = self.__graph.number_of_nodes() - 1  # Save the new parent node for later
                    # Read all the information of the link and add it
                    self.__add_link_information(links, num_links, branch, parent_node)

                    # Check which link is calling, if last call is bigger, set it to last call
                    if last_call_link > it_links + (int(description[num_calls]) - branch):
                        links_to_call = last_call_link
                    else:
                        links_to_call = it_links + (int(description[num_calls] - branch))
                    # Call the recursive for the new branch, we save the last call link to recover it when we return
                    # after the branch created by this recursive call is finished
                    last_call_link, num_calls = self.__recursive_create_network(description, links, new_parent,
                                                                                num_calls + 1, links_to_call)

                return last_call_link, num_calls  # Return when all branches have been created and closed

        except IndexError:
            raise ValueError("The network description is wrongly formulated, there are open branches")

    def __create_topology(self):
        """
        Creates a network with the network and link description
        The topology description string with the data of the network, it follows an special description
        The link description string with the description of all the links, if None, all are wired and 100 MBs
        There are numbers divided by semicolons, every number indicates the number of children for the actual switch
        If the number is negative, it means it has x end systems: ex: -5 means 5 end systems
        If the number is 0, it means the actual switch is actually an end_system
        The order of the descriptions goes with depth. If a node has no more children you backtrack to describe the next
        node.
        Every link has a description of its speed and its type (wired/wireless), first letter is the type and number
        w100 => wired with 100 MBs
        x10 => wireless with 10 MBs
        :return: nothing
        :rtype: None
        """
        # If the description is with the new cyclic, call that function
        if self.__topology_description is None:
            self.__create_cyclic_topology()
            return

        # Prepare the topology description in an array of integers
        description_array = self.__topology_description.split(';')
        description = [int(numeric_string) for numeric_string in description_array]

        # Separate also the link description if exist
        links = self.__link_description.split(';')

        # Start the recursive call with parent switch 0
        self.__add_switch()
        # Num links and num calls are auxiliary variables to map the order in which the links are created and to check
        # if the creation of the network was successful
        num_links, num_calls = self.__recursive_create_network(description, links, 0, 0, 0)

        # Check if there are additional elements that should not be in the network
        if num_calls != len(description) - 1:
            raise ValueError("The network description is wrongly formulated, there are extra elements")

    def __generate_paths_frames(self):
        """
        Generates the paths for all frames, balancing the utilization as best as we can (we only seek one, for every
        node, which of all the possible options has least utilization in all the simplest path)
        :return:
        :rtype:
        """
        # Prepare the utilization variables to be used
        self.__utilization = 0
        self.__max_link_utilization = 0
        self.__link_utilization = [0 for _ in range(len(self.__links))]
        self.__frame_instances = 0

        # Add utilization of the protocol
        if self.__period_protocol is not None:
            for index, link in enumerate(self.__links_objects):
                self.__link_utilization[index] += int((self.__hyper_period / self.__period_protocol) *
                                                      self.__time_protocol)
                self.__frame_instances += int((self.__hyper_period / self.__period_protocol))

        # For all frames in the network
        for index_frame, frame in enumerate(self.__frames):
            frame.clean_path()
            # For all receivers of the frame, we can assign its path
            for receiver in frame.get_receivers():

                # Get all the simple paths (paths without repetition of node)
                # Variables to calculate the path with less overall utilization
                paths = []
                paths_cost = []
                for node_path in nx.all_simple_paths(self.__graph, frame.get_sender(), receiver):
                    first_iteration = False
                    previous_node = None
                    path = []
                    for node in node_path:
                        if not first_iteration:
                            first_iteration = True
                        else:  # Find the index in the link list with the actual and previous node
                            path.append(self.__links.index([previous_node, node]))
                        previous_node = node
                    paths.append(path)
                    paths_cost.append(0)

                    # For all link in the path, see the overall utilization
                    for link in path:
                        paths_cost[-1] += self.__link_utilization[link]

                # Calculate which path to choose, it will be the path that after adding the frame has less utilization
                for id_path, path in enumerate(paths):
                    for link in path:
                        if not self.__frames[index_frame].link_in_path(link):
                            paths_cost[id_path] += int(((frame.get_size() * 1000000) /
                                                        self.__links_objects[link].get_speed()) *
                                                       (self.__hyper_period / frame.get_period()))
                # Choose the minimum now
                min_index = paths_cost.index(min(paths_cost))

                # Add the utilization of the path done now
                for link in paths[min_index]:
                    if not self.__frames[index_frame].link_in_path(link):
                        self.__link_utilization[link] += int(((frame.get_size() * 1000000) /
                                                              self.__links_objects[link].get_speed()) *
                                                             (self.__hyper_period / frame.get_period()))
                        self.__frame_instances += int((self.__hyper_period / frame.get_period()))
                # After adding the utilization, we can add the new path to the frame
                frame.set_new_path(paths[min_index])

        # Update all the utilization variables to floats and see if the network can be scheduled
        for id_link in range(len(self.__link_utilization)):
            self.__link_utilization[id_link] = float(self.__link_utilization[id_link])
            self.__link_utilization[id_link] /= self.__hyper_period
            self.__utilization += self.__link_utilization[id_link]
        self.__utilization /= len(self.__link_utilization)
        self.__max_link_utilization = max(self.__link_utilization)

    def __generate_paths(self):
        """
        Generate all the shortest paths from every end systems to every other end system
        Fills the 3 dimensions path matrix, first dimension is the sender, second dimension is the receiver,
        third dimension is a list of INDEXES for the dataflow link list (not links ids, pointers to the link lists)
        :return: None
        :rtype: None
        """
        # Init the path 3-dimension matrix with empty arrays
        for i in range(self.__graph.number_of_nodes()):
            self.__paths.append([])
            for j in range(self.__graph.number_of_nodes()):
                self.__paths[i].append([])

        # We iterate over all the senders and receivers
        for sender in self.__end_systems:
            for receiver in self.__end_systems:
                if sender != receiver:  # If they are not the same search the path
                    # As the paths save the indexes, of the links, we need to search them with tuples of nodes, we then
                    # iterate through all nodes find by the shortest path function of Networkx, and skip the first
                    # iteration as we do not have a tuple of nodes yet
                    first_iteration = False
                    previous_node = None
                    for node in nx.shortest_path(self.__graph, sender, receiver):  # For all nodes in the path
                        if not first_iteration:
                            first_iteration = True
                        else:  # Find the index in the link list with the actual and previous node
                            self.__paths[sender][receiver].append(self.__links.index([previous_node, node]))
                        previous_node = node

    def __add_frame_params(self):
        """
        Add the frame parameters to the already created frames
        :return: Nothing
        :rtype: None
        """
        # Normalize percentages
        self.__frames_percentages = \
            [float(per_period) / sum(self.__frames_percentages) for per_period in self.__frames_percentages]

        # For all frames, add the new parameters
        for i in range(len(self.__frames)):
            type_period = random()
            accumulate_period = 0
            # For all possible periods
            for j, per_period in enumerate(self.__frames_percentages):
                if type_period < per_period + accumulate_period:                # Choice one period for the frame

                    self.__frames[i].set_period(self.__periods[j])              # Set a period to the frame

                    self.__frames[i].set_deadline(int(self.__deadlines[j]))     # Set the deadline

                    self.__frames[i].set_size(self.__sizes[j])                  # Set the size

                    self.__frames[i].set_end_to_end(self.__end_to_ends[j])      # Set end to end constraint

                    break  # Once selected, go out
                else:
                    accumulate_period += per_period  # If not, advance in the list

    @staticmethod
    def __calculate_splits(paths):
        """
        Calculate the splits matrix of a given path matrix.
        For every column on the matrix path, search if there are different links and add then in a new split row.
        :param paths: 3-dimensional path matrix
        :type paths: list of list of list of int
        :return: 3-dimensional split matrix
        :rtype: list of list of list of int
        """
        splits = []                             # Matrix to save all the splits
        path_index = 0                          # Horizontal index of the path matrix
        split_index = 0                         # Vertical index of the split matrix
        first_path_flag = False                 # Flag to identify a first found path
        found_split_flag = False                # Flag to identify a split has been found
        paths_left = len(paths)                 # Paths left to be checked
        while paths_left > 1:                   # While we did not finish all different splits
            paths_left = 0                      # Initialize the paths left
            for i in range(len(paths)):         # For all the different paths
                try:                            # The try is needed because a path ended will raise an exception
                    if not first_path_flag:     # Check if it is the first path found
                        first_split = paths[i][
                            path_index]         # Save to compare with other paths to check new splits
                        first_path_flag = True
                    else:
                        if paths[i][path_index] != first_split:  # If it is difference, a new split has been found
                            if not found_split_flag:             # If is the first split, save both links
                                splits.append([])
                                splits[split_index].append(first_split)
                                found_split_flag = True
                            if paths[i][path_index] not in splits[split_index]:     # If the split is not in the list
                                splits[split_index].append(paths[i][path_index])    # Save the link split
                    paths_left += 1             # The path has not ended if no exception had been raised
                except IndexError:              # If there is an exception, the path ended and we continue
                    pass
            if found_split_flag:                # If a split has been found
                split_index += 1                # Increase the split index
                found_split_flag = False
            first_path_flag = False             # Update the path flags and the path index for next iteration
            path_index += 1
        return splits                           # Return the filled splits matrix

    def __generate_frames(self):
        """
        Generate frames for the network. You can choose the number of frames and the percentage of every type
        This is the basic function and will create all possible attributes of the network to default
        Percentages will be balanced alone
        :return: None
        :rtype: None
        """
        # Normalize the percentage so the sum is always 1.0
        sum_per = float(self.__per_broadcast_frames + self.__per_single_frames + self.__per_local_frames +
                        self.__per_multiple_frames)
        self.__per_broadcast_frames /= sum_per
        self.__per_multiple_frames /= sum_per
        self.__per_local_frames /= sum_per
        self.__per_single_frames /= sum_per

        # Iterate for all the frames that needs to be created
        for frame in range(self.__num_frames):
            frame_type = random()                           # Generate random to see which type of frame is
            sender = choice(self.__end_systems)             # Select the sender end system

            # Select receivers depending of the frame type
            # Broadcast frame
            if frame_type < self.__per_broadcast_frames:
                receivers = list(self.__end_systems)        # List of all end systems but the sender
                receivers.remove(sender)

            # Single frame
            elif frame_type < self.__per_broadcast_frames + self.__per_single_frames:
                receivers = list(self.__end_systems)        # Select single receiver that is not the sender
                receivers.remove(sender)
                receivers = [choice(receivers)]

            # Multi frame
            elif frame_type < self.__per_broadcast_frames + self.__per_single_frames + self.__per_multiple_frames:
                receivers = list(self.__end_systems)        # Select a random number of receivers
                receivers.remove(sender)
                shuffle(receivers)
                num_receivers = randint(1, len(receivers))
                receivers = receivers[0:num_receivers]

            # Locally frame
            else:
                possible_receivers = list(self.__end_systems)
                possible_receivers.remove(sender)
                distances = [len(self.__paths[sender][receiver]) for receiver in possible_receivers]
                min_distance = min(distance for distance in distances)      # Find the minimum distance
                receivers = []
                for receiver in possible_receivers:         # Copy receivers with min_distance
                    if len(self.__paths[sender][receiver]) == min_distance:
                        receivers.append(receiver)

            self.__frames.append(Frame(sender, receivers))  # Add the frame to the list of frames

    def __calculate_hyper_period(self):
        """
        Calculates the hyper_period of the network
        :return: the hyper_period
        :rtype: int
        """
        return lcm_multiple(*self.__periods)

    def __calculate_utilization(self):
        """
        Calculates the utilization on all its links, and then return the total utilization of the network
        It also calculates the frame instances (needed to do everything actually)
        :return: utilization of the network, utilization of the link with max utilization
        and True if all links have less than 1 utilization
        :rtype: float, float, bool
        """
        # Init the list where the utilization will be saved for every link
        link_utilization = []
        for _ in self.__links:
            link_utilization.append(0)

        # For all frames in the network
        self.__frame_instances = 0
        for frame in self.__frames:
            # Get all the unique links in the paths of the frame
            unique_links = []
            for receiver in frame.get_receivers():
                # For all links in the path
                for link in self.__paths[frame.get_sender()][receiver]:
                    if link not in unique_links:        # If it is unique, add it
                        unique_links.append(link)

            # Once we have all the links in the path, calculate the ns to transmit for all links
            for link in unique_links:
                # First calculate the time occupied by normal transmissions and its period instances
                link_utilization[link] += int(((frame.get_size() * 1000000) / self.__links_objects[link].get_speed()) *
                                              (self.__hyper_period / frame.get_period()))
                self.__frame_instances += int((self.__hyper_period / frame.get_period()))

        # Last, add the time occupied by the protocols
        if self.__period_protocol is not None:
            for index, link in enumerate(self.__links_objects):
                link_utilization[index] += int((self.__hyper_period / self.__period_protocol) * self.__time_protocol)
                self.__frame_instances += int((self.__hyper_period / self.__period_protocol))

        # Now calculate the utilization in float for every link and calculate the total utilization of the network
        utilization = 0.0
        possible = True
        for index, link in enumerate(link_utilization):
            link_utilization[index] /= self.__hyper_period
            if link_utilization[index] > 1.0:       # Check if is possible to schedule all of its links
                possible = False
            utilization += link_utilization[index]
        return utilization / len(link_utilization), max(link_utilization), possible

    # Input functions #

    def parse_network_xml(self, configuration):
        """
        Creates a network given a network configuration file
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: nothing
        :rtype: None
        """
        self.__min_time_switch = self.__read_min_time_switch(configuration)
        self.__period_protocol, self.__time_protocol = self.__read_protocol_parameters(configuration)
        self.__time_between_frames = self.__read_time_between_frames(configuration)
        self.__topology_description, self.__link_description, self.__cyclic_description, self.__nodes_description = \
            self.__read_network_topology(configuration)
        self.__num_frames, self.__per_single_frames, self.__per_local_frames, self.__per_multiple_frames, \
            self.__per_broadcast_frames = self.__read_traffic_information(configuration)
        self.__periods, self.__deadlines, self.__sizes, self.__end_to_ends, self.__frames_percentages = \
            self.__read_frame_descriptions(configuration)

    @staticmethod
    def __read_min_time_switch(configuration):
        """
        Read the minimum time a frame can be in a switch
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: the min time switch in ns
        :rtype: int
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read the value, convert it to the needed unit and return it
        min_time_switch_xml = root.find('Topology/TopologyInformation/MinTimeSwitch')
        min_time_switch = int(min_time_switch_xml.text)
        # Check the unit and change to fit us
        if min_time_switch_xml.attrib['unit'] == 'ms':
            min_time_switch *= 1000000
        elif min_time_switch_xml.attrib['unit'] == 's':
            min_time_switch *= 1000000000
        elif min_time_switch_xml.attrib['unit'] == 'us':
            min_time_switch *= 1000
        elif min_time_switch_xml.attrib['unit'] == 'ns':
            pass
        else:
            raise TypeError('I do not know this unit for the minimum time for a frame in a switch => '
                            + min_time_switch_xml.attrib['unit'])
        return min_time_switch

    @staticmethod
    def __read_protocol_parameters(configuration):
        """
        Read the protocol parameters of the network
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: the period and time of the protocol in ns in a list of two integers
        :rtype: list of int
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read the value, convert it to the needed unit and return it
        period_protocol_xml = root.find('Topology/TopologyInformation/PeriodProtocol')
        period_protocol = int(period_protocol_xml.text)
        # Check the unit and change to fit us
        if period_protocol_xml.attrib['unit'] == 'ms':
            period_protocol *= 1000000
        elif period_protocol_xml.attrib['unit'] == 's':
            period_protocol *= 1000000000
        elif period_protocol_xml.attrib['unit'] == 'us':
            period_protocol *= 1000
        elif period_protocol_xml.attrib['unit'] == 'ns':
            pass
        else:
            raise TypeError(
                'I do not know this unit for the period protocol => ' + period_protocol_xml.attrib['unit'])

        # Read the value, convert it to the needed unit and return it
        time_protocol_xml = root.find('Topology/TopologyInformation/TimeProtocol')
        time_protocol = int(time_protocol_xml.text)
        # Check the unit and change to fit us
        if time_protocol_xml.attrib['unit'] == 'ms':
            time_protocol *= 1000000
        elif time_protocol_xml.attrib['unit'] == 's':
            time_protocol *= 1000000000
        elif time_protocol_xml.attrib['unit'] == 'us':
            time_protocol *= 1000
        elif time_protocol_xml.attrib['unit'] == 'ns':
            pass
        else:
            raise TypeError(
                'I do not know this unit for the time protocol => ' + time_protocol_xml.attrib['unit'])
        return period_protocol, time_protocol

    @staticmethod
    def __read_time_between_frames(configuration):
        """
        Read the time that the frames have to wait between frames transmissions
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: the time between frames transmissions in ns
        :rtype: int
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read the value, convert it to the needed unit and return it
        time_between_frames_xml = root.find('Topology/TopologyInformation/TimeBetweenFrames')
        time_between_frames = int(time_between_frames_xml.text)
        # Check the unit and change to fit us
        if time_between_frames_xml.attrib['unit'] == 'ms':
            time_between_frames *= 1000000
        elif time_between_frames_xml.attrib['unit'] == 's':
            time_between_frames *= 1000000000
        elif time_between_frames_xml.attrib['unit'] == 'us':
            time_between_frames *= 1000
        elif time_between_frames_xml.attrib['unit'] == 'ns':
            pass
        else:
            raise TypeError('I do not know this unit for the time between frame transmissions => '
                            + time_between_frames_xml.attrib['unit'])
        return time_between_frames

    @staticmethod
    def __read_cyclic_network_topology(configuration):
        """
        Returns the network description of a
        :param configuration: :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: returns a matrix with list of [1, str] and the nodes description
        :rtype: list of [int, str], list of str
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read all the nodes of the topology
        nodes_xml = root.findall('Topology/Description/Node')
        network_description = []
        nodes_description = []

        # For all nodes in the topology, read their connections
        for node_xml in nodes_xml:

            # Read the type of node and add it to the nodes list
            network_description.append([])
            nodes_description.append(node_xml.attrib['category'])

            # Read all the connections
            for connection_xml in node_xml.findall('Connection'):
                node_connection = int(connection_xml.find('NodeID').text)
                link_xml = connection_xml.find('Link')

                # Save the type information and check if is correct
                if link_xml.attrib['category'] == 'wired':
                    link_str = 'w'
                elif link_xml.attrib['category'] == 'wireless':
                    link_str = 'x'
                else:
                    raise TypeError('The type of the link is not wired neither wireless')

                # Save the speed of the link and convert it to MB/s (standard used in the Network Class)
                speed_xml = link_xml.find('Speed')
                speed = int(speed_xml.text)
                if speed_xml.attrib['unit'] == 'KB/s':  # Convert the speed if is in KB/s
                    speed /= 1000
                if speed_xml.attrib['unit'] == 'GB/s':  # Convert the speed if is in GB/s
                    speed *= 1000
                link_str += str(speed)

                network_description[-1].append([node_connection, link_str])

        return network_description, nodes_description

    def __read_network_topology(self, configuration):
        """
        Returns the network description (including the link description if exist) from the xml file
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: strings with the network description and the link description (formatted to work in the network
        function)
        If the topology has cycles, returns a matrix with list of [1, str]
        :rtype: str, str, list of [int, str]
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read all the bifurcations of the topology
        bifurcations_xml = root.findall('Topology/Description/Bifurcation')
        if not bifurcations_xml:    # If there is not bifurcation, it is the new cyclic network description
            network_matrix, nodes_information = self.__read_cyclic_network_topology(configuration)
            return None, None, network_matrix, nodes_information

        # Initialize strings to be returned with the description
        network_description_line = ''
        link_info_line = ''
        links_found = False
        link = 0

        # For all bifurcations in the topology, read the links information
        for bifurcation in bifurcations_xml:

            # Find the number of links in the bifurcation
            network_description_line += bifurcation.find('NumberLinks').text + ';'
            number_links = int(bifurcation.find('NumberLinks').text)
            links_xml = bifurcation.find('Link')
            links_counter = 0
            links_found = False

            # See if there is also links description
            if links_xml is not None:
                links_found = True
                for link_xml in bifurcation.findall('Link'):  # For all links information
                    links_counter += 1

                    # Save the type information and check if is correct
                    if link_xml.attrib['category'] == 'wired':
                        link_info_line += 'w'
                    elif link_xml.attrib['category'] == 'wireless':
                        link_info_line += 'x'
                    else:
                        raise TypeError('The type of the link is not wired neither wireless')
                    link += 1

                    # Save the speed of the link and convert it to MB/s (standard used in the Network Class)
                    speed_xml = link_xml.find('Speed')
                    speed = speed_xml.text
                    if speed_xml.attrib['unit'] == 'KB/s':  # Convert the speed if is in KB/s
                        speed /= 1000
                    if speed_xml.attrib['unit'] == 'GB/s':  # Convert the speed if is in GB/s
                        speed *= 1000
                    link_info_line += str(speed) + ';'

            # Check if the number of links said by the bifurcation and the encounter links match
            if abs(number_links) != links_counter:
                raise ValueError('The number of links is incorrect, they should be the same as the bifurcations')

        # Return the description string, and the link description string if exists, we do not return the last character
        # of the string as it is a useless ';'
        if not links_found:
            return network_description_line[0:-1], None, None, None
        else:
            return network_description_line[0:-1], link_info_line[0:-1], None, None

    @staticmethod
    def __read_traffic_information(configuration):
        """
        Get the traffic information from the configuration file
        :param configuration: path and name of the configuration file
        :type configuration: str
        :return: the number of frames, percentage of single, local, multiple and broadcast, in this order
        :rtype: list of int and float
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Position the branch on the top of the traffic information and read all needed variables
        traffic_information_xml = root.find('Traffic/TrafficInformation')
        num_frames = int(traffic_information_xml.find('NumberFrames').text)
        single = float(traffic_information_xml.find('Single').text)
        local = float(traffic_information_xml.find('Local').text)
        multiple = float(traffic_information_xml.find('Multiple').text)
        broadcast = float(traffic_information_xml.find('Broadcast').text)

        # Return everything
        return num_frames, single, local, multiple, broadcast

    @staticmethod
    def __read_frame_descriptions(configuration):
        """
        Get the frame descriptions from the configuration file
        :param configuration: path and name of the configuration file
        :type configuration: str
        :return: list of periods, list of deadlines, list of sizes, list of end to end constraints and list of
        percentages (all times are expressed in ns)
        :rtype: list of int, list of int, list of int, list of int, list of float
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Position the branch to the traffic that is going to be read
        frames_description_xml = root.find('Traffic/FrameDescription')

        # Init lists of variables to return
        periods = []
        deadlines = []
        sizes = []
        end_to_ends = []
        frame_percentages = []

        # For all frame types, add the period, percentage, deadlines, sizes and end to end to the list of variables
        frame_types_xml = frames_description_xml.findall('FrameType')
        for frame_type_xml in frame_types_xml:

            # Get the period
            period_xml = frame_type_xml.find('Period')
            period = int(period_xml.text)
            # Check the unit and change the period to fit us
            if period_xml.attrib['unit'] == 'ms':
                period *= 1000000
            elif period_xml.attrib['unit'] == 'ns':
                pass
            elif period_xml.attrib['unit'] == 's':
                period *= 1000000000
            elif period_xml.attrib['unit'] == 'us':
                period *= 1000
            else:
                raise TypeError('I do not know this unit for the period => ' + period_xml.attrib['unit'])
            periods.append(period)

            # Get the deadline
            if frame_type_xml.find('Deadline') is None:  # If there is no deadline, add 0
                deadlines.append(0)
            else:
                # Check the unit and change the period to fit us
                deadline_xml = frame_type_xml.find('Deadline')
                deadline = int(deadline_xml.text)
                # Check the unit and change the period to fit us
                if deadline_xml.attrib['unit'] == 'ms':
                    deadline *= 1000000
                elif deadline_xml.attrib['unit'] == 'ns':
                    pass
                elif deadline_xml.attrib['unit'] == 's':
                    deadline *= 1000000000
                elif deadline_xml.attrib['unit'] == 'us':
                    deadline *= 1000
                else:
                    raise TypeError('I do not know this unit for the deadline => ' + deadline_xml.attrib['unit'])
                deadlines.append(deadline)

            # Get the end to end constraint
            end_to_end_xml = frame_type_xml.find('EndToEnd')
            end_to_end = int(end_to_end_xml.text)
            # Check the unit and change the period to fit us
            if end_to_end_xml.attrib['unit'] == 'ms':
                end_to_end *= 1000000
            elif end_to_end_xml.attrib['unit'] == 'ns':
                pass
            elif end_to_end_xml.attrib['unit'] == 's':
                end_to_end *= 1000000000
            elif end_to_end_xml.attrib['unit'] == 'us':
                end_to_end *= 1000
            else:
                raise TypeError('I do not know this unit for the end to end constraint => ' +
                                end_to_end_xml.attrib['unit'])
            end_to_ends.append(end_to_end)

            # Get the size
            sizes.append(int(frame_type_xml.find('Size').text))

            # Get the frame percentage
            frame_percentages.append(float(frame_type_xml.find('Percentage').text))

        return periods, deadlines, sizes, end_to_ends, frame_percentages

    # Output functions #

    def write_network_xml(self, name):
        """
        Writes all the information of the network in a xml file and stores it in the given path + name
        :param name: path and name of the xml file to create
        :type name: str
        :return: nothing
        :rtype: None
        """
        # Create top of the xml file
        network_input_xml = Xml.Element('Network')

        # Write the general information of the network
        general_information_xml = Xml.SubElement(network_input_xml, 'GeneralInformation')
        Xml.SubElement(general_information_xml, 'NumberFrames').text = str(self.__num_frames)
        Xml.SubElement(general_information_xml, 'NumberLinks').text = str(len(self.__links))
        Xml.SubElement(general_information_xml, 'NumberSwitches').text = str(len(self.__switches))
        Xml.SubElement(general_information_xml, 'NumberEndSystems').text = str(len(self.__end_systems))
        Xml.SubElement(general_information_xml, 'MinimumTimeSwitch').text = str(self.__min_time_switch)
        Xml.SubElement(general_information_xml, 'HyperPeriod').text = str(self.__hyper_period)
        Xml.SubElement(general_information_xml, 'Utilization').text = str(self.__utilization)
        Xml.SubElement(general_information_xml, 'MaximumLinkUtilization').text = str(self.__max_link_utilization)
        Xml.SubElement(general_information_xml, 'FrameInstances').text = str(self.__frame_instances)
        Xml.SubElement(general_information_xml, 'PeriodProtocol').text = str(self.__period_protocol)
        Xml.SubElement(general_information_xml, 'TimeProtocol').text = str(self.__time_protocol)
        Xml.SubElement(general_information_xml, 'TimeBetweenFrames').text = str(self.__time_between_frames)

        # Write the network description
        self.__generate_network_description_xml(network_input_xml)

        # Write the traffic information
        traffic_information_xml = Xml.SubElement(network_input_xml, 'TrafficInformation')
        self.__generate_frames_xml(traffic_information_xml)

        # Write the final file
        output_xml = minidom.parseString(Xml.tostring(network_input_xml)).toprettyxml(indent="   ")
        with open(name, "w") as f:
            f.write(output_xml)

    def __generate_network_description_xml(self, top):
        """
        Generates the network description, including the nodes, links and collision domains information
        :param top: top of the xml tree where to add the network description
        :type top: Xml.SubElement
        :return: nothing
        :rtype: none
        """
        # Create the node to attach all the network description
        network_description_xml = Xml.SubElement(top, 'NetworkDescription')

        # For all nodes, attach its information
        nodes_xml = Xml.SubElement(network_description_xml, 'Nodes')
        for index, node in self.__graph.nodes_iter(data=True):
            node_type = ''
            if node['type'].get_type() == NodeType.end_system:     # Check the type of the node (End system or Switch)
                node_type = 'End System'
            elif node['type'].get_type() == NodeType.switch:
                node_type = 'Switch'
            node_xml = Xml.SubElement(nodes_xml, 'Node')
            node_xml.set('category', node_type)         # Set the category to either End System or Wireless

            # Add the information of the node
            Xml.SubElement(node_xml, 'ID').text = str(index)

            # Add the links connected to that node
            connections_xml = Xml.SubElement(node_xml, 'Connections')
            for link in self.__links:
                if link[0] == index or link[1] == index:    # If the link is connected to the node, add its ID
                    Xml.SubElement(connections_xml, 'Link').text = str(self.__links.index(link))

        # For all links, attach its information
        links_xml = Xml.SubElement(network_description_xml, 'Links')
        for index, link in enumerate(self.__links_objects):
            link_type = ''
            if link.get_type() == LinkType.wireless:        # Check if the link type is wired or wireless
                link_type = 'Wireless'
            elif link.get_type() == LinkType.wired:
                link_type = 'Wired'
            link_xml = Xml.SubElement(links_xml, 'Link')
            link_xml.set('category', link_type)
            Xml.SubElement(link_xml, 'ID').text = str(index)
            Xml.SubElement(link_xml, 'Speed').text = str(link.get_speed())
            Xml.SubElement(link_xml, 'Source').text = str(self.__links[index][0])
            Xml.SubElement(link_xml, 'Destination').text = str(self.__links[index][1])

    def __generate_frames_xml(self, top):
        """
        Generate the output XML for the frames
        :param top: top element where to add the frames
        :type top: Xml.SubElement
        :return: nothing
        :rtype: None
        """
        # For all frames, attach the frame information
        frames_xml = Xml.SubElement(top, 'Frames')
        for index, frame in enumerate(self.__frames):
            frame_xml = Xml.SubElement(frames_xml, 'Frame')
            Xml.SubElement(frame_xml, 'ID').text = str(index)
            Xml.SubElement(frame_xml, 'Period').text = str(frame.get_period())
            Xml.SubElement(frame_xml, 'Starting').text = str(frame.get_starting())
            Xml.SubElement(frame_xml, 'Deadline').text = str(frame.get_deadline())
            Xml.SubElement(frame_xml, 'Size').text = str(frame.get_size())
            Xml.SubElement(frame_xml, 'EndToEnd').text = str(frame.get_end_to_end())

            # Write all paths
            paths_xml = Xml.SubElement(frame_xml, 'Paths')
            paths = []                                  # List to save the paths, necessary to calculate the splits
            for receiver in frame.get_receivers():      # Every path is from the sender to one of the frame receivers
                path_str = ''
                paths.append([])                        # Init the current path
                for link in frame.get_path_receiver(receiver):              # For all links in the path
                    path_str += str(link) + ';'
                    paths[-1].append(link)              # Save the link to calculate the split later on
                Xml.SubElement(paths_xml, 'Path').text = path_str[:-1]      # Save the path once finished

            # Write all splits
            splits_xml = Xml.SubElement(frame_xml, 'Splits')
            splits = self.__calculate_splits(paths)     # Calculate the paths for the given splits
            if len(splits) > 0:                         # If there are any splits
                for split in splits:                    # For all splits
                    split_str = ''
                    for link in split:                  # For all links in the split, do the same that in paths
                        split_str += str(link) + ';'
                    Xml.SubElement(splits_xml, 'Split').text = split_str[:-1]
