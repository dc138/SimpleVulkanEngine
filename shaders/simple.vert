#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 in_color;

layout(location = 0) out vec3 out_color;

layout(push_constant) uniform PushData { 
  mat4 transform; 
} push_data;

void main() {
  gl_Position = push_data.transform * vec4(position, 1.0);
  out_color   = in_color;
}
