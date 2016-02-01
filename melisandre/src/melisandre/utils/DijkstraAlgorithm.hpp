#pragma once

#include <list>
#include "Graph.hpp"

namespace mls {

template<typename DistanceType>
struct DijkstraNode {
    GraphNodeIndex predecessor;
    DistanceType distance;

    DijkstraNode():
        predecessor(UNDEFINED_NODE), distance(std::numeric_limits<DistanceType>::max()) {
    }

    DijkstraNode(GraphNodeIndex predecessor, DistanceType distance):
        predecessor(predecessor), distance(distance) {
    }
};

template<typename DistanceType>
using DijkstraShortestPathVector = std::vector<DijkstraNode<DistanceType>>; // For each node of a graph, give the predecessor in the shortest path

template<typename DistanceType>
void reorderList(GraphNodeIndex id_suc, DistanceType newdist,
                 std::list<std::pair<GraphNodeIndex, DistanceType>>& nodeSet);

template<typename DistanceFunction>
auto computeDijkstraShortestPaths(const Graph& graph, GraphNodeIndex root, const DistanceFunction& distance)
    -> DijkstraShortestPathVector<decltype(distance(GraphNodeIndex(), GraphNodeIndex()))> {
    using DistanceType = decltype(distance(GraphNodeIndex(), GraphNodeIndex()));
    using DijkstraElement = std::pair<GraphNodeIndex, DistanceType>;
    std::vector<DistanceType> distances(graph.size(), std::numeric_limits<DistanceType>::max()); // Will store the distances to the light node
    DijkstraShortestPathVector<DistanceType> result(graph.size(), DijkstraNode<DistanceType>());
    std::list<DijkstraElement> node_set;
    distances[root] = 0;
    result[root] = DijkstraNode<DistanceType>(root, 0);
    node_set.push_front(DijkstraElement(root, 0));

    // Dijkstra algorithm
    while (!node_set.empty()) {
        DijkstraElement elt = node_set.front(); // Next node to handle
        DistanceType tmp = distances[elt.first];

        // No path exists from the light to this node: the computation is over
        if (tmp == std::numeric_limits<DistanceType>::max()) {
            break;
        }

        node_set.pop_front();

        // For each successor, update the distances
        for (GraphNodeIndex i = 0; i < graph[elt.first].size(); i++) {
            GraphNodeIndex idx_suc = graph[elt.first][i];

            DistanceType cur_dist = distance(elt.first, idx_suc) + distances[elt.first];

            // If the new distance is less than the current distance, update
            if (cur_dist < distances[idx_suc]) {
                distances[idx_suc] = cur_dist;
                result[idx_suc] = DijkstraNode<DistanceType>(elt.first, distances[idx_suc]);

                reorderList(idx_suc, cur_dist, node_set);
            }
        }
    }

    return result;
}

template<typename DistanceType>
void reorderList(GraphNodeIndex id_suc, DistanceType newdist,
                 std::list<std::pair<GraphNodeIndex, DistanceType>>& nodeSet) {
    typedef std::pair<GraphNodeIndex, DistanceType> DijkstraElement;
    typename std::list<DijkstraElement>::iterator it;

    // Searching for the node "id_suc" in the list
    for (it = nodeSet.begin(); it != nodeSet.end(); it++) {
        if (it->first == id_suc) {
            break;
        }
    }

    // 'it' is in the right place...
    if (it != nodeSet.end()) {
        nodeSet.erase(it);
    }

    // Searching for the new position of "id_suc"
    for (it = nodeSet.begin(); (it != nodeSet.end()) && (it->second < newdist);
            it++) {
        // do nothing
    }

    nodeSet.insert(it, DijkstraElement(id_suc, newdist));
}

template<typename DistanceType>
inline DistanceType computeMaxDistance(const DijkstraShortestPathVector<DistanceType>& shortestPaths) {
    DistanceType maxDistance = 0;
    for(const auto& node: shortestPaths) {
        if(node.predecessor != UNDEFINED_NODE) {
            maxDistance = max(maxDistance, node.distance);
        }
    }
    return maxDistance;
}

}
