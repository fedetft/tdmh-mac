/***************************************************************************
 *   Copyright (C)  2017 by Polidori Paolo, Terraneo Federico,             *
 *                          Riccardi Fabiano                               *
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

#include "roundtrip_subphase.h"
#include "../../../mac_context.h"

namespace mxnet {
class ListeningRoundtripPhase : public RoundtripSubphase  {
public:
    explicit ListeningRoundtripPhase(MACContext& ctx) : RoundtripSubphase(ctx) {};
    ListeningRoundtripPhase() = delete;
    ListeningRoundtripPhase(const ListeningRoundtripPhase& orig) = delete;
    virtual ~ListeningRoundtripPhase() {};
    void execute(long long slotStart) override;
    /*
private:
    bool isRoundtripAskPacket(const Packet& packet, miosix::RecvResult rcvResult) {
        throw std::runtime_error("ListeningRoundtripPhase::execute: This code should never be called");
        
        auto panId = ctx.getNetworkConfig().getPanId();
        return rcvResult.error == miosix::RecvResult::OK && rcvResult.timestampValid
                && rcvResult.size == askPacketSize
                && packet[0] == 0x46 && packet[1] == 0x08
                && packet[2] == ctx.getHop() + 1
                && packet[3] == static_cast<unsigned char> (panId >> 8)
                && packet[4] == static_cast<unsigned char> (panId & 0xff)
                && packet[5] == 0xff && packet[6] == 0xff;
        
    }*/

};
}

