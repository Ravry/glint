#include "mvk_core.h"

namespace Mvk { 
    void createFramebuffers(Context& context) {
        context.swapchainFramebuffers.resize(context.swapchainImageViews.size());

        for (size_t i {0}; i < context.swapchainImageViews.size(); i++) {
            VkImageView attachments[] { context.swapchainImageViews[i] };
            
            VkFramebufferCreateInfo framebufferInfo { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            framebufferInfo.renderPass = context.renderpass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = context.swapchainExtent.width;
            framebufferInfo.height = context.swapchainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(context.device, &framebufferInfo, nullptr, &context.swapchainFramebuffers[i]) != VK_SUCCESS) {
                THROW("failed to create framebuffer!");
            }
        }
    }
}