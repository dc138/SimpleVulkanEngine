#ifndef SVKE_PIPELINE_HPP
#define SVKE_PIPELINE_HPP

#include "device.hpp"
#include "pch.hpp"

namespace svke {
  struct PipelineConfig {
    PipelineConfig(const PipelineConfig&) = delete;
    PipelineConfig& operator=(const PipelineConfig&) = delete;

    VkViewport                             viewport;
    VkRect2D                               scissor;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
    VkPipelineRasterizationStateCreateInfo rasterization_info;
    VkPipelineMultisampleStateCreateInfo   multisample_info;
    VkPipelineColorBlendAttachmentState    colorblend_attachment;
    VkPipelineColorBlendStateCreateInfo    colorblend_info;
    VkPipelineDepthStencilStateCreateInfo  depth_stencil_info;
    VkPipelineLayout                       pipeline_layout = nullptr;
    VkRenderPass                           render_pass = nullptr;
    uint32_t                               subpass = 0;
  };

  class Pipeline {
   public:
    Pipeline(Device&               device,
             const std::string&    vertex_path,
             const std::string&    fragment_path,
             const PipelineConfig& config);
    ~Pipeline();

    Pipeline(const Pipeline& other) = delete;
    Pipeline& operator=(const Pipeline& other) = delete;

    void Bind(VkCommandBuffer command_buffer);

    static void DefaultPipelineConfig(PipelineConfig& config, uint32_t width, uint32_t height);

   private:
    static std::vector<char> pReadFile(const std::string& path);
    void                     pCreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);

   private:
    Device&        pDevice;
    VkPipeline     pGraphicsPipeline;
    VkShaderModule pVertShaderModule;
    VkShaderModule pFragShaderModule;
  };
}

#endif