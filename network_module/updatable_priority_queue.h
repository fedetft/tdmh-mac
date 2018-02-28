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

#ifndef NETWORK_MODULE_UPDATABLE_PRIORITY_QUEUE_H_
#define NETWORK_MODULE_UPDATABLE_PRIORITY_QUEUE_H_

#include <utility>
#include <map>

namespace miosix {

template<class keyType, class valType, class prioType>
class UpdatablePriorityQueue {
public:
    UpdatablePriorityQueue() {
        // TODO Auto-generated constructor stub

    }
    virtual ~UpdatablePriorityQueue() {
        // TODO Auto-generated destructor stub
    }
    typename std::map<keyType, valType>::iterator dequeue();
    std::pair<prioType, valType&> top();
    bool enqueue(keyType key, prioType priority, const valType& val);
    bool removeElement(keyType key);
    bool update(keyType key, const valType& val);
    bool update(keyType key, prioType priority);
    bool update(keyType key, prioType priority, const valType& val);
    std::pair<prioType, valType&> getByKey(keyType key);
    bool hasKey(keyType key);
    bool isEmpty();
private:
    std::map<keyType, valType> data;
    std::map<keyType, prioType> elemPrio;
    std::multimap<prioType, keyType, std::greater<prioType>> queues;
};

template<class keyType, class valType, class prioType>
typename std::map<keyType, valType>::iterator UpdatablePriorityQueue<keyType, valType, prioType>::dequeue() {
    if (queues.empty()) return data.end();
    std::pair<prioType, keyType> firstElem = queues.begin();
    keyType elem = firstElem->second;
    elemPrio.erase(elem);
    queues.erase(firstElem);
    auto retVal = data.find(elem);
    data.erase(elem);
    return retVal;
}

template<class keyType, class valType, class prioType>
std::pair<prioType, valType&> UpdatablePriorityQueue<keyType, valType, prioType>::top() {
    if (queues.empty()) throw new std::runtime_error("empty");
    auto firstElem = queues.begin();
    return std::make_pair<prioType, valType&>(firstElem->first, data[firstElem->second]);
}

template<class keyType, class valType, class prioType>
bool miosix::UpdatablePriorityQueue<keyType, valType, prioType>::enqueue(keyType key, prioType priority, const valType& val) {
    if(!hasKey(key)) return false;
    queues.insert(std::make_pair(priority,  key));
    elemPrio[key] = priority;
    data[key] = val;
    return true;
}

template<class keyType, class valType, class prioType>
bool miosix::UpdatablePriorityQueue<keyType, valType, prioType>::removeElement(keyType key) {
    if(!hasKey(key)) return false;
    prioType prio = elemPrio[key];
    elemPrio.erase(key);
    queues.erase(std::make_pair(prio, key));
    data.erase(key);
    return true;
}

template<class keyType, class valType, class prioType>
bool miosix::UpdatablePriorityQueue<keyType, valType, prioType>::update(keyType key, const valType& val) {
    if(!hasKey(key)) return false;
    data[key] = val;
    return true;
}

template<class keyType, class valType, class prioType>
bool miosix::UpdatablePriorityQueue<keyType, valType, prioType>::update(keyType key, prioType priority, const valType& val) {
    if(!hasKey(key)) return false;
    prioType oldPrio = elemPrio[key];
    queues.erase(std::make_pair(oldPrio, key));
    queues.insert(std::make_pair(priority,  key));
    data[key] = val;
    return true;
}

template<class keyType, class valType, class prioType>
bool miosix::UpdatablePriorityQueue<keyType, valType, prioType>::update(keyType key, prioType priority) {
    if(!hasKey(key)) return false;
    prioType oldPrio = elemPrio[key];
    queues.erase(std::make_pair(oldPrio, key));
    queues.insert(std::make_pair(priority,  key));
    return true;
}

template<class keyType, class valType, class prioType>
inline std::pair<prioType, valType&> miosix::UpdatablePriorityQueue<keyType, valType, prioType>::getByKey(keyType key) {
    if (!hasKey(key)) throw new std::runtime_error("empty");
    return std::make_pair(elemPrio[key], data[key]);
}

template<class keyType, class valType, class prioType>
inline bool UpdatablePriorityQueue<keyType, valType, prioType>::hasKey(keyType key) {
    return data.count(key);
}

template<class keyType, class valType, class prioType>
bool UpdatablePriorityQueue<keyType, valType, prioType>::isEmpty() {
    return data.size() == 0;
}

} /* namespace miosix */

#endif /* NETWORK_MODULE_UPDATABLE_PRIORITY_QUEUE_H_ */
