#ifndef NULLSOFT_UTILITY_PTRDEQUE2_H
#define NULLSOFT_UTILITY_PTRDEQUE2_H

/* benski>
Identical to PtrDeque, but this one makes you inherit from nu::PtrDequeNode
which reduces memory usage
*/

namespace nu
{
struct PtrDequeNode
{
	nu::PtrDequeNode *next, *prev;
};

class PtrDeque2Base
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
		tail=tail->prev;
		if (tail)
			tail->next = 0;
		if (!tail)
			head=0;
		list_size--;
	}

	void pop_front()
	{
		head=head->next;
		if (head)
			head->prev=0;
		if (!head)
			tail=0;
		list_size--;
	}


protected:
	PtrDeque2Base() : head(0), tail(0), list_size(0)
	{
	}

	void push_front(nu::PtrDequeNode *data)
	{
		nu::PtrDequeNode *new_head = data;
		new_head->next=head;
		new_head->prev=0;
		
		if (head)
			head->prev = new_head;
		head=new_head;
		if (!tail)
			tail=head;
		list_size++;
	}

	void push_back(nu::PtrDequeNode *data)
	{
		nu::PtrDequeNode *new_tail = data;
		new_tail->next=0;
		new_tail->prev=tail;
		
		if (tail)
			tail->next = new_tail;
		tail=new_tail;
		if (!head)
			head=tail;
		list_size++;
	}

	nu::PtrDequeNode *head, *tail;
	size_t list_size;
};

template <class T>
class PtrDeque2 : public nu::PtrDeque2Base
{
public:
	class iterator
	{
	public:
		iterator(T *t) : t(t) {}

		T *operator ->() { return t; }
		void operator++(int)
		{
			t = static_cast<T *>(t->next);
		}
		bool operator ==(const iterator &comp) { return t==comp.t; }
		bool operator !=(const iterator &comp) { return t!=comp.t; }
		T *operator *() { return t; }
	private:
			T* t;	
	};

	class const_iterator
	{
	public:
		const_iterator(const T *t) : t((T *)t) {}
		const_iterator(iterator &t) : t(t.operator *()) {}

		T *operator ->() { return t; }
		void operator++(int)
		{
			t = static_cast<T *>(t->next);
		}
		bool operator ==(const const_iterator &comp) { return t==comp.t; }
		bool operator !=(const const_iterator &comp) { return t!=comp.t; }
		T *operator *() { return t; }
	private:
		T* t;	
	};

	
		
	iterator end() const
	{
		return iterator(0);
	}

	iterator begin() const
	{
		return iterator(static_cast<T *>(head));
	}

	T *front()
	{
		return (T *)head;
	}

	const T *front() const
	{
		return (const T *)head;
	}


	T *back()
	{
		return (T *)tail;
	}
	
	void push_front(T *data)
	{
		PtrDeque2Base::push_front(data);
	}
		
	void push_back(T *data)
	{
		PtrDeque2Base::push_back(data);
	}
	void deleteAll()
	{
		while (!empty())
		{
			T *d = front();
			pop_front();
			delete d;
		}
	}
	void erase(T *t)
	{
		if (t->prev)
			t->prev->next = t->next;
		else // no prev pointer means it's the head of list
			head = t->next;

		if (t->next)
			t->next->prev = t->prev;
		else // no next pointer mean it's the tail of list
			tail = t->prev;
		list_size--;
	}
};
}
#endif