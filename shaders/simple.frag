#version 450

layout (location = 0) in vec3 in_color;

layout (location = 0) out vec4 out_color;

layout (push_constant) uniform PushData {
  vec2 offset;
} push_data;

void main() {
  out_color = vec4(in_color, 1.0f);
}