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

#include "mastermacround.h"
#include "../slots_management/masterslotsnegotiator.h"
#include "../topology_discovery/topology_context.h"

namespace miosix {
    
    MasterMACRound::~MasterMACRound() {
    }

    void MasterMACRound::run(MACContext& ctx) {
        MACRound::run(ctx);
        ctx.setNextRound(new MasterMACRound(ctx, roundStart + roundDuration));
    }

    MACRound* MasterMACRound::MasterMACRoundFactory::create(MACContext& ctx) const {
        return new MasterMACRound(ctx);
    }
    
    SlotsNegotiator* MasterMACRound::MasterMACRoundFactory::getSlotsNegotiator(MACContext& ctx) const {
        return new MasterSlotsNegotiator(ctx, 120, 1);
    }

TopologyContext* MasterMACRound::MasterMACRoundFactory::getTopologyContext(MACContext& ctx) const {
    return ctx.getNetworkConfig()->topologyMode == NetworkConfiguration::TopologyMode::NEIGHBOR_COLLECTION?
                        ((TopologyContext*) new MasterMeshTopologyContext(ctx)):
                        ((TopologyContext*) new MasterTreeTopologyContext(ctx));
}

}

