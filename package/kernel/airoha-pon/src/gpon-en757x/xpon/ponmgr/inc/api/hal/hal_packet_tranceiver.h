/*
 * hal_raw_socket_tranceiver.h
 *
 *  Created on: Jan 2, 2014
 *      Author: root
 */

#ifndef HAL_PACKET_TRANCEIVER_H_
#define HAL_PACKET_TRANCEIVER_H_

#include "hal_mdu_typedef.h"

struct PacketTranceiverImpl;

struct PacketTranceiver
{
	PacketTranceiver();
	~PacketTranceiver();
	bool sendRequest(MduMsgType msgType, char* buffer, int size);
	bool shouldReceiveResponse(MduMsgType msgType);

private:
	PacketTranceiverImpl* This;
};


#endif /* HAL_PACKET_TRANCEIVER_H_ */
