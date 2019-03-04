//a simple implementation of blocking list
#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>
#include <cmath>

namespace sjtu {

    template<class T>
    class deque {
    private:

        struct node {
            T data;
            node *next;
            node *prev;

            node (const T &data , node *next = nullptr , node *prev = nullptr) : next(next) ,
                                                                                                prev(prev) ,
                                                                                                data(data) {}
            node(){}
            node (node *next  , node *prev ) : next(next) ,
                                                                                prev(prev)  {}
        };

        struct block {
            block *next;
            block *prev;
            node *head;
            node *tail;
            int sz;

            block (block *next = nullptr , block *prev = nullptr) : next(next) , prev(prev) {
                head=(node*)malloc(sizeof(node));
                head->prev= nullptr;

                tail=(node*)malloc(sizeof(node));
                tail->next= nullptr;
                head->next = tail;
                tail->prev = head;
                sz=0;
            }

            block (const block &other) {
                head=(node*)malloc(sizeof(node));
                head->prev= nullptr;
                auto p = head;
                for (auto i = other.head->next; i != other.tail; i = i->next) {
                    p->next = new node(i->data , nullptr , p);
                    p = p->next;
                }
                p->next =(node*)malloc(sizeof(node));
                tail = p->next;
                tail->next= nullptr;
                tail->prev=p;
                sz=other.sz;
            }

            ~block () {
                auto p = head->next;
                while (p != tail) {
                    auto tmp = p;
                    p = p->next;
                    delete tmp;
                }
                free(head);
                free(tail);
            }
        };

    public:
        friend class const_iterator;

        class const_iterator;

        friend class iterator;

        class iterator {
            friend class deque;

        private:
            deque *thisdeque;
            block *outer;
            node *inner;
        public:

            iterator (deque *thisdeque = nullptr , block *outer = nullptr , node *inner = nullptr) : thisdeque
                                                                                                                      (thisdeque) ,
                                                                                                              outer(outer) ,
                                                                                                              inner(inner) {}

            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, the behaviour is **undefined**.
             * as well as operator-
             */
            iterator operator+ (const int &n) const {
                if (n < 0) {
                    return operator-(-n);
                }
                auto tmp = *this;
                for (int i = 0; i < n; i++) {
                    ++tmp;
                }
                return tmp;
            }

            iterator operator- (const int &n) const {
                if (n < 0) {
                    return operator+(-n);
                }
                auto tmp = *this;
                for (int i = 0; i < n; i++) {
                    --tmp;
                }
                return tmp;
            }

            // return th distance between two iterator,
            // if these two iterators points to different vectors, throw invaild_iterator.

            int operator- ( iterator rhs) const {
                if (rhs.thisdeque != thisdeque) {
                    throw invalid_iterator();
                }
                int tmp1=thisdeque->order(outer);
                int tmp2=rhs.thisdeque->order(rhs.outer);
                if (tmp1 < tmp2 || (tmp1 == tmp2 && thisdeque->order(outer , inner) < thisdeque->order(rhs.outer , rhs
                .inner)
                )) {
                    return -rhs.operator-(*this);
                }
                int result = 0;
                while (rhs != *this) {
                    rhs++;
                    result++;
                }
                return result;
            }

            iterator operator+= (const int &n) {
                *this = *this + n;
                return *this;
            }

            iterator operator-= (const int &n) {
                *this = *this - n;
                return *this;
            }


            iterator operator++ (int) {
                auto tmp = *this;
                operator++();
                return tmp;
            }


            iterator &operator++ () {
                if (inner->next == outer->tail) {
                    outer = outer->next;
                    inner = outer->head->next;
                } else {
                    inner = inner->next;
                }
                return *this;
            }


            iterator operator-- (int) {
                auto tmp = *this;
                operator--();
                return tmp;
            }


            iterator &operator-- () {

                if (inner->prev == outer->head) {
                    outer = outer->prev;
                    inner = outer->tail->prev;
                } else {
                    inner = inner->prev;
                }
            }


            T &operator* () const {
                if (*this == thisdeque->end()||*this==(--thisdeque->begin())||this->inner== nullptr) {
                    throw invalid_iterator();
                }
                return inner->data;
            }

            T *operator-> () const noexcept {
                return &(*(*this));
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            bool operator== (const iterator &rhs) const {
                return outer == rhs.outer && inner == rhs.inner && thisdeque == rhs.thisdeque;
            }

            bool operator== (const const_iterator &rhs) const {
                return outer == rhs.outer && inner == rhs.inner && thisdeque == rhs.thisdeque;
            }

            /**
             * some other operator for iterator.
             */
            bool operator!= (const iterator &rhs) const {
                return !operator==(rhs);
            }

            bool operator!= (const const_iterator &rhs) const {
                return !operator==(rhs);
            }
        };

        class const_iterator : public iterator {

        public:
            const_iterator () : iterator() {

            }

            const_iterator (const const_iterator &other) : iterator(other) {

            }

            const_iterator (const iterator &other) : iterator(other) {

            }

        };

    private:

        block *head;
        block *tail;
        int totalsz = 0;

