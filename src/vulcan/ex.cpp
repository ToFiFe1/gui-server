#include "ex.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <stdexcept>
#include <optional>
#include <algorithm>
#include <limits>


// ============================================================
// Hilfsfunktionen
// ============================================================

static void checkVk(VkResult result, const char* message)
{
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error(message);
    }
}


// ============================================================
// Queue-Familien
// ============================================================

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphics;
    std::optional<uint32_t> present;

    bool complete() const
    {
        return graphics.has_value() &&
               present.has_value();
    }
};


// ============================================================
// Swapchain
// ============================================================

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities{};

    std::vector<VkSurfaceFormatKHR> formats;

    std::vector<VkPresentModeKHR> presentModes;
};


// ============================================================
// Renderer
// ============================================================

class BitVulkanRenderer
{
public:

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }


private:

    // --------------------------------------------------------
    // GLFW
    // --------------------------------------------------------

    GLFWwindow* window = nullptr;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;


    // --------------------------------------------------------
    // Vulkan
    // --------------------------------------------------------

    VkInstance instance = VK_NULL_HANDLE;

    VkSurfaceKHR surface = VK_NULL_HANDLE;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    VkDevice device = VK_NULL_HANDLE;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;


    // --------------------------------------------------------
    // Swapchain
    // --------------------------------------------------------

    VkSwapchainKHR swapChain = VK_NULL_HANDLE;

    std::vector<VkImage> swapChainImages;

    VkFormat swapChainImageFormat;

    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;


    // --------------------------------------------------------
    // Rendering
    // --------------------------------------------------------

    VkRenderPass renderPass = VK_NULL_HANDLE;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    VkCommandPool commandPool = VK_NULL_HANDLE;

    std::vector<VkCommandBuffer> commandBuffers;


    // --------------------------------------------------------
    // Synchronisation
    // --------------------------------------------------------

    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;

    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;

    VkFence inFlightFence = VK_NULL_HANDLE;


