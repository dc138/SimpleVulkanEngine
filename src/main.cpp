#include <svke/svke.hpp>
using namespace svke;

int main() {
  Window window {512, 512, "First Application"};
  Device device {window};
  Pipeline pipeline {device, "shaders/simple.vert.spv", "shaders/simple.frag.spv",
                     Pipeline::DefaultPipelineConfig(512, 512)};

  while (!window.ShouldClose()) {
    window.PollEvents();
  }

  return 0;
}