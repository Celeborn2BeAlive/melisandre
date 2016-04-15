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

#include <melisandre/itertools/range.hpp>

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

enum ComputeVarMod {
    ComputeVarMod_In = 1 << 0,
    ComputeVarMod_Out = 1 << 1,
    ComputeVarMod_InOut = ComputeVarMod_In | ComputeVarMod_Out
};

inline bool drawNodeVarGUI(const std::string& name, ComputeVarMod mode, int& ref) {
    if (mode & ComputeVarMod_In) {
        return ImGui::InputInt(name.c_str(), &ref);
    }
    ImGui::Value(name.c_str(), ref);
    return false;
}

class ComputeNode 
{
public:
    static const auto In = ComputeVarMod_In;
    static const auto Out = ComputeVarMod_Out;
    static const auto InOut = ComputeVarMod_InOut;

    class IVar 
    {
    public:
        virtual ~IVar() {
        }

        explicit IVar(ComputeNode& thisNode, ComputeVarMod mod, const std::string& name) :
            m_Name{ name },
            m_nMod{ mod }
        {
            init(thisNode);
        }

        const std::string& name() const {
            return m_Name;
        }

        ComputeVarMod mode() const {
            return m_nMod;
        }

        virtual bool drawGUI() = 0;
    private:
        void init(ComputeNode& thisNode) {
            if (m_nMod & In) {
                ++thisNode.m_nInputCount;
            }
            if (m_nMod & Out) {
                ++thisNode.m_nOutputCount;
            }

            auto ptrOffset = (size_t) this - (size_t)&thisNode;

            thisNode.m_VarAddressOffsets.emplace_back(ptrOffset);
        }

        std::string m_Name;
        ComputeVarMod m_nMod;
    };

    template<typename T>
    class Var: public IVar
    {
    public:
        explicit Var(ComputeNode& thisNode, ComputeVarMod mod, const std::string& name) :
            IVar{ thisNode, mod, name } {
        }

        template<typename... Args>
        explicit Var(ComputeNode& thisNode, ComputeVarMod mod, const std::string& name, Args&&... args) :
            IVar{ thisNode, mod, name },
            m_Var{ std::forward<Args&&>(args)... } {
        }

        const T& get() const {
            return m_Var;
        }

        T& get() {
            return m_Var;
        }

        virtual bool drawGUI() {
            return drawNodeVarGUI(name(), mode(), m_Var);
        }
    private:
        T m_Var;
    };
private:
    friend class ComputeGraph;

    virtual void compute() = 0;

    virtual std::unique_ptr<ComputeNode> clone() const = 0;

    virtual int getInputsCount() const {
        return m_nInputCount;
    }

    virtual int getOutputsCount() const {
        return m_nOutputCount;
    }

    virtual void drawGUI() {
        for (auto ptrOffset : m_VarAddressOffsets) {
            IVar* ptr = (IVar*)((size_t) this + ptrOffset);
            ptr->drawGUI();
        }
    }

public:
    virtual ~ComputeNode() {
    }

private:
    std::vector<size_t> m_VarAddressOffsets;

    size_t m_nInputCount = 0;
    size_t m_nOutputCount = 0;
};

class ComputeGraph
{
public:
    typedef size_t NodeID;

    ComputeGraph(const std::string& name) {}

    // @todo: there is a bug here in the toposort, find it
    void compute(const std::unordered_set<NodeID>& visitSet) {
        std::unordered_map<NodeID, size_t> parentCount;
        for (auto node : visitSet) {
            const auto& links = m_Nodes[node].inputLinks;
            parentCount[node] = std::count_if(begin(links), end(links), [&](const auto& linkIdx) {
                return visitSet.find(m_Links[linkIdx].srcNodeIdx) != end(visitSet);
            });
        }

        std::stack<NodeID> visitStack;
        for (auto pair : parentCount) {
            if (pair.second == 0u) {
                visitStack.emplace(pair.first);
            }
        }

        while (!visitStack.empty()) {
            auto node = visitStack.top();
            visitStack.pop();

            std::cerr << "Computing node index " << node << std::endl;
            m_Nodes[node].node->compute();

            for (auto out : m_Nodes[node].outputLinks) {
                auto childNode = m_Links[out].dstNodeIdx;
                auto count = --parentCount[childNode];
                if (count == 0u) {
                    visitStack.emplace(childNode);
                }
            }
        }
    }

