#!/usr/bin/env python3

# Author: Federico Amedeo Izzo, federico.izzo42@gmail.com

import argparse
from graphviz import Graph

### GREEDY SCHEDULING ALGORITHM

## INPUTS:
# - Topology map: graph as list of edges without repeated arcs
# This is because every established link is bidirectional and the Network Graph
# is an unoriented graph
# ex: (1,2) and (2,1) is saved as (1,2)
#    - edge: tuple of (nodeID, nodeID)
# Diamond topology example
topology_1 = [(0, 1), (0, 2), 
              (1, 2), (1, 3),
              (2, 3)]
  
# RTSS Paper topology example
topology_2 = [(0, 1), (0, 3), (0, 5), (0, 7),
              (1, 3), (1, 5), (1, 7),
              (2, 4), (2, 6), (2, 7), (2, 8),
              (3, 5),
              (4, 5), (4, 6), (4, 7), (4, 8),
              (5, 7), (5, 8),
              (6, 8),
              (7, 8)]

# - List of required streams (broken down in 1-hop transmissions)
#    - transmission: (source, destination) hop not listed (=1)
# example 1, TX-RX conflict (3,1)
stream_list_1 = [(0, 1),
                 (3, 2)]         
# example 2, TX-RX conflict (2,1)         
stream_list_2 = [(0, 1), 
                 (2, 3)]
# example 3, RTSS Paper streams
stream_list_3 = [(3, 0), 
                 (6, 0),
                 (4, 0)]
                 
# - Number of data slots per slotframe
data_slots_1 = 10

## OUTPUTS
# - Schedule: list of ScheduleElement:
#   - ScheduleElement: tuple of (timeslot, node, activity, [destination node])
# schedule = []

def check_unicity_conflict(schedule, timeslot, stream):
    # Unicity check: no activity for src or dst node on a given timeslot
    src, dst = stream;
    link = {src,dst}
    conflict_set = [e for e in schedule if ((e[0] == timeslot) and (e[1] in link))]
    if conflict_set:
        print('Conflict Detected! src or dst node are busy on timeslot ' + repr(timeslot))
        return True;
    else:
        return False

def check_interference_conflict(schedule, topology, timeslot, node, activity):
    # Interference check: no TX and RX for nodes at 1-hop distance in the same timeslot
    # Checks if nodes at 1-hop distance of 'node' are doing 'activity'
    conflict = False;
    # one_hop is list of adjacence of node 'node'
    one_hop = [edge[1] for edge in topology if edge[0] == node] \
    + [edge[0] for edge in topology if edge[1] == node]
    #print(repr(one_hop))
    for n in one_hop:
        for elem in schedule:
            #print(repr(node)+repr(elem))
            if elem == (timeslot, n, activity):
                conflict = True;
                print('Conflict Detected! TX-RX conflict between node ' + repr(node) + ' and node ' + repr(n))
    return conflict;

# option A iteration
# TODO sort streams according to chosen metric
def scheduler(topology, stream_list, data_slots):
    schedule = []
    for stream in stream_list:
        for timeslot in range(data_slots):
            print('Checking stream ' + repr(stream) + ' on timeslot ' + repr(timeslot))
            
            # Stream tuple unpacking
            src, dst = stream;
            
            err_conflict = False;
            err_unreachable = False;
            
            ## Connectivity check: edge between src and dst nodes
            if (src,dst) not in topology and (dst,src) not in topology:
                err_unreachable = True;
                print('Nodes are not reachable in topology, cannot schedule stream ' + repr(stream))
                break;    #Cannot schedule transmission
                
            ## Conflict checks
            # Unicity check: no activity for src or dst node on a given timeslot
            err_conflict |= check_unicity_conflict(schedule, timeslot, stream)        
            # Interference check: no TX and RX for nodes at 1-hop distance in the same timeslot
            # Check TX node for RX neighbors
            err_conflict |= check_interference_conflict(schedule, topology, timeslot, src, 'RX')            
            # Check RX node for TX neighbors
            err_conflict |= check_interference_conflict(schedule, topology, timeslot, dst, 'TX')
                                            
            # Checks evaluation
            if err_conflict:
                print('Cannot schedule stream ' + repr(stream) + ' on timeslot ' + repr(timeslot))
                continue;  #Try to schedule in next timeslot
            else:
                # Adding stream to schedule
                schedule.append((timeslot, src, 'TX'));
                schedule.append((timeslot, dst, 'RX'));
                print('Scheduled stream ' + repr(stream) + ' on timeslot ' + repr(timeslot))
                break;     #Successfully scheduled transmission, break timeslot cycle

    ### Print resulting Schedule
    print('\nResulting schedule')
    print('Time, Node, Activity')
    for x in schedule:
        print(' {}     {}      {}'.format(x[0], x[1], x[2]))

    return schedule;

def draw_graph(topology):
    # Create directed graph (Digraph)
    dot = Graph(format='pdf', comment='Network Topology', strict = True)
    
    # Flatten topology (list of tuples)
    nodes = list(sum(topology, ()))
    # Get unique nodes
    nodes = set(nodes)
    for x in nodes:
        dot.node(repr(x))
    
    ed = [''.join((repr(e[0]), repr(e[1]))) for e in topology]
    dot.edges(ed)
    dot.render('topology.gv', view=True)  # doctest: +SKIP

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument("command", help="examples: plot, run")
    args = parser.parse_args()
    if (args.command == "plot"):
        draw_graph(topology_2)
    if (args.command == "run"):
        scheduler(topology_2, stream_list_3, data_slots_1)
        #scheduler(topology_1, stream_list_2, data_slots_1)
    
