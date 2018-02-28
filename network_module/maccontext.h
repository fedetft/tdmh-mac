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

#ifndef MACCONTEXT_H
#define MACCONTEXT_H

#include "network_configuration.h"
#include "interfaces-impl/transceiver.h"
#include "topology_discovery/topology_context.h"
#include <type_traits>
#include <limits>

namespace miosix {
    class MACRound;
    class MACRoundFactory;
    class MediumAccessController;
    class SyncStatus;
    class SlotsNegotiator;
    class TopologyContext;
    class MACContext {
    public:
        MACContext() = delete;
        MACContext(const MACContext& orig) = delete;
        MACContext(const MACRoundFactory* const roundFactory, const MediumAccessController& mac, const NetworkConfiguration* const);
        virtual ~MACContext();
        MACRound* getCurrentRound() const { return currentRound; }
        MACRound* getNextRound() const { return nextRound; }
        MACRound* shiftRound();
        inline const MediumAccessController& getMediumAccessController() { return mac; }
        void setHop(unsigned char num) { hop = num; }
        unsigned char getHop() { return hop; }
        void setNextRound(MACRound* round);
        void initializeSyncStatus(SyncStatus* syncStatus);
        inline SyncStatus* getSyncStatus() { return syncStatus; }
        inline const TransceiverConfiguration& getTransceiverConfig() { return transceiverConfig; }
        inline const TransceiverConfiguration getTransceiverConfig(bool crc, bool strictTimeout=true) {
            return TransceiverConfiguration(transceiverConfig.frequency, transceiverConfig.txPower, crc, strictTimeout);
        }
        inline const SlotsNegotiator* getSlotsNegotiator() { return slotsNegotiator; }
        inline const NetworkConfiguration* getNetworkConfig() const { return networkConfig; }
        inline TopologyContext* getTopologyContext() { return topologyContext; }

        unsigned short getNetworkId() const {
            return networkId;
        }

        void setNetworkId(unsigned short networkId) {
            if (!networkConfig->dynamicNetworkId)
                throw std::runtime_error("Cannot dynamically set network id if not explicitly configured");
            this->networkId = networkId;
        }

        template<typename T>
        static typename std::enable_if<std::is_integral<T>::value>::type
        setRightmost(T origValue, T value, unsigned short length) {
            T mask = ~((T) 0);
            mask <<= length;
            return origValue & mask | value & ~mask;
        }
        template<typename T>
        static typename std::enable_if<std::is_integral<T>::value>::type
        setLeftmost(T origValue, T value, unsigned short length) {
            T mask = ~((T) 0);
            mask >>= length;
            origValue &= mask;
            mask = ~mask;
        }
        template<typename T>
        static void bitwisePopulateBitArr(
                typename std::enable_if<std::is_integral<T>::value>::type* arr,
                unsigned short arrLen,
                T value,
                unsigned short startPos,
                unsigned short length) {
            assert(startPos + length < std::numeric_limits<T>::digits * arrLen);
            unsigned short startBit = startPos % (std::numeric_limits<T>::digits);
            unsigned short startIndex = startPos / (std::numeric_limits<T>::digits);
            auto endBit = length  + startBit;
            unsigned short cellsLen = (endBit - 1) / (std::numeric_limits<T>::digits);
            unsigned short trailingBits = endBit % (std::numeric_limits<T>::digits);
            T ones = ~((T) 0);
            arr[startIndex] &= ~(ones >> startBit);
            unsigned short i;
            for (i = startIndex; i < startIndex + cellsLen - 1; i++)
                arr[i] = 0;
            if (cellsLen > 0)
                arr[i] = value >> trailingBits;
            if (cellsLen == 0) {
                T mask = ones >> trailingBits;
                arr[i] = arr[i] & mask | (value << (std::numeric_limits<T>::digits - trailingBits)) & ~mask;
            }
        }

