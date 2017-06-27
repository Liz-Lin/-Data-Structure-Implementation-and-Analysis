// Submitter: lizhenl(Lizhen, Lin)
// Partner  : qiaoh3(Qiao, He)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming

#ifndef HASH_SET_HPP_
#define HASH_SET_HPP_

#include <string>
#include <iostream>
#include <sstream>
#include <initializer_list>
#include "ics_exceptions.hpp"
#include "pair.hpp"


namespace ics {


#ifndef undefinedhashdefined
#define undefinedhashdefined
template<class T>
int undefinedhash (const T& a) {return 0;}
#endif /* undefinedhashdefined */

//Instantiate the templated class supplying thash(a): produces a hash value for a.
//If thash is defaulted to undefinedhash in the template, then a constructor must supply chash.
//If both thash and chash are supplied, then they must be the same (by ==) function.
//If neither is supplied, or both are supplied but different, TemplateFunctionError is raised.
//The (unique) non-undefinedhash value supplied by thash/chash is stored in the instance variable hash.
template<class T, int (*thash)(const T& a) = undefinedhash<T>> class HashSet {
  public:
    typedef int (*hashfunc) (const T& a);

    //Destructor/Constructors
    ~HashSet ();

    HashSet (double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const T& k) = undefinedhash<T>);
    HashSet (const HashSet<T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);
    explicit HashSet (const std::initializer_list<T>& il, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);

    //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
    template <class Iterable>
    explicit HashSet (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const T& a) = undefinedhash<T>);


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
    HashSet<T,thash>& operator = (const HashSet<T,thash>& rhs);
    bool operator == (const HashSet<T,thash>& rhs) const;
    bool operator != (const HashSet<T,thash>& rhs) const;
    bool operator <= (const HashSet<T,thash>& rhs) const;
    bool operator <  (const HashSet<T,thash>& rhs) const;
    bool operator >= (const HashSet<T,thash>& rhs) const;
    bool operator >  (const HashSet<T,thash>& rhs) const;

    template<class T2, int (*hash2)(const T2& a)>
    friend std::ostream& operator << (std::ostream& outs, const HashSet<T2,hash2>& s);



  private:
    class LN;

  public:
    class Iterator {
      public:
        typedef pair<int,LN*> Cursor;

        //Private constructor called in begin/end, which are friends of HashSet<T,thash>
        ~Iterator();
        T           erase();
        std::string str  () const;
        HashSet<T,thash>::Iterator& operator ++ ();
        HashSet<T,thash>::Iterator  operator ++ (int);
        bool operator == (const HashSet<T,thash>::Iterator& rhs) const;
        bool operator != (const HashSet<T,thash>::Iterator& rhs) const;
        T& operator *  () const;
        T* operator -> () const;
        friend std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>::Iterator& i) {
          outs << i.str(); //Use the same meaning as the debugging .str() method
          return outs;
        }
        friend Iterator HashSet<T,thash>::begin () const;
        friend Iterator HashSet<T,thash>::end   () const;

      private:
        //If can_erase is false, current indexes the "next" value (must ++ to reach it)
        Cursor              current; //Bin Index + LN* pointer; stops if LN* == nullptr
        HashSet<T,thash>*   ref_set;
        int                 expected_mod_count;
        bool                can_erase = true;

        //Helper methods
        void advance_cursors();

        //Called in friends begin/end
        Iterator(HashSet<T,thash>* iterate_over, bool from_begin);
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

public:
  int (*hash)(const T& k);   //Hashing function used (from template or constructor)
private:
  LN** set      = nullptr;   //Pointer to array of pointers: each bin stores a list with a trailer node
  double load_threshold;     //used/bins <= load_threshold
  int bins      = 1;         //# bins in array (should start >= 1 so hash_compress doesn't divide by 0)
  int used      = 0;         //Cache for number of key->value pairs in the hash table
  int mod_count = 0;         //For sensing concurrent modification


  //Helper methods
  int   hash_compress        (const T& key)              const;  //hash function ranged to [0,bins-1]
  LN*   find_element         (const T& element)          const;  //Returns reference to element's node or nullptr
  LN*   copy_list            (LN*   l)                   const;  //Copy the elements in a bin (order irrelevant)
  LN**  copy_hash_table      (LN** ht, int bins)         const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

  void  ensure_load_threshold(int new_used);                     //Reallocate if load_threshold > load_threshold
  void  delete_hash_table    (LN**& ht, int bins);               //Deallocate all LN in ht (and the ht itself; ht == nullptr)
};





