// Submitter: lizhenl(Lizhen, Lin)
// Partner  : qiaoh3(Qiao, He)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming

#ifndef LINKED_QUEUE_HPP_
#define LINKED_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"


namespace ics {


template<class T> class LinkedQueue {
  public:
    //Destructor/Constructors
    ~LinkedQueue();

    LinkedQueue          ();
    LinkedQueue          (const LinkedQueue<T>& to_copy);
    explicit LinkedQueue (const std::initializer_list<T>& il);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedQueue (const Iterable& i);


    //Queries
    bool empty      () const;
    int  size       () const;
    T&   peek       () const;
    std::string str () const; //supplies useful debugging information; contrast to operator <<


    //Commands
    int  enqueue (const T& element);
    T    dequeue ();
    void clear   ();

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    int enqueue_all (const Iterable& i);


    //Operators
    LinkedQueue<T>& operator = (const LinkedQueue<T>& rhs);
    bool operator == (const LinkedQueue<T>& rhs) const;
    bool operator != (const LinkedQueue<T>& rhs) const;

    template<class T2>
    friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T2>& q);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedQueue<T>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedQueue<T>::Iterator& operator ++ ();
        LinkedQueue<T>::Iterator  operator ++ (int);
        bool operator == (const LinkedQueue<T>::Iterator& rhs) const;
        bool operator != (const LinkedQueue<T>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedQueue<T>::begin () const;
        friend Iterator LinkedQueue<T>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev = nullptr;  //if nullptr, current at front of list
        LN*             current;         //current == prev->next (if prev != nullptr)
        LinkedQueue<T>* ref_queue;
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedQueue<T>* iterate_over, LN* initial);
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
        LN* next = nullptr;
    };


    LN* front     =  nullptr;
    LN* rear      =  nullptr;
    int used      =  0;            //Cache count of nodes in linked list
    int mod_count =  0;            //Alllows sensing concurrent modification

    //Helper methods
    void delete_list(LN*& front);  //Deallocate all LNs, and set front's argument to nullptr;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedQueue class and related definitions

//Destructor/Constructors

template<class T>
LinkedQueue<T>::~LinkedQueue() {
    clear();
}


template<class T>
LinkedQueue<T>::LinkedQueue() {
}


template<class T>
LinkedQueue<T>::LinkedQueue(const LinkedQueue<T>& to_copy) {
    if(to_copy.size() ==0){
        front = rear = nullptr;
        return;
    }
    rear = front = new LN(to_copy.front->value, nullptr);
    LN* source = to_copy.front->next;
    for( ; source != nullptr; source = source->next)
        rear = rear->next = new LN(source->value, nullptr);
    used = to_copy.used;
    ++mod_count;
}


template<class T>
LinkedQueue<T>::LinkedQueue(const std::initializer_list<T>& il) {
    clear();
    for(const auto& temp : il)
        enqueue(temp);
}


