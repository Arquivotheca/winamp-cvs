#ifndef NULLSOFT_UTILITY_PTRDEQUE_H
#define NULLSOFT_UTILITY_PTRDEQUE_H

#include <stdlib.h>

namespace nu
{
struct Node
{
	void *data;
	nu::Node *next, *prev;
};

class PtrDequeBase
{
public:


	size_t size()
	{
		return list_size;
	}
	bool empty()
	{
		return (list_size == 0);
	}

	void pop_back()
	{
		nu::Node *old = tail;
		tail=tail->prev;
		if (tail)
			tail->next = 0;
		free(old);
		if (!tail)
			head=0;
		list_size--;
	}

	void pop_front()
	{
		nu::Node *old = head;
		head=head->next;
		if (head)
			head->prev=0;
		free(old);
		if (!head)
			tail=0;
		list_size--;
	}


protected:
	PtrDequeBase() : head(0), tail(0), list_size(0)
	{
	}

	void push_front(void *data)
	{
		nu::Node *new_head = (nu::Node *)malloc(sizeof(nu::Node));
		new_head->next=head;
		new_head->prev=0;
		new_head->data=data;
		
		if (head)
			head->prev = new_head;
		head=new_head;
		if (!tail)
			tail=head;
		list_size++;
	}

	void push_back(void *data)
	{
		nu::Node *new_tail = (nu::Node *)malloc(sizeof(nu::Node));
		new_tail->next=0;
		new_tail->prev=tail;
		new_tail->data=data;
		
		if (tail)
			tail->next = new_tail;
		tail=new_tail;
		if (!head)
			head=tail;
		list_size++;
	}

	nu::Node *head, *tail;
	size_t list_size;
};

template <class T>
class PtrDeque : public nu::PtrDequeBase
{
public:
	
	T *&front()
	{
		return (T *&)head->data;
	}

	T *&back()
	{
		return (T *&)tail->data;
	}
	
	void push_front(T *data)
	{
		PtrDequeBase::push_front((void *)data);
	}

	void push_back(T *data)
	{
		PtrDequeBase::push_back((void *)data);
	}
};
}
#endif