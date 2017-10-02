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
    __size = None                           # Size of the frame in bytes (Frame Standard between 72 and 1526 bytes)
    __period = None                         # Period in nanoseconds of the frame
    __deadline = None                       # Deadline in nanoseconds of the frame (if 0 => same as period)

    # Standard function definitions #

    def __init__(self, sender, receivers, period, deadline, size):
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
        """
        self.__sender = sender
        self.__receivers = receivers
        self.__period = period
        if deadline == 0:
            self.__deadline = period
        else:
            self.__deadline = deadline
        self.__size = size

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
        return_text += "    Deadline      : " + str(self.__deadline) + " nanoseconds\n"
        return_text += "    Size          : " + str(self.__size) + " bytes"
        return return_text

    # Private function definitions #
