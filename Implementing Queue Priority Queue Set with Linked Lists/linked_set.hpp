#ifndef LINKED_SET_HPP_
#define LINKED_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedSet {
  public:
    //Destructor/Constructors
    ~LinkedSet();

    LinkedSet          ();
    explicit LinkedSet (int initialLength);
    LinkedSet          (const LinkedSet<T>& to_copy);
    explicit LinkedSet (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedSet (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    bool contains   (const T& element) const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    bool contains_all (const Iterable& i) const;


    //Commands
    int  insert (const T& element);
    int  erase  (const T& element);
    void clear  ();

    //Iterable class must support "for" loop: .begin()/.end() and prefix ++ on returned result

    template <class Iterable>
    int insert_all(const Iterable& i);

    template <class Iterable>
    int erase_all(const Iterable& i);

    template<class Iterable>
    int retain_all(const Iterable& i);


    //Operators
    LinkedSet<T>& operator = (const LinkedSet<T>& rhs);
    bool operator == (const LinkedSet<T>& rhs) const;
    bool operator != (const LinkedSet<T>& rhs) const;
    bool operator <= (const LinkedSet<T>& rhs) const;
    bool operator <  (const LinkedSet<T>& rhs) const;
    bool operator >= (const LinkedSet<T>& rhs) const;
    bool operator >  (const LinkedSet<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedSet<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedSet<T>::Iterator& operator ++ ();
        LinkedSet<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedSet<T>::Iterator& rhs) const;
        bool operator != (const LinkedSet<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedSet<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedSet<T>::begin () const;
        friend Iterator LinkedSet<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*           current;  //if can_erase is false, this value is unusable
        LinkedSet<T>* ref_set;
        int           expected_mod_count;
        bool          can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedSet<T>* iterate_over, LN* initial);
    };


    Iterator begin () const;
    Iterator end   () const;


  private:
    class LN {
      public:
        LN ()                      {}
        LN (const LN& ln)          : value(ln.value), next(ln.next){}
        LN (T v,  LN* n = nullptr) : value(v), next(n){}

        T   value;
        LN* next   = nullptr;
    };


    LN* front     = new LN();
    LN* trailer   = front;         //Always point to the special trailer LN
    int used      =  0;            //Cache the number of values in linked list
    int mod_count = 0;             //For sensing concurrent modification

    //Helper methods
    int  erase_at   (LN* p);
    void delete_list(LN*& front);  //Deallocate all LNs (but trailer), and set front's argument to trailer;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedSet class and related definitions

//Destructor/Constructors

template<class T>
LinkedSet<T>::~LinkedSet() {
    delete_list(front);
}


template<class T>
LinkedSet<T>::LinkedSet() {
}


template<class T>
LinkedSet<T>::LinkedSet(const LinkedSet<T>& to_copy) : used(to_copy.used) {
    if(to_copy.size() ==0)
        return;
    LN* target = front->next;
    for(LN* source = to_copy.front->next; source != to_copy.trailer; source = source->next,target = target->next)
        target = new LN(source->value, nullptr);
    trailer = target = new LN();
    ++mod_count;
}
//copy one by one first and then add tralier

template<class T>
LinkedSet<T>::LinkedSet(const std::initializer_list<T>& il) {
    for(const auto& temp : il)
        insert(temp);
}


template<class T>
template<class Iterable>
LinkedSet<T>::LinkedSet(const Iterable& i) {
    for (const T& v : i)
        insert(v);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedSet<T>::empty() const {
    return used == 0;
}


template<class T>
int LinkedSet<T>::size() const {
    return used;
}


template<class T>
bool LinkedSet<T>::contains (const T& element) const {
    if(used == 0)
        return false;
    for(LN* current = front ->next; current != trailer; current= current->next )
        if(current->value == element)
            return true;
    return false;
}

//linked_set[c->b->a->TRAILER](used=3,front=0x7424c8,trailer=0x742498,mod_count=3).
template<class T>
std::string LinkedSet<T>::str() const {
    std::ostringstream answer;
    answer <<"linked_set[";
    LN* current = front->next;
    if (current != nullptr) {
        while (current != trailer){
            answer << current ->value <<"->";
            current = current ->next;
        }
    }
    answer <<"TRAILER]" <<"(used=" << size()<<",front="<<&front << ",trailer="<<&trailer
           <<",mod_count="<<mod_count<<");"<<std::endl;
    return answer.str();
}


template<class T>
template<class Iterable>
bool LinkedSet<T>::contains_all (const Iterable& i) const {
    for(const T& temp: i )
        if(!contains(temp))
            return false;
    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands


template<class T>
int LinkedSet<T>::insert(const T& element) {
    if(contains(element))
        return 0;
    LN* nextN = front->next;
    if(nextN == nullptr) {
        trailer = new LN();
        front->next = new LN(element, trailer);
    }
    else
        front ->next = new LN(element, nextN);
    ++used;
    ++mod_count;
    return 1;
}


template<class T>
int LinkedSet<T>::erase(const T& element) {
    if(!contains(element))
        return 0;
    if (this->empty())
        throw EmptyError("LinkedSet::erase");
    for(LN* current = front->next; current != trailer; current = current->next)
        if(current->value == element)
            return erase_at(current);
}


template<class T>
void LinkedSet<T>::clear() {
    delete_list(front);
    used =0;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::insert_all(const Iterable& i) {
    int count = 0;
    for(const auto& source : i)
        count += insert(source);
    ++ mod_count;
    return count;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::erase_all(const Iterable& i) {
    int count = 0;
    for (const T& v : i)
        count += erase(v);
    ++ mod_count;
    return count;
}


template<class T>
template<class Iterable>
int LinkedSet<T>::retain_all(const Iterable& i) {
    if(used == 0)
        return 0;
    LinkedSet<T> s(i);
    int count = 0;
    LN* current = front->next;
    while(current != trailer){
        if(!s.contains(current->value)) {
            erase_at(current);
            ++count;
            //continue;
        }
        else
            current = current->next;
    }
    return count;
}

/*
 * Expected: s
      Which is: set[j,i,h,g,f,e,d,c,b,a]
To be equal to: s1
      Which is: set[j,i,d,b,a]
 */

////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedSet<T>& LinkedSet<T>::operator = (const LinkedSet<T>& rhs) {
    if (this == &rhs )
        return *this;
    if (rhs.size() == 0){
        delete_list(front);
        return *this;
    }

//    LN* current = front;
//    if(current == trailer){
//        current->next =
//    }

    LN* source = rhs.front->next;
    LN* p = front->next;
    for(; p != trailer; p = p->next,source = source ->next){//iter through every nodes in the linkedQueue
        if(source ->next == rhs.trailer){ //length <= rhs.size
            p ->value = source->value;
            *trailer = *(rhs.trailer);
        }
        else{
            delete_list(p);
            break;
        }
    }
    while(source != rhs.trailer){
        if(p == trailer)
            p ->value = source->value;
        else{
            p = new LN(source->value, nullptr);
        }
        p = p->next;
        source = source ->next;
    }
    if(trailer->next != nullptr)
        trailer = p;

    ++mod_count;
    used = rhs.used;

    return *this;
}


template<class T>
bool LinkedSet<T>::operator == (const LinkedSet<T>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.size())
        return false;
    if(used == 0 && rhs.size()==0 )
        return true;

    LN* compare = rhs.front ->next;
    for(LN* compare = rhs.front ->next; compare != rhs.trailer; compare = compare->next){
        if(!contains(compare->value))
            return false;
    }
    return true;

}


template<class T>
bool LinkedSet<T>::operator != (const LinkedSet<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
bool LinkedSet<T>::operator <= (const LinkedSet<T>& rhs) const {
    if (this == &rhs)
        return true;
    if (used == 0)
        return true;
    if (used > rhs.size())
        return false;

     for(LN* current = front->next; current != trailer; current = current ->next)
        if (!rhs.contains(current->value))
            return false;
    return true;
}

//s1 >= s2
    //"a" >= ""
template<class T>
bool LinkedSet<T>::operator < (const LinkedSet<T>& rhs) const {
    if (this == &rhs)
        return false;
    if (rhs.size()==0)
        return false;
    if (used == 0)
        return true;
    if (used >= rhs.size())
        return false;

    for(LN* current = front->next; current != trailer; current = current ->next)
        if (!rhs.contains(current->value))
            return false;

    return true;
}
//"a">""-->""<"a"
template<class T>
bool LinkedSet<T>::operator >= (const LinkedSet<T>& rhs) const {
    return rhs <= *this;
}


template<class T>
bool LinkedSet<T>::operator > (const LinkedSet<T>& rhs) const {
    return rhs < *this;
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedSet<T>& s) {
    outs << "set[";
    if(s.used != 0) {
        outs <<s.front->next->value;
        for(typename LinkedSet<T>::LN* current = s.front->next->next;current != s.trailer; current = current->next)
            outs<<"," << current->value;
    }
    outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedSet<T>::begin () const -> LinkedSet<T>::Iterator {
    return Iterator(const_cast<LinkedSet<T>*>(this),front->next);
}


template<class T>
auto LinkedSet<T>::end () const -> LinkedSet<T>::Iterator {
    return Iterator(const_cast<LinkedSet<T>*>(this), trailer);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
int LinkedSet<T>::erase_at(LN* p) {
    if(p->next == trailer)
        trailer = p;
    LN* to_delete = p->next;
    *p = *to_delete;
    delete to_delete;
    --used;
    ++mod_count;
    return 1;

}

template<class T>
void LinkedSet<T>::delete_list(LN*& front) {
    while(front->next != nullptr)
        erase_at(front);
    trailer = front;

}
////Deallocate all LNs (but trailer), and set front's argument to trailer;




////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedSet<T>::Iterator::Iterator(LinkedSet<T>* iterate_over, LN* initial)
:current(initial), ref_set(iterate_over), expected_mod_count(ref_set->mod_count)
{
}


template<class T>
LinkedSet<T>::Iterator::~Iterator()
{}


template<class T>
T LinkedSet<T>::Iterator::erase() {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor already erased");
    if (current == nullptr)
        throw CannotEraseError("LinkedSet::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    T to_return = current->value;
    ref_set->erase_at(current);
    expected_mod_count = ref_set->mod_count;
    return to_return;
}


template<class T>
std::string LinkedSet<T>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_set->str() << "(current=" << current << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ () -> LinkedSet<T>::Iterator& {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++");

    if (current == ref_set->trailer)
        return *this;

    if (can_erase)
        current = current->next;
    else
        can_erase = true;  //current already indexes "one beyond" erased value

    return *this;
}


template<class T>
auto LinkedSet<T>::Iterator::operator ++ (int) -> LinkedSet<T>::Iterator {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ++(int)");

    if (current == ref_set->trailer)
        return *this;

    Iterator to_return(*this);
    if (can_erase)
        current= current->next;
    else
        can_erase = true;  //current already indexes "one beyond" erased value

    return to_return;
}


template<class T>
bool LinkedSet<T>::Iterator::operator == (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedSet::Iterator::operator ==");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator ==");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator ==");

    return current == rhsASI->current;
}


template<class T>
bool LinkedSet<T>::Iterator::operator != (const LinkedSet<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedSet::Iterator::operator !=");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator !=");
    if (ref_set != rhsASI->ref_set)
        throw ComparingDifferentIteratorsError("LinkedSet::Iterator::operator !=");

    return current != rhsASI->current;
}


template<class T>
T& LinkedSet<T>::Iterator::operator *() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator *");
    if (!can_erase || current == ref_set->front || current == ref_set->trailer|| current == nullptr) {
        std::ostringstream where;
        where << current << " when size = " << ref_set->size();
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator * Iterator illegal: "+where.str());
    }

    return current->value;
}


template<class T>
T* LinkedSet<T>::Iterator::operator ->() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("LinkedSet::Iterator::operator *");
    if (!can_erase || current == nullptr) {//current == ref_set->front || current == ref_set->trailer||
        std::ostringstream where;
        where << current << " when size = " << ref_set->size();
        throw IteratorPositionIllegal("LinkedSet::Iterator::operator * Iterator illegal: "+where.str());
    }

    return current;
}


}

#endif /* LINKED_SET_HPP_ */
