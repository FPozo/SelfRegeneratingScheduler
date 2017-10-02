"""* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                                                     *
 *  Link Class                                                                                                         *
 *  Network Generator                                                                                                  *
 *                                                                                                                     *
 *  Created by Francisco Pozo on 01/10/17.                                                                             *
 *  Copyright © 2017 Francisco Pozo. All rights reserved.                                                              *
 *                                                                                                                     *
 *  Class for the links in the network. Quite simple class that only defines the speed and link type (wired or         *
 *  wireless)                                                                                                          *
 *                                                                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * """

from enum import Enum


class LinkType(Enum):
    """
    Class to define the different link types
    """
    wired = 1
    wireless = 2


class Link:
    """
    Class that has all the information of a communication link (speed and type)
    """

    # Variable definitions #

    __speed = 0  # Speed in MB/s
    __link_type = 0  # Link type

    # Standard function definitions #
    def __init__(self, speed=100, link_type=LinkType.wired):
        """
        Initialization of the link, if no values given it creates an standard Deterministic Ethernet Link
        :param speed: Speed of the link in MB/s
        :type speed: int
        :param link_type: Type of the link (wired or wireless)
        :type link_type: LinkType
        """
        self.__speed = speed
        self.__link_type = link_type

    def __str__(self):
        """
        String call of the link class
        :return: a string with the information
        :rtype: str
        """
        # Check what kind of link it is
        if self.__link_type == LinkType.wired:
            return "Wired link with speed " + str(self.__speed) + "MB/s"
        else:
            return "Wireless link with speed " + str(self.__speed) + "MB/s"

    # Private function definitions #
