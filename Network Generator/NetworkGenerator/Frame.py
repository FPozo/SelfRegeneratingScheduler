"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Frame Class                                                                                                        *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 01/10/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Class for the frames in the network. A frame has only one node (end system) sender and one or multiple (also end   *
 *  systems) receivers. All frames must have a period in nanoseconds and a in bytes in the range of the Ethernet       *
 *  Standard frames. They may also have a deadline (also in nanoseconds).                                              *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """


class Frame:
    """
    Class that has all the information of a time-triggered frame in the network
    """

    # Variable definitions #

    __sender = None                         # End system sender id of the frame
    __receivers = []                        # List of end systems receivers id of the frame
    __path = []                             # List of list indexes of the links to arrive from a sender to a receiver
    __size = None                           # Size of the frame in bytes (Frame Standard between 72 and 1526 bytes)
    __starting = None                       # Staring time in nanoseconds of the frame
    __period = None                         # Period in nanoseconds of the frame
    __deadline = None                       # Deadline in nanoseconds of the frame (if 0 => same as period)
    __end_to_end = None                     # End to end constraint in nanoseconds of the frame

    # Standard function definitions #

    def __init__(self, sender, receivers, period=None, deadline=None, size=None, starting=0, end_to_end=None):
        """
        Initialization of the needed values of a time-triggered frame
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
        self.__sender = sender
        self.__receivers = receivers
        self.__period = period
        if deadline == 0:
            self.__deadline = period
        else:
            self.__deadline = deadline
        self.__size = size
        self.__starting = starting
        self.__end_to_end = end_to_end

    def __str__(self):
        """
        String call of the frame class
        :return: a string with the information
        :rtype: str
        """
        return_text = "Frame information =>\n"
        return_text += "    Sender id     : " + str(self.__sender) + "\n"
        return_text += "    Receivers ids : " + str(self.__receivers) + "\n"
        return_text += "    Period        : " + str(self.__period) + " nanoseconds\n"
        return_text += "    Starting      : " + str(self.__starting) + " nanoseconds\n"
        return_text += "    Deadline      : " + str(self.__deadline) + " nanoseconds\n"
        return_text += "    Size          : " + str(self.__size) + " bytes"
        return return_text

    def get_sender(self):
        """
        Get the sender of the frame
        :return: sender id of the frame
        :rtype: int
        """
        return self.__sender

    def get_receivers(self):
        """
        Get all the receivers of the frame
        :return: list with all the receivers id
        :rtype: list of int
        """
        return self.__receivers

    def get_paths(self):
        """
        Get the path matrix of the frame
        :return: the path matrix
        :rtype: list of list of int
        """
        return self.__path

    def get_path_receiver(self, receiver):
        """
        Get the path for the given receiver
        :param receiver: receiver node
        :type receiver: int
        :return: path
        :rtype: list of int
        """
        return self.__path[self.__receivers.index(receiver)]

    def clean_path(self):
        self.__path = []

    def set_new_path(self, new_path):
        """
        Set a new path for the receiver (important to add the path in the same order as receivers)
        :param new_path: list of links
        :type new_path: list of int
        :return: nothing
        :rtype: None
        """
        self.__path.append(new_path)

    def link_in_path(self, link):
        """
        Search if the given link is already in the path
        :param link: index of the link
        :type link: int
        :return: True if the given link is in the path, False otherwise
        :rtype: Boolean
        """
        for path in self.__path:
            for path_link in path:
                if link == path_link:
                    return True
        return False

    def get_link_predecessor(self, link):
        """
        Get the link predecessor of a given link in the path of the frame
        :param link: link identifier
        :type link: int
        :return: link identifier, -1 if is the first link, -2 if not found
        :rtype: int
        """
        # For all paths, search if the given link is in the path
        for path in self.__path:
            previous_link = -1          # We store the previous link to return if we found the link
            for path_link in path:
                if path_link == link:   # If we found it, return the previous link, if not, continue
                    return previous_link
                previous_link = path_link
        return -2

    def get_link_successor(self, link):
        """
        Get the link successor of a given link in the path of the frame
        :param link: link identifier
        :type link: int
        :return: link identifier, -1 if is the last link, -2 if not found
        :rtype: int
        """
        # For all paths, search if the given link is in the path
        for path in self.__path:
            found_link = False          # If we found the link, see if there are next link
            for path_link in path:
                if found_link:          # If the previous link was the same as the given link, this is the successor
                    return path_link
                if path_link == link:   # If we find the given link, if is not the last link of the path, continue
                    if path[-1] != link:
                        found_link = True
        return -1                       # Is the last link of the path

    def get_period(self):
        """
        Get the period of the frame in ns
        :return: period of the frame in ns
        :rtype: int
        """
        return self.__period

    def set_period(self, period):
        """
        Set the period of the frame
        :param period: period of the frame in ns
        :type period: int
        :return: nothing
        :rtype: None
        """
        self.__period = period

    def get_deadline(self):
        """
        Get the deadline of the frame in ns
        :return: deadline of the frame in ns
        :rtype: int
        """
        return self.__deadline

    def set_deadline(self, deadline):
        """
        Set the deadline of the frame
        :param deadline: deadline of the frame in ns
        :type deadline: int
        :return: nothing
        :rtype: none
        """
        if deadline == 0:
            self.__deadline = self.__period
        else:
            self.__deadline = deadline

    def get_starting(self):
        """
        Get the starting time
        :return: starting time in nanoseconds
        :rtype: int
        """
        return self.__starting

    def set_starting(self, starting):
        """
        Set the starting
        :param starting: starting in nanoseconds
        :type starting: int
        :return: nothing
        :rtype: None
        """
        self.__starting = starting

    def get_size(self):
        """
        Get the size of the frame in bytes
        :return: size of the frame in bytes
        :rtype: int
        """
        return self.__size

    def set_size(self, size):
        """
        Set the size of the frame
        :param size: size of the frame in bytes
        :type size: int
        :return: nothing
        :rtype: None
        """
        self.__size = size

    def get_end_to_end(self):
        """
        Get the end to end constraint of the frame in ns
        :return: end to end constraint of the frame in ns
        :rtype: int
        """
        return self.__end_to_end

    def set_end_to_end(self, end_to_end):
        """
        Set the end to end constraint of the frame
        :param end_to_end: end to end constraint of the frame in ns
        :type end_to_end: int
        :return: nothing
        :rtype: None
        """
        self.__end_to_end = end_to_end

    # Private function definitions #