        template<typename T>
        static void bitwisePopulateBitArrBot(
                T* arr,
                unsigned short arrLen,
                T* value,
                unsigned short valLen,
                unsigned short startPos,
                unsigned short length) { //TODO fixme
            assert(startPos + length < (std::numeric_limits<T>::digits) * arrLen);
            unsigned short startBit = startPos % (std::numeric_limits<T>::digits);
            unsigned short startIndex = startPos / (std::numeric_limits<T>::digits);
            auto endBit = length  + startBit;
            unsigned short cellsLen = (endBit - 1) / (std::numeric_limits<T>::digits);
            unsigned short trailingBits = endBit % (std::numeric_limits<T>::digits);
            T ones = ~((T) 0);
            if (cellsLen) { // > 0
                unsigned short j = startIndex + cellsLen;
                arr[j] = (arr[j] & (ones >> trailingBits)) | (value[--valLen] << (std::numeric_limits<T>::digits - trailingBits));
                //cells populated with vals
                for (j--; valLen > 0 && j > startIndex; j--) {
                    arr[j] = (value[valLen] >> trailingBits);
                    arr[j] |=  (value[--valLen] << ((std::numeric_limits<T>::digits) - trailingBits));
                }
                //check if we are at the start cell of arr
                if (valLen) { // > 0
                    arr[j] = (arr[j] & (ones << (std::numeric_limits<T>::digits - startBit))) | (value[valLen] >> trailingBits);
                } else { // <=> valLen == 0: clear the first arr cell
                    arr[startIndex] &= ones << (std::numeric_limits<T>::digits - startBit);
                    if (j > startIndex) { //first value cell in middle of arr
                        arr[j--] = value[0] >> trailingBits;
                        for (; j > startIndex; j--) //clear the non-populated cells
                            arr[j] = 0;
                    } else //first value cell, first arr cell
                        arr[startIndex] |= value[0] >> trailingBits;
                }
            } else { //single cell impacted
                T mask = ones << (std::numeric_limits<T>::digits - length) >> startBit;
                arr[startIndex] = (arr[startIndex] & ~mask) | ((value[valLen - 1] << trailingBits) & mask);
            }
        }

        template<typename T>
        static void bitwisePopulateBitArrTop(
                typename std::enable_if<std::is_integral<T>::value, T>::type* arr,
                unsigned short arrLen,
                typename std::enable_if<std::is_integral<T>::value, T>::type* value,
                unsigned short valLen,
                unsigned short startPos,
                unsigned short length) {
            assert(startPos + length < std::numeric_limits<T>::digits * arrLen);
            unsigned short startBit = startPos % (std::numeric_limits<T>::digits);
            unsigned short startIndex = startPos / (std::numeric_limits<T>::digits);
            auto endBit = length  + startBit;
            unsigned short cellsLen = (endBit - 1) / (std::numeric_limits<T>::digits);
            auto lastCell = startIndex + cellsLen;
            unsigned short trailingBits = endBit % (std::numeric_limits<T>::digits);
            T ones = ~((T) 0);
            auto leftShiftCount = std::numeric_limits<T>::digits - startBit;
            if (cellsLen) { // > 0
                unsigned short j = startIndex;
                unsigned short i = 0;
                arr[j] = (arr[j] & (ones << leftShiftCount)) | //masking the bits to keep
                        (value[i] >> startBit);
                //cells populated with vals
                for (j++; i < valLen && j < lastCell; j++) {
                    arr[j] = value[i] << leftShiftCount; //preceding value
                    arr[j] |= value[++i] >> startBit;
                }
                //check if we are at the last cell of arr
                T remMask = ones >> trailingBits;
                if (i < valLen) {
                    arr[j] = (arr[j] & (remMask)) | (((value[i] << leftShiftCount ) | (value[i + 1] >> startBit)) & ~remMask);
                } else { // <=> i == (valLen - 1): clear the last arr cell
                    arr[lastCell] &= remMask;
                    if (j < lastCell) { //last value cell in middle of arr
                        arr[j++] = value[i] << leftShiftCount;
                        for (; j < lastCell; j++) //clear the non-populated cells
                            arr[j] = 0;
                    } else //last value cell, last arr cell
                        arr[lastCell] |= (value[i] << leftShiftCount) & ~remMask;
                }
            } else { //single cell of arr impacted
                T mask = ones << (std::numeric_limits<T>::digits - length) >> startBit;
                arr[startIndex] = (arr[startIndex] & ~mask) |
                        ((value[0] >> startBit) & mask);
            }
        }

