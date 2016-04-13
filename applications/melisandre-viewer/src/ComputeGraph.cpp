#pragma once

#include "ComputeGraph.hpp"

#include <iostream>

namespace mls
{

class DummyNode : public ComputeNode {
private:
    void compute() override {
        std::cerr << "Dummy !! <3" << std::endl;
    }

    std::unique_ptr<ComputeNode> clone() const override {
        return std::make_unique<DummyNode>(*this);
    }

    float   Value;
    ImVec4  Color;
    int     InputsCount, OutputsCount;

public:
    DummyNode(float value, const ImVec4& color, int inputs_count, int outputs_count) {
        Value = value; Color = color; InputsCount = inputs_count; OutputsCount = outputs_count;
    }

    int getInputsCount() const {
        return InputsCount;
    }

    int getOutputsCount() const {
        return OutputsCount;
    }

    void drawGUI() {
        ImGui::SliderFloat("##value", &Value, 0.0f, 1.0f, "Alpha %.2f");
        ImGui::ColorEdit3("##color", &Color.x);
    }
};

class AddIntNode : public ComputeNode {
private:
    void compute() override {
        std::cerr << "Add" << std::endl;
    }

    std::unique_ptr<ComputeNode> clone() const override {
        return std::make_unique<AddIntNode>(*this);
    }

    int m_nLhs = 0;
    int m_nRhs = 0;
public:
    void drawGUI() {
        ImGui::InputInt("lhs", &m_nLhs);
        ImGui::InputInt("rhs", &m_nRhs);

        ImGui::Value("result", m_nLhs + m_nRhs);
    }

    int getInputsCount() const {
        return 2;
    }

