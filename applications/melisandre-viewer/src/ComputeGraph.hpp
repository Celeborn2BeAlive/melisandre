#pragma once

#include <string>
#include <unordered_map>

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

// Dummy
struct Node
{
    int     ID;
    char    Name[32];
    ImVec2  Pos, Size;
    float   Value;
    ImVec4  Color;
    int     InputsCount, OutputsCount;

    Node(int id, const char* name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) { ID = id; strncpy(Name, name, 31); Name[31] = 0; Pos = pos; Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count; }

    ImVec2 GetInputSlotPos(int slot_no) const { return ImVec2(Pos.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)InputsCount + 1)); }
    ImVec2 GetOutputSlotPos(int slot_no) const { return ImVec2(Pos.x + Size.x, Pos.y + Size.y * ((float)slot_no + 1) / ((float)OutputsCount + 1)); }
};

struct NodeLink
{
    int     InputIdx, InputSlot, OutputIdx, OutputSlot;

    NodeLink(int input_idx, int input_slot, int output_idx, int output_slot) { InputIdx = input_idx; InputSlot = input_slot; OutputIdx = output_idx; OutputSlot = output_slot; }
};

// Really dumb data structure provided for the example.
// Note that we storing links are INDICES (not ID) to make example code shorter, obviously a bad idea for any general purpose code.
void ShowExampleAppCustomNodeGraph(ImVector<Node>& nodes, ImVector<NodeLink>& links, size_t width, size_t height, bool* opened = nullptr);

class ComputeGraph;

class ComputeNode {
public:
    class Input {
    public:
        using EventDispatcher = EventDispatcher<void()>;
        using EventListener = EventDispatcher::ListenerType;

        template<typename Functor>
        EventListener onChanged(Functor&& f) {
            m_Dispatcher.addListener(f);
        }
    private:
        EventDispatcher m_Dispatcher;
    };

    class Output {
    public:
        void notify(); // Notify all connected inputs that the output changed
    private:
    };

    ComputeNode(const ComputeGraph& graph) {
    }

protected:
    template<typename T>
    Input addInput(const std::string& name, T& ref) {
        return Input{};
    }

    template<typename T>
    Output addOutput(const std::string& name, T& ref, std::vector<Input> dependencies = {}) {
        return Output{};
    }
private:
    std::unordered_map<std::string, Input> m_InputMap;
    std::unordered_map<std::string, Output> m_OutputMap;
};

class ComputeGraph
{
public:
    ComputeGraph(const std::string& name);

    ComputeNode getNode(const std::string& id);

    void connect(ComputeNode::Input in, ComputeNode::Output out);

private:
    std::unordered_map<std::string, ComputeNode> m_NodeMap;
};

template<typename T>
size_t addNodeType(const std::string& name) {
    return 0;
}

}