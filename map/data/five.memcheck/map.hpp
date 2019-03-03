#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"
#include <iostream>
#define ENDPOINT (node *)17

namespace sjtu 
{
template<class Key, class T, class Compare = std::less<Key>> 
class map
{
public:
	typedef pair<const Key, T> value_type;
private: 
	class node
	{
		friend map;
	private:
		node *l, *r, *prev, *next;
		value_type *data;
		int height;
	public:
		node(): l(NULL), r(NULL), prev(NULL), prev(NULL), data(NULL), height(0){}
		
		node(node *const temL, node *const temR, node *const temPrev, node *const temNext, value_type *const temData,const int temHeight)
			: l(temL), r(temR), prev(temPrev), next(temNext), data(temData), height(temHeight) {}
	} *root;
	
	int totalSize;
	
	Compare cmp;

	inline int max(const int &s1, const int &s2) { return s1 > s2 ? s1 : s2;}
	
	inline int getH(node *const &p) const { return p == NULL ? 0 : p -> height; }
	
	inline void update(node *&p)
	{
		if(p == NULL) return;
		p -> height = max(getH(p -> l), getH(p -> r)) + 1;
	}
	
	inline void Lrotate(node *&p)
	{
		node *tem = p -> r -> l;
		p -> r -> l = p;
		p = p -> r;
		p -> l -> r = tem;
		
		update(p -> l);
		update(p);
	}
	
	inline void Rrotate(node *&p)
	{
		node *tem = p -> l -> r;
		p -> l -> r = p;
		p = p -> l;
		p -> r -> l = tem;
		
		update(p -> r); 
		update(p);
	}
	
	inline void maintain(node *&p)
	{
		if(p == NULL) return;
		
		if(getH(p -> l) - getH(p -> r) > 1)
		{
			if(getH(p -> l -> l) >= getH(p -> l -> r)) 
			{
				Rrotate(p);
			}
			else
			{
				Lrotate(p -> l);
				Rrotate(p);
			}
		}
		else if(getH(p -> r) - getH(p -> l) > 1)
		{
			if(getH(p -> r -> r) >= getH(p -> r -> l))
			{
				Lrotate(p);
			}
			else
			{
				Rrotate(p -> r);
				Lrotate(p);
			}
		}	
	}
	 
	void avlInsert(node *&p, node *&temPrev, node *&temNext, const value_type &temData, bool &success, node *&tem)
	{
		if(p == NULL)
		{
			++totalSize;
			tem = p = new node(NULL, NULL, temPrev, temNext, new value_type(temData), 1);
			if(temPrev != NULL)
			{
				temPrev -> next = p;
			}
			if(temNext != NULL)
			{
				temNext -> prev = p;
			}
			success = true;
			return ;
		}	
		
		if(cmp(temData.first, p -> data -> first))
		{
			avlInsert(p -> l, temPrev, p, temData, success, tem);
		}
		else if(cmp(p -> data -> first, temData.first))
		{
			avlInsert(p -> r, p, temNext, temData, success, tem);
		}
		else
		{
			tem = p;
			success = false;
			return;
		}
		
		if(success)
		{
			update(p);
			maintain(p);
		}
	}
	
	node * findMax(node *p) const
	{
		while(p != NULL && p -> r != NULL) p = p -> r;
		return p; 
	}
	
	node * findMin(node *p) const
	{
		while(p != NULL && p -> l != NULL) p = p -> l;
		return p;
	}
	
	node * getPrev(node *p) const
	{
		for(p = p -> l; p -> r != NULL; p = p -> r);
		return p;
	}
	
	node * getNext(node *p) const
	{
		for(p = p -> r; p -> l != NULL; p = p -> l);
		return p;
	}
	
	inline void erasePoint(node *&p)
	{
		--totalSize;
		if(p -> prev != NULL) p -> prev -> next = p -> next;
		if(p -> next != NULL) p -> next -> prev = p -> prev;
		delete p -> data;
		delete p;
		p = NULL;
	}
	
	inline void swapNode(node *&A, node *fB, node *B)
	{
		if(A == fB)
		{
			node *Bl = B -> l, *Br = B -> r;
			if(A -> l == B)
			{
				B -> l = A;	
				B -> r = A -> r;
			}
			else
			{
				B -> r = A;
				B -> l = A -> l;
			}
			A -> l = Bl;
			A -> r = Br;
		}
		else
		{	if(fB != NULL)
			{
				if(fB -> l == B)
				{
					fB -> l = A;
				}
				else
				{
					fB -> r = A;
				}
			}
			node *tem = A -> l;
			A -> l = B -> l;
			B -> l = tem;
			tem = A -> r;
			A -> r = B -> r;
			B -> r = tem;
		}
		A = B;
	}
	
