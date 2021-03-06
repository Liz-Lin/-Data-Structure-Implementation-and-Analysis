//submitter: lizhenl (Lizhen, Lin)
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "array_priority_queue.hpp"
#include "array_set.hpp"
#include "array_map.hpp"


typedef ics::ArrayQueue<std::string>              CandidateQueue;
typedef ics::ArraySet<std::string>                CandidateSet;
typedef ics::ArrayMap<std::string,int>            CandidateTally;

typedef ics::ArrayMap<std::string,CandidateQueue> Preferences;
typedef ics::pair<std::string,CandidateQueue>     PreferencesEntry;
typedef ics::ArrayPriorityQueue<PreferencesEntry> PreferencesEntryPQ; //Must supply gt at construction

bool pref_entry_gt (const PreferencesEntry& a, const PreferencesEntry& b)
{return a.first<b.first;}

typedef ics::pair<std::string,int>                TallyEntry;
typedef ics::ArrayPriorityQueue<TallyEntry>       TallyEntryPQ;

bool talley_entry_gt (const TallyEntry& a, const TallyEntry& b) //votes then names
{
  if (a.second != b.second)
    return a.second>b.second;
  else
    return a.first < b.first;
}
bool talley_entry_gt3 (const TallyEntry& a, const TallyEntry& b) //name
{ return a.first < b.first; }
bool talley_entry_gt2 (const TallyEntry& a, const TallyEntry& b) //just by votes
{ return a.second < b.second; }

//Read an open file stating voter preferences (each line is (a) a voter
//  followed by (b) all the candidates the voter would vote for, in
//  preference order (from most to least preferred candidate, separated
//  by semicolons), and return a Map of preferences: a Map whose keys are
//  voter names and whose values are a queue of candidate preferences.
Preferences read_voter_preferences(std::ifstream &file) {
    Preferences map;
    std::string line;
    while (getline(file, line)) {
        std::vector<std::string> words = ics::split(line, ";");
        for (std::vector<std::string>::iterator i = words.begin() + 1; i != words.end(); ++i)
            map[words.at(0)].enqueue(*i);
  }
  file.close();
  return map;
}

//Print a label and all the entries in the preferences Map, in alphabetical
//  order according to the voter.
//Use a "->" to separate the voter name from the Queue of candidates.
void print_voter_preferences(const Preferences& preferences) {
  std::cout <<std::endl<<"Voter name -> queue[Preferences]"<<std::endl;
  PreferencesEntryPQ sorted(pref_entry_gt);
  sorted.enqueue_all(preferences);
  for(const PreferencesEntry& temp : sorted )
    std::cout << temp.first << "->" <<temp.second <<std::endl;
    std::cout <<std::endl;
}

CandidateSet convertCandiSet (const Preferences& preferences) {
    CandidateSet candiSet;
    for(const auto& voter : preferences)
        for(const std::string& candi : voter.second)
            candiSet.insert(candi);
    return candiSet;
}
//Print the message followed by all the entries in the CandidateTally, in
//  the order specified by has_higher_priority: i is printed before j, if
//  has_higher_priority(i,j) returns true: sometimes alphabetically by candidate,
//  other times by decreasing votes for the candidate.
//Use a "->" to separate the candidat name from the number of votes they
//  received.
void print_tally(std::string message, const CandidateTally& tally, bool (*has_higher_priority)(const TallyEntry& i,const TallyEntry& j)) {
    std::cout<<message<<std::endl;
    TallyEntryPQ sortedN (*has_higher_priority);
    sortedN.enqueue_all(tally);
    for (const TallyEntry& temp : sortedN)
        std::cout << "  "<< temp.first << " -> " <<temp.second << std::endl;
    std::cout <<std::endl;
}


//Return the CandidateTally: a Map of candidates (as keys) and the number of
//  votes they received, based on the unchanging Preferences (read from the
//  file) and the candidates who are currently still in the election (which changes).
//Every possible candidate should appear as a key in the resulting tally.
//Each voter should tally one vote: for their highest-ranked candidate who is
//  still in the the election.
CandidateTally evaluate_ballot(const Preferences& preferences, const CandidateSet& candidates) {
  CandidateTally details; //<name, #of vtes>
     for(auto& candi : candidates)
        details[candi] =0;//make sure not to lose any candis if they didnt get to be voted
  for( PreferencesEntry& temp : preferences) {
      auto i = temp.second.begin();
      while(!candidates.contains(*i))
          ++i;
      details[*i] += 1;
  }
  return details;
}


//Return the Set of candidates who are still in the election, based on the
//  tally of votes: compute the minimum number of votes and return a Set of
//  all candidates receiving more than that minimum; if all candidates
//  receive the same number of votes (that would be the minimum), the empty
//  Set is returned.
CandidateSet remaining_candidates(const CandidateTally& tally) {
  TallyEntryPQ sorted(talley_entry_gt2);
  sorted.enqueue_all(tally);
  int min = sorted.peek().second;
  CandidateSet list;
  for (const auto& temp : sorted) //what if the sorted at this time already empty
    if(temp.second > min)
      list.insert(temp.first);
  return list;
}


//Prompt the user for a file, create a voter preference Map, and print it.
//Determine the Set of all the candidates in the election, from this Map.
//Repeatedly evaluate the ballot based on the candidates (still) in the
//  election, printing the vote count (tally) two ways: with the candidates
//  (a) shown alphabetically increasing and (b) shown with the vote count
//  decreasing (candidates with equal vote counts are shown alphabetically
//  increasing); from this tally, compute which candidates remain in the
//  election: all candidates receiving more than the minimum number of votes;
//  continue this process until there are less than 2 candidates.
//Print the final result: there may 1 candidate left (the winner) or 0 left
//   (no winner).
int main() {
  try {
      std::ifstream file;
      std::string default_name = "/Users/lizhenlin/CLionProjects/program1/input\ files/votepref2.txt";
      ics::safe_open (file,"Enter some voter preferences file name:",default_name);
      Preferences preferences =read_voter_preferences(file);
      print_voter_preferences(preferences);
      CandidateSet candiSet = convertCandiSet(preferences);
      CandidateTally candiTelly;
      int index= 1;
      bool done = false;
      while(!done){
          candiTelly = evaluate_ballot(preferences,candiSet);
          std:: ostringstream os1,os2;
          os1 <<"Vote count on ballot #" <<index <<": candidates (alphabetically ordered) with remaining candidates = "<<candiSet;
          print_tally(os1.str(),candiTelly,talley_entry_gt3 );
          os2 <<"Vote count on ballot #" <<index <<": candidates (numerically ordered) with remaining candidates = "<<candiSet;
          print_tally(os2.str(),candiTelly,talley_entry_gt );
          candiSet = remaining_candidates(candiTelly);
          if(candiSet.size() ==1) {
              std::cout << "Winner is " << *(candiSet.begin()) << std::endl;
              done = true;
          }
          else if (candiSet.empty()){
              std::cout << "Not any unique winner: election is a tie among all the candidates remaining on the last ballot"<<std::endl;
              done = true;
          }
          ++index;
      }
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
