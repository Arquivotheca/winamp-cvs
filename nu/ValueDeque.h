#pragma once

namespace nu
{
	template <class T>
	class ValueDeque
	{
	private:
		struct Node
		{
			T data;
			Node *next, *prev;
		};
	public:
		class iterator
		{
		public:
			iterator(Node *_v) : ptr(_v)
			{
			}
			Node *ptr;
			bool operator ==(const iterator &comp) const
			{
				return ptr == comp.ptr;
			}
			bool operator !=(const iterator &comp) const
			{
				return ptr != comp.ptr;
			}

			iterator &operator++()
			{
				if (ptr)
					ptr = ptr->next;
				return *this;
			}

			void operator++(int)
			{
				if (ptr)
					ptr = ptr->next;
			}

			T &operator *()
			{
				return ptr->data;
			}
		};

		iterator end() const
		{
			return iterator(0);
		}

		iterator begin() const
		{
			return iterator(head);
		}

		void erase(const iterator &itr)
		{
			const Node *i = itr.ptr;
			if (i)
			{
				Node *prev = i->prev;
				Node *next = i->next;
				if (prev)
					prev->next = next;	

				if (next)
					next->prev = prev;

				if (head == i)
					head = next;

				if (tail == i)
					tail = prev;
			}
		}

		size_t size() const
		{
			return list_size;
		}
		bool empty() const
		{
			return (list_size == 0);
		}

		void pop_back()
		{
			Node *old = tail;
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
			Node *old = head;
			head=head->next;
			if (head)
				head->prev=0;
			free(old);
			if (!head)
				tail=0;
			list_size--;
		}

		T *&front()
		{
			return (T *&)head->data;
		}

		T *&back()
		{
			return (T *&)tail->data;
		}

		ValueDeque() : head(0), tail(0), list_size(0)
		{
		}

		void push_front(T &data)
		{
			Node *new_head = (Node *)malloc(sizeof(Node));
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

		void push_back(T &data)
		{
			Node *new_tail = (Node *)malloc(sizeof(Node));
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

	protected:
		Node *head, *tail;
		size_t list_size;
	};

}