	void avlErase(node *&f, node *&p, const Key &temKey)
	{
		if(p == NULL) return ;
		
		if(cmp(temKey, p -> data -> first)) 
		{
			avlErase(p, p -> l, temKey);
		}
		else if(cmp(p -> data -> first, temKey)) 
		{
			avlErase(p, p -> r, temKey);
		}
		else
		{
			if(p -> l != NULL && p -> r != NULL)
			{
				node *B = p -> r, *fB = p;
				while(B -> l != NULL)
				{
					fB = B;
					B = B -> l;
				}
				swapNode(p, fB, B);
				avlErase(p, p -> r, temKey);
			}
			else if(p -> l != NULL)
			{
				swapNode(p, p, p -> l);
				erasePoint(p -> l);
			}
			else if(p -> r != NULL)
			{
				swapNode(p, p, p -> r);
				erasePoint(p -> r);
			}
			else
			{
				erasePoint(p);
			}
		}
		
		if(p != NULL)
		{
			update(p);
			maintain(p);
		}
	}

	node * avlFind(node *p, const Key &temKey) const
	{
		while(true)
		{ 
			if(p == NULL) return NULL;
			if(cmp(temKey, p -> data -> first))
			{
				p = p -> l;
			}
			else if(cmp(p -> data -> first, temKey))
			{
				p = p -> r;
			}
			else
			{
				return p;
			}
		}
	}
	
	void avlClear(node *&p)
	{
		if(p == NULL) return;
		avlClear(p -> l);
		avlClear(p -> r);
		--totalSize;
		delete p -> data;
		delete p;
		p = NULL;
	}
	
	void copyPrev(node *&p, node *&prev)
	{
		if(p == NULL) return;
		copyPrev(p -> l, prev);
		p -> prev = prev;
		copyPrev(p -> r, prev = p);
	}
	
	void copyNext(node *&p, node *&next)
	{
		if(p == NULL) return;
		copyNext(p -> r, next);
		p -> next = next;
		copyNext(p -> l, next = p);
	}
	
	void copyNode(node *&p, node *const &tem)
	{
		if(tem == NULL)
		{
			p = NULL;
			return ;
		}
		p = new node(NULL, NULL, NULL, NULL, new value_type(*(tem -> data)), tem -> height);
		copyNode(p -> l, tem -> l);
		copyNode(p -> r, tem -> r);
	}
	
	inline void avlCopy(const map &other)
	{
		totalSize = other.totalSize;
		copyNode(root, other.root);
		node *tem = NULL;
		copyPrev(root, tem = NULL);
		copyNext(root, tem = NULL);
	}
	
public:
	class iterator;
	class const_iterator;
	
	class iterator
	{
		friend map;
	private:
		map<Key, T, Compare> *it;
		node* position;
	public:
		iterator(const map<Key, T, Compare> *temIt, node *const temPosition) 
		{
			it = (map<Key, T, Compare>*)temIt;
			position = temPosition;
		}
		
		iterator() : it(NULL), position(NULL){}
		
		iterator operator++(int) 
		{
			node *temPosition = position;
			if(position == ENDPOINT || position == NULL) 
			{
				throw invalid_iterator("map");
			}
			position = position -> next == NULL ? ENDPOINT : position -> next;
			return iterator(it, temPosition);
		}
		
		iterator operator--(int) 
		{ 
			node *temPosition = position;
			if(position == ENDPOINT)
			{
				if(it == NULL || it -> root == NULL)
				{
					throw invalid_iterator("map");
				}
				position = it -> findMax(it -> root);
			}
			else if(position == NULL || position -> prev == NULL) 
			{
				throw invalid_iterator("map");
			}
			else
			{
				position = position -> prev;
			}
			return iterator(it, temPosition);
		}
		
		iterator & operator++() 
		{ 
			if(position == ENDPOINT || position == NULL) 
			{
				throw invalid_iterator("map");
			}
			position = position -> next == NULL ? ENDPOINT : position -> next;
			return *this;
		}
		
		iterator & operator--() 
		{
			if(position == ENDPOINT)
			{
				if(it == NULL || it -> root == NULL)
				{
					throw invalid_iterator("map");
				}
				position = it -> findMax(it -> root);
			}
			else if(position == NULL || position -> prev == NULL) 
			{
				throw invalid_iterator("map");
			}
			else
			{
				position = position -> prev;
			}
			return *this;
		}
		
		value_type & operator*() const
		{
			if(position == NULL || position == ENDPOINT) 
			{
				throw invalid_iterator("map");
			}
			return *(position -> data);
		}
		
		bool operator==(const iterator &rhs) const { return it == rhs.it && position == rhs.position; }
		bool operator!=(const iterator &rhs) const { return it != rhs.it || position != rhs.position; }
		bool operator==(const const_iterator &rhs) const { return it == rhs.it && position == rhs.position; }
		bool operator!=(const const_iterator &rhs) const { return it != rhs.it || position != rhs.position; }
		
		value_type * operator->() noexcept 
		{ 
			if(position == NULL || position == ENDPOINT)
			{
				throw invalid_iterator("map");
			}
			return position -> data;
		}
	};
	
	class const_iterator
	{
		friend map;
	private:
		map<Key, T, Compare> *it;
		node* position;
	public:
		const_iterator(const map<Key, T, Compare> *temIt, node *const temPosition) 
		{
			it = (map<Key, T, Compare>*)temIt;
			position = temPosition;
		}
		
