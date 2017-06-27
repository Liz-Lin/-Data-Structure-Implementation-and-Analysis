// Submitter: lizhenl(Lizhen, Lin)
// Partner  : qiaoh3(Qiao, He)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming

#ifndef LINKED_PRIORITY_QUEUE_HPP_
#define LINKED_PRIORITY_QUEUE_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "array_stack.hpp"      //See operator <<


namespace ics {


#ifndef undefinedgtdefined
#define undefinedgtdefined
template<class T>
bool undefinedgt (const T& a, const T& b) {return false;}
#endif /* undefinedgtdefined */

//Instantiate the templated class supplying tgt(a,b): true, iff a has higher priority than b.
//If tgt is defaulted to undefinedgt in the template, then a constructor must supply cgt.
//If both tgt and cgt are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedgt value supplied by tgt/cgt is stored in the instance variable gt.
template<class T, bool (*tgt)(const T& a, const T& b) = undefinedgt<T>> class LinkedPriorityQueue {
  public:
    //Destructor/Constructors
    ~LinkedPriorityQueue();

    LinkedPriorityQueue          (bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    LinkedPriorityQueue          (const LinkedPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);
    explicit LinkedPriorityQueue (const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit LinkedPriorityQueue (const Iterable& i, bool (*cgt)(const T& a, const T& b) = undefinedgt<T>);


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
    LinkedPriorityQueue<T,tgt>& operator = (const LinkedPriorityQueue<T,tgt>& rhs);
    bool operator == (const LinkedPriorityQueue<T,tgt>& rhs) const;
    bool operator != (const LinkedPriorityQueue<T,tgt>& rhs) const;

    template<class T2, bool (*gt2)(const T2& a, const T2& b)>
    friend std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T2,gt2>& pq);



  private:
    class LN;

  public:
    class Iterator {
      public:
        //Private constructor called in begin/end, which are friends of LinkedPriorityQueue<T,tgt>
        ~Iterator();
        T           erase();
        std::string str  () const;
        LinkedPriorityQueue<T,tgt>::Iterator& operator ++ ();
        LinkedPriorityQueue<T,tgt>::Iterator  operator ++ (int);
        bool operator == (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const;
        bool operator != (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T,tgt>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator LinkedPriorityQueue<T,tgt>::begin () const;
        friend Iterator LinkedPriorityQueue<T,tgt>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        LN*             prev = ref_pq->front;            //initialize prev to the header
        LN*             current;         //current == prev->next
        LinkedPriorityQueue<T,tgt>* ref_pq;
        int             expected_mod_count;
        bool            can_erase = true;

        //Called in friends begin/end
        Iterator(LinkedPriorityQueue<T,tgt>* iterate_over, LN* initial);
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


    bool (*gt) (const T& a, const T& b); // The gt used by enqueue (from template or constructor)
    LN* front     =  new LN();
    int used      =  0;                  //Cache count of nodes in linked list
    int mod_count =  0;                  //Allows sensing concurrent modification

    //Helper methods
    void delete_list(LN*& front);        //Deallocate all LNs, and set front's argument to nullptr;
    std::string printReverse(LN*& front)const;
};





////////////////////////////////////////////////////////////////////////////////
//
//LinkedPriorityQueue class and related definitions

//Destructor/Constructors

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::~LinkedPriorityQueue() {
    delete_list(front ->next);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(bool (*cgt)(const T& a, const T& b))
: gt(tgt != undefinedgt<T> ? tgt : cgt) {
    if (gt == undefinedgt<T>)
        throw TemplateFunctionError("LinkedPriorityQueue::default constructor: neither specified");
    if (tgt != undefinedgt<T> && cgt != undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("LinkedPriorityQueue::default constructor: both specified and different");
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const LinkedPriorityQueue<T,tgt>& to_copy, bool (*cgt)(const T& a, const T& b))
: gt(tgt != undefinedgt<T> ? tgt : cgt) {
    if (gt == undefinedgt<T>)
        gt = to_copy.gt;//throw TemplateFunctionError("ArrayPriorityQueue::copy constructor: neither specified");
    if (tgt != undefinedgt<T> && cgt != undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("LinkedPriorityQueue::copy constructor: both specified and different");
    if(to_copy.size() ==0)
        return;

    front->next = new LN(to_copy.front->next->value, nullptr);
    LN* target = front ->next;
    LN* source = to_copy.front->next->next;
    for( ; source != nullptr; source = source->next, target = target->next)
        target->next = new LN(source->value, nullptr);
    used = to_copy.used;
    ++mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const std::initializer_list<T>& il, bool (*cgt)(const T& a, const T& b))
: gt(tgt != undefinedgt<T> ? tgt : cgt) {
    if (gt == undefinedgt<T>)
        throw TemplateFunctionError("LinkedPriorityQueue::initializer_list constructor: neither specified");
    if (tgt != undefinedgt<T> && cgt != undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("LinkedPriorityQueue::initializer_list constructor: both specified and different");

    for(const auto& temp : il)
        enqueue(temp);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template<class Iterable>
LinkedPriorityQueue<T,tgt>::LinkedPriorityQueue(const Iterable& i, bool (*cgt)(const T& a, const T& b))
: gt(tgt != undefinedgt<T> ? tgt : cgt) {
    if (gt == undefinedgt<T>)
        throw TemplateFunctionError("LinkedPriorityQueue::Iterable constructor: neither specified");
    if (tgt != undefinedgt<T> && cgt != undefinedgt<T> && tgt != cgt)
        throw TemplateFunctionError("LinkedPriorityQueue::Iterable constructor: both specified and different");
    clear();
    for (const T& temp : i)
        enqueue(temp);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::empty() const {
    return used == 0;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
int LinkedPriorityQueue<T,tgt>::size() const {
    return used;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T& LinkedPriorityQueue<T,tgt>::peek () const {
    if(this -> empty())
        throw EmptyError("LinkedQueue::peek");
    return front ->next ->value;
}

//linked_priority_queue[HEADER->a->b->c](used=3,front=0x3f2498,mod_count=4);
template<class T, bool (*tgt)(const T& a, const T& b)>
std::string LinkedPriorityQueue<T,tgt>::str() const {
    std::ostringstream answer;
    answer <<"linked_priority_queue[HEADER";
    LN* current = front->next;
    if (current != nullptr) {
        answer <<"->"<< current ->value;
        current = current ->next;
        while (current != nullptr){
            answer <<"->"<< current ->value;
            current = current ->next;
        }
    }
    answer <<"]" <<"(used=" << size()<<",front="<<&front
           <<",mod_count="<<mod_count<<");"<<std::endl;
    return answer.str();
}



////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, bool (*tgt)(const T& a, const T& b)>
int LinkedPriorityQueue<T,tgt>::enqueue(const T& element) {
    if(front -> next == nullptr)
        front ->next = new LN(element, nullptr);
    else{
        LN* prev = front;
        LN* current = front->next;
        while(current != nullptr){
            if(gt(element, current->value)) {
                prev->next = new LN(element, current);
                break;
            }
            prev = current;
            current = current->next;
        }
        if(current == nullptr)
            prev ->next = new LN(element, nullptr);
    }
    ++used;
    ++mod_count;
//    std::cout <<str();
    return 1;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
T LinkedPriorityQueue<T,tgt>::dequeue() {
    if (this->empty())
        throw EmptyError("LinkedPriorityQueue::dequeue");
    LN* to_delete = front ->next;
    T to_return = to_delete->value;
    front->next = front ->next->next;
    delete  to_delete;
    --used;
    ++mod_count;
    return to_return;
}



template<class T, bool (*tgt)(const T& a, const T& b)>
void LinkedPriorityQueue<T,tgt>::clear() {
    delete_list(front->next);
    front ->next = nullptr;
    ++mod_count;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
template <class Iterable>
int LinkedPriorityQueue<T,tgt>::enqueue_all (const Iterable& i) {
    int count = 0;
    for(const auto& source : i)
        count += enqueue(source);
    ++ mod_count;
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>& LinkedPriorityQueue<T,tgt>::operator = (const LinkedPriorityQueue<T,tgt>& rhs) {
    if (this == &rhs)
        return *this;
    gt = rhs.gt;   // if tgt != undefinedgt, gts are already equal (or compiler error)

    LN* source = rhs.front->next;
    LN* p = front->next;
    for(; p != nullptr; p = p->next){//iter through every nodes in the linkedQueue
        if(source != nullptr){ //length <= rhs.size
            p ->value = source->value;
            p ->next = nullptr;
            source = source ->next;
        }
        else{
            delete_list(p);
            break;
        }
    }
    while(source != nullptr){
        enqueue(source->value);
        source = source ->next;
    }
    ++mod_count;
    used = rhs.used;

    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::operator == (const LinkedPriorityQueue<T,tgt>& rhs) const {
    if (this == &rhs)
        return true;
    if (gt != rhs.gt) //For PriorityQueues to be equal, they need the same gt function, and values
        return false;
    if (used != rhs.size())
        return false;

    LN* current = front ->next;
    LN* compare = rhs.front ->next;
    while(current != nullptr){
        if(current->value != compare->value)
            return false;
        current = current ->next;
        compare = compare ->next;
    }
    return true;
}



template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::operator != (const LinkedPriorityQueue<T,tgt>& rhs) const {
    return !(*this == rhs);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
std::ostream& operator << (std::ostream& outs, const LinkedPriorityQueue<T,tgt>& pq) {
    outs << "priority_queue[";
    typename LinkedPriorityQueue<T,tgt>::LN* p = pq.front->next;
    if(p != nullptr) {
        if(p->next != nullptr)
            outs << pq.printReverse(p->next);
        outs << p->value;
    }
    outs << "]:highest";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::begin () const -> LinkedPriorityQueue<T,tgt>::Iterator {
    return Iterator(const_cast<LinkedPriorityQueue<T,tgt>*>(this),front->next);
    //return Iterator(const_cast<LinkedQueue<T>*>(this),front);
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::end () const -> LinkedPriorityQueue<T,tgt>::Iterator {
    return Iterator(const_cast<LinkedPriorityQueue<T,tgt>*>(this), nullptr);
}


////////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, bool (*tgt)(const T& a, const T& b)>
void LinkedPriorityQueue<T,tgt>::delete_list(LN*& front) {
    while(front != nullptr){
        LN* to_delete = front;
        front = front ->next;
        delete to_delete;
        --used;
    }
}

template<class T, bool (*tgt)(const T& a, const T& b)>
std::string LinkedPriorityQueue<T,tgt>::printReverse (LN*& front)const {
    if(front  == nullptr)
        return "";
    else
        return printReverse(front->next) + front->value + ",";
}




////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::Iterator::Iterator(LinkedPriorityQueue<T,tgt>* iterate_over, LN* initial)
: prev(iterate_over->front),current(initial),ref_pq(iterate_over),expected_mod_count(ref_pq->mod_count)
{
}


template<class T, bool (*tgt)(const T& a, const T& b)>
LinkedPriorityQueue<T,tgt>::Iterator::~Iterator()
{}


template<class T, bool (*tgt)(const T& a, const T& b)>
T LinkedPriorityQueue<T,tgt>::Iterator::erase() {
    //std::cout <<expected_mod_count <<"  "<<ref_pq->mod_count<<std::endl;
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("LinkedPriorityQueue::Iterator::erase Iterator cursor already erased");
    if (current == nullptr)
        throw CannotEraseError("LinkedPriorityQueue::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    T to_return = current->value;

    LN *to_delete = current;
    prev ->next =current = current->next;
    delete  to_delete;
    ref_pq -> used--;
    ref_pq->mod_count++;
    expected_mod_count = ref_pq->mod_count;

    return to_return;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
std::string LinkedPriorityQueue<T,tgt>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_pq->str() << "(current=";
    current != nullptr ? answer<< current->value : answer <<"nullptr";
    answer<< ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::Iterator::operator ++ () -> LinkedPriorityQueue<T,tgt>::Iterator& {
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ++");
    if (current == nullptr)
        return *this;
    if (can_erase){
        prev = current;
        current = current ->next;
    }
    else
        can_erase = true;  //current already indexes "one beyond" deleted value
    return *this;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
auto LinkedPriorityQueue<T,tgt>::Iterator::operator ++ (int) -> LinkedPriorityQueue<T,tgt>::Iterator {
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ++(int)");
    if (current == nullptr)
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


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::Iterator::operator == (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedPriorityQueue::Iterator::operator ==");
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ==");
    if (ref_pq != rhsASI->ref_pq)
        throw ComparingDifferentIteratorsError("LinkedPriorityQueue::Iterator::operator ==");

    return current == rhsASI->current;
}


template<class T, bool (*tgt)(const T& a, const T& b)>
bool LinkedPriorityQueue<T,tgt>::Iterator::operator != (const LinkedPriorityQueue<T,tgt>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("LinkedPriorityQueue::Iterator::operator !=");
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator !=");
    if (ref_pq != rhsASI->ref_pq)
        throw ComparingDifferentIteratorsError("LinkedPriorityQueue::Iterator::operator !=");
    return current != rhsASI->current;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
T& LinkedPriorityQueue<T,tgt>::Iterator::operator *() const {
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator *");
    if (!can_erase || current == nullptr) {
        std::ostringstream where;
        where << " when front = " << ref_pq->front;
        throw IteratorPositionIllegal("LinkedPriorityQueue::Iterator::operator * Iterator illegal: "+where.str());
    }

    return current ->value;
}

template<class T, bool (*tgt)(const T& a, const T& b)>
T* LinkedPriorityQueue<T,tgt>::Iterator::operator ->() const {
    if (expected_mod_count != ref_pq->mod_count)
        throw ConcurrentModificationError("LinkedPriorityQueue::Iterator::operator ->");
    if (!can_erase || current == nullptr) {
        std::ostringstream where;
        where << " when front = " << ref_pq->front;
        throw IteratorPositionIllegal("LinkedPriorityQueue::Iterator::operator -> Iterator illegal: "+where.str());
    }

    return current;
}


}

#endif /* LINKED_PRIORITY_QUEUE_HPP_ */
