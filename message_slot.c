#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE


#include <linux/kernel.h>   
#include <linux/module.h>   
#include <linux/fs.h>       
#include <asm/uaccess.h>    
#include <linux/string.h>   
#include <linux/slab.h>
#include "message_slot.h"

MODULE_LICENSE("GPL");
static Msgslot* GlobalMessageSlot = NULL; // Global Variable to hold our Message slot head in a linked list


// HELPER FUNCTIONS

/*
* Function to create a new Message Slot based on the received minor number
* @param int minorNum- signifies the minor number of the character device file
* @ret Msgslot* a pointer to the created Message Slot or NULL in case of failure
*/


static Msgslot* createMessageSlot(int minorNum){
	Msgslot* res = NULL;
	res = (Msgslot*)kmalloc(sizeof(Msgslot), GFP_KERNEL);
	if(res == NULL){
		return NULL;
	}
	res->minorNum = minorNum;
	res->numOfMsgs = 0;
	res-> allMessages = NULL;
	res-> next = NULL;

	return res;
}

/*
* Function to create a new message within a Message Slot. 
* @param int channelID- Which channel to create the message on
* @ param const char message[] - the actual message to be written to the channel
* @ ret msg* a pointer to the created message or NULL in case of failure 
*/

static msg* createNewMessage(int channelID, const char message[]){
	msg* res = NULL;
	res = (msg*)kmalloc(sizeof(msg), GFP_KERNEL);
	if(res == NULL){
		return NULL;
	}
	res->channelID = channelID;
	strcpy(res->message,message);
	res->messageSize = strlen(message);
	res->next = NULL;
	return res;
}

/*
* Function to delete a message and free all resources
* @param msg* message - a pointer to the message being deleted
* @ ret - void
*/

static void deleteMessages(msg* message){
	msg* head = NULL;
	msg* last = NULL;
	head = message;
	while(head!=NULL){
		last = head;
		head = head->next;
		kfree(last);
	}
}


/*
* Function to delete a message slot and free all resources
* @param Msgslot* msgslot - a pointer to the Message Slot being deleted
* @ ret - void
*/

static void deleteMessageSlot(Msgslot* msgslot){
	Msgslot* head = NULL;
	Msgslot* last = NULL;
	head = msgslot;
	while(head!=NULL){
		deleteMessages(head->allMessages);
		last = head;
		head = head->next;
		kfree(last);
	}
	
}

/*
* function to validate that a message can be written. i.e. the buffer is sufficient
* @param size_t length- the length of the buffer to write
* @ret 0 on success a negative number on Failure
*/

static int validateWrite(size_t length){
	if(length < 0){
		return -EINVAL;
	}
	else if(length > BUF_LEN){
		return -EINVAL;
	}
	else{
		return 0;
	}
}



//DEVICE Functions
/*
* Function to open a device 
*
*/
static int device_open(struct inode* inode, struct file* file){
	int minor = 0;
	Msgslot* copy = NULL;
	Msgslot* last = NULL;
	minor = iminor(inode);
	if (GlobalMessageSlot == NULL){
		GlobalMessageSlot = createMessageSlot(minor);
	}
	
	copy = GlobalMessageSlot;
	while(copy!=NULL){
		if(minor == copy->minorNum){
			return 0;
		}
		last = copy;
		copy = copy->next;
	}
	//copy is NULL andd we reached the end of the linked list
	copy = createMessageSlot(minor);
	last->next = copy;
	if(copy == NULL){
		return -ENOMEM;
	}
	return 0;
}


static int device_release(struct inode * inode, struct file* file){
	return 0;
}


static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset){
	int i = 0;
	int errors = 0; 
	int minor = 0;
	int channelID = 0;
	msg * curr_msg = NULL;
	Msgslot* copy = NULL;
	
	minor = iminor(file->f_inode);
	channelID = (unsigned int)(file->private_data);
	copy = GlobalMessageSlot;

	while(copy!=NULL && copy->minorNum != minor){
			copy = copy->next;
			if(copy == NULL){
				return -EINVAL;
			}
	}
	
	if(copy->numOfMsgs == 0){
		return -EWOULDBLOCK;
	}
	curr_msg = copy->allMessages;
	for(i = 0 ; i < copy->numOfMsgs ; i ++){
		if(curr_msg->channelID == channelID){
			break;
		}
		curr_msg = curr_msg->next;
	}
	if(curr_msg == NULL){
		return -EWOULDBLOCK;
	}

	if( curr_msg->messageSize > length){
		return -ENOSPC;
	}


	errors = copy_to_user(buffer, curr_msg->message, curr_msg->messageSize);
	if(errors){
		return -1 ;

	}
	return curr_msg->messageSize;
}





static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset){
	int i = 0;
	int minor = 0;
	int channelID = 0;
	int test = 0;
	Msgslot* curr_message_slot = NULL;
	msg* curr_msg =NULL;
	msg* last = NULL;
	
	minor = iminor(file->f_inode);
	channelID = (unsigned int)(file->private_data);
	test = validateWrite(length);
	
	if(test < 0){
		return -EINVAL;
	}

	curr_message_slot = GlobalMessageSlot;
	
	while(curr_message_slot != NULL && curr_message_slot->minorNum != minor){
	curr_message_slot = curr_message_slot->next;
	}

	if(curr_message_slot ==NULL){
		return -EINVAL;
	}
	curr_msg = curr_message_slot->allMessages;

	while(curr_msg!=NULL && curr_msg->channelID!=channelID){
		last =curr_msg;
		curr_msg = curr_msg->next;
	}

	if(curr_msg == NULL){
		curr_msg = createNewMessage(channelID,buffer);
		curr_message_slot->numOfMsgs+=1;
		if(curr_msg == NULL){ 
			return -ENOMEM;
		}

		if(last!=NULL){
			last->next = curr_msg;
		}
		else{

			curr_message_slot->allMessages = curr_msg;
		}

	}

	for(i = 0 ; i <= length && i <=BUF_LEN ; i++){
		get_user(curr_msg->message[i],&buffer[i]);
	}
	curr_msg->messageSize = i;
	return i;
}

static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param){

	int i = ioctl_command_id;
	if(i == MSG_SLOT_CHANNEL){
		file->private_data = (void*)ioctl_param;
	}
	else{
		return -EINVAL;
	}

	return 0; 
}


//==================== DEVICE SETUP =============================

// This structure will hold the functions to be called
// when a process does something to the device we created
struct file_operations Fops =
{
  .read           = device_read,
  .write          = device_write,
  .open           = device_open,
  .unlocked_ioctl = device_ioctl,
  .release        = device_release,
};

static int __init char_dev_init(void){
	int rc = -1;
	printk(KERN_INFO "message_slot: registered major number %d\n",MAJOR_NUM);
	
	rc = register_chrdev(MAJOR_NUM,DEVICE_RANGE_NAME, &Fops);
	if (rc < 0)
	{
    printk(KERN_ALERT "%s registraion failed for  %d\n", DEVICE_FILE_NAME, MAJOR_NUM );
    	return rc;	
	}
return 0;
}



static void __exit char_dev_exit(void){
	deleteMessageSlot(GlobalMessageSlot);
	unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);

}

//---------------------------------------------------------------
module_init(char_dev_init);
module_exit(char_dev_exit);

//========================= END OF FILE =========================