        block *find_outer_block (int p , int &used_size)const {
            auto i = head->next;
            for (; i != tail && p - i->sz >= 0; i = i->next) {
                p -= i->sz;
                used_size += i->sz;
            }
            return i;
        }

        node *find_inner_block (block *outer , int p) const{
            auto i = outer->head->next;
            for (; i != outer->tail && p > 0; i = i->next) {
                p--;
            }
            return i;
        }

        //[a,b]-->[a,c),[c,b]
        void split (block *&outer , node *inner) {
            if (inner == outer->head->next || inner == outer->tail) {
                return;
            }
            auto i = order(outer , inner);
            auto tmp = new block(outer->next , outer);
            outer->next = tmp;
            tmp->next->prev = tmp;
            tmp->head->next = inner;
            auto p=inner->prev;
            inner->prev->next = outer->tail;
            inner->prev = tmp->head;
            outer->tail->prev->next = tmp->tail;
            tmp->tail->prev = outer->tail->prev;

            outer->tail->prev =p;
            tmp->sz = outer->sz - i;
            outer->sz = i ;
            outer = outer->next;

        }

        //[a,b][b+1,c]->[a,c]
        block *merge (block *outer1 , block *outer2) {

            auto outer = new block(outer2->next , outer1->prev);
            outer->sz = outer1->sz + outer2->sz;
            outer->prev->next = outer;
            outer->next->prev = outer;
            outer->head->next = outer1->head->next;
            outer1->head->next->prev = outer->head;
            outer1->tail->prev->next = outer2->head->next;
            outer2->head->next->prev = outer1->tail->prev;
            outer2->tail->prev->next = outer->tail;
            outer->tail->prev = outer2->tail->prev;

            free(outer1->head);
            free(outer1->tail);
            free(outer2->head);
            free(outer2->tail);
            free(outer1);
            free(outer2);
            return outer;
        }

        static int order (block *outer , node *inner) {
            int cnt = 0;
            for (auto i = outer->head->next; i != inner; i = i->next) {
                cnt++;
            }
            return cnt;
        }

        void split_half (block *outer) {
            auto i=outer->sz/2;
            auto inner=find_inner_block(outer,i);
            split(outer,inner);
        }
        block* maintain (block *p= nullptr) {

            for (auto i = head->next; i != tail->prev&&i!=tail; i = i->next) {
                if (i->sz + i->next->sz <= sqrt(totalsz)) {
                    if (i == p || i->next == p) {
                        p=i=merge(i,i->next);
                    } else {
                        i=merge(i,i->next);
                    }
                }
            }
            return p;
        }

        int order (block *outer){
            int cnt = 0;
            for (auto i = head->next; i != outer; i = i->next) {
                cnt++;
            }
            return cnt;
        }

    public:
        deque () {
            head = new block;
            head->next = tail = new block;
            tail->prev = head;
        }

        deque (const deque &other) {
            head = new block;
            auto p = head;
            for (auto i = other.head->next; i != other.tail; i = i->next) {
                p->next = new block(*i);
                p->next->prev = p;
                p = p->next;
            }
            p->next = new block(nullptr , p);
            tail = p->next;
            tail->prev=p;
            totalsz=other.totalsz;
        }


        ~deque () {
            auto p = head;
            while (p != nullptr) {
                auto tmp = p;
                p = p->next;
                delete tmp;
            }
        }


        deque &operator= (const deque &other) {
            if (&other == this) {
                return *this;
            }
            clear();
            auto tmp = head;
            auto p = other.head->next;
            while (p != other.tail) {
                tmp->next = new block(*p);
                tmp->next->prev = tmp;
                tmp = tmp->next;
                p = p->next;
            }
            tmp->next = tail;
            tail->prev = tmp;
            totalsz=other.totalsz;
            return *this;
        }

        /**
         * access specified element with bounds checking
         * throw index_out_of_bound if out of bound.
         */
        T &at (const size_t &pos) {
            if (pos >= totalsz || pos < 0) {
                throw index_out_of_bound();
            }
            int left = 0;
            auto outer = find_outer_block(pos , left);
            auto inner = find_inner_block(outer , pos-left);
            return inner->data;
        }

        const T &at (const size_t &pos) const {
            if (pos >= totalsz || pos < 0) {
                throw index_out_of_bound();
            }
            int left = 0;
            auto outer = find_outer_block(const_cast<size_t& >(pos) , left);
            auto inner = find_inner_block(outer , const_cast<size_t& >(pos)-left);
            return inner->data;
        }

        T &operator[] (const size_t &pos) {
            return at(pos);
        }

        const T &operator[] (const size_t &pos) const {
            return at(pos);
        }

        /**
         * access the first element
         * throw container_is_empty when the container is empty.
         */
        const T &front () const {
            if (totalsz == 0) {
                throw container_is_empty();
            }
            return head->next->head->next->data;
        }

        /**
         * access the last element
         * throw container_is_empty when the container is empty.
         */
        const T &back () const {
            if (totalsz == 0) {
                throw container_is_empty();
            }
            return tail->prev->tail->prev->data;
        }

