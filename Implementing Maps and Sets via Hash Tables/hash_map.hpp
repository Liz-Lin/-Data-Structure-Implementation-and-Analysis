// Submitter: lizhenl(Lizhen, Lin)
// Partner  : qiaoh3(Qiao, He)
// We certify that we worked cooperatively on this programming
//   assignment, according to the rules for pair programming

#ifndef HASH_MAP_HPP_
#define HASH_MAP_HPP_

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
    template<class KEY,class T, int (*thash)(const KEY& a) = undefinedhash<KEY>> class HashMap {
    public:
        typedef ics::pair<KEY,T>   Entry;
        typedef int (*hashfunc) (const KEY& a);

        //Destructor/Constructors
        ~HashMap ();

        HashMap          (double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
        explicit HashMap (int initial_bins, double the_load_threshold = 1.0, int (*chash)(const KEY& k) = undefinedhash<KEY>);
        HashMap          (const HashMap<KEY,T,thash>& to_copy, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);
        explicit HashMap (const std::initializer_list<Entry>& il, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);

        //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
        template <class Iterable>
        explicit HashMap (const Iterable& i, double the_load_threshold = 1.0, int (*chash)(const KEY& a) = undefinedhash<KEY>);


        //Queries
        bool empty      () const;
        int  size       () const;
        bool has_key    (const KEY& key) const;
        bool has_value  (const T& value) const;
        std::string str () const; //supplies useful debugging information; contrast to operator <<


        //Commands
        T    put   (const KEY& key, const T& value);
        T    erase (const KEY& key);
        void clear ();

        //Iterable class must support "for-each" loop: .begin()/.end() and prefix ++ on returned result
        template <class Iterable>
        int put_all(const Iterable& i);


        //Operators

        T&       operator [] (const KEY&);
        const T& operator [] (const KEY&) const;
        HashMap<KEY,T,thash>& operator = (const HashMap<KEY,T,thash>& rhs);
        bool operator == (const HashMap<KEY,T,thash>& rhs) const;
        bool operator != (const HashMap<KEY,T,thash>& rhs) const;

        template<class KEY2,class T2, int (*hash2)(const KEY2& a)>
        friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY2,T2,hash2>& m);



    private:
        class LN;

    public:
        class Iterator {
        public:
            typedef pair<int,LN*> Cursor;

            //Private constructor called in begin/end, which are friends of HashMap<T>
            ~Iterator();
            Entry       erase();
            std::string str  () const;
            HashMap<KEY,T,thash>::Iterator& operator ++ ();
            HashMap<KEY,T,thash>::Iterator  operator ++ (int);
            bool operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const;
            bool operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const;
            Entry& operator *  () const;
            Entry* operator -> () const;
            friend std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>::Iterator& i) {
                outs << i.str(); //Use the same meaning as the debugging .str() method
                return outs;
            }
            friend Iterator HashMap<KEY,T,thash>::begin () const;
            friend Iterator HashMap<KEY,T,thash>::end   () const;

        private:
            //If can_erase is false, current indexes the "next" value (must ++ to reach it)
            Cursor                current; //Bin Index + LN* pointer; stops if LN* == nullptr
            HashMap<KEY,T,thash>* ref_map;
            int                   expected_mod_count;
            bool                  can_erase = true;

            //Helper methods
            void advance_cursors();

            //Called in friends begin/end
            Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin);
        };


        Iterator begin () const;
        Iterator end   () const;


    private:
        class LN {
        public:
            LN ()                         : next(nullptr){}
            LN (const LN& ln)             : value(ln.value), next(ln.next){}
            LN (Entry v, LN* n = nullptr) : value(v), next(n){}

            Entry value;
            LN*   next;
        };

        int (*hash)(const KEY& k);  //Hashing function used (from template or constructor)
        LN** map      = nullptr;    //Pointer to array of pointers: each bin stores a list with a trailer node
        double load_threshold;      //used/bins <= load_threshold
        int bins      = 1;          //# bins in array (should start >= 1 so hash_compress doesn't divide by 0)
        int used      = 0;          //Cache for number of key->value pairs in the hash table
        int mod_count = 0;          //For sensing concurrent modification


        //Helper methods
        int   hash_compress        (const KEY& key)          const;  //hash function ranged to [0,bins-1]
        LN*   find_key             (const KEY& key) const;           //Returns reference to key's node or nullptr
        LN*   copy_list            (LN*   l)                 const;  //Copy the keys/values in a bin (order irrelevant)
        LN**  copy_hash_table      (LN** ht, int bins)       const;  //Copy the bins/keys/values in ht tree (order in bins irrelevant)

        void  ensure_load_threshold(int new_used);                   //Reallocate if load_factor > load_threshold
        void  delete_hash_table    (LN**& ht, int bins);             //Deallocate all LN in ht (and the ht itself; ht == nullptr)
    };



