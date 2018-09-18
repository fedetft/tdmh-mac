/***************************************************************************
 *   Copyright (C)  2018 by Polidori Paolo                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#pragma once

#include "neighbor.h"
#include "topology_message.h"
#include "../uplink_message.h"
#include "../../updatable_queue.h"
#include "topology_map.h"
#include <list>
#include <algorithm>

namespace mxnet {

class MACContext;

/**
 * Class storing the status of the topology information received and collected by the other nodes
 */
class TopologyContext {
public:
    TopologyContext() = delete;
    TopologyContext(MACContext& ctx) : ctx(ctx) {};
    virtual ~TopologyContext() {}

    /**
     * Checks wether a node is a successor of the current node
     * @param nodeId the id of the successor
     * @return if such node is a successor for the current one
     */
    bool hasSuccessor(unsigned char nodeId) {
        return std::find_if(successors.begin(), successors.end(), [nodeId](Successor s){
            return s.getNodeId() == nodeId;
        }) != successors.end();
    }
    
    /**
     * Checks wether a node is a successor of another node
     * @param node1 and node2: id of the two nodes
     * @return if such node is a successor for the current one
     */
     bool areSuccessors(unsigned char node1, unsigned char node2) {
         return std::find_if(successors.begin(), successors.end(), [=](Successor s){
             return s.getNodeId() == node2;
         }) != successors.end();
     }

    /**
     * Returns a vector of the neighbors in the current topology
     * @param nodeId the id of the current node
     * @return a vector containing the neighbors of the node
     */
     std::vector<unsigned char> getNeighbors(unsigned char nodeId) {
         std::vector<unsigned char> neighbors;
         for(auto s : successors){
             neighbors.push_back(s.getNodeId());
         }
         return neighbors;
     }

    /**
     * @return the TopologyMode in use by the network
     */
    virtual NetworkConfiguration::TopologyMode getTopologyType() const = 0;

    /**
     * Updates the context with the content of the uplink message,
     * managing the list of successors.
     * @param msg the received message
     * @param sender the network id of the node to which the uplink slot is assigned to
     * @param rssi the rssi measured while receiving the message
     */
    virtual void receivedMessage(UplinkMessage msg, unsigned char sender, short rssi);

    /**
     * Updates the context by marking a message not received during the slot assigned to a given node,
     * managing the list of successors.
     * @param sender the network id of the node to which the uplink slot is assigned to
     */
    virtual void unreceivedMessage(unsigned char sender);

protected:
    MACContext& ctx;
    std::list<Successor> successors;
};

class MasterTopologyContext : public TopologyContext {
public:
    MasterTopologyContext() = delete;
    MasterTopologyContext(MACContext& ctx) : TopologyContext(ctx) {};
    virtual ~MasterTopologyContext() {}

    /**
     * Updates the context with the content of the uplink message,
     * adding it to the list of neighbors or refreshing its state.
     * The base class method is also called, see TopologyContext::receivedMessage
     * @param msg the received message
     * @param sender the network id of the node to which the uplink slot is assigned to
     * @param rssi the rssi measured while receiving the message
     */
    void receivedMessage(UplinkMessage msg, unsigned char sender, short rssi) override;

    /**
     * Updates the context by marking a message not received during the slot assigned to a given node,
     * managing its presence in the list of directly connectedneighbors.
     * The base class method is also called, see TopologyContext::unreceivedMessage
     * @param sender the network id of the node to which the uplink slot is assigned to
     */
    void unreceivedMessage(unsigned char sender) override;
protected:
    std::map<unsigned char, unsigned char> neighborsUnseenFor;
    TopologyMap<unsigned char> topology;
};

class DynamicTopologyContext : public TopologyContext {
public:
    DynamicTopologyContext() = delete;
    DynamicTopologyContext(MACContext& ctx) : TopologyContext(ctx) {};
    virtual ~DynamicTopologyContext() {}

    /**
     * Updates the context with the content of the uplink message,
     * adding it to the list of predecessor or managing its state if belonging to the previous hop.
     * The base class method is also called, see TopologyContext::receivedMessage
     * @param msg the received message
     * @param sender the network id of the node to which the uplink slot is assigned to
     * @param rssi the rssi measured while receiving the message
     */
    void receivedMessage(UplinkMessage msg, unsigned char sender, short rssi) override;

    /**
     * Updates the context by marking a message not received during the slot assigned to a given node,
     * managing its presence in the list of predecessor, if the message comes from the previous hop.
     * The base class method is also called, see TopologyContext::unreceivedMessage
     * @param sender the network id of the node to which the uplink slot is assigned to
     */
    void unreceivedMessage(unsigned char sender) override;

    /**
     * Removes from the queue a given amount of TopologyElement, in order to be forwarded
     * @param count the number of elements to pull
     * @return the list of removed elements
     */
    std::vector<TopologyElement*> dequeueMessages(std::size_t count);

    /**
     * Returns the topology information to send in the uplink message
     */
    virtual TopologyMessage* getMyTopologyMessage() = 0;

    /**
     * @return the network id of the predecessor, chosen using the rssi as metric
     */
    virtual unsigned char getBestPredecessor();

    /**
     * @return if the node has a predecessor
     */
    virtual bool hasPredecessor();

    /**
     * Changes the hop to which the neighbor belong. Happens only after a resync
     */
    virtual void changeHop(unsigned hop);

protected:
    UpdatableQueue<unsigned char, TopologyElement*> enqueuedTopologyMessages;
    std::list<Predecessor> predecessors;
};

} /* namespace mxnet */
