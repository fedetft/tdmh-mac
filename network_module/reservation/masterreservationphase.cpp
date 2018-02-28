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

#include "masterreservationphase.h"
#include <vector>

namespace miosix {
    MasterReservationPhase::~MasterReservationPhase() {
    }
    void MasterReservationPhase::execute(MACContext& ctx)
    {
        transceiver.configure(ctx.getTransceiverConfig(false, false));
        transceiver.turnOn();
        receiveOrGetEmpty(ctx);
        transceiver.turnOff();
        ledOff();
    }
    
    std::vector<unsigned char>* MasterReservationPhase::getUpstreamReservations() {
        auto* slots = new std::vector<unsigned char> (240);
        unsigned char o = 0;
        for (unsigned char i = 1, j = 7; j < reservationPacketSize; i+=2, j++) {
            if(packet[j] & 0xF0){
                (*slots)[o++] = i; 
            }
            if(packet[j] & 0x0F){
                (*slots)[o++] = i+1;
            }
        }
        slots->resize(o);
        return slots;
    }

}