////////////////////////////////////////////////////////////////////////////////
//
//HashMap class and related definitions

//Destructor/Constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::~HashMap() {
    delete_hash_table(map, bins);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(double the_load_threshold, int (*chash)(const KEY& k))
    :hash(thash != (hashfunc)undefinedhash<KEY>? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::default constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::default constructor: both specified");

    map = new LN*[bins];
    map[0] = new LN();
//    for(int i =0; i <bins; ++i)
//        map[i] = new LN();
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(int initial_bins, double the_load_threshold, int (*chash)(const KEY& k))
    :hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), bins(initial_bins), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::length constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::length constructor: both specified");
    if(bins <1)
        bins = 1;

    map = new LN*[bins];
    for(int i =0; i <bins; ++i)
        map[i] = new LN();
}


//    For the copy constructor: if the hash function for the object being constructed is the same
// as the function for the map to copy, we can copy the values in each bin of the hash table
// without rehashing anything; but if the hash function is different, then the best we can do
// in the copy constructor is to put all the key/value pairs into the new map. Finally, for the
// operator=, also remember to copy the hash function from the rhs map.
template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const HashMap<KEY,T,thash>& to_copy, double the_load_threshold, int (*chash)(const KEY& a))
:hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        hash = to_copy.hash;//throw TemplateFunctionError("HashMap::length constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::length constructor: both specified");

    if(hash == to_copy.hash) {
        used = to_copy.used;
        bins = to_copy.bins;
        map = copy_hash_table(to_copy.map, bins);

    }else{
        map = new LN*[bins];
        map[0] = new LN();
        put_all(to_copy);
    }

}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::HashMap(const std::initializer_list<Entry>& il, double the_load_threshold, int (*chash)(const KEY& k))
:hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::initializer_list constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::initializer_list constructor: both specified");

    map = new LN*[bins];
    map[0] = new LN();
//    for(int i =0; i <bins; ++i)
//        map[i] = new LN();

    for(const Entry& entry:il)
        put(entry.first, entry.second);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template <class Iterable>
HashMap<KEY,T,thash>::HashMap(const Iterable& i, double the_load_threshold, int (*chash)(const KEY& k))
    :hash(thash != (hashfunc)undefinedhash<KEY> ? thash : chash), load_threshold(the_load_threshold) {
    if (hash == (hashfunc)undefinedhash<KEY>)
        throw TemplateFunctionError("HashMap::iterable constructor: neither specified");
    if (thash != (hashfunc)undefinedhash<KEY> && chash != (hashfunc)undefinedhash<KEY> && thash != chash)
        throw TemplateFunctionError("HashMap::iterable constructor: both specified");

    map = new LN*[bins];
    map[0] = new LN();
    for(const Entry& entry:i)
        put(entry.first, entry.second);
}


////////////////////////////////////////////////////////////////////////////////
//
//Queries

