/*
 * message_slot.h
 *
 *  Created on: 6 Dec 2017
 *      Author: ranerez
 */

#ifndef MESSAGE_SLOT_H_
#define MESSAGE_SLOT_H_

#include <linux/ioctl.h>


#define MAJOR_NUM 244
#define MSG_SLOT_CHANNEL 1
#define BUF_LEN 128

#define DEVICE_RANGE_NAME "messageSlot"
#define DEVICE_FILE_NAME "DevFileName"


 /*
* The Message Slot implementation is based on a Linked list
* The Msgslot struct is a linked list of Message Slot, for each minor a new element is added
* The msg struct is another linked list of messages, altough 1 linked list would have been sufficient, I decided to use 2
* using another linked list in the msg struct enables me to hold all messages and filter by channel ID
 */

 typedef struct msg_t
 {
 	char message[BUF_LEN]; //Fixed length message buffer
 	int channelID; // The channel ID received from ioctl
 	int messageSize; // the actual message size
 	struct msg_t * next; // a pointer to the next message in the Message slot
 }msg;

 typedef struct msgSlot_t
 {
 	int minorNum; // The unique minor ID per Message Slot
 	int numOfMsgs; // Number of total messages contained in the Message Slot (regardless of channels)
 	struct msgSlot_t * next; // A pointer to the next Message Slot struct if exists
 	msg * allMessages; // a Pointer to the first message received in this slot (Beginning of a linked list)
 }Msgslot;

#endif