template<class T>
template<class Iterable>
LinkedQueue<T>::LinkedQueue(const Iterable& i) {
    clear();
    for(const auto& temp : i)
        enqueue(temp);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T>
bool LinkedQueue<T>::empty() const {
    return used == 0;
}


template<class T>
int LinkedQueue<T>::size() const {
    return used;
}


template<class T>
T& LinkedQueue<T>::peek () const {
    if(this -> empty())
        throw EmptyError("LinkedQueue::peek");
    return front->value;
}

//linked_queue[a->b->c](used=3,front=0x642498,rear=0x6424b8,mod_count=3);
template<class T>
std::string LinkedQueue<T>::str() const {
    std::ostringstream answer;
    answer <<"linked_queue[";
    LN* current = front;
    if (current != nullptr) {
        answer << front ->value;
        current = current ->next;
        while (current != nullptr){
            answer <<"->"<< current ->value;
            current = current ->next;
        }
    }
    answer <<"]" <<"(used=" << size()<<",front="<<&front<<",rear="<<&rear
           <<",mod_count="<<mod_count<<");"<<std::endl;
    return answer.str();
}

////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T>
int LinkedQueue<T>::enqueue(const T& element) {
    if(front == nullptr)
        rear= front = new LN(element, nullptr);
    else
        rear = rear->next = new LN(element, nullptr);
    ++used;
    ++mod_count;
    return 1;
}


template<class T>
T LinkedQueue<T>::dequeue() {
    if(front == nullptr)
        throw EmptyError("LinkedQueue::dequeue");
    LN* to_delete = front;
    T answer = front->value;
    front = front->next;
    delete to_delete;
    --used;
    ++mod_count;
    return answer;
}


template<class T>
void LinkedQueue<T>::clear() {
    while(front != nullptr)
        dequeue();
    rear = front = nullptr;
    ++mod_count;
}


template<class T>
template<class Iterable>
int LinkedQueue<T>::enqueue_all(const Iterable& i) {
    int count = 0;
    for(const auto& source : i)
        count += enqueue(source);
    ++ mod_count;
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T>
LinkedQueue<T>& LinkedQueue<T>::operator = (const LinkedQueue<T>& rhs) {
    if (this == &rhs)
        return *this;
    LN* source = rhs.front;
    for(LN* p = front; p != nullptr; p = p->next){//iter through every nodes in the linkedQueue
        if(source != nullptr){ //length <= rhs.size
            p ->value = source->value;
            source = source ->next;
            rear = p;
        }
        else{//length > rhs.size
           delete_list(p);
            break;
        }
    }
    while(source != nullptr){
        enqueue(source ->value);
        source = source ->next;
    }
    rear -> next = nullptr;
    used = rhs.used;
}


template<class T>
bool LinkedQueue<T>::operator == (const LinkedQueue<T>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.used)
        return false;

    LN* current = front;
    LN* compare = rhs.front;
    while(current != nullptr){
        if(current->value != compare->value)
            return false;
        current = current ->next;
        compare = compare ->next;
    }
    return true;
}


template<class T>
bool LinkedQueue<T>::operator != (const LinkedQueue<T>& rhs) const {
    return !(*this == rhs);
}


template<class T>
std::ostream& operator << (std::ostream& outs, const LinkedQueue<T>& q) {
    outs << "queue[";
    if(q.front != nullptr) {
        for (typename LinkedQueue<T>::LN *p = q.front; p != q.rear; p = p->next)
            outs << p ->value <<",";
        outs << q.rear ->value;
    }
    outs << "]:rear";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T>
auto LinkedQueue<T>::begin () const -> LinkedQueue<T>::Iterator {
    return Iterator(const_cast<LinkedQueue<T>*>(this),front);
}

template<class T>
auto LinkedQueue<T>::end () const -> LinkedQueue<T>::Iterator {
    return Iterator(const_cast<LinkedQueue<T>*>(this), nullptr);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T>
void LinkedQueue<T>::delete_list(LN*& front) {
    while(front != nullptr){
        LN* to_delete = front;
        front = front ->next;
        delete to_delete;
    }
    front = nullptr;
}





////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T>
LinkedQueue<T>::Iterator::Iterator(LinkedQueue<T>* iterate_over, LN* initial)
    :current(initial), ref_queue(iterate_over), expected_mod_count(ref_queue->mod_count)
{
}


template<class T>
LinkedQueue<T>::Iterator::~Iterator()
{
}


template<class T>
T LinkedQueue<T>::Iterator::erase() {
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor already erased");
    if (current == nullptr)
        throw CannotEraseError("LinkedQueue::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    T to_return = current->value;
    if(prev == nullptr) {
        current = current->next;
        ref_queue ->dequeue();
        //return (*ref_queue).dequeue();
    }
    else {
        LN *to_delete = current;

        prev->next = current = current->next;
        delete to_delete;
        if(current == nullptr)
            ref_queue->rear = prev;
        ref_queue -> used--;

    }
    expected_mod_count = ref_queue->mod_count;
    return to_return;
}


template<class T>
std::string LinkedQueue<T>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_queue->str() << "(current=";
    current != nullptr ? answer<< current->value : answer <<"nullptr";
    answer<< ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ () -> LinkedQueue<T>::Iterator& {
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++");
    if (current == ref_queue->rear->next)
        return *this;

    if (can_erase){
        prev = current;
        current = current ->next;
    }
    else
        can_erase = true;  //current already indexes "one beyond" deleted value
    return *this;
}


template<class T>
auto LinkedQueue<T>::Iterator::operator ++ (int) -> LinkedQueue<T>::Iterator {
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ++(int)");
    if (current == ref_queue->rear->next)
        return *this;

    Iterator to_return(*this);
    if (can_erase) {
        prev = current;
        current = current ->next;
    }
    else
        can_erase = true;  //current already indexes "one beyond" deleted value

    return to_return;
}
//post

template<class T>
bool LinkedQueue<T>::Iterator::operator == (const LinkedQueue<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedQueue::Iterator::operator ==");
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ==");
    if (ref_queue != rhsASI->ref_queue)
        throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator ==");

    return current == rhsASI->current;
}


template<class T>
bool LinkedQueue<T>::Iterator::operator != (const LinkedQueue<T>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedQueue::Iterator::operator !=");
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator !=");
    if (ref_queue != rhsASI->ref_queue)
        throw ComparingDifferentIteratorsError("LinkedQueue::Iterator::operator !=");

    return current != rhsASI->current;
}


template<class T>
T& LinkedQueue<T>::Iterator::operator *() const {
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator *");
    if (!can_erase || current == nullptr) {
        std::ostringstream where;
        where << " when front = " << ref_queue->front
              << " and rear = " << ref_queue->rear;
        throw IteratorPositionIllegal("LinkedQueue::Iterator::operator * Iterator illegal: "+where.str());
    }

    return current ->value;
}


template<class T>
T* LinkedQueue<T>::Iterator::operator ->() const {
    if (expected_mod_count != ref_queue->mod_count)
        throw ConcurrentModificationError("LinkedQueue::Iterator::operator ->");
    if (!can_erase || current == nullptr) {
        std::ostringstream where;
        where << " when front = " << ref_queue->front
              << " and rear = " << ref_queue->rear;
        throw IteratorPositionIllegal("LinkedQueue::Iterator::operator -> Iterator illegal: "+where.str());
    }

    return current;
}


}

#endif /* LINKED_QUEUE_HPP_ */
