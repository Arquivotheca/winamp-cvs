#pragma once
#include "nu/lfmpscq.h"
#include "foundation/types.h"
#include <semaphore.h>

namespace nu
{

	/* you can inherit from message_node_t (or combine inside a struct)
	but make sure that your message isn't > 64 bytes */
	struct message_node_t : public queue_node_t
	{
		uint32_t message;
	};
	
	class MessageLoop
	{
	public:
		MessageLoop();
		~MessageLoop();

		/* API for Message senders */
		message_node_t *AllocateMessage(); // returns a message for you to fill out
		void PostMessage(message_node_t *message);	

		/* API for Message receivers */
		void FreeMessage(message_node_t *message);
		message_node_t *GetMessage(); // waits forever
		message_node_t *PeekMessage();
		message_node_t *PeekMessage(unsigned int milliseconds);
	private:
		void RefillCache();

		sem_t message_notification;
		mpscq_t message_queue;
	};
}