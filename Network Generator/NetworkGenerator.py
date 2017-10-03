from NetworkGenerator.Network import *
import datetime


def test():
    start_time = datetime.datetime.utcnow()
    network = Network()
    network.parse_network_xml('/Users/fpo01/OneDrive - Mälardalens högskola/Software/Self-Regenerating Scheduler/NetworkConfigurations/prueba.xml')
    network.create_network()
    network.write_network_xml('../Networks/auto.xml')
    end_time = datetime.datetime.utcnow()
    print('Time check schedule in ms => ' + str((end_time - start_time).total_seconds() * 1000))


test()
