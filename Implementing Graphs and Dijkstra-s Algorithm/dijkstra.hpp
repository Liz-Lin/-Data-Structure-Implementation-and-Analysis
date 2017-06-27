#ifndef DIJKSTRA_HPP_
#define DIJKSTRA_HPP_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>                    //Biggest int: std::numeric_limits<int>::max()
#include "array_queue.hpp"
#include "array_stack.hpp"
#include "heap_priority_queue.hpp"
#include "hash_graph.hpp"


namespace ics {
    int hash_string(const std::string& s) {std::hash<std::string> str_hash; return str_hash(s);}

class Info {
  public:
    Info() { }

    Info(std::string a_node) : node(a_node) { }

    bool operator==(const Info &rhs) const { return cost == rhs.cost && from == rhs.from; }

    bool operator!=(const Info &rhs) const { return !(*this == rhs); }

    friend std::ostream &operator<<(std::ostream &outs, const Info &i) {
      outs << "Info[" << i.node << "," << i.cost << "," << i.from << "]";
      return outs;
    }

    //Public instance variable definitions
    std::string node = "?";
    int cost = std::numeric_limits<int>::max();
    std::string from = "?";
  };


  bool gt_info(const Info &a, const Info &b) { return a.cost < b.cost; }

  typedef ics::HashGraph<int>                  DistGraph;
  typedef ics::HeapPriorityQueue<Info, gt_info> CostPQ;
  typedef ics::HashMap<std::string, Info,hash_string>       CostMap;
  typedef ics::pair<std::string, Info>          CostMapEntry;


//Return the final_map as specified in the lecture-note description of
//  extended Dijkstra algorithm
  CostMap extended_dijkstra(const DistGraph &g, std::string start_node) {
        CostMap answer_map;//leave it empty
        CostMap info_map;
        for(const auto& nodeEntry: g.all_nodes())
            info_map.put(nodeEntry.first, Info(nodeEntry.first));
        info_map[start_node].cost = 0; //set start node's cost to 0


        CostPQ info_PQ;
        for(const auto& infoEntry : info_map)
            info_PQ.enqueue(infoEntry.second);

        while(!info_map.empty()) {
            Info info = info_PQ.dequeue();
            if(info.cost == std::numeric_limits<int>::max())
                break;//cannot reach
            while(answer_map.has_key(info.node))
                info = info_PQ.dequeue();

            answer_map.put(info.node, info);
            info_map.erase(info.node);

            for(const auto& nodes: g.out_nodes(info.node)){
                if(!answer_map.has_key(nodes)){
                    int costSum = g.edge_value(info.node, nodes);
                    costSum += info.cost;
                    if(costSum < info_map[nodes].cost) {
                        info_map[nodes].cost = costSum;
                        info_map[nodes].from = info.node;
                        info_PQ.enqueue(info_map[nodes]);
                    }
                }
            }
        }

        return answer_map;
  }



//Return a queue whose front is the start node (implicit in answer_map) and whose
//  rear is the end node
  ArrayQueue <std::string> recover_path(const CostMap &answer_map, std::string end_node) {
        ArrayQueue<std::string> to_return;
        to_return.enqueue(end_node);
        std::string startNode, tempNode = answer_map[end_node].from;

        for(const auto& entry: answer_map){
            if(entry.second.cost ==0)
                startNode = entry.first;
        }
        while(tempNode != startNode){
            to_return.enqueue(tempNode);
            tempNode = answer_map[tempNode].from;
        }
        to_return.enqueue(startNode);

        ArrayStack<std::string> temp;
        while(!to_return.empty())
            temp.push(to_return.dequeue());
        while(!temp.empty())
            to_return.enqueue(temp.pop());

        return to_return;
  }


}

#endif /* DIJKSTRA_HPP_ */