    void compute() {
        auto r = range(size(m_Nodes));
        compute(std::unordered_set<NodeID> { std::begin(r), std::end(r) });
    }

    void computeForward(NodeID srcNode) {
        std::unordered_set<NodeID> visitSet;
        depthFirstForwardNodeSearch(srcNode, [&](auto node) { return false; }, visitSet);
        compute(visitSet);
    }

    void computeBackward(NodeID srcNode) {
        std::unordered_set<NodeID> visitSet;
        depthFirstBackwardNodeSearch(srcNode, [&](auto node) { return false; }, visitSet);
        compute(visitSet);
    }

    void drawGUI(size_t width, size_t height, bool* pOpened = nullptr);

private:
    struct NodeLink
    {
        size_t srcNodeIdx, srcNodeSlotIdx, dstNodeIdx, dstNodeSlotIdx;

        NodeLink(size_t input_idx, size_t input_slot, size_t output_idx, size_t output_slot) { srcNodeIdx = input_idx; srcNodeSlotIdx = input_slot; dstNodeIdx = output_idx; dstNodeSlotIdx = output_slot; }
    };

    void addDummyNode(const std::string& name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count);

    void addIntNode(const std::string& name, const ImVec2& pos);

    ImVec2 GetInputSlotPos(size_t node_idx, size_t slot_no) const;

    ImVec2 GetOutputSlotPos(size_t node_idx, size_t slot_no) const;

    // This function must:
    // - test if the link does not exists already
    // - test if the types match
    int addLink(size_t srcNode, size_t outputIdx, size_t dstNode, size_t inputIdx);

    template<typename Functor>
    bool depthFirstForwardNodeSearch(size_t srcNode, Functor&& predicate, std::unordered_set<NodeID>& visitSet) {
        std::stack<NodeID> visitStack;

        visitStack.emplace(srcNode);
        visitSet.emplace(srcNode);

        while (!visitStack.empty()) {
            auto currentNode = visitStack.top();
            visitStack.pop();

            if (predicate(currentNode)) {
                return true;
            }

            for (auto out : m_Nodes[currentNode].outputLinks) {
                const auto& linkOut = m_Links[out];
                auto node = linkOut.dstNodeIdx;
                if (visitSet.find(node) == end(visitSet)) {
                    visitStack.push(node);
                    visitSet.emplace(node);
                }
            }
        }

        return false;
    }

    template<typename Functor>
    bool depthFirstForwardNodeSearch(size_t srcNode, Functor&& predicate) {
        std::unordered_set<NodeID> visitSet;
        return depthFirstForwardNodeSearch(srcNode, std::forward<Functor&&>(predicate), visitSet);
    }

    bool depthFirstForwardNodeSearch(size_t srcNode, size_t dstNode) {
        return depthFirstForwardNodeSearch(srcNode, [&](auto currentNode) {
            return currentNode == dstNode;
        });
    }

    template<typename Functor>
    bool depthFirstBackwardNodeSearch(size_t srcNode, Functor&& predicate, std::unordered_set<NodeID>& visitSet) {
        std::stack<NodeID> visitStack;

        visitStack.emplace(srcNode);
        visitSet.emplace(srcNode);

        while (!visitStack.empty()) {
            auto currentNode = visitStack.top();
            visitStack.pop();

            if (predicate(currentNode)) {
                return true;
            }

            for (auto in : m_Nodes[currentNode].inputLinks) {
                const auto& linkIn = m_Links[in];
                auto node = linkIn.srcNodeIdx;
                if (visitSet.find(node) == end(visitSet)) {
                    visitStack.push(node);
                    visitSet.emplace(node);
                }
            }
        }

        return false;
    }

    template<typename Functor>
    bool depthFirstBackwardNodeSearch(size_t srcNode, Functor&& predicate) {
        std::unordered_set<NodeID> visitSet;
        return depthFirstBackwardNodeSearch(srcNode, std::forward<Functor&&>(predicate), visitSet);
    }

    bool depthFirstBackwardNodeSearch(size_t srcNode, size_t dstNode) {
        return depthFirstBackwardNodeSearch(srcNode, [&](auto currentNode) {
            return currentNode == dstNode;
        });
    }

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

    std::string m_Message;
};

template<typename T>
size_t addNodeType(const std::string& name) {
    return 0;
}

}