
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {
    const bool BLACK = false;
    const bool RED = true;

    template<
            class Key ,
            class Value ,
            class Compare = std::less<Key>
    >
    class map {
    public:
        /**
         * the internal type of data.
         * it should have a default constructor, a copy constructor.
         * You can use sjtu::map as value_type by typedef.
         */
        typedef pair<Key,Value> value_type;
        struct Entry {
            Key first;
            Value second;
            bool color = BLACK;
            Entry *left;
            Entry *right;
            Entry *parent;

            Entry (Key key , Value val , bool color , Entry *left , Entry *right , Entry *parent) : first(key) ,
                                                                                                    second(val) ,
                                                                                                    color(color) ,
                                                                                                    left(left) ,
                                                                                                    right(right) ,
                                                                                                    parent(parent) {}

            Entry (const Entry &other) = default;
        };

        /**
         * see BidirectionalIterator at CppReference for help.
         *
         * if there is anything wrong throw invalid_iterator.
         *     like it = map.begin(); --it;
         *       or it = map.end(); ++end();
         */
        friend class iterator;
        class const_iterator;

        class iterator {
            friend class map;
        private:
            map* Map;
            Entry *pointer;
        public:
            iterator () {
                pointer = nullptr;
                Map=nullptr;
            }

            iterator (Entry *pointer,map* Map) : pointer(pointer),Map(Map) {}

            iterator (const iterator &other) {
                pointer = other.pointer;
                Map=other.Map;
            }

            /**
             * return a new iterator which pointer n-next elements
             *   even if there are not enough elements, just return the answer.
             * as well as operator-
             */

            iterator operator++ (int) {
                auto tmp = (*this);
                this->operator++();
                return tmp;
            }


            iterator &operator++ () {
                if (*this == Map->end()) {
                    throw invalid_iterator();
                }
                if (pointer->right != nullptr) {
                    pointer = pointer->right;
                    while (pointer->left != nullptr) {
                        pointer = pointer->left;
                    }
                } else {
                    auto tmp = pointer;
                    pointer = pointer->parent;
                    while (pointer!= nullptr &&tmp==pointer->right) {
                        tmp=pointer;
                        pointer=pointer->parent;

                    }
                }
                return (*this);
            }

            iterator operator-- (int) {
                auto tmp = (*this);
                this->operator--();
                return tmp;
            }


            iterator &operator-- () {
                if (*this == Map->begin()){
                    throw invalid_iterator();
                }
                if (pointer == nullptr) {
                    pointer=Map->root;
                    while (pointer->right != nullptr) {
                        pointer=pointer->right;
                    }
                    return (*this);
                }
                if (pointer->left != nullptr) {
                    pointer = pointer->left;
                    while (pointer->right != nullptr) {
                        pointer = pointer->right;
                    }
                } else {
                    auto tmp = pointer;
                    pointer = pointer->parent;
                    while (pointer!= nullptr &&tmp==pointer->left) {
                        tmp=pointer;
                        pointer=pointer->parent;
                    }
                }

                return (*this);
            }

            /**
             * a operator to check whether two iterators are same (pointing to the same memory).
             */
            Entry &operator* () const { return *pointer; }

            bool operator== (const iterator &rhs) const { return pointer == rhs.pointer&&Map==rhs.Map; }

            bool operator== (const const_iterator &rhs) const { return pointer == rhs.pointer&&Map==rhs.Map; }

            /**
             * some other operator for iterator.
             */
            bool operator!= (const iterator &rhs) const { return !((*this) == rhs); }

            bool operator!= (const const_iterator &rhs) const { return !((*this) == rhs); }

            /**
             * for the support of it->first.
             * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
             */
            Entry *operator-> () const noexcept { return pointer; }
            friend class const_iterator;
        };

        class const_iterator: public iterator {
            // it should has similar member method as iterator.
            //  and it should be able to construct from an iterator.

        public:
            const_iterator () :iterator() {

            }
            const_iterator(Entry* pointer,map* Map):iterator(pointer,Map){}
            const_iterator (const const_iterator &other) :iterator(other){}

            const_iterator (const iterator &other) :iterator(other){

            }

        };

    private:
        int length = 0;
        Entry *root;

        Entry *copytree (Entry *other) {
            if (other == nullptr) {
                return nullptr;
            }
            Entry* tmp= new Entry(other->first , other->second , other->color , copytree(other->left) , copytree
            (other->right) , nullptr);
            if (tmp->left != nullptr) {
                tmp->left->parent=tmp;
            }
            if (tmp->right != nullptr) {
                tmp->right->parent=tmp;
            }
            return tmp;

        }

        void cleartree (Entry *&root) {
            if (root != nullptr) {
                cleartree(root->left);
                cleartree(root->right);
                delete root;
            }
            root= nullptr;
        }

    public:
        map () {
            root = nullptr;
        }

        map (const map &other) {
            root = copytree(other.root);
            length=other.length;
        }


        map &operator= (const map &other) {
            if (this == &other) {
                return *this;
            }
            cleartree(root);
            root = copytree(other.root);
            length=other.length;
            return *this;
        }


        ~map () {
            cleartree(root);
        }

        /**
         *
         * access specified element with bounds checking
         * Returns a reference to the mapped value of the element with key equivalent to key.
         * If no such element exists, an exception of type `index_out_of_bound'
         */
    private:
        Entry *at (Entry *root , const Key &key)const {
            Compare comp =Compare();
            if (root == nullptr) {
                throw index_out_of_bound();
            }
            if (comp(root->first , key)) {
                return at(root->right , key);
            } else if (comp(key , root->first)) {
                return at(root->left , key);
            } else {
                return root;
            }
        }

    public:
        Value &at (const Key &key) {
            return at(root , key)->second;
        }

        const Value &at (const Key &key) const {
            return at(root , key)->second;
        }

        /**
         *
         * access specified element
         * Returns a reference to the value that is mapped to a key equivalent to key,
         *   performing an insertion if such key does not already exist.
         */
        Value &operator[] (const Key &key) {
            try {
                return at(key);
            } catch (index_out_of_bound) {
                Entry* a;
                root=(insert(root,pair<Key , Value>(key , Value()),a));
                length++;
                return this->at(key);
            }
        }

        /**
         * behave like at() throw index_out_of_bound if such key does not exist.
         */
        const Value &operator[] (const Key &key) const {
            return at(key);
        }

        /**
         * return a iterator to the beginning
         */
        iterator begin () {
            auto p = root;
            if (p == nullptr) {
                return iterator(p,this);
            }
            while (p->left != nullptr) {
                p = p->left;
            }
            return iterator(p,this);
        }

        const_iterator cbegin () const {
            auto p = root;
            if (p == nullptr) {
                return iterator(p, const_cast<map*>(this));
            }
            while (p->left != nullptr) {
                p = p->left;
            }
            return iterator(p, const_cast<map*>(this));
        }

        /**
         * return a iterator to the end
         * in fact, it returns past-the-end.
         */
        iterator end () {
            auto p = root;
            return iterator(nullptr,this);
        }

        const_iterator cend () const {
            auto p = root;
            return iterator(nullptr, const_cast<map*>(this));
        }

        /**
         * checks whether the container is empty
         * return true if empty, otherwise false.
         */
        bool empty () const {
            return root == nullptr;
        }

        /**
         * returns the number of elements.
         */
        size_t size () const {
            return length;
        }

        /**
         * clears the contents
         */
        void clear () {
            cleartree(root);
            length = 0;
        }

        /**
         * insert an element.
         * return a pair, the first of the pair is
         *   the iterator to the new element (or the element that prevented the insertion),
         *   the second one is true if insert successfully, or false.
         */
    private:
        bool isred (Entry *root) {
            return root != nullptr && root->color == RED;
        }


        Entry *insert (Entry *root , const pair<Key , Value> &keyval , Entry *&result) {
            Compare comp=Compare();
            if (root == nullptr) {
                result = new Entry(keyval.first , keyval.second , RED , nullptr , nullptr , nullptr);
                return result;
            }
            if (comp(root->first,keyval.first)) {
                root->right = insert(root->right , keyval,result);
                root->right->parent = root;
            } else if (comp(keyval.first, root->first)) {
                root->left = insert(root->left , keyval,result);
                root->left->parent = root;
            } else {
                result=root;
                throw InsertionFailure();
            }
            root=fixup(root);
            return root;


        }

    public:
        pair<iterator , bool> insert (const pair<Key , Value> &keyval) {
            Entry *p;
            if (root == nullptr) {
                root = new Entry(keyval.first , keyval.second , BLACK , nullptr , nullptr ,
                                 nullptr);
                length++;
                return pair<iterator,bool>(iterator(p,this),true);

            }
            try {
                root=insert(root , keyval , p);
                root->color = BLACK;
                length++;
                return pair<iterator , bool>(iterator(p,this) , true);
            } catch (InsertionFailure) {
                return pair<iterator , bool>(iterator(p,this) , false);
            }

        }

        /**
         * erase the element at pos.
         *
         * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
         */
    private:

        Entry *fixup (Entry *root) {
            if (root == nullptr) {
                return root;
            }
            if (isred(root->right)) {
                root = rotateleft(root);
            }
            if (isred(root->left) && isred(root->left->left)) {
                root = rotateright(root);
            }
            if (isred(root->left) && isred(root->right)) {
                colorflip(root);
            }
            return root;
        }

        Entry *moveredleft (Entry *root) {
            colorflip(root);
            if (root->right!= nullptr&&isred(root->right->left)) {
                root->right = rotateright(root->right);
                root = rotateleft(root);
                colorflip(root);
            }
            return root;
        }

        Entry *moveredright (Entry *root) {
            colorflip(root);
            if (root->left!=nullptr&&isred(root->left->left)) {
                root = rotateright(root);
                colorflip(root);
            }
            return root;
        }

        Entry *deleteMin (Entry *root , Entry *&deleted) {
            if (root == nullptr) {
                return nullptr;
            }
            if (root->left == nullptr) {
                auto tmp=root->right;
                deleted=root;
                return tmp;
            }
            if (!isred(root->left) && !isred(root->left->left)&&root->right!= nullptr) {
                root = moveredleft(root);
            }
            root->left = deleteMin(root->left , deleted);
            if(root->left!= nullptr) {
                root->left->parent = root;
            }
            root=fixup(root);
            return root;
        }


        Entry *erase (Entry *root , const Key &key , const Compare &comp ) {
            if (root == nullptr) {
                throw invalid_iterator();
            }
            if (comp(key, root->first)) {
                if (root->left!= nullptr&&!isred(root->left) && !isred(root->left->left)) {
                    root = moveredleft(root);
                }
                root->left = erase(root->left , key , comp );
            } else {
                if (isred(root->left)) {
                    root = rotateright(root);
                }
                if (!comp(key , root->first) && !comp(root->first , key) && root->right == nullptr) {
                    delete root;
                    return nullptr;
                }
                if (!isred(root->right) && !isred(root->right->left)) {
                    root = moveredright(root);
                }
                if(!comp(key , root->first) && !comp(root->first , key)) {

                    auto tmp = (++iterator(root,this)).pointer;
                    Entry* deleted;
                    root->right = deleteMin(root->right ,deleted);
                    if (root->parent != nullptr&&root->parent->left==root) {
                        root->parent->left=deleted;
                    } else if (root->parent != nullptr) {
                        root->parent->right=deleted;
                    }
                    if (root->left != nullptr) {
                        root->left->parent=deleted;
                    }
                    if (root->right != nullptr) {
                        root->right->parent=deleted;
                    }
                    deleted->color=root->color;
                    deleted->parent=root->parent;
                    deleted->right=root->right;
                    deleted->left=root->left;
                    delete root;
                    root=deleted;
                } else {
                    root->right= erase(root->right , key , comp );
                }

            }
            root=fixup(root);
            return root;
        }

    public:
        void erase (iterator iter) {
            if (iter == end()||iter.Map!=this) {
                throw invalid_iterator();
            }
            if (!isred(root->left) && !isred(root->right)) {
                root->color = RED;
            }
            Compare comp=Compare();

            root= erase(root , iter->first , comp );
            length--;

            if(root!= nullptr) {
                root->color = BLACK;
            }
        }

        /**
         * Returns the number of elements with key
         *   that compares equivalent to the specified argument,
         *   which is either 1 or 0
         *     since this container does not allow duplicates.
         * The default method of check the equivalence is !(a < b || b > a)
         */
        size_t count (const Key &key) const {
            try {
                at(root , key);
                return 1;
            } catch (index_out_of_bound) {
                return 0;
            }
        }

        /**
         * Finds an element with key equivalent to key.
         * key value of the element to search for.
         * Iterator to an element with key equivalent to key.
         *   If no such element is found, past-the-end (see end()) iterator is returned.
         */
        iterator find (const Key &key) {
            try {
                auto p = at(root , key);
                return iterator(p,this);
            } catch (index_out_of_bound) {
                return end();
            }


        }

        const_iterator find (const Key &key) const {
            try {
                auto p = at(root , key);
                return const_iterator(p, const_cast<map*>(this));
            } catch (index_out_of_bound) {
                return const_iterator(iterator(nullptr, const_cast<map*>(this)));
            }
        }

    private:
        void colorflip (Entry *root) {

            root->left->color = !root->left->color;
            root->color = !root->color;
            root->right->color = !root->right->color;
        }

        Entry *rotateleft (Entry *root) {
            auto tmp = root->right;
            root->right = root->right->left;
            if(root->right!=nullptr) {
                root->right->parent = root;
            }
            tmp->left = root;
            if(root->parent!= nullptr) {
                if (root->parent->left == root) {
                    root->parent->left = tmp;
                } else {
                    root->parent->right = tmp;
                }
            }
            tmp->parent = root->parent;
            root->parent = tmp;
            std::swap(root->color , tmp->color);
            return tmp;

        }

        Entry *rotateright (Entry *root) {
            auto tmp = root->left;
            root->left = root->left->right;
            if(root->left!= nullptr) {
                root->left->parent = root;
            }
            tmp->right = root;
            if(root->parent!= nullptr) {
                if (root->parent->left == root) {
                    root->parent->left = tmp;
                } else {
                    root->parent->right = tmp;
                }
            }
            tmp->parent = root->parent;
            root->parent = tmp;
            std::swap(root->color , tmp->color);
            return tmp;
        }
    };


}

#endif