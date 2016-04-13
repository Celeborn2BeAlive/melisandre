#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <iterator>
#include <algorithm>
#include <iostream>

#include <melisandre/utils/EventDispatcher.hpp>
#include <melisandre/viewer/gui.hpp>

namespace mls
{
// Creating a node graph editor for ImGui
// Quick demo, not production code! This is more of a demo of how to use ImGui to create custom stuff.
// Better version by @daniel_collin here https://gist.github.com/emoon/b8ff4b4ce4f1b43e79f2
// See https://github.com/ocornut/imgui/issues/306
// v0.02
// Animated gif: https://cloud.githubusercontent.com/assets/8225057/9472357/c0263c04-4b4c-11e5-9fdf-2cd4f33f6582.gif

// NB: You can use math functions/operators on ImVec2 if you #define IMGUI_DEFINE_MATH_OPERATORS and #include "imgui_internal.h"
// Here we only declare simple +/- operators so others don't leak into the demo code.
//static inline ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y); }
//static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

class ComputeGraph;

struct ComputeValue; // Generic value

class ComputeNode {
private:
    friend class ComputeGraph;

    virtual void compute() = 0;

    virtual std::unique_ptr<ComputeNode> clone() const = 0;

    virtual int getInputsCount() const = 0;

    virtual int getOutputsCount() const = 0;

    virtual void setInput(size_t slot, const ComputeValue& value) {
    }

    virtual ComputeValue& getOutput(size_t slot) const {
    }

    virtual void drawGUI() = 0;
public:
    virtual ~ComputeNode() {
    }
};

class ComputeGraph
{
public:
    typedef size_t NodeID;

    ComputeGraph(const std::string& name) {}

    std::unordered_set<NodeID> computeConnectedComponent(NodeID srcNode) {
        std::unordered_set<NodeID> visitSet;
        std::stack<NodeID> visitStack;

        visitStack.emplace(srcNode);
        visitSet.emplace(srcNode);

        std::cerr << "srcNode = " << srcNode << std::endl;

        while (!visitStack.empty()) {
            auto currentNode = visitStack.top();
            visitStack.pop();

            std::cerr << "currentNode = " << currentNode << std::endl;

            for (auto out : m_Nodes[currentNode].outputLinks) {
                const auto& linkOut = m_Links[out];
                auto node = linkOut.dstNodeIdx;
                if (visitSet.find(node) == end(visitSet)) {
                    visitStack.push(node);
                    visitSet.emplace(node);
                    std::cerr << "node = " << node << std::endl;
                }
            }
        }

        return visitSet;
    }

    // Compute the priority of a node for a given visit set, stored as keys of the map 'priorities'
    size_t computeForwardPriority(NodeID node, std::unordered_map<NodeID, int>& priorities) {
        // Priority not yet computed
        if (priorities[node] == 0) {
            priorities[node] = -1; // priority is being computed

            auto maxPriority = size_t{ 0 };

            // Compute the map of parent priotities:
            for (auto in : m_Nodes[node].inputLinks) {
                const auto& linkIn = m_Links[in];
                auto nodeIn = linkIn.srcNodeIdx;
                // Only affect a priority to nodes in the visit set
                if (priorities.find(nodeIn) == std::end(priorities) || priorities[nodeIn] == -1) {
                    continue;
                }
                auto nodeInPriority = computeForwardPriority(nodeIn, priorities);
                maxPriority = std::max(maxPriority, nodeInPriority);
            }

            priorities[node] = maxPriority + 1;
        }

        return priorities[node];
    }

    void computeForward(NodeID srcNode) {
        auto visitSet = computeConnectedComponent(srcNode);

        std::cerr << "nb node to visit " << size(visitSet) << std::endl;

        std::unordered_map<NodeID, int> priorities;
        for (auto node : visitSet) {
            priorities[node] = 0;
        }
        priorities[srcNode] = 1;

        for (auto node : visitSet) {
            computeForwardPriority(node, priorities);
        }

        std::vector<std::pair<NodeID, size_t>> nodes;
        nodes.reserve(size(priorities));
        std::copy(begin(priorities), end(priorities), std::back_inserter(nodes));
        std::sort(begin(nodes), end(nodes), [&](auto lhs, auto rhs) {
            return lhs.second < rhs.second;
        });

        for (auto nodePair : nodes) {
            auto currentNode = nodePair.first;

            std::cerr << "Computing node index " << currentNode << std::endl;
            m_Nodes[currentNode].node->compute();

            for (auto out : m_Nodes[currentNode].outputLinks) {
                const auto& link = m_Links[out];
                m_Nodes[link.dstNodeIdx].setInput(link.dstNodeSlotIdx, m_Nodes[currentNode].getOutput(link.srcNodeSlotIdx));
            }
        }
    }

    void drawGUI(size_t width, size_t height, bool* pOpened = nullptr);

private:
    struct NodeLink
    {
        int srcNodeIdx, srcNodeSlotIdx, dstNodeIdx, dstNodeSlotIdx;

        NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { srcNodeIdx = input_idx; srcNodeSlotIdx = input_slot; dstNodeIdx = output_idx; dstNodeSlotIdx = output_slot; }
    };

    void addDummyNode(const std::string& name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count);

    void addIntNode(const std::string& name, const ImVec2& pos);

    ImVec2 GetInputSlotPos(int node_idx, int slot_no) const;

    ImVec2 GetOutputSlotPos(int node_idx, int slot_no) const;

    // This function must:
    // - test if the link does not exists already
    // - test if the types match
    bool addLink(int srcNode, int outputIdx, int dstNode, int inputIdx);

    struct NodeInstance {
        std::string name;
        ImVec2 position;
        ImVec2 size;
        std::unique_ptr<ComputeNode> node;

        std::vector<size_t> inputLinks; // A list of indices pointing to m_Links.The following relation should hold : k in m_Nodes[i].inputLinks -> m_Links[k].dstNodeIdx == i
        std::vector<size_t> outputLinks; // A list of indices pointing to m_Links.The following relation should hold : k in m_Nodes[i].outputLinks -> m_Links[k].srcNodeIdx == i
    };

    std::vector<NodeInstance> m_Nodes;
    std::vector<NodeLink> m_Links;
};

template<typename T>
size_t addNodeType(const std::string& name) {
    return 0;
}

}