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
#include "RadioMessage.h"
#include <list>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

void NodeBase::waitAndDeletePackets(simtime_t timeDelta)
{
    cQueue queue;
    waitAndEnqueue(timeDelta,&queue);
    //TODO: is this efficient? And most important, why can't they use std::list?
    while(!queue.isEmpty()) delete queue.pop();
}

void NodeBase::nanoSleep(long long delta) {
    waitAndDeletePackets(SimTime(delta, SIMTIME_NS));
}

void NodeBase::nanoSleepUntil(long long when) {
    waitAndDeletePackets(simTime() + SimTime(delta, SIMTIME_NS));
}

void NodeBase::deepSleep(long long delta) {
    waitAndDeletePackets(SimTime(delta, SIMTIME_NS));
}

void NodeBase::deepSleepUntil(long long when) {
    waitAndDeletePackets(simTime() + SimTime(delta, SIMTIME_NS));
}
