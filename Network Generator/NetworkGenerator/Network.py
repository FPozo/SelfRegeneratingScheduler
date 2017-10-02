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
    __Links_objects = []            # List with all the links objects in the network (cannot be saved in the graph)
    __paths = []                    # Matrix with the number of end systems as index for x and y, it contains a list of
    # links to describe the path from end system x to end system y, None if x = y
    __frames = []                   # List with all the frames objects in the network
    __min_time_switch = None        # Minimum time for a frame to stay in the switch
    __period_protocol = None        # Period of the protocol to reserve bandwidth
    __time_protocol = None          # Time of the protocol to reserve bandwidth
    __time_between_frames = None    # Time the frames have to wait between transmissions
    __topology_description = []     # String with the information of the topology
    __link_description = []         # String with the information of the links
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

    # Standard function definitions #

    def __init__(self):
        """
        Initialization of an empty network
        """
        self.__graph = nx.Graph()
        self.__switches = []
        self.__end_systems = []
        self.__links = []
        self.__Links_objects = []
        self.__paths = []
        self.__frames = []

    # Private function definitions #

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
        self.__topology_description, self.__link_description = self.__read_network_topology(configuration)
        self.__num_frames, self.__per_single_frames, self.__per_local_frames, self.__per_multiple_frames, \
            self.__per_broadcast_frames = self.__read_traffic_information(configuration)
        self.__periods, self.__deadlines, self.__sizes, self.__end_to_ends, self.__frames_percentages = \
            self.__read_frame_descriptions(configuration)

        print(self.__min_time_switch)
        print(self.__period_protocol)
        print(self.__time_protocol)
        print(self.__time_between_frames)
        print(self.__topology_description)
        print(self.__link_description)
        print(self.__num_frames)
        print(self.__per_single_frames)
        print(self.__per_local_frames)
        print(self.__per_multiple_frames)
        print(self.__per_broadcast_frames)
        print(self.__periods)
        print(self.__deadlines)
        print(self.__sizes)
        print(self.__end_to_ends)
        print(self.__frames_percentages)

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
    def __read_network_topology(configuration):
        """
        Returns the network description (including the link description if exist) from the xml file
        :param configuration: path and name of the xml configuration file
        :type configuration: str
        :return: strings with the network description and the link description (formatted to work in the network
        function)
        :rtype: list of str
        """
        # Open the file if exists
        try:
            tree = Xml.parse(configuration)
        except Xml.ParseError:
            raise Exception("Could not read the configuration xml file")
        root = tree.getroot()

        # Read all the bifurcations of the topology
        bifurcations_xml = root.findall('Topology/Description/Bifurcation')

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
            return network_description_line[0:-1], None
        else:
            return network_description_line[0:-1], link_info_line[0:-1]

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

    def __read_frame_descriptions(self, configuration):
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
