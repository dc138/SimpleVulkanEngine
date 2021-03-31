#define UNUSED(x) (void(x))

#include "svke/pipeline.hpp"

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace svke {
  Pipeline::Pipeline(Device& device, const std::string& vertex_path, const std::string& fragment_path,
                     const PipelineConfig& config)
      : pDevice {device} {
    auto vert_code = pReadFile(vertex_path);
    auto frag_code = pReadFile(fragment_path);

    pCreateShaderModule(vert_code, &pVertShaderModule);
    pCreateShaderModule(frag_code, &pFragShaderModule);

    VkPipelineShaderStageCreateInfo shader_stages[2];

    shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_stages[0].module = pVertShaderModule;
    shader_stages[0].pName = "main";
    shader_stages[0].flags = 0;
    shader_stages[0].pNext = nullptr;
    shader_stages[0].pSpecializationInfo = nullptr;

    shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_stages[1].module = pFragShaderModule;
    shader_stages[1].pName = "main";
    shader_stages[1].flags = 0;
    shader_stages[1].pNext = nullptr;
    shader_stages[1].pSpecializationInfo = nullptr;

    VkPipelineVertexInputStateCreateInfo vertex_input_info {};

    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr;
    vertex_input_info.pVertexBindingDescriptions = nullptr;

    VkPipelineViewportStateCreateInfo viewportInfo {};

    viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = &config.viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = &config.scissor;

    VkGraphicsPipelineCreateInfo pipeline_info {};

    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &config.input_assembly_info;
    pipeline_info.pViewportState = &viewportInfo;
    pipeline_info.pRasterizationState = &config.rasterization_info;
    pipeline_info.pMultisampleState = &config.multisample_info;
    pipeline_info.pColorBlendState = &config.colorblend_info;
    pipeline_info.pDynamicState = nullptr;  // Optional
    pipeline_info.pDepthStencilState = &config.depth_stencil_info;

    pipeline_info.layout = config.pipeline_layout;
    pipeline_info.renderPass = config.render_pass;
    pipeline_info.subpass = config.subpass;

    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipeline_info.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(device.getDevice(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pGraphicsPipeline) !=
        VK_SUCCESS) {
      throw std::runtime_error("Pipeline creation failed");
    }
  }

  Pipeline::~Pipeline() {
    vkDestroyShaderModule(pDevice.getDevice(), pVertShaderModule, nullptr);
    vkDestroyShaderModule(pDevice.getDevice(), pFragShaderModule, nullptr);
    vkDestroyPipeline(pDevice.getDevice(), pGraphicsPipeline, nullptr);
  }

  void Pipeline::Bind(VkCommandBuffer command_buffer) {
    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pGraphicsPipeline);
  }

  void Pipeline::DefaultPipelineConfig(PipelineConfig& config, uint32_t width, uint32_t height) {
    config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    config.input_assembly_info.primitiveRestartEnable = VK_FALSE;

    config.viewport.x = 0.0f;
    config.viewport.y = 0.0f;
    config.viewport.width = static_cast<float>(width);
    config.viewport.height = static_cast<float>(height);
    config.viewport.minDepth = 0.0f;
    config.viewport.maxDepth = 1.0f;

    config.scissor.offset = {0, 0};
    config.scissor.extent = {width, height};

    config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    config.rasterization_info.depthClampEnable = VK_FALSE;
    config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
    config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
    config.rasterization_info.lineWidth = 1.0f;
    config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
    config.rasterization_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    config.rasterization_info.depthBiasEnable = VK_FALSE;
    config.rasterization_info.depthBiasConstantFactor = 0.0f;  // Optional
    config.rasterization_info.depthBiasClamp = 0.0f;           // Optional
    config.rasterization_info.depthBiasSlopeFactor = 0.0f;     // Optional

    config.multisample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    config.multisample_info.sampleShadingEnable = VK_FALSE;
    config.multisample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    config.multisample_info.minSampleShading = 1.0f;           // Optional
    config.multisample_info.pSampleMask = nullptr;             // Optional
    config.multisample_info.alphaToCoverageEnable = VK_FALSE;  // Optional
    config.multisample_info.alphaToOneEnable = VK_FALSE;       // Optional

    config.colorblend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    config.colorblend_attachment.blendEnable = VK_FALSE;
    config.colorblend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config.colorblend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config.colorblend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
    config.colorblend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
    config.colorblend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
    config.colorblend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

    config.colorblend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    config.colorblend_info.logicOpEnable = VK_FALSE;
    config.colorblend_info.logicOp = VK_LOGIC_OP_COPY;  // Optional
    config.colorblend_info.attachmentCount = 1;
    config.colorblend_info.pAttachments = &config.colorblend_attachment;
    config.colorblend_info.blendConstants[0] = 0.0f;  // Optional
    config.colorblend_info.blendConstants[1] = 0.0f;  // Optional
    config.colorblend_info.blendConstants[2] = 0.0f;  // Optional
    config.colorblend_info.blendConstants[3] = 0.0f;  // Optional

    config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    config.depth_stencil_info.depthTestEnable = VK_TRUE;
    config.depth_stencil_info.depthWriteEnable = VK_TRUE;
    config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
    config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
    config.depth_stencil_info.minDepthBounds = 0.0f;  // Optional
    config.depth_stencil_info.maxDepthBounds = 1.0f;  // Optional
    config.depth_stencil_info.stencilTestEnable = VK_FALSE;
    config.depth_stencil_info.front = {};  // Optional
    config.depth_stencil_info.back = {};   // Optional
  }

  std::vector<char> Pipeline::pReadFile(const std::string& path) {
    std::ifstream file {path, std::ios::ate | std::ios::binary};

    if (!file.is_open()) {
      throw std::runtime_error("Cannot open provided filepath: " + path);
    }

    uint64_t          size = file.tellg();
    std::vector<char> buffer(size);

    file.seekg(0);
    file.read(buffer.data(), size);

    file.close();
    return buffer;
  }

  void Pipeline::pCreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module) {
    VkShaderModuleCreateInfo create_info {};

    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = code.size();
    create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(pDevice.getDevice(), &create_info, nullptr, shader_module) != VK_SUCCESS) {
      throw std::runtime_error("Failed to create shader module");
    }
  }
}