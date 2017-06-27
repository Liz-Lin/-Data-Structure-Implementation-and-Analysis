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


typedef ics::ArraySet<std::string>          NodeSet;
typedef ics::pair<std::string,NodeSet>      GraphEntry;

bool graph_entry_gt (const GraphEntry& a, const GraphEntry& b)
{return a.first<b.first;}

typedef ics::ArrayPriorityQueue<GraphEntry,graph_entry_gt> GraphPQ;
typedef ics::ArrayMap<std::string,NodeSet>  Graph;


//Read an open file of edges (node names separated by semicolons, with an
//  edge going from the first node name to the second node name) and return a
//  Graph (Map) of each node name associated with the Set of all node names to
//  which there is an edge from the key node name.
Graph read_graph(std::ifstream &file) {
	Graph graph;
    std::string line;
    while (getline(file, line)){
        std::vector<std::string> words = ics::split(line, ";");
        graph[words.at(0)].insert(words.at(1));
    }
    file.close();
    return graph;
}


//Print a label and all the entries in the Graph in alphabetical order
//  (by source node).
//Use a "->" to separate the source node name from the Set of destination
//  node names to which it has an edge.
void print_graph(const Graph& graph) {
    std::cout <<std::endl<<"Graph: source node -> set[destination nodes]" <<std::endl;
    GraphPQ sorted(graph);
    for(const GraphEntry& temp : sorted)
        std::cout << temp.first << "->" << temp.second << std::endl;
    std::cout<<std::endl;
}


//Return the Set of node names reaching in the Graph starting at the
//  specified (start) node.
//Use a local Set and a Queue to respectively store the reachable nodes and
//  the nodes that are being explored.
NodeSet reachable(const Graph& graph, std::string start) {
    NodeSet reach;
    reach.insert(start);
    reach.insert_all(graph[start]);
    ics::ArrayQueue<std::string> values(graph[start]);
    while(!values.empty())
    {
        if(graph.has_key(values.peek())) {
            reach.insert_all(graph[values.peek()]);
            values.enqueue_all(graph[values.peek()]);
        }
        values.dequeue();
    }
    return reach;
}





//Prompt the user for a file, create a graph from its edges, print the graph,
//  and then repeatedly (until the user enters "quit") prompt the user for a
//  starting node name and then either print an error (if that the node name
//  is not a source node in the graph) or print the Set of node names
//  reachable from it by using the edges in the Graph.
int main() {
  try {
      std::ifstream file;
      std::string default_name = "/Users/lizhenlin/CLionProjects/program1/input\ files/graph1.txt";
      ics::safe_open (file,"Enter some graph file name:",default_name);
      Graph graph = read_graph(file);
      print_graph(graph);
      while(true) {
          std::string attempt = ics::prompt_string("Enter some starting node name (else quit)");
          if (attempt == "quit")
              break;
          else {
              if(graph.has_key(attempt))
                std::cout<< "From " << attempt <<" the reachable nodes are "<<reachable(graph, attempt) <<std::endl;
              else
                  std::cout<< " " << attempt << " is not a source node name in the graph" <<std::endl;
              std::cout <<std::endl;
          }
      }

  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }
  return 0;
}
