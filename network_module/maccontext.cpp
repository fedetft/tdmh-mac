/***************************************************************************
 *   Copyright (C)  2017 by Polidori Paolo                                 *
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

#include "maccontext.h"
#include "macround/macroundfactory.h"
#include "flooding/syncstatus.h"
#include "slots_management/slotsnegotiator.h"
#include "interfaces-impl/transceiver.h"

namespace miosix {
    
    MACContext::MACContext(const MACRoundFactory* const roundFactory, const miosix::MediumAccessController& mac, const NetworkConfiguration* const config) :
            mac(mac),
            transceiverConfig(new TransceiverConfiguration(config->baseFrequency, config->txPower, true, false)),
            currentRound(roundFactory->create(*this)),
            slotsNegotiator(roundFactory->getSlotsNegotiator(*this)),
            networkConfig(config) {
        delete roundFactory;
    }

    MACContext::~MACContext() {
        delete currentRound;
        delete nextRound;
        delete transceiverConfig;
        delete syncStatus;
    }

    MACRound* MACContext::shiftRound() {
        delete currentRound;
        currentRound = nextRound;
        nextRound = nullptr;
        return currentRound;
    }
    
    void MACContext::setNextRound(MACRound* round) {
        delete nextRound;
        nextRound = round;
    }
    
    void MACContext::initializeSyncStatus(SyncStatus* syncStatus) { this->syncStatus = syncStatus; }

}
