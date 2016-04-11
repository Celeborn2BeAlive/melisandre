#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <iterator>
#include <algorithm>

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

class ComputeNode {
public:
    enum class VarType {
        In, Out, InOut
    };

    template<typename T, VarType type = VarType::InOut>
    struct Var {
        Var(std::string name, const T& defaultValue = T{});

        const T& get() const;

        T& get();

        void notify();

        bool hasChanged();
    };

    //ComputeNode(const ComputeGraph& graph) {
    //}

    virtual int getInputsCount() const = 0;

    virtual int getOutputsCount() const = 0;

    virtual void drawGUI() = 0;

private:
    friend class ComputeGraph;

    virtual void compute() = 0;

protected:
    template<typename T>
    Var<T, VarType::In> addVarIn(const std::string& name, T& ref) {
        
    }

    template<typename T>
    Var<T, VarType::Out> addVarOut(const std::string& name, T& ref) {
        
    }

    template<typename T>
    Var<T, VarType::InOut> addVarInOut(const std::string& name, T& ref) {

    }

private:
    //std::unordered_map<std::string, Input> m_InputMap;
    //std::unordered_map<std::string, Output> m_OutputMap;
};

class ComputeGraph
{
public:
    typedef size_t NodeID;

    struct Output;

    struct Input {
        NodeID getNode();
        Output* getConnectedOutput();
    };

    struct Output {
        NodeID getNode();
        bool hasChanged();
        std::vector<Input> getConnectedInputs();
    };

    ComputeGraph(const std::string& name) {}

    ComputeNode* getNode(const std::string& id);

    ComputeNode* getNode(NodeID id);

    //void connect(ComputeNode::Input in, ComputeNode::Output out);

    std::unordered_set<NodeID> computeConnectedComponent(NodeID srcNode) {
        std::unordered_set<NodeID> visitSet;
        std::stack<NodeID> visitStack;
        visitStack.push(srcNode);
        visitStack.emplace(srcNode);

        while (!visitStack.empty()) {
            auto currentNode = visitStack.top();
            visitStack.pop();

            for (auto out : getOutputs(currentNode)) {
                for (auto in : out.getConnectedInputs()) {
                    auto node = in.getNode();
                    if (visitSet.find(node) == end(visitSet)) {
                        visitStack.push(node);
                        visitSet.emplace(node);
                    }
                }
            }
        }

        return visitSet;
    }

    // Compute the priority of a node for a given visit set, stored as keys of the map 'priorities'
    size_t computeForwardPriority(NodeID node, std::unordered_map<NodeID, size_t>& priorities) {
        // Priority not yet computed
        if (!priorities[node]) {
            auto maxPriority = size_t{ 0 };

            // Compute the map of parent priotities:
            for (auto in : getInputs(node)) {
                auto pOutput = in.getConnectedOutput();
                if (!pOutput) {
                    continue;
                }
                auto neighbor = pOutput->getNode();
                // Only affect a priority to nodes in the visit set
                if (priorities.find(neighbor) == std::end(priorities)) {
                    continue;
                }
                auto neighborPriority = computeForwardPriority(neighbor, priorities);
                maxPriority = std::max(maxPriority, neighborPriority);
            }

            priorities[node] = maxPriority + 1;
        }

        return priorities[node];
    }

    void computeForward(NodeID srcNode) {
        auto visitSet = computeConnectedComponent(srcNode);

        std::unordered_map<NodeID, size_t> priorities;
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
            getNode(nodePair.first)->compute();
        }
    }

    std::vector<Input> getInputs(NodeID node);

    std::vector<Output> getOutputs(NodeID node);

    void drawGUI(size_t width, size_t height, bool* pOpened = nullptr);

private:
    struct NodeWidget {
        std::string name;
        ImVec2  position;
        ImVec2 size;
    };

    struct NodeLink
    {
        int     InputIdx, InputSlot, OutputIdx, OutputSlot;

        NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
    };

    void addDummyNode(const std::string& name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count);

    ImVec2 GetInputSlotPos(int node_idx, int slot_no) const;

    ImVec2 GetOutputSlotPos(int node_idx, int slot_no) const;

    // This function must:
    // - test if the link does not exists already
    // - test if the types match
    void addLink(int srcNode, int outputIdx, int dstNode, int intputIdx);

    std::vector<std::unique_ptr<ComputeNode>> m_Nodes;
    std::vector<NodeWidget> m_NodeWidgets;
    std::unordered_map<std::string, NodeID> m_NodeMap;

    std::vector<NodeLink> m_Links;

    std::vector<std::vector<Input>> m_NodeInputs;
    std::vector<std::vector<Output>> m_NodeOutputs;
};

template<typename T>
size_t addNodeType(const std::string& name) {
    return 0;
}

}