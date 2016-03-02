#version 330 core

layout(location = 0) out vec3 fColor;
layout(location = 1) out uvec4 fObjectID;

void main() {
    fColor = vec3(1,0,0);
    fObjectID = uvec4(0);
}
