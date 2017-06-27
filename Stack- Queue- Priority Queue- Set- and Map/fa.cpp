//submitter: lizhenl (Lizhen, Lin)
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>                InputsQueue;
typedef ics::ArrayMap<std::string,std::string>      InputStateMap;

typedef ics::ArrayMap<std::string,InputStateMap>    FA;
typedef ics::pair<std::string,InputStateMap>        FAEntry;

bool gt_FAEntry (const FAEntry& a, const FAEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<FAEntry,gt_FAEntry> FAPQ;

typedef ics::pair<std::string,std::string>          Transition;
typedef ics::ArrayQueue<Transition>                 TransitionQueue;


//Read an open file describing the finite automaton (each line starts with
//  a state name followed by pairs of transitions from that state: (input
//  followed by new state, all separated by semicolons), and return a Map
//  whose keys are states and whose associated values are another Map with
//  each input in that state (keys) and the resulting state it leads to.
const FA read_fa(std::ifstream &file) {
    FA m;
    std::string line;
    while (getline(file, line)) {
        std::vector<std::string> words = ics::split(line, ";");
        InputStateMap temp;
        for(std::vector<std::string>::iterator i = (words.begin()+1); i != words.end(); ++i) {
            std::string trans = *(i++);
            temp.put(trans,(*i));
        }
        m[words.front()] = temp;
    }
    return m;
}


//Print a label and all the entries in the finite automaton Map, in
//  alphabetical order of the states: each line has a state, the text
//  "transitions:" and the Map of its transitions.
void print_fa(const FA& fa) {
    FAPQ sorted(fa);
    std::cout <<std::endl<< "The Finite Automaton Description" <<std::endl;
    for(const FAEntry& temp : sorted)
        std::cout << "  "<< temp.first << " transitions: " << temp.second << std::endl;
    std::cout <<std::endl;
}


InputsQueue convert(std::vector<std::string> words){
    InputsQueue iq;
    words.erase(words.begin());
    for(const auto& temp: words )
        iq.enqueue(temp);
    return iq;
}

//Return a queue of the calculated transition pairs, based on the finite
//  automaton, initial state, and queue of inputs; each pair in the returned
//  queue is of the form: input, new state.
//The first pair contains "" as the input and the initial state.
//If any input i is illegal (does not lead to a state in the finite
//  automaton), then the last pair in the returned queue is i,"None".
TransitionQueue process(const FA& fa, std::string state, const InputsQueue& inputs) {
    TransitionQueue tq;
    tq.enqueue(Transition("", state));
    for (const auto& trans: inputs) {
        if(fa[state].has_key(trans)) {
            state = fa[state][trans];
            tq.enqueue(Transition(trans, state));
        }
        else
            tq.enqueue(Transition(trans, "None"));
    }
    return tq;
}

//queue[pair[,even],pair[1,odd],pair[0,odd],pair[1,even],pair[1,odd],pair[0,odd],pair[1,even]]:rear

//Print a TransitionQueue (the result of calling the process function above)
// in a nice form.
//Print the Start state on the first line; then print each input and the
//  resulting new state (or "illegal input: terminated", if the state is
//  "None") indented on subsequent lines; on the last line, print the Stop
//  state (which may be "None").
void interpret(TransitionQueue& tq) {  //or TransitionQueue or TransitionQueue&&
    Transition transPair = tq.dequeue();
    std::cout << "Statrt state = " << transPair.second<<std::endl;
    std::string finalState;
    for(auto trans: tq){
        if(trans.second != "None" )
            std::cout << "  Input = " << trans.first << "; new state = " << trans.second << std::endl;
        else
            std::cout << "  Input = " << trans.first << "; illegal input: simulation terminated" << trans.second << std::endl;
        finalState = trans.second;
    }
    std::cout << "Stop state = " << finalState <<std::endl;
}



//Prompt the user for a file, create a finite automaton Map, and print it.
//Prompt the user for a file containing any number of simulation descriptions
//  for the finite automaton to process, one description per line; each
//  description contains a start state followed by its inputs, all separated by
//  semicolons.
//Repeatedly read a description, print that description, put each input in a
//  Queue, process the Queue and print the results in a nice form.
int main() {
  try {
      std::ifstream file;
      std::string default_name = "/Users/lizhenlin/CLionProjects/program1/input\ files/faparity.txt";
      ics::safe_open (file,"Enter some FA file name:",default_name);
      FA fa = read_fa(file);
      print_fa(fa);
      std::ifstream file1;
      std::string default_name1 = "/Users/lizhenlin/CLionProjects/program1/input\ files/fainputparity.txt";
      ics::safe_open (file1,"Enter some file name with start-state and inputs",default_name1);
      std::string line;
      while (getline(file1, line)){
          std::cout <<std::endl<< "Starting up a new simulation with description: " << line <<std::endl;
          std::vector<std::string> words = ics::split(line, ";");
          std::string state = words[0];
          InputsQueue iq = convert(words);
          TransitionQueue tq = process(fa,state,iq);
          interpret(tq);
      }
    } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