private:

    // ========================================================
    // Fenster
    // ========================================================

    void initWindow()
    {
        if (!glfwInit())
        {
            throw std::runtime_error(
                "GLFW konnte nicht gestartet werden."
            );
        }

        glfwWindowHint(
            GLFW_CLIENT_API,
            GLFW_NO_API
        );

        glfwWindowHint(
            GLFW_RESIZABLE,
            GLFW_FALSE
        );

        window = glfwCreateWindow(
            WIDTH,
            HEIGHT,
            "Bit Vulkan Renderer",
            nullptr,
            nullptr
        );

        if (!window)
        {
            glfwTerminate();

            throw std::runtime_error(
                "GLFW Fenster konnte nicht erstellt werden."
            );
        }
    }


    // ========================================================
    // Vulkan
    // ========================================================

    void initVulkan()
    {
        createInstance();

        createSurface();

        pickPhysicalDevice();

        createLogicalDevice();

        createSwapChain();

        createImageViews();

        createRenderPass();

        createFramebuffers();

        createCommandPool();

        createCommandBuffers();

        createSyncObjects();
    }


    // ========================================================
    // Vulkan Instance
    // ========================================================

    void createInstance()
    {
        VkApplicationInfo appInfo{};

        appInfo.sType =
            VK_STRUCTURE_TYPE_APPLICATION_INFO;

        appInfo.pApplicationName =
            "Bit Renderer";

        appInfo.applicationVersion =
            VK_MAKE_VERSION(1, 0, 0);

        appInfo.pEngineName =
            "Bit";

        appInfo.engineVersion =
            VK_MAKE_VERSION(1, 0, 0);

        appInfo.apiVersion =
            VK_API_VERSION_1_3;


        uint32_t extensionCount = 0;

        const char** extensions =
            glfwGetRequiredInstanceExtensions(
                &extensionCount
            );

        if (!extensions)
        {
            throw std::runtime_error(
                "GLFW Vulkan Extensions konnten nicht gefunden werden."
            );
        }


        VkInstanceCreateInfo createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

        createInfo.pApplicationInfo =
            &appInfo;

        createInfo.enabledExtensionCount =
            extensionCount;

        createInfo.ppEnabledExtensionNames =
            extensions;


        checkVk(
            vkCreateInstance(
                &createInfo,
                nullptr,
                &instance
            ),
            "Vulkan Instance konnte nicht erstellt werden."
        );


        std::cout
            << "Vulkan Instance erstellt.\n";
    }


    // ========================================================
    // Surface
    // ========================================================

    void createSurface()
    {
        checkVk(
            glfwCreateWindowSurface(
                instance,
                window,
                nullptr,
                &surface
            ),
            "Vulkan Surface konnte nicht erstellt werden."
        );


        std::cout
            << "Vulkan Surface erstellt.\n";
    }


    // ========================================================
    // GPU
    // ========================================================

    void pickPhysicalDevice()
    {
        uint32_t deviceCount = 0;

        vkEnumeratePhysicalDevices(
            instance,
            &deviceCount,
            nullptr
        );


        if (deviceCount == 0)
        {
            throw std::runtime_error(
                "Keine Vulkan GPU gefunden."
            );
        }


        std::vector<VkPhysicalDevice> devices(
            deviceCount
        );


        vkEnumeratePhysicalDevices(
            instance,
            &deviceCount,
            devices.data()
        );


        for (auto deviceCandidate : devices)
        {
            QueueFamilyIndices indices =
                findQueueFamilies(deviceCandidate);

            if (indices.complete())
            {
                physicalDevice =
                    deviceCandidate;

                break;
            }
        }


        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error(
                "Keine geeignete Vulkan GPU gefunden."
            );
        }


        VkPhysicalDeviceProperties properties{};

        vkGetPhysicalDeviceProperties(
            physicalDevice,
            &properties
        );


        std::cout
            << "GPU: "
            << properties.deviceName
            << '\n';


        std::cout
            << "Vulkan: "
            << VK_VERSION_MAJOR(
                properties.apiVersion
            )
            << "."
            << VK_VERSION_MINOR(
                properties.apiVersion
            )
            << "."
            << VK_VERSION_PATCH(
                properties.apiVersion
            )
            << '\n';
    }


    // ========================================================
    // Queue Families suchen
    // ========================================================

    QueueFamilyIndices findQueueFamilies(
        VkPhysicalDevice deviceCandidate
    )
    {
        QueueFamilyIndices indices;


        uint32_t queueFamilyCount = 0;

        vkGetPhysicalDeviceQueueFamilyProperties(
            deviceCandidate,
            &queueFamilyCount,
            nullptr
        );


        std::vector<VkQueueFamilyProperties>
            queueFamilies(
                queueFamilyCount
            );


        vkGetPhysicalDeviceQueueFamilyProperties(
            deviceCandidate,
            &queueFamilyCount,
            queueFamilies.data()
        );


        uint32_t i = 0;


        for (const auto& queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags &
                VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphics = i;
            }


            VkBool32 presentSupport = false;


            vkGetPhysicalDeviceSurfaceSupportKHR(
                deviceCandidate,
                i,
                surface,
                &presentSupport
            );


            if (presentSupport)
            {
                indices.present = i;
            }


            if (indices.complete())
            {
                break;
            }


            i++;
        }


        return indices;
    }


    // ========================================================
    // Logical Device
    // ========================================================

   void createLogicalDevice()
{
    QueueFamilyIndices indices =
        findQueueFamilies(
            physicalDevice
        );


    std::vector<VkDeviceQueueCreateInfo>
        queueCreateInfos;


    std::vector<uint32_t> uniqueFamilies =
    {
        indices.graphics.value(),
        indices.present.value()
    };


    std::sort(
        uniqueFamilies.begin(),
        uniqueFamilies.end()
    );


    uniqueFamilies.erase(
        std::unique(
            uniqueFamilies.begin(),
            uniqueFamilies.end()
        ),
        uniqueFamilies.end()
    );


    float queuePriority = 1.0f;


    for (uint32_t queueFamily :
         uniqueFamilies)
    {
        VkDeviceQueueCreateInfo queueInfo{};

        queueInfo.sType =
            VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;

        queueInfo.queueFamilyIndex =
            queueFamily;

        queueInfo.queueCount = 1;

        queueInfo.pQueuePriorities =
            &queuePriority;


        queueCreateInfos.push_back(
            queueInfo
        );
    }


    // ========================================================
    // WICHTIG:
    // Swapchain Extension aktivieren
    // ========================================================

    const char* deviceExtensions[] =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };


    VkPhysicalDeviceFeatures deviceFeatures{};


    VkDeviceCreateInfo createInfo{};

    createInfo.sType =
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(
            queueCreateInfos.size()
        );

    createInfo.pQueueCreateInfos =
        queueCreateInfos.data();

    createInfo.pEnabledFeatures =
        &deviceFeatures;


    // Vulkan Swapchain
    createInfo.enabledExtensionCount = 1;

    createInfo.ppEnabledExtensionNames =
        deviceExtensions;


    checkVk(
        vkCreateDevice(
            physicalDevice,
            &createInfo,
            nullptr,
            &device
        ),
        "Logical Device konnte nicht erstellt werden."
    );


    vkGetDeviceQueue(
        device,
        indices.graphics.value(),
        0,
        &graphicsQueue
    );


    vkGetDeviceQueue(
        device,
        indices.present.value(),
        0,
        &presentQueue
    );


    std::cout
        << "Vulkan Logical Device erstellt.\n";
}


    // ========================================================
    // Swapchain Support
    // ========================================================

    SwapChainSupportDetails querySwapChainSupport(
        VkPhysicalDevice deviceCandidate
    )
    {
        SwapChainSupportDetails details;


        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            deviceCandidate,
            surface,
            &details.capabilities
        );


        uint32_t formatCount = 0;

        vkGetPhysicalDeviceSurfaceFormatsKHR(
            deviceCandidate,
            surface,
            &formatCount,
            nullptr
        );


        if (formatCount > 0)
        {
            details.formats.resize(
                formatCount
            );

            vkGetPhysicalDeviceSurfaceFormatsKHR(
                deviceCandidate,
                surface,
                &formatCount,
                details.formats.data()
            );
        }


        uint32_t presentModeCount = 0;

        vkGetPhysicalDeviceSurfacePresentModesKHR(
            deviceCandidate,
            surface,
            &presentModeCount,
            nullptr
        );


        if (presentModeCount > 0)
        {
            details.presentModes.resize(
                presentModeCount
            );

            vkGetPhysicalDeviceSurfacePresentModesKHR(
                deviceCandidate,
                surface,
                &presentModeCount,
                details.presentModes.data()
            );
        }


        return details;
    }


    // ========================================================
    // Swapchain Format
    // ========================================================

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& formats
    )
    {
        for (const auto& format : formats)
        {
            if (
                format.format ==
                VK_FORMAT_B8G8R8A8_SRGB &&

                format.colorSpace ==
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            )
            {
                return format;
            }
        }


        return formats[0];
    }


    // ========================================================
    // Present Mode
    // ========================================================

    VkPresentModeKHR chooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& modes
    )
    {
        for (const auto& mode : modes)
        {
            if (mode ==
                VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return mode;
            }
        }


        return VK_PRESENT_MODE_FIFO_KHR;
    }


    // ========================================================
    // Swapchain Größe
    // ========================================================

    VkExtent2D chooseSwapExtent(
        const VkSurfaceCapabilitiesKHR& capabilities
    )
    {
        if (
            capabilities.currentExtent.width !=
            std::numeric_limits<uint32_t>::max()
        )
        {
            return capabilities.currentExtent;
        }


        VkExtent2D extent =
        {
            WIDTH,
            HEIGHT
        };


        extent.width =
            std::clamp(
                extent.width,
                capabilities.minImageExtent.width,
                capabilities.maxImageExtent.width
            );


        extent.height =
            std::clamp(
                extent.height,
                capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height
            );


        return extent;
    }


    // ========================================================
    // Swapchain
    // ========================================================

    void createSwapChain()
    {
        SwapChainSupportDetails support =
            querySwapChainSupport(
                physicalDevice
            );


        VkSurfaceFormatKHR surfaceFormat =
            chooseSwapSurfaceFormat(
                support.formats
            );


        VkPresentModeKHR presentMode =
            chooseSwapPresentMode(
                support.presentModes
            );


        VkExtent2D extent =
            chooseSwapExtent(
                support.capabilities
            );


        uint32_t imageCount =
            support.capabilities.minImageCount + 1;


        if (
            support.capabilities.maxImageCount > 0 &&
            imageCount >
            support.capabilities.maxImageCount
        )
        {
            imageCount =
                support.capabilities.maxImageCount;
        }


        VkSwapchainCreateInfoKHR createInfo{};

        createInfo.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        createInfo.surface =
            surface;

        createInfo.minImageCount =
            imageCount;

        createInfo.imageFormat =
            surfaceFormat.format;

        createInfo.imageColorSpace =
            surfaceFormat.colorSpace;

        createInfo.imageExtent =
            extent;

        createInfo.imageArrayLayers = 1;

        createInfo.imageUsage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;


        QueueFamilyIndices indices =
            findQueueFamilies(
                physicalDevice
            );


        uint32_t queueFamilyIndices[] =
        {
            indices.graphics.value(),
            indices.present.value()
        };


        if (
            indices.graphics !=
            indices.present
        )
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_CONCURRENT;

            createInfo.queueFamilyIndexCount = 2;

            createInfo.pQueueFamilyIndices =
                queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode =
                VK_SHARING_MODE_EXCLUSIVE;
        }


        createInfo.preTransform =
            support.capabilities.currentTransform;

        createInfo.compositeAlpha =
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode =
            presentMode;

        createInfo.clipped =
            VK_TRUE;


        checkVk(
            vkCreateSwapchainKHR(
                device,
                &createInfo,
                nullptr,
                &swapChain
            ),
            "Swapchain konnte nicht erstellt werden."
        );


        vkGetSwapchainImagesKHR(
            device,
            swapChain,
            &imageCount,
            nullptr
        );


        swapChainImages.resize(
            imageCount
        );


        vkGetSwapchainImagesKHR(
            device,
            swapChain,
            &imageCount,
            swapChainImages.data()
        );


        swapChainImageFormat =
            surfaceFormat.format;


        swapChainExtent =
            extent;


        std::cout
            << "Swapchain erstellt.\n";
    }


    // ========================================================
    // Image Views
    // ========================================================

    void createImageViews()
    {
        swapChainImageViews.resize(
            swapChainImages.size()
        );


        for (size_t i = 0;
             i < swapChainImages.size();
             i++)
        {
            VkImageViewCreateInfo createInfo{};

            createInfo.sType =
                VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

            createInfo.image =
                swapChainImages[i];

            createInfo.viewType =
                VK_IMAGE_VIEW_TYPE_2D;

            createInfo.format =
                swapChainImageFormat;


            createInfo.components.r =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.g =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.b =
                VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.components.a =
                VK_COMPONENT_SWIZZLE_IDENTITY;


            createInfo.subresourceRange.aspectMask =
                VK_IMAGE_ASPECT_COLOR_BIT;

            createInfo.subresourceRange.baseMipLevel =
                0;

            createInfo.subresourceRange.levelCount =
                1;

            createInfo.subresourceRange.baseArrayLayer =
                0;

            createInfo.subresourceRange.layerCount =
                1;


            checkVk(
                vkCreateImageView(
                    device,
                    &createInfo,
                    nullptr,
                    &swapChainImageViews[i]
                ),
                "ImageView konnte nicht erstellt werden."
            );
        }


        std::cout
            << "ImageViews erstellt.\n";
    }


    // ========================================================
    // Render Pass
    // ========================================================

    void createRenderPass()
    {
        VkAttachmentDescription colorAttachment{};

        colorAttachment.format =
            swapChainImageFormat;

        colorAttachment.samples =
            VK_SAMPLE_COUNT_1_BIT;

        colorAttachment.loadOp =
            VK_ATTACHMENT_LOAD_OP_CLEAR;

        colorAttachment.storeOp =
            VK_ATTACHMENT_STORE_OP_STORE;

        colorAttachment.stencilLoadOp =
            VK_ATTACHMENT_LOAD_OP_DONT_CARE;

        colorAttachment.stencilStoreOp =
            VK_ATTACHMENT_STORE_OP_DONT_CARE;

        colorAttachment.initialLayout =
            VK_IMAGE_LAYOUT_UNDEFINED;

        colorAttachment.finalLayout =
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


        VkAttachmentReference colorAttachmentRef{};

        colorAttachmentRef.attachment = 0;

        colorAttachmentRef.layout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


        VkSubpassDescription subpass{};

        subpass.pipelineBindPoint =
            VK_PIPELINE_BIND_POINT_GRAPHICS;

        subpass.colorAttachmentCount = 1;

        subpass.pColorAttachments =
            &colorAttachmentRef;


        VkSubpassDependency dependency{};

        dependency.srcSubpass =
            VK_SUBPASS_EXTERNAL;

        dependency.dstSubpass = 0;

        dependency.srcStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        dependency.dstStageMask =
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        dependency.dstAccessMask =
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;


        VkRenderPassCreateInfo renderPassInfo{};

        renderPassInfo.sType =
            VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;

        renderPassInfo.attachmentCount = 1;

        renderPassInfo.pAttachments =
            &colorAttachment;

        renderPassInfo.subpassCount = 1;

        renderPassInfo.pSubpasses =
            &subpass;

        renderPassInfo.dependencyCount = 1;

        renderPassInfo.pDependencies =
            &dependency;


        checkVk(
            vkCreateRenderPass(
                device,
                &renderPassInfo,
                nullptr,
                &renderPass
            ),
            "RenderPass konnte nicht erstellt werden."
        );


        std::cout
            << "RenderPass erstellt.\n";
    }


    // ========================================================
    // Framebuffers
    // ========================================================

    void createFramebuffers()
    {
        swapChainFramebuffers.resize(
            swapChainImageViews.size()
        );


        for (size_t i = 0;
             i < swapChainImageViews.size();
             i++)
        {
            VkImageView attachments[] =
            {
                swapChainImageViews[i]
            };


            VkFramebufferCreateInfo framebufferInfo{};

            framebufferInfo.sType =
                VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

            framebufferInfo.renderPass =
                renderPass;

            framebufferInfo.attachmentCount = 1;

            framebufferInfo.pAttachments =
                attachments;

            framebufferInfo.width =
                swapChainExtent.width;

            framebufferInfo.height =
                swapChainExtent.height;

            framebufferInfo.layers = 1;


            checkVk(
                vkCreateFramebuffer(
                    device,
                    &framebufferInfo,
                    nullptr,
                    &swapChainFramebuffers[i]
                ),
                "Framebuffer konnte nicht erstellt werden."
            );
        }


        std::cout
            << "Framebuffers erstellt.\n";
    }


    // ========================================================
    // Command Pool
    // ========================================================

    void createCommandPool()
    {
        QueueFamilyIndices indices =
            findQueueFamilies(
                physicalDevice
            );


        VkCommandPoolCreateInfo poolInfo{};

        poolInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

        poolInfo.queueFamilyIndex =
            indices.graphics.value();


        checkVk(
            vkCreateCommandPool(
                device,
                &poolInfo,
                nullptr,
                &commandPool
            ),
            "CommandPool konnte nicht erstellt werden."
        );
    }


    // ========================================================
    // Command Buffers
    // ========================================================

    void createCommandBuffers()
    {
        commandBuffers.resize(
            swapChainFramebuffers.size()
        );


        VkCommandBufferAllocateInfo allocInfo{};

        allocInfo.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

        allocInfo.commandPool =
            commandPool;

        allocInfo.level =
            VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        allocInfo.commandBufferCount =
            static_cast<uint32_t>(
                commandBuffers.size()
            );


        checkVk(
            vkAllocateCommandBuffers(
                device,
                &allocInfo,
                commandBuffers.data()
            ),
            "CommandBuffers konnten nicht erstellt werden."
        );


        for (size_t i = 0;
             i < commandBuffers.size();
             i++)
        {
            VkCommandBufferBeginInfo beginInfo{};

            beginInfo.sType =
                VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


            checkVk(
                vkBeginCommandBuffer(
                    commandBuffers[i],
                    &beginInfo
                ),
                "CommandBuffer konnte nicht gestartet werden."
            );


            VkClearValue clearColor{};

            clearColor.color =
            {
                0.02f,
                0.02f,
                0.03f,
                1.0f
            };


            VkRenderPassBeginInfo renderPassInfo{};

            renderPassInfo.sType =
                VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

            renderPassInfo.renderPass =
                renderPass;

            renderPassInfo.framebuffer =
                swapChainFramebuffers[i];

            renderPassInfo.renderArea.offset =
            {
                0,
                0
            };

            renderPassInfo.renderArea.extent =
                swapChainExtent;

            renderPassInfo.clearValueCount = 1;

            renderPassInfo.pClearValues =
                &clearColor;


            vkCmdBeginRenderPass(
                commandBuffers[i],
                &renderPassInfo,
                VK_SUBPASS_CONTENTS_INLINE
            );


            // Hier kommen später:
            //
            // vkCmdBindPipeline(...)
            // vkCmdDraw(...)
            //
            // Dein Bit-Grid.


            vkCmdEndRenderPass(
                commandBuffers[i]
            );


            checkVk(
                vkEndCommandBuffer(
                    commandBuffers[i]
                ),
                "CommandBuffer konnte nicht beendet werden."
            );
        }


        std::cout
            << "CommandBuffers erstellt.\n";
    }


    // ========================================================
    // Synchronisation
    // ========================================================

    void createSyncObjects()
    {
        VkSemaphoreCreateInfo semaphoreInfo{};

        semaphoreInfo.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;


        VkFenceCreateInfo fenceInfo{};

        fenceInfo.sType =
            VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

        fenceInfo.flags =
            VK_FENCE_CREATE_SIGNALED_BIT;


        checkVk(
            vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &imageAvailableSemaphore
            ),
            "Image-Semaphore konnte nicht erstellt werden."
        );


        checkVk(
            vkCreateSemaphore(
                device,
                &semaphoreInfo,
                nullptr,
                &renderFinishedSemaphore
            ),
            "Render-Semaphore konnte nicht erstellt werden."
        );


        checkVk(
            vkCreateFence(
                device,
                &fenceInfo,
                nullptr,
                &inFlightFence
            ),
            "Fence konnte nicht erstellt werden."
        );
    }


    // ========================================================
    // Render Loop
    // ========================================================

    void mainLoop()
    {
        while (
            !glfwWindowShouldClose(window)
        )
        {
            glfwPollEvents();

            drawFrame();
        }


        vkDeviceWaitIdle(
            device
        );
    }


    // ========================================================
    // Frame zeichnen
    // ========================================================

    void drawFrame()
    {
        vkWaitForFences(
            device,
            1,
            &inFlightFence,
            VK_TRUE,
            UINT64_MAX
        );


        vkResetFences(
            device,
            1,
            &inFlightFence
        );


        uint32_t imageIndex = 0;


        VkResult result =
            vkAcquireNextImageKHR(
                device,
                swapChain,
                UINT64_MAX,
                imageAvailableSemaphore,
                VK_NULL_HANDLE,
                &imageIndex
            );


        if (result != VK_SUCCESS &&
            result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error(
                "Swapchain Image konnte nicht erworben werden."
            );
        }


        VkSemaphore waitSemaphores[] =
        {
            imageAvailableSemaphore
        };


        VkPipelineStageFlags waitStages[] =
        {
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        };


        VkSemaphore signalSemaphores[] =
        {
            renderFinishedSemaphore
        };


        VkSubmitInfo submitInfo{};

        submitInfo.sType =
            VK_STRUCTURE_TYPE_SUBMIT_INFO;

        submitInfo.waitSemaphoreCount = 1;

        submitInfo.pWaitSemaphores =
            waitSemaphores;

        submitInfo.pWaitDstStageMask =
            waitStages;

        submitInfo.commandBufferCount = 1;

        submitInfo.pCommandBuffers =
            &commandBuffers[imageIndex];

        submitInfo.signalSemaphoreCount = 1;

        submitInfo.pSignalSemaphores =
            signalSemaphores;


        checkVk(
            vkQueueSubmit(
                graphicsQueue,
                1,
                &submitInfo,
                inFlightFence
            ),
            "Render Command konnte nicht eingereicht werden."
        );


        VkSwapchainKHR swapChains[] =
        {
            swapChain
        };


        VkPresentInfoKHR presentInfo{};

        presentInfo.sType =
            VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;

        presentInfo.pWaitSemaphores =
            signalSemaphores;

        presentInfo.swapchainCount = 1;

        presentInfo.pSwapchains =
            swapChains;

        presentInfo.pImageIndices =
            &imageIndex;


        result =
            vkQueuePresentKHR(
                presentQueue,
                &presentInfo
            );


        if (result != VK_SUCCESS &&
            result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error(
                "Frame konnte nicht dargestellt werden."
            );
        }
    }


    // ========================================================
    // Cleanup
    // ========================================================

    void cleanup()
    {
        if (device != VK_NULL_HANDLE)
        {
            vkDeviceWaitIdle(device);
        }


        if (imageAvailableSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(
                device,
                imageAvailableSemaphore,
                nullptr
            );
        }


        if (renderFinishedSemaphore != VK_NULL_HANDLE)
        {
            vkDestroySemaphore(
                device,
                renderFinishedSemaphore,
                nullptr
            );
        }


        if (inFlightFence != VK_NULL_HANDLE)
        {
            vkDestroyFence(
                device,
                inFlightFence,
                nullptr
            );
        }


        for (auto framebuffer :
             swapChainFramebuffers)
        {
            vkDestroyFramebuffer(
                device,
                framebuffer,
                nullptr
            );
        }


        if (commandPool != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(
                device,
                commandPool,
                nullptr
            );
        }


        if (renderPass != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(
                device,
                renderPass,
                nullptr
            );
        }


        for (auto imageView :
             swapChainImageViews)
        {
            vkDestroyImageView(
                device,
                imageView,
                nullptr
            );
        }


        if (swapChain != VK_NULL_HANDLE)
        {
            vkDestroySwapchainKHR(
                device,
                swapChain,
                nullptr
            );
        }


        if (device != VK_NULL_HANDLE)
        {
            vkDestroyDevice(
                device,
                nullptr
            );
        }


        if (surface != VK_NULL_HANDLE)
        {
            vkDestroySurfaceKHR(
                instance,
                surface,
                nullptr
            );
        }


        if (instance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(
                instance,
                nullptr
            );
        }


        if (window)
        {
            glfwDestroyWindow(
                window
            );
        }


        glfwTerminate();


        std::cout
            << "Bit Vulkan Renderer beendet.\n";
    }
};


// ============================================================
// Öffentliche Funktion für dein Projekt
// ============================================================

void renderVulkan()
{
    try
    {
        BitVulkanRenderer renderer;

        renderer.run();
    }
    catch (const std::exception& e)
    {
        std::cerr
            << "Vulkan Fehler: "
            << e.what()
            << '\n';
    }
}