        /**
         * returns an iterator to the beginning.
         */
        iterator begin () {
            return iterator(this , head->next , head->next->head->next);
        }

        const_iterator cbegin () const {
            return const_cast<deque*>(this)->begin();
        }

        /**
         * returns an iterator to the end.
         */
        iterator end () {
            return iterator(this , tail , tail->tail);
        }

        const_iterator cend () const { return const_cast<deque*>(this)->end(); }

        /**
         * checks whether the container is empty.
         */
        bool empty () const { return totalsz == 0; }

        /**
         * returns the number of elements
         */
        size_t size () const { return totalsz; }

        /**
         * clears the contents
         */
        void clear () {
            totalsz=0;
            auto p = head->next;
            while (p != tail) {
                auto tmp = p;
                p = p->next;
                delete tmp;
            }
            head->next = tail;
            tail->prev = head;
        }

        /**
         * inserts elements at the specified locat on in the container.
         * inserts value before pos
         * returns an iterator pointing to the inserted value
         *     throw if the iterator is invalid or it point to a wrong place.
         */
        iterator insert (iterator pos , const T &value) {
            if (pos.thisdeque != this || pos.inner == nullptr || pos.outer == nullptr) {
                throw invalid_iterator();
            }
            totalsz++;
            split(pos.outer , pos.inner);
            auto tmp = new block(pos.outer , pos.outer->prev);
            tmp->sz = 1;
            node *result2 = tmp->head->next = new node(value , tmp->tail , tmp->head);
            result2->next->prev = result2;
            tmp->next->prev = tmp;
            tmp->prev->next = tmp;
            block *result=maintain(tmp);
            return iterator(this , result , result2);


        }

        /**
         * removes specified element at pos.
         * removes the element at pos.
         * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
         * throw if the container is empty, the iterator is invalid or it points to a wrong place.
         */
        iterator erase (iterator pos) {
            if (pos.thisdeque != this || pos.inner == nullptr || pos.outer == nullptr) {
                throw invalid_iterator();
            }
            if (totalsz == 0) {
                throw container_is_empty();
            }
            totalsz--;
            split(pos.outer , pos.inner);
            auto tmp = pos.outer;
            split(tmp , pos.inner->next);
            pos.outer->next->prev = pos.outer->prev;
            pos.outer->prev->next = pos.outer->next;
            tmp = pos.outer->next;
            auto result2 = tmp->head->next;
            delete pos.outer;
            block* result1=tmp;
            if(tmp!=tail && tmp->prev!=head) {
                result1 = merge(tmp->prev , tmp);
            }
            return iterator(this , result1 , result2);
        }

        /**
         * adds an element to the end
         */
        void push_back (const T &value) {
            totalsz++;
            if (totalsz == 1) {
                head->next=new block(tail,head);
                head->next->head->next=new node(value,head->next->tail,head->next->head);
                head->next->sz=1;
                head->next->tail->prev=head->next->head->next;
                tail->prev=head->next;
                return;
            }
            tail->prev->tail->prev=new node(value,tail->prev->tail,tail->prev->tail->prev);
            tail->prev->tail->prev->prev->next=tail->prev->tail->prev;
            tail->prev->sz++;
            if (tail->prev->sz >= 2 * sqrt(totalsz)) {
                split_half(tail->prev);
                maintain();
            }
        }

        /**
         * removes the last element
         *     throw when the container is empty.
         */
        void pop_back () {
            if (empty()) {
                throw container_is_empty();
            }
            totalsz--;
            if (tail->prev->sz == 1) {
                auto tmp=tail->prev;
                tail->prev->prev->next=tail;
                tail->prev=tail->prev->prev;
                delete tmp;
            } else {
                tail->prev->sz--;
                auto tmp=tail->prev->tail->prev;
                tmp->next->prev=tmp->prev;
                tmp->prev->next=tmp->next;
                delete tmp;
            }
        }

        /**
         * inserts an element to the beginning.
         */
        void push_front (const T &value) {
            totalsz++;
            if (totalsz == 1) {
                head->next=new block(tail,head);
                head->next->head->next=new node(value,head->next->tail,head->next->head);
                head->next->sz=1;
                head->next->tail->prev=head->next->head->next;
                tail->prev=head->next;
                return;
            }
            head->next->head->next=new node(value,head->next->head->next,head->next->head);
            head->next->head->next->next->prev=head->next->head->next;
            head->next->sz++;
            if (head->next->sz >= 2 * sqrt(totalsz)) {
                split_half(head->next);
                maintain();
            }
        }

        /**
         * removes the first element.
         *     throw when the container is empty.
         */
        void pop_front () {
            if (empty()) {
                throw container_is_empty();
            }
            totalsz--;
            if (head->next->sz == 1) {
                auto tmp=head->next;
                tmp->next->prev=tmp->prev;
                tmp->prev->next=tmp->next;
                delete tmp;
            } else {
                head->next->sz--;
                auto tmp=head->next->head->next;
                tmp->next->prev=tmp->prev;
                tmp->prev->next=tmp->next;
                delete tmp;
            }
        }

    };


}

#endif