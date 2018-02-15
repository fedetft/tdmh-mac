/***************************************************************************
 *   Copyright (C)  2017 by Terraneo Federico, Polidori Paolo              *
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

#include "hookingfloodingphase.h"
#include "periodiccheckfloodingphase.h"
#include "../debug_settings.h"

namespace miosix {

HookingFloodingPhase::~HookingFloodingPhase() {
}

void HookingFloodingPhase::execute(MACContext& ctx)
{   
    if (ENABLE_FLOODING_INFO_DBG)
        print_dbg("[F] Resync\n");
    auto* status = ctx.getSyncStatus();
    auto* synchronizer = status->synchronizer;
    synchronizer->reset();
    transceiver.configure(*ctx.getTransceiverConfig());
    transceiver.turnOn();
    unsigned char packet[syncPacketSize];
    auto networkConfig = *ctx.getNetworkConfig();
    //TODO: attach to strongest signal, not just to the first received packet
    RecvResult result;
    ledOn();
    for (bool success = false; !success; success = isSyncPacket(result, packet, networkConfig.panId)) {
        try {
            result = transceiver.recv(packet, syncPacketSize, infiniteTimeout);
        } catch(std::exception& e) {
            if (ENABLE_RADIO_EXCEPTION_DBG)
                print_dbg("%s\n", e.what());
        }
        if (ENABLE_PKT_INFO_DBG)
            if(result.size){
                print_dbg("Received packet, error %d, size %d, timestampValid %d: ", result.error, result.size, result.timestampValid);
                if (ENABLE_PKT_DUMP_DBG)
                    memDump(packet, result.size);
            } else print_dbg("No packet received, timeout reached\n");
    }
    ++packet[2];
    rebroadcast(result.timestamp, packet, networkConfig.maxHops);
    
    ledOff();
    transceiver.turnOff();
    
    if (ENABLE_FLOODING_INFO_DBG)
        print_dbg("[F] ERT=%lld, RT=%lld\n", status->computedFrameStart, result.timestamp);
    
    status->initialize(synchronizer->getReceiverWindow(), result.timestamp);
    ctx.setHop(packet[2]);
    
    //Correct frame start considering hops
    //measuredFrameStart-=hop*packetRebroadcastTime;
    if (ENABLE_FLOODING_INFO_DBG)
        print_dbg("[F] hop=%d, w=%d, rssi=%d\n", packet[2], status->receiverWindow, result.rssi);
}
}

