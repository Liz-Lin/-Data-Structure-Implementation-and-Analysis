////submitter: lizhenl (Lizhen, Lin)
//#include <string>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <vector>
//#include "ics46goody.hpp"
//#include "array_queue.hpp"
//#include "array_priority_queue.hpp"
//#include "array_set.hpp"
//#include "array_map.hpp"
//
//
//typedef ics::ArraySet<std::string>                     States;
//typedef ics::ArrayQueue<std::string>                   InputsQueue;
//typedef ics::ArrayMap<std::string,States>              InputStatesMap;
//
//typedef ics::ArrayMap<std::string,InputStatesMap>       NDFA;
//typedef ics::pair<std::string,InputStatesMap>           NDFAEntry;
//
//bool gt_NDFAEntry (const NDFAEntry& a, const NDFAEntry& b)
//{return a.first<b.first;}
//
//typedef ics::ArrayPriorityQueue<NDFAEntry,gt_NDFAEntry> NDFAPQ;
//
//typedef ics::pair<std::string,States>                   Transitions;
//typedef ics::ArrayQueue<Transitions>                    TransitionsQueue;
//
//
////Read an open file describing the non-deterministic finite automaton (each
////  line starts with a state name followed by pairs of transitions from that
////  state: (input followed by a new state, all separated by semicolons), and
////  return a Map whose keys are states and whose associated values are another
////  Map with each input in that state (keys) and the resulting set of states it
////  can lead to.
//const NDFA read_ndfa(std::ifstream &file) {
//    NDFA ndfa;
//    std::string line;
//    while (getline(file, line)){
//        std::vector<std::string> words = ics::split(line,";");
//        InputStatesMap stateMap;
//        for(std::vector<std::string>::iterator i = (words.begin()+1); i != words.end(); ++i) {
//            std::string trans = *(i++);
//            stateMap[trans].insert(*i);
//        }
//        ndfa[words.front()] = stateMap;
//    }
//  return ndfa;
//}
//
////Print a label and all the entries in the finite automaton Map, in
////  alphabetical order of the states: each line has a state, the text
////  "transitions:" and the Map of its transitions.
//void print_ndfa(const NDFA& ndfa) {
//    NDFAPQ pq(ndfa);
//    std::cout <<std::endl<<"The Non-Deterministic Finite Automaton Description"<<std::endl;
//    for(const auto& ndEntry : pq)
//        std::cout << ndEntry.first <<" transitions: " <<ndEntry.second <<std::endl;
//    std::cout <<std::endl;
//}
//
//InputsQueue convert(std::vector<std::string> words){
//    InputsQueue iq;
//    words.erase(words.begin());
//    for(const auto& temp: words )
//        iq.enqueue(temp);
//    return iq;
//}
//
////Return a queue of the calculated transition pairs, based on the non-deterministic
////  finite automaton, initial state, and queue of inputs; each pair in the returned
////  queue is of the form: input, set of new states.
////The first pair contains "" as the input and the initial state.
////If any input i is illegal (does not lead to any state in the non-deterministic finite
////  automaton), ignore it.
//
//TransitionsQueue process(const NDFA& ndfa, std::string state, const InputsQueue& inputs) {
//    TransitionsQueue tq;
//    States totalSet;
//    totalSet.insert(state);
//    tq.enqueue(Transitions("", totalSet));
//    for (const auto &trans: inputs) {
//        States statesSet;
//        for(auto& eachState: totalSet){
//            if (ndfa[eachState].has_key(trans))
//                statesSet.insert_all(ndfa[eachState][trans]);
//        }
//        tq.enqueue(Transitions(trans, statesSet));
//        totalSet = statesSet;
//    }
//    return tq;
//}
//
//
//
////Print a TransitionsQueue (the result of calling process) in a nice form.
////Print the Start state on the first line; then print each input and the
////  resulting new states indented on subsequent lines; on the last line, print
////  the Stop state.
//
//void interpret(TransitionsQueue &tq) {  //or TransitionsQueue or TransitionsQueue&&
//    Transitions transPair = tq.dequeue();
//    std::cout << "Statrt state = " << transPair.second<<std::endl;
//    States finalState;
//    for(auto& trans: tq){
//        std::cout << "  Input = " << trans.first << "; new possible states = " << trans.second << std::endl;
//        finalState = trans.second;
//    }
//    std::cout << "Stop state(s) = " << finalState <<std::endl;
//}
//
//
//
////Prompt the user for a file, create a finite automaton Map, and print it.
////Prompt the user for a file containing any number of simulation descriptions
////  for the finite automaton to process, one description per line; each
////  description contains a start state followed by its inputs, all separated by
////  semicolons.
////Repeatedly read a description, print that description, put each input in a
////  Queue, process the Queue and print the results in a nice form.
//int main() {
//  try {
//      std::ifstream file;
//      std::string default_name = "/Users/lizhenlin/CLionProjects/program1/input\ files/ndfaendin01.txt";
//      ics::safe_open (file,"Enter some non-deterministic finite automaton file name: ",default_name);
//      NDFA ndfa = read_ndfa(file);
//      print_ndfa(ndfa);
//      std::ifstream file1;
//      std::string default_name1 = "/Users/lizhenlin/CLionProjects/program1/input\ files/ndfainputendin01.txt";
//      ics::safe_open (file1,"Enter some file name with start-state and inputs",default_name1);
//      std::string line;
//      while (getline(file1, line)) {
//          std::cout <<std::endl<< "Starting up new simulation with description: " << line << std::endl;
//          std::vector<std::string> words = ics::split(line, ";");
//          std::string state = words[0];
//          TransitionsQueue tq = process(ndfa, state, convert(words));
//          interpret(tq);
//      }
//  } catch (ics::IcsError& e) {
//    std::cout << e.what() << std::endl;
//  }
//  return 0;
//}
