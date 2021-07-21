#version 450

layout (location = 0) in vec2 position;
layout (location = 1) in vec3 in_color;

layout (location = 0) out vec3 out_color;

layout (push_constant) uniform PushData {
  mat2 transform;
  vec2 offset;
} push_data;

void main() {
  gl_Position = vec4(push_data.transform * position + push_data.offset, 0.0, 1.0);
  out_color = in_color;
}