//HashSet class and related definitions

////////////////////////////////////////////////////////////////////////////////
//
//Destructor/Constructors

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::~HashSet() {
    delete_hash_table(set, bins);
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(double the_load_threshold, int (*chash)(const T& element))
        :hash(thash != (hashfunc)undefinedhash<T>? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::default constructor: both specified");

    set = new LN*[bins];
    for(int i =0; i <bins; ++i)
        set[i] = new LN();
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(int initial_bins, double the_load_threshold, int (*chash)(const T& element))
        :hash(thash != (hashfunc)undefinedhash<T>? thash : chash), bins(initial_bins), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::default constructor: both specified");
    if(bins <1)
        bins = 1;

    set = new LN*[bins];
    for(int i =0; i <bins; ++i)
        set[i] = new LN();
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const HashSet<T,thash>& to_copy, double the_load_threshold, int (*chash)(const T& element))
        :hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        hash = to_copy.hash;//throw TemplateFunctionError("HashMap::length constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::length constructor: both specified");

    if(hash == to_copy.hash) {
        used = to_copy.used;
        bins = to_copy.bins;
        set = copy_hash_table(to_copy.set, bins);

    }else{
        set = new LN*[bins];
        set[0] = new LN();
        insert_all(to_copy);
    }

}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::HashSet(const std::initializer_list<T>& il, double the_load_threshold, int (*chash)(const T& element))
        :hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::initializer_list constructor: both specified");

    set = new LN*[bins];
    for(int i =0; i <bins; ++i)
        set[i] = new LN();

    for(const T& t:il)
        insert(t);
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
HashSet<T,thash>::HashSet(const Iterable& i, double the_load_threshold, int (*chash)(const T& a))
        :hash(thash != (hashfunc)undefinedhash<T> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<T>)
        throw TemplateFunctionError("HashSet::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<T> && chash != (hashfunc)undefinedhash<T> && thash != chash)
        throw TemplateFunctionError("HashSet::initializer_list constructor: both specified");

    set = new LN*[bins];
    for(int i =0; i <bins; ++i)
        set[i] = new LN();

    for(const T& t:i)
        insert(t);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::empty() const {
    return used == 0;
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::size() const {
    return used;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::contains (const T& element) const {
    return find_element(element) != nullptr;
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::str() const {
    std::ostringstream answer;
    for(int i=0; i<bins; ++i){
        answer<<"bin["<<i<<"]:";
        for(LN* current = set[i]; current->next != nullptr; current = current->next)
            answer<<current->value<<" -> ";
        answer <<"TRAILER"<<std::endl;
    }
    answer << "(bins=" << bins << ",used=" << used << ",mod_count=" << mod_count << ")";
    return answer.str();
}


template<class T, int (*thash)(const T& a)>
template <class Iterable>
bool HashSet<T,thash>::contains_all(const Iterable& i) const {
    for (auto v : i)
        if (!contains(v))
            return false;

    return true;
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::insert(const T& element) {
    if (contains(element))
        return 0;

    ensure_load_threshold(++used);
    LN* current = set[hash_compress(element)];
    set[hash_compress(element)] = new LN(element,current);
    ++mod_count;
    return 1;
}


template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::erase(const T& element) {
    LN* location = find_element(element);
    if(location != nullptr){
        LN* to_delete = location->next;
        *location = *(location->next);
        delete to_delete;
        --used;
        ++mod_count;
        return 1;
    }
    return 0;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::clear() {
    delete_hash_table(set, bins);
    used = 0;
    ++mod_count;
    set = new LN*[bins];
    for(int i =0; i <bins; ++i)
        set[i] = new LN();
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::insert_all(const Iterable& i) {
    int count =0;
    for(const T&  v:i){
        ++count;
        insert(v);
    }
    return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::erase_all(const Iterable& i) {
    int count =0;
    for(const T&  v:i){
        ++count;
        erase(v);
    }
    return count;
}


template<class T, int (*thash)(const T& a)>
template<class Iterable>
int HashSet<T,thash>::retain_all(const Iterable& i) {
    HashSet s(i);
    int count = 0;
    for(int i =0; i<bins; ++i){
        for(LN* p = set[i]; p ->next != nullptr; ){
            if (!s.contains(p->value)) {
                erase(p->value);
                ++count;
            }else
                p = p->next;
        }
    }

    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class T, int (*thash)(const T& a)>
HashSet<T,thash>& HashSet<T,thash>::operator = (const HashSet<T,thash>& rhs) {
    if (this == &rhs)
        return *this;
    delete_hash_table(set,bins);

    if(rhs.hash == hash) {
        used = rhs.used;
        bins = rhs.bins;
        set = copy_hash_table(rhs.set, bins);
    }else{
        hash = rhs.hash;
        set = new LN*[bins];
        set[0] = new LN();
        insert_all(rhs);
    }

    ++mod_count;
    return *this;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator == (const HashSet<T,thash>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.size())
        return false;

    for (int i=0; i<rhs.bins; ++i)
        for(LN* current = rhs.set[i]; current->next != nullptr; current = current->next)
            if(!contains(current->value))
                return false;
    return true;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator != (const HashSet<T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator <= (const HashSet<T,thash>& rhs) const {
    if (this == &rhs)
        return true;
    if (used > rhs.size())
        return false;
    for (int i=0; i<bins; ++i)
        for(LN* current = set[i]; current->next != nullptr; current = current->next)
            if(!rhs.contains(current->value))
                return false;

    return true;
}

template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator < (const HashSet<T,thash>& rhs) const {
    if (this == &rhs)
        return false;
    if (used >= rhs.size())
        return false;
    for (int i=0; i<bins; ++i)
        for(LN* current = set[i]; current->next != nullptr; current = current->next)
            if(!rhs.contains(current->value))
                return false;

    return true;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator >= (const HashSet<T,thash>& rhs) const {
    return rhs <= *this;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::operator > (const HashSet<T,thash>& rhs) const {
    return rhs < *this;
}


template<class T, int (*thash)(const T& a)>
std::ostream& operator << (std::ostream& outs, const HashSet<T,thash>& s) {
    outs<<"set[";
    if (s.used != 0) {
        typename HashSet<T,thash>::Iterator i = s.begin();
        outs << *i;
        ++i;
        for (/*See above*/; i != s.end(); ++i)
            outs << "," << *i;
    }

    outs << "]";
    return outs;
}


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::begin () const -> HashSet<T,thash>::Iterator {
    return Iterator(const_cast<HashSet<T,thash>*>(this),true);
}


template<class T, int (*thash)(const T& a)>
auto HashSet<T,thash>::end () const -> HashSet<T,thash>::Iterator {
    return Iterator(const_cast<HashSet<T,thash>*>(this),false);
}


///////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class T, int (*thash)(const T& a)>
int HashSet<T,thash>::hash_compress (const T& element) const {
    return (abs(hash(element))% bins);
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::find_element (const T& element) const {
    for(LN* to_return = set[hash_compress(element)]; to_return->next != nullptr; to_return = to_return->next)
        if(to_return->value == element)
            return to_return;
    return nullptr;
}

template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN* HashSet<T,thash>::copy_list (LN* l) const {
    LN* to_return = new LN();
    for(LN* source = l; l->next != nullptr; l = l->next )
        to_return = new LN(l->value, to_return);

    return to_return;
}


template<class T, int (*thash)(const T& a)>
typename HashSet<T,thash>::LN** HashSet<T,thash>::copy_hash_table (LN** ht, int bins) const {
    LN** tempSet = new LN*[bins];
    for(int i = 0; i< bins; ++i)
        tempSet[i] = copy_list(ht[i]);
    return tempSet;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::ensure_load_threshold(int new_used) {
    if((double)(new_used/bins) <= load_threshold)
        return;
    bins *= 2;
    LN** tempSet = new LN*[bins];
    for(int i=0; i<bins; ++i)
        tempSet[i] = new LN();//tralier
    for(int i=0; i<(bins/2); ++i){
        LN* current = set[i];
        for(; current->next != nullptr;/*see body*/){//not copying the trailer
            LN* source = current;
            current = current->next;
            LN* target = tempSet[hash_compress(source->value)];//rehash
            source ->next = target;
            tempSet[hash_compress(source->value)] = source;
        }
        delete current;//delete trailer node
    }
    delete[] set;
    set = tempSet;
}


template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::delete_hash_table (LN**& ht, int bins) {
    for(int i =0; i<bins; ++i){
        for (LN* p = ht[i]; p != nullptr; /*see body*/) {
            LN* to_delete = p;
            p = p->next;
            delete to_delete;
        }
    }
    delete[] ht;
    ht = nullptr;
    bins = 1;
    used =0;
}






////////////////////////////////////////////////////////////////////////////////
//
//Iterator class definitions

template<class T, int (*thash)(const T& a)>
void HashSet<T,thash>::Iterator::advance_cursors() {
    if(current == Cursor(-1, nullptr))
        return;
    current.second =current.second->next;
    if(current.second->next == nullptr) {//trailer node
        for(int i = (current.first+1); i < ref_set->bins ; ++i){//next bin
            LN* node = ref_set->set[i];
            if(node ->next != nullptr){//it's not a trailer
                current = Cursor(i, node);
                return;
            }
        }
        current = Cursor(-1, nullptr);
    }else
        return;
}


template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::Iterator(HashSet<T,thash>* iterate_over, bool begin)
: ref_set(iterate_over), expected_mod_count(ref_set->mod_count) {
    if(ref_set->empty() || !begin )//ref_map->map == nullptr
        current = Cursor(-1, nullptr);
    else {//from begin
        for(int i=0; i <ref_set->bins; ++i) {
            if (ref_set->set[i]->next != nullptr) {
                current = Cursor(i, ref_set->set[i]);
                return;//how to use advance cursor
            }
        }
    }
}



template<class T, int (*thash)(const T& a)>
HashSet<T,thash>::Iterator::~Iterator()
{}


template<class T, int (*thash)(const T& a)>
T HashSet<T,thash>::Iterator::erase() {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashSet::Iterator::erase Iterator cursor already erased");
    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        throw CannotEraseError("HashSet::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    T to_return = current.second->value;
    if(current.second ->next->next == nullptr)//current is at the one node before trailer node
        advance_cursors();//update current //or already update since replace the value with the next LN object
    ref_set->erase(to_return);
    expected_mod_count = ref_set->mod_count;
    return to_return;
}


template<class T, int (*thash)(const T& a)>
std::string HashSet<T,thash>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_set->str()  << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ () -> HashSet<T,thash>::Iterator& {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ++");

    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        return *this;

    if (can_erase)
        advance_cursors();
    else
        can_erase = true;

    return *this;
}


template<class T, int (*thash)(const T& a)>
auto  HashSet<T,thash>::Iterator::operator ++ (int) -> HashSet<T,thash>::Iterator {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ++");

    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        return *this;

    Iterator to_return(*this);
    if (can_erase)
        advance_cursors();
    else
        can_erase = true;

    return to_return;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator == (const HashSet<T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashSet::Iterator::operator ==");
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ==");
    if (ref_set != rhsASI->ref_set)//rhs.ref_map
        throw ComparingDifferentIteratorsError("HashSet::Iterator::operator ==");

    return current == rhsASI->current;
}


template<class T, int (*thash)(const T& a)>
bool HashSet<T,thash>::Iterator::operator != (const HashSet<T,thash>::Iterator& rhs) const {
    return !(*this == rhs);
}

template<class T, int (*thash)(const T& a)>
T& HashSet<T,thash>::Iterator::operator *() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator *");
    if (!can_erase || current.second == nullptr ||current.first == -1)
        throw IteratorPositionIllegal("HashSet::Iterator::operator * Iterator illegal");

    return current.second->value;
}

template<class T, int (*thash)(const T& a)>
T* HashSet<T,thash>::Iterator::operator ->() const {
    if (expected_mod_count != ref_set->mod_count)
        throw ConcurrentModificationError("HashSet::Iterator::operator ->");
    if (!can_erase || current.second == nullptr || current.first == -1)//LN* nullptr or index out of bound
        throw IteratorPositionIllegal("HashSet::Iterator::operator -> Iterator illegal");

    return &current.second->value;
}

}

#endif /* HASH_SET_HPP_ */
