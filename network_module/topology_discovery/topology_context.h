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

#ifndef NETWORK_MODULE_TOPOLOGY_DISCOVERY_TOPOLOGY_CONTEXT_H_
#define NETWORK_MODULE_TOPOLOGY_DISCOVERY_TOPOLOGY_CONTEXT_H_

#include "topology_message.h"
#include "topology_map.h"
#include "../resettable_priority_queue.h"
#include "../network_configuration.h"
#include <map>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace miosix {

class MACContext;
class TopologyContext {
public:
    TopologyContext(MACContext& ctx) : ctx(ctx) {};
    virtual ~TopologyContext() {
        // TODO Auto-generated destructor stub
    }

    virtual NetworkConfiguration::TopologyMode getTopologyType()=0;
    virtual unsigned short receivedMessage(unsigned char* pkt, unsigned short len, unsigned short nodeIdByTopologySlot, short rssi)=0;
    virtual void unreceivedMessage(unsigned short nodeIdByTopologySlot)=0;

protected:
    MACContext& ctx;
};

class MasterTopologyContext : public TopologyContext {
public:
    MasterTopologyContext(MACContext& ctx) : TopologyContext(ctx) {};
    virtual ~MasterTopologyContext() {
        // TODO Auto-generated destructor stub
    }
protected:
    TopologyMap<unsigned short> topology;
    std::map<std::pair<unsigned short, unsigned short>, unsigned short> streams; //src, dst --> dataRate
};

class DynamicTopologyContext : public TopologyContext {
public:
    DynamicTopologyContext(MACContext& ctx) : TopologyContext(ctx) {};
    virtual ~DynamicTopologyContext() {
        // TODO Auto-generated destructor stub
    }

    virtual std::vector<TopologyMessage*> dequeueMessages(unsigned short count)=0;
    virtual TopologyMessage* getMyTopologyMessage()=0;
    virtual unsigned short getBestPredecessor();
    virtual bool hasPredecessor();
    ResettablePriorityQueue<std::pair<unsigned short, unsigned short>, std::tuple<unsigned short, unsigned short, unsigned short>> enqueuedSMEs; //src, dst --> dataRate

protected:
    ResettablePriorityQueue<unsigned short, TopologyMessage*> enqueuedTopologyMessages;
    std::map<unsigned short, std::pair<short, unsigned short>> predecessorsRSSIUnseenSince;
    std::map<unsigned short, unsigned short> neighborsUnseenSince;
    struct CompareRSSI {
        CompareRSSI(unsigned short maxUnseenSince) : mus(maxUnseenSince) {}
        bool operator()(const std::pair<unsigned short, std::pair<short, unsigned short>>& left,
                const std::pair<unsigned short, std::pair<short, unsigned short>>& right) const {
            return (left.second.second < mus && right.second.second < mus) ||
                    left.second.second < mus;
        }
        unsigned short mus;
    };
};

class DynamicMeshTopologyContext : public DynamicTopologyContext {
public:
    DynamicMeshTopologyContext(MACContext& ctx) : DynamicTopologyContext(ctx) {};
    virtual ~DynamicMeshTopologyContext() {}
    virtual NetworkConfiguration::TopologyMode getTopologyType() {
        return NetworkConfiguration::TopologyMode::NEIGHBOR_COLLECTION;
    }
    virtual unsigned short receivedMessage(unsigned char* pkt, unsigned short len, unsigned short nodeIdByTopologySlot, short rssi);
    virtual void unreceivedMessage(unsigned short nodeIdByTopologySlot);
    virtual std::vector<TopologyMessage*> dequeueMessages(unsigned short count);
    virtual TopologyMessage* getMyTopologyMessage();
protected:
    virtual void checkEnqueueOrUpdate(NeighborMessage* msg);
};

class MasterMeshTopologyContext : public MasterTopologyContext {
public:
    MasterMeshTopologyContext(MACContext& ctx) : MasterTopologyContext(ctx) {};
    virtual ~MasterMeshTopologyContext() {}

    virtual NetworkConfiguration::TopologyMode getTopologyType() {
        return NetworkConfiguration::TopologyMode::NEIGHBOR_COLLECTION;
    }
    virtual unsigned short receivedMessage(unsigned char* pkt, unsigned short len, unsigned short nodeIdByTopologySlot, short rssi);
    virtual void unreceivedMessage(unsigned short nodeIdByTopologySlot);
    virtual void print();

};


class DynamicTreeTopologyContext : public DynamicTopologyContext {
public:
    DynamicTreeTopologyContext(MACContext& ctx) : DynamicTopologyContext(ctx) { throw std::runtime_error("not implemented, do it!"); }
    virtual ~DynamicTreeTopologyContext() {}
    virtual NetworkConfiguration::TopologyMode getTopologyType() {
        return NetworkConfiguration::TopologyMode::ROUTING_VECTOR;
    }
    virtual unsigned short receivedMessage(unsigned char* pkt, unsigned short len, unsigned short nodeIdByTopologySlot, short rssi) {};
    virtual void unreceivedMessage(unsigned short nodeIdByTopologySlot) {};
    virtual std::vector<TopologyMessage*> dequeueMessages(unsigned short count) { return std::vector<TopologyMessage*>(); }
    virtual TopologyMessage* getMyTopologyMessage() { return nullptr; }
};


class MasterTreeTopologyContext : public DynamicTopologyContext {
public:
    MasterTreeTopologyContext(MACContext& ctx) : DynamicTopologyContext(ctx) { throw std::runtime_error("not implemented, do it!"); }
    virtual ~MasterTreeTopologyContext() {}
    virtual NetworkConfiguration::TopologyMode getTopologyType() {
        return NetworkConfiguration::TopologyMode::ROUTING_VECTOR;
    }
    virtual unsigned short receivedMessage(unsigned char* pkt, unsigned short len, unsigned short nodeIdByTopologySlot, short rssi) {};
    virtual void unreceivedMessage(unsigned short nodeIdByTopologySlot) {};
    virtual std::vector<TopologyMessage*> dequeueMessages(unsigned short count) { return std::vector<TopologyMessage*>(); }
    virtual TopologyMessage* getMyTopologyMessage() { return nullptr; }
};


} /* namespace miosix */

#endif /* NETWORK_MODULE_TOPOLOGY_DISCOVERY_TOPOLOGY_CONTEXT_H_ */
