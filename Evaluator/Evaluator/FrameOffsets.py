"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  FrameOffsets Class                                                                                                 *
 *  Network Evaluator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 09/10/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *  Class with the information of the frame of the network and its transmission times.                                 *
 *  It is a Class extension of Frame from the network generator, but this one also has information about transmission  *
 *  times, instances and replicas.                                                                                     *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from NetworkGenerator.Frame import Frame


class Offset:
    """
    Class that contains the transmission times of a frame in a link
    """

    # Variable definitions #

    __offset_starting = []      # List to save all the starting transmission times
    __offset_ending = []        # List to save all the ending transmission times
    __num_instances = None      # Number of instances in the frame
    __link = None               # Link identification

    # Standard function definitions #

    def __init__(self, offset_starting, offset_ending, num_instances, link):
        """
        Init the offset
        :param offset_starting: list with the starting transmission times in the link
        :type offset_starting: list of int
        :param offset_ending: list with the ending transmission times in the link
        :type offset_ending: list of int
        :param num_instances: number of instances
        :type num_instances: int
        :param link: identification fo the link
        :type link: int
        """
        self.__num_instances = num_instances
        self.__link = link
        self.__offset_starting = offset_starting
        self.__offset_ending = offset_ending

    def get_link(self):
        """
        Get the link of the offset
        :return: identification of the link
        :rtype: int
        """
        return self.__link

    def set_link(self, link):
        """
        Set the link of the Offset
        :param link: new link id
        :type link: int
        :return: nothing
        :rtype: None
        """
        self.__link = link

    def get_ending_time(self, instance):
        """
        Get the ending time of the given instance
        :param instance: number of instance
        :type instance: int
        :return: ending of transmission time
        :rtype: int
        """
        return self.__offset_ending[instance]

    def get_starting_time(self, instance):
        """
        Get the starting time of the given instance
        :param instance: number of instance
        :type instance: int
        :return: ending of transmission time
        :rtype: int
        """
        return self.__offset_starting[instance]


class FrameOffset(Frame):
    """
    Extension of the frame class that has added information about transmission times, instances and replicas
    """

    # Variable definitions #

    __link_offset = []              # List with all the offsets of every link in the frame

    # Standard function definitions #

    def __init__(self, sender, receivers, period, deadline, size, starting, end_to_end):
        """
        Init the frame
        :param sender: end system sender id
        :type sender: int
        :param receivers: list of end systems receivers id
        :type receivers: list of int
        :param period: period in nanoseconds
        :type period: int
        :param deadline: deadline in nanoseconds (0 => same as period)
        :type deadline: int
        :param size: size in bytes of the frame, recommended between 72 and 1526 (Ethernet standard)
        :type size: int
        :param starting: starting in nanoseconds
        :type starting:int
        :param end_to_end: end to end time of the frame in nanoseconds
        :type end_to_end: int
        """
        super(FrameOffset, self).__init__(sender, receivers, period, deadline, size, starting, end_to_end)
        self.__link_offset = []

    def add_offset(self, offset_starting, offset_ending, num_instances, link):
        """
        Add a new offset to the link offset list if the link is not already there
        :param offset_starting: list with the starting transmission times in the link
        :type offset_starting: list of int
        :param offset_ending: list with the ending transmission times in the link
        :type offset_ending: list of int
        :param num_instances: number of instances
        :type num_instances: int
        :param link: identification fo the link
        :type link: int
        :return: nothing
        :rtype: None
        """
        # Check if we already added this link transmission times
        for link_offset in self.__link_offset:
            if link_offset.get_link() == link:
                return

        # If not, we create it
        self.__link_offset.append(Offset(offset_starting, offset_ending, num_instances, link))

    def get_offset_link(self, link):
        """
        Get the offset of a frame with the same link as the given one, nothing if it does not have it
        :param link: identifier of the link
        :type link: int
        :return: offset of the given link
        :rtype: Offset
        """
        # For all the offsets, search if one has the same link as the given one
        for offset in self.__link_offset:
            if offset.get_link() == link:
                return offset

    def get_predecessor_path_receiving_time(self, link, instance):
        """
        Get the time when the predecessor path of a given link is received for the given instance
        :param link: identifier of the link
        :type link: int
        :param instance: instance of the transmission time asked
        :type instance: int
        :return: transmission time
        :rtype: int
        """
        previous_link = self.get_link_predecessor(link)
        if previous_link == -1:     # If it has no predecessor, return the first possible transmission time
            return self.get_period() * instance
        # Search the offset of the previous link
        for offset in self.__link_offset:
            if previous_link == offset.get_link():
                return offset.get_ending_time(instance)

    def get_successor_path_transmission_time(self, link, instance):
        """
        Get the time when the successor path of a given link is sent for the given instance
        :param link: identifier of the link
        :type link: int
        :param instance: instance of the transmission time asked
        :type instance: int
        :return: transmission time
        :rtype: int
        """
        successor_link = self.get_link_successor(link)
        if successor_link == -1:    # If it has no successor, return the last possible transmission time
            return (self.get_period() * instance) + self.get_deadline()
        # Search the offset of the successor link
        for offset in self.__link_offset:
            if successor_link == offset.get_link():
                return offset.get_starting_time(instance)
