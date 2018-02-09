//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "NodeBase.h"
#include <list>
#include <vector>
#include <algorithm>
#include <cstring>

const std::string NodeBase::timeoutPktName = "TIMEOUT";

void NodeBase::waitAndDeletePackets(simtime_t timeDelta)
{
    cQueue queue;
    waitAndEnqueue(timeDelta,&queue);
    //TODO: is this efficient? And most important, why can't they use std::list?
    while(!queue.isEmpty()) delete queue.pop();
}

 RecvResult NodeBase::receive(void* packet, int size, simtime_t timeout, bool strictTimeout) {
    RecvResult result;
    auto waitDelta = timeout - simTime();
    auto waitDeltaNs = waitDelta.inUnit(SIMTIME_NS);
    EV_INFO << "Awaiting packet for " << waitDeltaNs << " (until " << timeout.inUnit(SIMTIME_NS) << ") ns" << endl;
    scheduleAt(timeout, &timeoutMsg);
    cQueue interferringMsgs, collidingMsgs;
    cMessage *msg = cSimpleModule::receive(waitDelta);
    if (msg->isSelfMessage() && msg->getName() == timeoutPktName.c_str() && msg->getKind() == KIND_TIMEOUT) {
        result.error = RecvResult::TIMEOUT;
        delete msg;
        return result; //no message received
    }
    cancelEvent(&timeoutMsg);
    if (msg == nullptr || !msg->isPacket()) {
        result.error = RecvResult::TIMEOUT;
        delete msg;
        return result; //no message received
    }
    auto pkt = (cPacket*) msg;
    auto msgDataTimeNs =  pkt->getBitLength() * 4000;
    result.timestamp = msg->getSendingTime().inUnit(SIMTIME_NS);
    result.timestampValid = true;
    if (SimTime((strictTimeout? 0 : msgDataTimeNs) + preambleSfdTimeNs, SIMTIME_NS) + msg->getSendingTime() > timeout) {
        result.error = RecvResult::TIMEOUT;
        delete msg;
        return result;//packet received but exceeds the timeout
    }
    //wait for the max confidence time to obtain a constructive interference
    EV_INFO << "Awaiting interfering packets for " << constructiveInterferenceTimeNs << " (until " << simTime().inUnit(SIMTIME_NS) + constructiveInterferenceTimeNs << ")" << endl;
    waitAndEnqueue(SimTime(constructiveInterferenceTimeNs, SIMTIME_NS), &interferringMsgs);
    //and wait for the whole message length
    auto msgDeadlineDelta = msgDataTimeNs + preambleSfdTimeNs - constructiveInterferenceTimeNs;
    EV_INFO << "Awaiting for the whole message to arrive for " << msgDeadlineDelta << " (until " << simTime().inUnit(SIMTIME_NS) + msgDataTimeNs + preambleSfdTimeNs << ")" << endl;
    waitAndEnqueue(SimTime(msgDeadlineDelta, SIMTIME_NS), &collidingMsgs);
    if (!collidingMsgs.isEmpty()) {
        //TODO add CRC fail if CRC enabled, else return random bytes array of random length
        result.error = RecvResult::CRC_FAIL;
        delete msg;
        while(!collidingMsgs.isEmpty()) delete collidingMsgs.pop();
        while(!interferringMsgs.isEmpty()) delete interferringMsgs.pop();
        return result; //message collided and arrived corrupted
    }
    result.error = RecvResult::OK;
    int corrLen = pkt->getByteLength();
    result.size = corrLen;
    unsigned char* correlated;
    if (!interferringMsgs.isEmpty()) {
        std::vector<cPacket*> packets;
        std::list<int> lengths;
        packets.push_back(pkt);
        lengths.push_back(corrLen);
        for (cQueue::Iterator it(interferringMsgs); !it.end(); it++){
            if (((cMessage*) *it)->isPacket()){
                packets.push_back((cPacket*) *it);
                auto len = ((cPacket*) *it)->getByteLength();
                if (len > corrLen) corrLen = len;
                lengths.push_back(len);
            }
        }
        correlated = new unsigned char[corrLen];
        //warning: supposing that if interfering, no timing offset in packet symbols is involved
        // and also it is avoided to correlate the bytes by their PN sequence.
        //Only the theoretical results of Glossy are taken into account.
        for(int i = 0; i < corrLen; i++) {
            auto numPkts = std::count_if(lengths.begin(), lengths.end(), [i](int e){return e > i;});
            auto chosenPkt = uniform(0, numPkts, 0);
            auto actualPkt = -1;
            int j = 0;
            for (auto len = lengths.begin(); j <= chosenPkt; len++) {
                if (*len > i) j++;
                actualPkt++;
            }
            correlated[i] = static_cast<unsigned char*>(packets[actualPkt]->getContextPointer())[i];
        }
        while(!interferringMsgs.isEmpty()) delete interferringMsgs.pop();
    } else {
        correlated = new unsigned char[corrLen];
        auto* ctx = pkt->getContextPointer();
        memcpy(correlated, ctx, corrLen);
    }
    delete msg;
    if (corrLen > size) {
        result.error = RecvResult::TOO_LONG;
        return result;
    }
    memcpy(packet, correlated, std::min(size, corrLen));
    if (size > corrLen) memset(((unsigned char*) packet) + corrLen, 0, size - corrLen);
    return result;
}

void NodeBase::sendAt(void* packet, int size, simtime_t when, std::string pktName) {
    waitAndDeletePackets(when - simTime());
    cPacket* pkt;
    for(int i=0; i<gateSize("wireless"); i++){
        pkt = new cPacket(pktName.c_str());
        pkt->setByteLength(size);
        void* data = new unsigned char[size];
        memcpy(data, packet, size);
        pkt->setContextPointer(data);
        send(pkt, "wireless$o",i);
    }
    EV_INFO << "starting to send packet " << simTime().inUnit(SIMTIME_NS) << endl;
    waitAndDeletePackets(SimTime(pkt->getBitLength() * 4000 + preambleSfdTimeNs, SIMTIME_NS));
    EV_INFO << "finishing to send packet " << simTime().inUnit(SIMTIME_NS) << endl;
}