template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::empty() const {
    return used == 0;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::size() const {
    return used;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_key (const KEY& key) const {
    return (find_key(key) != nullptr);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::has_value (const T& value) const {
    for(int i =0; i<bins; ++i)
        for(LN* current = map[i]; current->next != nullptr; current = current->next)//no need to check trailer
            if(current->value.second == value)
                return true;
    return false;
}

/*
    bin[0]: b->2 -> a->1 -> TRAILER
    bin[1]: TRAILER
    bin[2]: c->3 -> TRAILER
*/
template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::str() const {
    std::ostringstream answer;
    for(int i=0; i<bins; ++i){
        answer<<"bin["<<i<<"]:";
        for(LN* current = map[i]; current->next != nullptr; current = current->next)
            answer<<current->value.first <<"->"<<current->value.second <<" -> ";
        answer <<"TRAILER"<<std::endl;
    }
        answer << "(bins=" << bins << ",used=" << used << ",mod_count=" << mod_count << ")";
    return answer.str();
}


////////////////////////////////////////////////////////////////////////////////
//
//Commands

template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::put(const KEY& key, const T& value) {
    LN* location = find_key(key);
    T to_return;
    if(location != nullptr) {//already exist and return old value
        to_return = location->value.second;
        location->value.second = value;
    } else{
        ensure_load_threshold(++used);
        LN* current = map[hash_compress(key)];
        map[hash_compress(key)] = new LN(Entry(key, value),current);
        to_return = map[hash_compress(key)]->value.second;
    }
    ++mod_count;
    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
T HashMap<KEY,T,thash>::erase(const KEY& key) {
    LN* location = find_key(key);
    if(location != nullptr){
        LN* to_delete = location->next;
        T to_return = location->value.second;
        *location = *(location->next);
        delete to_delete;
        --used;
        ++mod_count;
        return to_return;
    }
    std::ostringstream answer;
    answer << "HashMap::erase: key(" << key << ") not in Map";
    throw KeyError(answer.str());

}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::clear() {
    delete_hash_table(map, bins);
    used = 0;
    ++mod_count;
    map = new LN*[bins];
    for(int i =0; i <bins; ++i)
        map[i] = new LN();
}


template<class KEY,class T, int (*thash)(const KEY& a)>
template<class Iterable>
int HashMap<KEY,T,thash>::put_all(const Iterable& i) {
    int count =0;
    for(const Entry&  entry:i){
        ++count;
        put(entry.first, entry.second);
    }
    return count;
}


////////////////////////////////////////////////////////////////////////////////
//
//Operators

template<class KEY,class T, int (*thash)(const KEY& a)>
T& HashMap<KEY,T,thash>::operator [] (const KEY& key) {
    LN* location = find_key(key);
    if(location == nullptr) {
        ensure_load_threshold(used+1);
        put(key, T());
        location = find_key(key);
    }
    return location->value.second;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
const T& HashMap<KEY,T,thash>::operator [] (const KEY& key) const {
    LN* location = find_key(key);
    if(location != nullptr)
        return location->value.second;

    std::ostringstream answer;
    answer << "HashMap::operator []: key(" << key << ") not in Map";
    throw KeyError(answer.str());
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>& HashMap<KEY,T,thash>::operator = (const HashMap<KEY,T,thash>& rhs) {
    if (this == &rhs)
        return *this;
    delete_hash_table(map,bins);

    if(rhs.hash == hash) {
        used = rhs.used;
        bins = rhs.bins;
        map = copy_hash_table(rhs.map, bins);
    }else{
        hash = rhs.hash;
        map = new LN*[bins];
        map[0] = new LN();
        put_all(rhs);
    }

    ++mod_count;
    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator == (const HashMap<KEY,T,thash>& rhs) const {
    if (this == &rhs)
        return true;
    if (used != rhs.size())
        return false;

    for (int i=0; i<rhs.bins; ++i) {
        for(LN* current = rhs.map[i]; current->next != nullptr; current = current->next){
            LN* location = find_key(current->value.first);
            if(location == nullptr || location->value.second != current->value.second)
                return false;
        }
    }
    return true;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::operator != (const HashMap<KEY,T,thash>& rhs) const {
    return !(*this == rhs);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::ostream& operator << (std::ostream& outs, const HashMap<KEY,T,thash>& m) {
    outs<<"map[";
    if (m.used != 0) {
        typename HashMap<KEY,T,thash>::Iterator i = m.begin();
        outs << i->first << "->" << i->second;
        ++i;
        for (/*See above*/; i != m.end(); ++i)
            outs << "," << i->first << "->" << i->second;
    }
    outs << "]";
    return outs;
}//from Prof. Pattis


////////////////////////////////////////////////////////////////////////////////
//
//Iterator constructors

template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::begin () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY,T,thash>*>(this),true); //from_begin = true
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::end () const -> HashMap<KEY,T,thash>::Iterator {
    return Iterator(const_cast<HashMap<KEY,T,thash>*>(this),false);
}


///////////////////////////////////////////////////////////////////////////////
//
//Private helper methods

template<class KEY,class T, int (*thash)(const KEY& a)>
int HashMap<KEY,T,thash>::hash_compress (const KEY& key) const {
    return (abs(hash(key))% bins);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::find_key (const KEY& key) const {
    for(LN* to_return = map[hash_compress(key)]; to_return->next != nullptr; to_return = to_return->next)
        if(to_return->value.first == key)
            return to_return;
    return nullptr;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN* HashMap<KEY,T,thash>::copy_list (LN* l) const {
    LN* to_return = new LN();
    for(LN* source = l; l->next != nullptr; l = l->next )
        to_return = new LN(l->value, to_return);
    return to_return;
}

template<class KEY,class T, int (*thash)(const KEY& a)>
typename HashMap<KEY,T,thash>::LN** HashMap<KEY,T,thash>::copy_hash_table (LN** ht, int bins) const {
    LN** tempMap = new LN*[bins];
    for(int i = 0; i< bins; ++i)
        tempMap[i] = copy_list(ht[i]);
    return tempMap;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::ensure_load_threshold(int new_used) {
    if((double)(new_used/bins) <= load_threshold)
        return;
    bins *= 2;
    LN** tempMap = new LN*[bins];
    for(int i=0; i<bins; ++i)
        tempMap[i] = new LN();//tralier
    for(int i=0; i<(bins/2); ++i){
        LN* current = map[i];
        for(; current->next != nullptr;/*see body*/){//not copying the trailer
            LN* source = current;
            current = current->next;
            LN* target = tempMap[hash_compress(source->value.first)];//rehash
            source ->next = target;
            tempMap[hash_compress(source->value.first)] = source;
        }
        delete current;//delete trailer node
    }
    delete[] map;
    map = tempMap;
}

template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::delete_hash_table (LN**& ht, int bins) {
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
//We advance a non-"end" cursor by moving its second to the next LN in the linked-list,
// but if that is the trailer node, by moving to higher bin indexes until we either
// (a) find a bin whose linked-list stores more than just a trailer node or
// (b) run out of bins to check.
// For (a) we used that bin index and a pointer to the first LN in that bin's linked-list;
// for (b) we use an "end" cursor: -1 and nullptr. I declared an advance_cursors helper method in Iterator
// to help peform this operation (which can even be used to compute the "begin" cursor for a hash table)
template<class KEY,class T, int (*thash)(const KEY& a)>
void HashMap<KEY,T,thash>::Iterator::advance_cursors(){
    if(current == Cursor(-1, nullptr))
        return;
    current.second =current.second->next;
    if(current.second->next == nullptr) {//trailer node
        for(int i = (current.first+1); i < ref_map->bins ; ++i){//next bin
            LN* node = ref_map->map[i];
            if(node ->next != nullptr){//it's not a trailer
                current = Cursor(i, node);
                return;
            }
        }
        current = Cursor(-1, nullptr);
    }else
        return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::Iterator(HashMap<KEY,T,thash>* iterate_over, bool from_begin)
: ref_map(iterate_over), expected_mod_count(ref_map->mod_count) {
    if(ref_map->empty() || !from_begin )//ref_map->map == nullptr
        current = Cursor(-1, nullptr);
    else {//from begin
        for(int i=0; i <ref_map->bins; ++i) {
            if (ref_map->map[i]->next != nullptr) {
                current = Cursor(i, ref_map->map[i]);
                return;//how to use advance cursor??
            }
        }
    }
}



template<class KEY,class T, int (*thash)(const KEY& a)>
HashMap<KEY,T,thash>::Iterator::~Iterator()
{}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto HashMap<KEY,T,thash>::Iterator::erase() -> Entry {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::erase");
    if (!can_erase)
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor already erased");
    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        throw CannotEraseError("HashMap::Iterator::erase Iterator cursor beyond data structure");

    can_erase = false;
    Entry to_return = current.second->value;
    if(current.second ->next->next == nullptr)//current is at the one node before trailer node
        advance_cursors();//update current //or already update since replace the value with the next LN object
    ref_map->erase(to_return.first);
    expected_mod_count = ref_map->mod_count;
    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
std::string HashMap<KEY,T,thash>::Iterator::str() const {
    std::ostringstream answer;
    answer << ref_map->str()  << ",expected_mod_count=" << expected_mod_count << ",can_erase=" << can_erase << ")";
    return answer.str();
}

template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ () -> HashMap<KEY,T,thash>::Iterator& {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++");

    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        return *this;

    if (can_erase)
        advance_cursors();
    else
        can_erase = true;

    return *this;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
auto  HashMap<KEY,T,thash>::Iterator::operator ++ (int) -> HashMap<KEY,T,thash>::Iterator {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ++");

    if (current.first == -1 || current.second == nullptr)//ref_map->empty()
        return *this;

    Iterator to_return(*this);
    if (can_erase)
        advance_cursors();
    else
        can_erase = true;

    return to_return;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator == (const HashMap<KEY,T,thash>::Iterator& rhs) const {
    const Iterator* rhsASI = dynamic_cast<const Iterator*>(&rhs);
    if (rhsASI == 0)
        throw IteratorTypeError("HashMap::Iterator::operator ==");
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ==");
    if (ref_map != rhsASI->ref_map)//rhs.ref_map
        throw ComparingDifferentIteratorsError("HashMap::Iterator::operator ==");

    return current == rhsASI->current;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
bool HashMap<KEY,T,thash>::Iterator::operator != (const HashMap<KEY,T,thash>::Iterator& rhs) const {
    return !(*this == rhs);
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>& HashMap<KEY,T,thash>::Iterator::operator *() const {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator *");
    if (!can_erase || current.second == nullptr ||current.first == -1)
        throw IteratorPositionIllegal("HashMap::Iterator::operator * Iterator illegal");

    return current.second->value;
}


template<class KEY,class T, int (*thash)(const KEY& a)>
pair<KEY,T>* HashMap<KEY,T,thash>::Iterator::operator ->() const {
    if (expected_mod_count != ref_map->mod_count)
        throw ConcurrentModificationError("HashMap::Iterator::operator ->");
    if (!can_erase || current.second == nullptr || current.first == -1)//LN* nullptr or index out of bound
        throw IteratorPositionIllegal("HashMap::Iterator::operator -> Iterator illegal");

    return &current.second->value;
}

}

#endif /* HASH_MAP_HPP_ */