		const_iterator() : it(NULL), position(NULL){}
		
		const_iterator(const iterator &other): it(other.it), position(other.position){}
		
		const_iterator operator++(int) 
		{
			node *temPosition = position;
			if(position == ENDPOINT || position == NULL) 
			{
				throw invalid_iterator("map");
			}
			position = position -> next == NULL ? ENDPOINT : position -> next;
			return iterator(it, temPosition);
		}
		
		const_iterator operator--(int) 
		{ 
			node *temPosition = position;
			if(position == ENDPOINT)
			{
				if(it == NULL || it -> root == NULL)
				{
					throw invalid_iterator("map");
				}
				position = it -> findMax(it -> root);
			}
			else if(position == NULL || position -> prev == NULL) 
			{
				throw invalid_iterator("map");
			}
			else
			{
				position = position -> prev;
			}
			return iterator(it, temPosition);
		}
		
		const_iterator & operator++() 
		{
			if(position == ENDPOINT || position == NULL) 
			{
				throw invalid_iterator("map");
			}
			position = position -> next == NULL ? ENDPOINT : position -> next;
			return *this;
		}
		
		const_iterator & operator--() 
		{
			if(position == ENDPOINT)
			{
				if(it == NULL || it -> root == NULL)
				{
					throw invalid_iterator("map");
				}
				position = it -> findMax(it -> root);
			}
			else if(position == NULL || position -> prev == NULL) 
			{
				throw invalid_iterator("map");
			}
			else
			{
				position = position -> prev;
			}
			return *this;
		}
		
		value_type & operator*() const
		{
			if(position == NULL || position == ENDPOINT) 
			{
				throw invalid_iterator("map");
			}
			return *(position -> data);
		}
		
		bool operator==(const iterator &rhs) const { return it == rhs.it && position == rhs.position; }
		bool operator!=(const iterator &rhs) const { return it != rhs.it || position != rhs.position; }
		bool operator==(const const_iterator &rhs) const { return it == rhs.it && position == rhs.position; }
		bool operator!=(const const_iterator &rhs) const { return it != rhs.it || position != rhs.position; }
		
		value_type * operator->() noexcept 
		{ 
			if(position == NULL || position == ENDPOINT)
			{
				throw invalid_iterator("map");
			}
			return position -> data;
		}
	};
	
	map() :root(NULL), totalSize(0){}	
	
	map(const map &other) 
	{
		avlCopy(other);
	}
	
	
	map &operator=(const map &other) 
	{
		if(&other == this) return *this;
		avlClear(root);
		avlCopy(other); 	
		return *this;
	}
	
	~map() 
	{
		avlClear(root);
	}
	
	inline T & at(const Key &key) 
	{
		node *x = avlFind(root, key);
		if(x == NULL) throw index_out_of_bound("map");
		return x -> data -> second;
	}
	
	inline const T & at(const Key &key) const 
	{ 
		node *x = avlFind(root, key);
		if(x == NULL) throw index_out_of_bound("map");
		return x -> data -> second;
	}
	
	inline T & operator[](const Key &temKey) 
	{
		node *prev = NULL, *next = NULL, *tem = NULL;
		bool success = false;
		avlInsert(root, prev, next, value_type(temKey, T()), success, tem);
		return tem -> data -> second;
	}
	inline const T & operator[](const Key &key) const 
	{ 
		node *x = avlFind(root, key);
		if(x == NULL) throw index_out_of_bound("map");
		return x -> data -> second;
	}
	
	iterator begin() { return iterator(this, root == NULL ? ENDPOINT : findMin(root)); }
	iterator end() { return iterator(this, ENDPOINT); }
	
	const_iterator cbegin() const { return const_iterator(this, root == NULL ? ENDPOINT : findMin(root)); }
	const_iterator cend() const { return const_iterator(this, ENDPOINT); }
	
	bool empty() const { return root == NULL; }
	
	void clear() { avlClear(root); }
	
	size_t size() const { return totalSize; }
	
	size_t count(const Key &temKey) const { return avlFind(root, temKey) == NULL ? 0 : 1; }

	
	pair<iterator, bool> insert(const value_type &temValue) 
	{
		node *prev = NULL, *next = NULL, *tem = NULL;
		bool success = false;
		avlInsert(root, prev, next, temValue, success, tem);
		return pair<iterator, bool>(iterator(this, tem), success);
	}
	
	void erase(iterator pos) 
	{
		if(pos.it != this) throw invalid_iterator("map :: iterator");
		if(pos.position == NULL || pos.position == ENDPOINT) throw index_out_of_bound("map"); 
		node *tem = NULL;
		avlErase(tem, root, pos.position -> data -> first); 
	}
	
	iterator find(const Key &temKey)
	{ 
		node *tem = avlFind(root, temKey);
		return iterator(this, tem == NULL ? ENDPOINT : tem);
	}
	
	const_iterator find(const Key &temKey) const 
	{ 
		node *tem = avlFind(root, temKey);
		return const_iterator(this, tem == NULL ? ENDPOINT : tem);
	}
};
}

#endif
