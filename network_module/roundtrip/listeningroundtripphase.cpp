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

#include "listeningroundtripphase.h"
#include "led_bar.h"
#include "../macround/macround.h"
#include <stdio.h>
#include "../debug_settings.h"

namespace miosix{
ListeningRoundtripPhase::~ListeningRoundtripPhase() {
}

void ListeningRoundtripPhase::execute(MACContext& ctx) {
    //TODO add a way to use the syncStatus also with the master for having an optimized receiving window
    //maybe with a different class for the master node?
    long long timeoutTime = globalFirstActivityTime + receiverWindow;
    //Transceiver configured with non strict timeout
    transceiver.configure(*ctx.getTransceiverConfig());
    transceiver.turnOn();
    
    unsigned char packet[askPacketSize];
    auto deepsleepDeadline = globalFirstActivityTime - MediumAccessController::receivingNodeWakeupAdvance;
    if (ENABLE_ROUNDTRIP_INFO_DBG)
        print_dbg("[RTT] WU=%lld TO=%lld\n", deepsleepDeadline, timeoutTime);
    RecvResult result;
    bool success = false;
    
    if(getTime() < deepsleepDeadline)
        pm.deepSleepUntil(deepsleepDeadline);
    greenLed::high();
    for(; !(success || result.error == RecvResult::ErrorCode::TIMEOUT);
            success = isRoundtripAskPacket(result, packet, ctx.getNetworkConfig()->panId, ctx.getHop()))
    {
        try {
            result = transceiver.recv(packet, replyPacketSize, timeoutTime);
        } catch(std::exception& e) {
            if (ENABLE_RADIO_EXCEPTION_DBG)
                print_dbg("%s\n", e.what());
        }
        if (ENABLE_PKT_INFO_DBG)
            if(result.size){
                print_dbg("[RTT] Received packet, error %d, size %d, timestampValid %d: ", result.error, result.size, result.timestampValid);
                if (ENABLE_PKT_DUMP_DBG)
                    memDump(packet, result.size);
            } else print_dbg("[RTT] No packet received, timeout reached\n");
    }
    
    if(success){
        auto replyTime = result.timestamp + replyDelay;
        if (ENABLE_ROUNDTRIP_INFO_DBG)
        print_dbg("[RTT] RT=%lld, LT=%lld\n", result.timestamp, replyTime);
        //TODO sto pacchetto non e` compatibile manco con se stesso, servono header di compatibilita`, indirizzo, etc etc
        LedBar<replyPacketSize> p;
        p.encode(7); //TODO: 7?! should put a significant cumulated RTT here.
        try {
            transceiver.sendAt(p.getPacket(), p.getPacketSize(), replyTime);
        } catch(std::exception& e) {
            if (ENABLE_RADIO_EXCEPTION_DBG)
                print_dbg("%s\n", e.what());
        }
    } else if (ENABLE_ROUNDTRIP_INFO_DBG) {
        print_dbg("[RTT] RT=null\n");
    }
    
    transceiver.turnOff();
    greenLed::low();
}
}

