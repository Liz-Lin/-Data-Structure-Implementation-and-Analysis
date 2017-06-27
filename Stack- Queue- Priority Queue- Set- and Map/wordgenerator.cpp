//submitter: lizhenl (Lizhen, Lin)
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                           //I used std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>         WordQueue;
typedef ics::ArraySet<std::string>           FollowSet;
typedef ics::pair<WordQueue,FollowSet>       CorpusEntry;
typedef ics::ArrayPriorityQueue<CorpusEntry> CorpusPQ;     //Convenient to supply gt at construction
typedef ics::ArrayMap<WordQueue,FollowSet>   Corpus;


//Return a random word in the words set (use in produce_text)
std::string random_in_set(const FollowSet& words) {
  int index = ics::rand_range(1, words.size());
  int i = 0;
  for (const std::string& s : words)
    if (++i == index)
      return s;
  return "?";
}


//Read an open file of lines of words (separated by spaces) and return a
//  Corpus (Map) of each sequence (Queue) of os (Order-Statistic) words
//  associated with the Set of all words that follow them somewhere in the
//  file.
Corpus read_corpus(int os, std::ifstream &file) {
    std::string line;
    WordQueue wq;
    while (getline(file, line)) {
        std::vector<std::string> words = ics::split(line, " ");
        for(std::string temp : words)
            wq.enqueue(temp);
    }
    Corpus corpus;
    for(auto i = wq.begin(); i != wq.end(); ++i ){
        WordQueue key;
        auto follow = i;
        for(int j=0; j < os; ++j)
            ++follow;
        if(follow != wq.end()){
            for(auto tempIndex =i; tempIndex != follow; ++tempIndex)
                key.enqueue(*tempIndex);
            corpus[key].insert(*follow);
        }
    }
    return corpus;
}


//Print "Corpus" and all entries in the Corpus, in lexical alphabetical order
//  (with the minimum and maximum set sizes at the end).
//Use a "can be followed by any of" to separate the key word from the Set of words
//  that can follow it.

//One queue is lexically greater than another, if its first value is smaller; or if
//  its first value is the same and its second value is smaller; or if its first
//  and second values are the same and its third value is smaller...
//If any of its values is greater than the corresponding value in the other queue,
//  the first queue is not greater.
//Note that the queues sizes are the same: each stores Order-Statistic words
//Important: Use iterators for examining the queue values: DO NOT CALL DEQUEUE.

bool queue_gt(const CorpusEntry& a, const CorpusEntry& b) {
    auto ia = a.first.begin();
    auto ib = b.first.begin();
    while(ia != a.first.end()){
        if(*ia == *ib){
            ++ia;
            ++ib;
        }
        else
            return *ia < *ib;
    }
}

void print_corpus(const Corpus& corpus) {
    int max = std::numeric_limits<int>::min();
    int min = std::numeric_limits<int>::max();
    CorpusPQ sorted(queue_gt);
    sorted.enqueue_all(corpus);
    std::cout << std::endl <<"Corpus had " <<corpus.size()<< " Entries" <<std::endl;
    for(auto& temp : sorted) {
        if(temp.second.size() > max)
            max = temp.second.size();
        else if ((temp.second.size() < min))
            min = temp.second.size();
        std::cout << "  " << temp.first << " -> " << temp.second << std::endl;
    }
    std::cout <<"Corpus had " <<corpus.size()<< " Entries" <<std::endl;
    std::cout <<"max/min = "<< max <<"/"<<min <<std::endl;
}


//Return a Queue of words, starting with those in start and including count more
//  randomly selected words using corpus to decide which word comes next.
//If there is no word that follows the previous ones, put "None" into the queue
//  and return immediately this list (whose size is <= start.size() + count).
WordQueue produce_text(const Corpus& corpus, const WordQueue& start, int count) {
    WordQueue temp(start), final;
    int num =0;
    while(num < count){
        if(corpus.has_key(temp)){
            std::string newWord= random_in_set(corpus[temp]);
            final.enqueue(newWord);
            temp.dequeue();
            temp.enqueue(newWord);
            num++;
        }
        else{
            final.enqueue("None");
            num = count;
        }
    }
    temp = start;
    temp.enqueue_all(final);
    return temp;
}



//Prompt the user for (a) the order statistic and (b) the file storing the text.
//Read the text as a Corpus and print it appropriately.
//Prompt the user for order statistic words from the text.
//Prompt the user for number of random words to generate
//Call the above functions to solve the problem, and print the appropriate information
int main() {
  try {
      int os = ics::prompt_int("Enter some order statistic", 2);
      std::ifstream file;
      std::string default_name = "/Users/lizhenlin/CLionProjects/program1/input\ files/wginput1.txt";
      ics::safe_open (file,"Enter some file name to process",default_name);
      Corpus corpus = read_corpus(os, file);
      print_corpus(corpus);
      int num = (*(corpus.begin())).first.size(),index=1,count;
      std::cout <<std::endl <<"Enter "<<num <<" words for starting"<<std::endl;
      WordQueue start;
      while(index <= num){
          start.enqueue(ics::prompt_string("Enter word " + std::to_string(index), "", NULL));
          ++index;
      }
      std::cout <<"Enter # of words to generate:";
      std::cin >>count;
      std::cout <<"Random text = "<< produce_text(corpus,start,count) <<std::endl;
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