    int getOutputsCount() const {
        return 1;
    }
};

void ComputeGraph::addDummyNode(const std::string& name, const ImVec2& pos, float value, const ImVec4& color, int inputs_count, int outputs_count) {
    m_Nodes.emplace_back();
    auto& instance = m_Nodes.back();
    instance.name = name;
    instance.position = pos;
    instance.size.x = instance.size.y = 0;
    instance.node = std::make_unique<DummyNode>(value, color, inputs_count, outputs_count);
}

void ComputeGraph::addIntNode(const std::string& name, const ImVec2& pos) {
    m_Nodes.emplace_back();
    auto& instance = m_Nodes.back();
    instance.name = name;
    instance.position = pos;
    instance.size.x = instance.size.y = 0;
    instance.node = std::make_unique<AddIntNode>();
}

ImVec2 ComputeGraph::GetInputSlotPos(int node_idx, int slot_no) const { 
    const auto& instance = m_Nodes[node_idx];
    const auto& node = instance.node;
    return ImVec2(instance.position.x, instance.position.y + instance.size.y * ((float)slot_no + 1) / ((float)node->getInputsCount() + 1));
}

ImVec2 ComputeGraph::GetOutputSlotPos(int node_idx, int slot_no) const {
    const auto& instance = m_Nodes[node_idx];
    const auto& node = instance.node;
    return ImVec2(instance.position.x + instance.size.x, instance.position.y + instance.size.y * ((float)slot_no + 1) / ((float)node->getOutputsCount() + 1));
}

bool ComputeGraph::addLink(int srcNode, int outputIdx, int dstNode, int inputIdx) {


    for (auto i = 0u; i < m_Links.size(); ++i) {
        if (m_Links[i].dstNodeIdx == dstNode && m_Links[i].dstNodeSlotIdx == inputIdx) {
            m_Links[i] = NodeLink(srcNode, outputIdx, dstNode, inputIdx);

            m_Nodes[srcNode].outputLinks.emplace_back(i);

            return true;
        }
    }
    m_Links.push_back(NodeLink(srcNode, outputIdx, dstNode, inputIdx));

    m_Nodes[srcNode].outputLinks.emplace_back(m_Links.size() - 1);
    m_Nodes[dstNode].inputLinks.emplace_back(m_Links.size() - 1);

    return true;
}

void ComputeGraph::drawGUI(size_t width, size_t height, bool* pOpened) {
    ImGui::SetNextWindowSize(ImVec2(width, height));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    if (!ImGui::Begin("Example: Custom Node Graph", pOpened, ImVec2(width, height), -1.f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
    {
        ImGui::End();
        return;
    }

    static bool inited = false;
    static ImVec2 scrolling = ImVec2(0.0f, 0.0f);
    static bool show_grid = true;
    static int node_selected = -1;

    static int plug_selected = -1;
    static int plug_selected_type = 0; // 0 = no plug selected, 1 = output plug, 2 = input plug
    static int plug_selected_node_idx = -1;

    if (!inited)
    {
        addDummyNode("MainTex", ImVec2(40, 50), 0.5f, ImColor(255, 100, 100), 1, 1);
        addDummyNode("BumpMap", ImVec2(40, 150), 0.42f, ImColor(200, 100, 200), 1, 1);
        addDummyNode("Combine", ImVec2(270, 80), 1.0f, ImColor(0, 200, 100), 2, 2);
        addLink(0, 0, 2, 0);
        addLink(1, 0, 2, 1);
        inited = true;
    }

    // Draw a list of nodes on the left side
    bool open_context_menu = false;
    int node_hovered_in_list = -1;
    int node_hovered_in_scene = -1;
    ImGui::BeginChild("node_list", ImVec2(100, 0));
    ImGui::Text("Nodes");
    ImGui::Separator();
    for (int node_idx = 0; node_idx < m_Nodes.size(); node_idx++)
    {
        const auto& instance = m_Nodes[node_idx];

        ImGui::PushID(node_idx);
        if (ImGui::Selectable(instance.name.c_str(), node_idx == node_selected))
            node_selected = node_idx;
        if (ImGui::IsItemHovered())
        {
            node_hovered_in_list = node_idx;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::BeginGroup();

    const float NODE_SLOT_RADIUS = 6.0f;
    const ImVec2 NODE_WINDOW_PADDING(8.0f, 8.0f);

    // Create our child canvas
    ImGui::Text("Hold middle mouse button to scroll (%.2f,%.2f)", scrolling.x, scrolling.y);
    ImGui::SameLine(ImGui::GetWindowWidth() - 100);
    ImGui::Checkbox("Show grid", &show_grid);
    ImGui::Text("win_pos (%.2f,%.2f)", ImGui::GetCursorScreenPos().x, ImGui::GetCursorScreenPos().y);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(1, 1));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildWindowBg, ImColor(60, 60, 70, 200));
    ImGui::BeginChild("scrolling_region", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
    ImGui::PushItemWidth(120.0f);

    ImVec2 offset = ImGui::GetCursorScreenPos() - scrolling;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->ChannelsSplit(2);

    // Display grid
    if (show_grid)
    {
        ImU32 GRID_COLOR = ImColor(200, 200, 200, 40);
        float GRID_SZ = 64.0f;
        ImVec2 win_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_sz = ImGui::GetWindowSize();
        for (float x = fmodf(offset.x, GRID_SZ); x < canvas_sz.x; x += GRID_SZ)
            draw_list->AddLine(ImVec2(x, 0.0f) + win_pos, ImVec2(x, canvas_sz.y) + win_pos, GRID_COLOR);
        for (float y = fmodf(offset.y, GRID_SZ); y < canvas_sz.y; y += GRID_SZ)
            draw_list->AddLine(ImVec2(0.0f, y) + win_pos, ImVec2(canvas_sz.x, y) + win_pos, GRID_COLOR);
    }

    // Display links
    draw_list->ChannelsSetCurrent(0); // Background
    for (int link_idx = 0; link_idx < m_Links.size(); link_idx++)
    {
        const auto& link = m_Links[link_idx];
        const auto& node_inp = m_Nodes[link.srcNodeIdx];
        const auto& node_out = m_Nodes[link.dstNodeIdx];
        ImVec2 p1 = offset + GetOutputSlotPos(link.srcNodeIdx, link.srcNodeSlotIdx);
        ImVec2 p2 = offset + GetInputSlotPos(link.dstNodeIdx, link.dstNodeSlotIdx);
        draw_list->AddBezierCurve(p1, p1 + ImVec2(+50, 0), p2 + ImVec2(-50, 0), p2, ImColor(200, 200, 100), 3.0f);
    }

    // Display nodes
    for (int node_idx = 0; node_idx < m_Nodes.size(); node_idx++)
    {
        auto& instance = m_Nodes[node_idx];
        const auto& node = instance.node;

        ImGui::PushID(node_idx);
        ImVec2 node_rect_min = offset + instance.position;

        // Display node contents first
        draw_list->ChannelsSetCurrent(1); // Foreground
        bool old_any_active = ImGui::IsAnyItemActive();
        ImGui::SetCursorScreenPos(node_rect_min + NODE_WINDOW_PADDING);
        ImGui::BeginGroup(); // Lock horizontal position
        
        ImGui::Text("%s (%d)", instance.name.c_str(), node_idx);
        node->drawGUI();

        ImGui::EndGroup();

        // Save the size of what we have emitted and whether any of the widgets are being used
        bool node_widgets_active = (!old_any_active && ImGui::IsAnyItemActive());
        instance.size = ImGui::GetItemRectSize() + NODE_WINDOW_PADDING + NODE_WINDOW_PADDING;
        ImVec2 node_rect_max = node_rect_min + instance.size;

        ImGui::PushID(2); // Input widgets
        for (int slot_idx = 0; slot_idx < node->getInputsCount(); slot_idx++) {
            ImGui::PushID(slot_idx);

            auto nodeCenter = offset + GetInputSlotPos(node_idx, slot_idx);

            ImGui::SetCursorScreenPos(nodeCenter - ImVec2(NODE_SLOT_RADIUS, NODE_SLOT_RADIUS));
            ImGui::InvisibleButton("input", ImVec2(NODE_SLOT_RADIUS * 2, NODE_SLOT_RADIUS * 2));

            ImColor color = ImColor(150, 150, 150, 150);

            if (ImGui::IsItemHovered()) {
                if (plug_selected_type != 2) {
                    color.Value.w = 255;
                }

                if (ImGui::IsMouseClicked(0)) {
                    if (plug_selected_type == 1) {
                        plug_selected_type = 0;
                        addLink(plug_selected_node_idx, plug_selected, node_idx, slot_idx);
                    }
                    else {
                        plug_selected_type = 2;
                        plug_selected = slot_idx;
                        plug_selected_node_idx = node_idx;
                    }
                }
            }

            draw_list->AddCircleFilled(nodeCenter, NODE_SLOT_RADIUS, color);

            ImGui::PopID();
        }
        ImGui::PopID();

        ImGui::PushID(1); // Output widgets
        for (int slot_idx = 0; slot_idx < node->getOutputsCount(); slot_idx++) {
            ImGui::PushID(slot_idx);

            auto nodeCenter = offset + GetOutputSlotPos(node_idx, slot_idx);

            ImGui::SetCursorScreenPos(nodeCenter - ImVec2(NODE_SLOT_RADIUS, NODE_SLOT_RADIUS));
            ImGui::InvisibleButton("output", ImVec2(NODE_SLOT_RADIUS * 2, NODE_SLOT_RADIUS * 2));

            ImColor color = ImColor(150, 150, 150, 150);

            if (ImGui::IsItemHovered()) {
                if (plug_selected_type != 1) {
                    color.Value.w = 255;
                }

                if (ImGui::IsMouseClicked(0)) {
                    if (plug_selected_type == 2) {
                        plug_selected_type = 0;
                        addLink(node_idx, slot_idx, plug_selected_node_idx, plug_selected);
                    }
                    else {
                        plug_selected_type = 1;
                        plug_selected = slot_idx;
                        plug_selected_node_idx = node_idx;
                    }
                }
            }

            draw_list->AddCircleFilled(nodeCenter, NODE_SLOT_RADIUS, color);

            ImGui::PopID();
        }
        ImGui::PopID();

        // Display node box
        draw_list->ChannelsSetCurrent(0); // Background
        ImGui::SetCursorScreenPos(node_rect_min);
        ImGui::InvisibleButton("node", instance.size);
        if (ImGui::IsItemHovered())
        {
            node_hovered_in_scene = node_idx;
            open_context_menu |= ImGui::IsMouseClicked(1);
        }
        bool node_moving_active = ImGui::IsItemActive();
        if (node_widgets_active || node_moving_active)
            node_selected = node_idx;
        if (node_moving_active && ImGui::IsMouseDragging(0))
            instance.position = instance.position + ImGui::GetIO().MouseDelta;

        ImU32 node_bg_color = (node_hovered_in_list == node_idx || node_hovered_in_scene == node_idx || (node_hovered_in_list == -1 && node_selected == node_idx)) ? ImColor(75, 75, 75) : ImColor(60, 60, 60);
        draw_list->AddRectFilled(node_rect_min, node_rect_max, node_bg_color, 4.0f);
        draw_list->AddRect(node_rect_min, node_rect_max, ImColor(100, 100, 100), 4.0f);

        ImGui::PopID();
    }

    if (plug_selected_type > 0) {
        ImVec2 p1 = offset + (plug_selected_type == 1 ? GetOutputSlotPos(plug_selected_node_idx, plug_selected) : GetInputSlotPos(plug_selected_node_idx, plug_selected));
        ImVec2 p2 = ImGui::GetMousePos();
        draw_list->AddLine(p1, p2, ImColor(200, 200, 100), 3.0f);
    }

    if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(0)) {
        plug_selected_type = 0;
    }

    draw_list->ChannelsMerge();

    // Open context menu
    if (!ImGui::IsAnyItemHovered() && ImGui::IsMouseHoveringWindow() && ImGui::IsMouseClicked(1))
    {
        node_selected = node_hovered_in_list = node_hovered_in_scene = -1;
        open_context_menu = true;
    }
    if (open_context_menu)
    {
        ImGui::OpenPopup("context_menu");
        if (node_hovered_in_list != -1)
            node_selected = node_hovered_in_list;
        if (node_hovered_in_scene != -1)
            node_selected = node_hovered_in_scene;
    }

    // Draw context menu
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("context_menu"))
    {
        const auto& node = node_selected != -1 ? m_Nodes[node_selected].node.get() : nullptr;
        ImVec2 scene_pos = ImGui::GetMousePosOnOpeningCurrentPopup() - offset;
        if (node)
        {
            ImGui::Text("Node '%s'", m_Nodes[node_selected].name.c_str());
            ImGui::Separator();
            if (ImGui::MenuItem("Rename..", NULL, false, false)) {}
            if (ImGui::MenuItem("Delete", NULL, false, false)) {}
            if (ImGui::MenuItem("Copy", NULL, false, false)) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Compute Forward", NULL, false)) {
                computeForward(node_selected);
            }
        }
        else
        {
            if (ImGui::MenuItem("Add")) { 
                addIntNode("Add", scene_pos);
            }
            if (ImGui::MenuItem("Paste", NULL, false, false)) {}
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleVar();

    // Scrolling
    if (ImGui::IsWindowHovered() && !ImGui::IsAnyItemActive() && ImGui::IsMouseDragging(2, 0.0f))
        scrolling = scrolling - ImGui::GetIO().MouseDelta;

    ImGui::PopItemWidth();
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::EndGroup();

    ImGui::End();
}

}