        template<typename T>
        static void bitwisePopulateBitArrTop(
                typename std::enable_if<std::is_integral<T>::value, T>::type* arr,
                unsigned short arrLen,
                typename std::enable_if<std::is_integral<T>::value, T>::type* value,
                unsigned short valLen,
                unsigned short startPos,
                unsigned short length,
                unsigned short valStartPos) { //TODO fixme
            assert(startPos + length < std::numeric_limits<T>::digits * arrLen);
            unsigned short startBit = startPos % (std::numeric_limits<T>::digits);
            unsigned short startIndex = startPos / (std::numeric_limits<T>::digits);
            auto endBit = length  + startBit;
            unsigned short cellsLen = (endBit - 1) / (std::numeric_limits<T>::digits);
            auto lastCell = startIndex + cellsLen;
            unsigned short trailingBits = endBit % (std::numeric_limits<T>::digits);
            T ones = ~((T) 0);
            auto leftShiftCount = std::numeric_limits<T>::digits - startBit;
            unsigned short valStartBit = valStartPos % (std::numeric_limits<T>::digits);
            unsigned short valEndBit = std::numeric_limits<T>::digits - valStartBit;
            if (cellsLen) { // > 0
                unsigned short j = startIndex;
                unsigned short i = valStartPos / (std::numeric_limits<T>::digits);
                arr[j] = (arr[j] & (ones << leftShiftCount)) | //masking the bits to keep
                        (value[i] << valStartBit | ((i < valLen - 1)? value[i + 1] : 0) >> valEndBit) >> startBit;
                //cells populated with vals
                for (j++; i < (valLen - 1) && j < lastCell; j++) {
                    arr[j] = value[i] << (valStartBit + leftShiftCount);
                    arr[j] |= value[++i] >> (valEndBit + startBit);
                }
                //check if we are at the last cell of arr
                if (i < (valLen - 1)) {
                    arr[j] = (arr[j] & (ones >> trailingBits)) | (value[i] << (valStartBit + leftShiftCount));
                } else { // <=> i == (valLen - 1): clear the last arr cell
                    arr[lastCell] &= ones >> trailingBits;
                    if (j < lastCell) { //last value cell in middle of arr
                        arr[j++] = value[i] >> (valEndBit + startBit);
                        for (; j < lastCell; j++) //clear the non-populated cells
                            arr[j] = 0;
                    } else //last value cell, last arr cell
                        arr[lastCell] |= value[i] >> (valEndBit + startBit);
                }
            } else { //single cell of arr impacted
                T mask = ones << (std::numeric_limits<T>::digits - length) >> startBit;
                arr[startIndex] = (arr[startIndex] & ~mask) |
                        (value[0] << (valStartBit + trailingBits) |
                                (((valLen)? value[1] : 0) >> valEndBit << trailingBits) & mask);
            }
        }


        template<typename T, typename V>
        static void bitwisePopulateBitArr(
                typename std::enable_if<std::is_integral<T>::value>::type* arr,
                unsigned short arrLen,
                typename std::enable_if<std::is_integral<V>::value>::type* value,
                unsigned short valLen,
                unsigned short startPos,
                unsigned short length) {
            if (sizeof(T) < sizeof(V)) //downscaling to array of smaller data
                bitwisePopulateBitArr(arr, arrLen, ((T*) value), valLen * sizeof(V) / sizeof(T), length);
            else
                bitwisePopulateBitArr(((V*) arr), arrLen * sizeof(T) / sizeof(V), value, valLen, length);
        }

        template<typename T, typename std::enable_if<std::is_integral<T>::value, T>::type = 0>
        static unsigned char bitsForRepresentingCount(T count) {
            T temp = count;
            unsigned char retval = 0;
            while (temp >>= 1) ++retval;
            return count ^ (1 << retval)? retval + 1: retval;
        }
        static const unsigned short maxPacketSize = 127;
    private:
        unsigned char hop;
        const MediumAccessController& mac;
        SyncStatus* syncStatus;
        const TransceiverConfiguration transceiverConfig;
        const NetworkConfiguration* const networkConfig;
        TopologyContext* const topologyContext;
        MACRound* currentRound = nullptr;
        MACRound* nextRound = nullptr;
        SlotsNegotiator* slotsNegotiator;
        unsigned short networkId;
    };
}

#endif /* MACCONTEXT_H */

