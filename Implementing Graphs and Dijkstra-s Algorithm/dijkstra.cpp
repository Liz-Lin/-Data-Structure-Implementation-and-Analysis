#include <string>
#include <iostream>
#include <fstream>
#include "ics46goody.hpp"
#include "array_queue.hpp"
#include "hash_graph.hpp"
#include "dijkstra.hpp"



std::string get_node_in_graph(const ics::DistGraph& g, std::string prompt, bool allow_QUIT) {
  std::string node;
  for(;;) {
    node = ics::prompt_string(prompt + " (must be in graph" + (allow_QUIT ? " or QUIT" : "") + ")");
    if ( (allow_QUIT && node == "QUIT") || g.has_node(node) )
      break;
  }
  return node;
}


int main() {
  try {
      std::ifstream in_file;
      ics::safe_open(in_file,"Enter graph file name[flightcost.txt]:","flightdist.txt");
      ics::DistGraph g;
      g.load(in_file, ";");
      std::cout<<g<<std::endl;
      ics::Info info;
      std::string start = get_node_in_graph(g,"Enter start node ", false);
      ics::CostMap costMap =ics::extended_dijkstra(g, start);
      std::cout<<costMap<<std::endl;
      std::string stop;
      while(true) {
          std::cout << std::endl;
          stop = get_node_in_graph(g, "Enter stop node ", true);
          if(stop == "QUIT")
              break;
          ics::ArrayQueue<std::string> path = ics::recover_path(costMap, stop);
          std::cout <<"Cost is "<<costMap[stop].cost<<"; path is "<< path << std::endl;
      }
  } catch (ics::IcsError& e) {
    std::cout << e.what() << std::endl;
  }

  return 0;
}
