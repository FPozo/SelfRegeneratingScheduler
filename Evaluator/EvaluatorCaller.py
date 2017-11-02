from Evaluator import Evaluator


evaluator = Evaluator.Evaluator()
evaluator.read_network_xml('../Networks/auto.xml', '../Networks/schedule.xml')
evaluator.activate_protocol(9, 0)
