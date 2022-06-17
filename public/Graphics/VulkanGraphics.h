//#pragma once
//#include "EasyGuiForward.h"
//#include "VkBootstrap.h"
//#include "VkException.h"
//#include "ImGuiImpl.h"
//#include "EasyApplication.h"
//
//
//
//struct VulkanGraphics
//{
//public:
//
//	~VulkanGraphics()
//	{
//		vkDestroyFence(_Device, _RenderFence, nullptr);
//		vkDestroySemaphore(_Device, _PresentSemaphore, nullptr);
//		vkDestroySemaphore(_Device, _RenderSemaphore, nullptr);
//		//destroy the main renderpass
//		vkDestroyRenderPass(_Device, _RenderPass, nullptr);
//		vkDestroyCommandPool(_Device, _CommandPool, nullptr);
//
//		for (int i = 0; i < _SwapchainImageViews.size(); i++) {
//			vkDestroyFramebuffer(_Device, _Framebuffers[i], nullptr);
//			vkDestroyImageView(_Device, _SwapchainImageViews[i], nullptr);
//		}
//
//		vkb::destroy_swapchain(_VkBootstrapSwapchain);
//		vkb::destroy_device(_VkBootstrapDevice);
//		if (_Surface)
//		{
//			vkDestroySurfaceKHR(_Instance, _Surface, nullptr);
//		}
//		vkb::destroy_instance(_VkBootstrapInstance);
//	}
//
//	bool CreateInstance()
//	{
//		vkb::InstanceBuilder instance_builder;
//		vkb::detail::Result<vkb::Instance> build_instance_result = instance_builder.set_app_name("EasyEngine")
//			.request_validation_layers(true)
//			.use_default_debug_messenger()
//			.build();
//		if (!build_instance_result) return false;
//		_VkBootstrapInstance = build_instance_result.value();
//		_Instance = _VkBootstrapInstance.instance;
//		return true;
//	}
//
//	bool CreateDevice(VkSurfaceKHR surface)
//	{
//		vkb::PhysicalDeviceSelector physical_device_selector{ _VkBootstrapInstance };
//		vkb::detail::Result<vkb::PhysicalDevice> build_physical_device_result = physical_device_selector
//			.set_surface(surface)
//			.select();
//		if (!build_physical_device_result) return false;
//		_VkBootstrapPhysicalDevice = build_physical_device_result.value();
//		_PhysicalDevice = _VkBootstrapPhysicalDevice.physical_device;
//
//		vkb::DeviceBuilder device_builder{ _VkBootstrapPhysicalDevice };
//		vkb::detail::Result<vkb::Device> build_device_result = device_builder.build();
//		if (!build_device_result) return false;
//		_VkBootstrapDevice = build_device_result.value();
//		_Device = _VkBootstrapDevice.device;
//
//		_GraphicsQueue = _VkBootstrapDevice.get_queue(vkb::QueueType::graphics).value();
//		_GraphicsQueueFamily = _VkBootstrapDevice.get_queue_index(vkb::QueueType::graphics).value();
//		_Surface = surface;
//		return true;
//	}
//
//	bool CreateSwapchain(uint32_t width, uint32_t height)
//	{
//		// Swapchain
//		vkb::SwapchainBuilder swapchain_builder{ _VkBootstrapPhysicalDevice, _VkBootstrapDevice, _Surface };
//
//		vkb::detail::Result<vkb::Swapchain> vkb_swapchain_result = swapchain_builder
//			.use_default_format_selection()
//			//use vsync present mode
//			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
//			.set_desired_extent(width, height)
//			.build()
//			.value();
//
//		_VkBootstrapSwapchain = vkb_swapchain_result.value();
//		_Swapchain = _VkBootstrapSwapchain.swapchain;
//		_SwapchainImages = _VkBootstrapSwapchain.get_images().value();
//		_SwapchainImageViews = _VkBootstrapSwapchain.get_image_views().value();
//		_SwapchainImageFormat = _VkBootstrapSwapchain.image_format;
//		_Width = width;
//		_Height = height;
//		return true;
//	}
//
//	void RecreateSwapchain(VkSurfaceKHR surface, uint32_t width, uint32_t height)
//	{
//		vkb::SwapchainBuilder swapchain_builder{ _VkBootstrapPhysicalDevice, _VkBootstrapDevice, surface };
//		vkb::detail::Result<vkb::Swapchain> vkb_swapchain_result = swapchain_builder.set_old_swapchain(_VkBootstrapSwapchain)
//			.use_default_format_selection()
//			//use vsync present mode
//			.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
//			.set_desired_extent(width, height)
//			.build();
//		if (!vkb_swapchain_result) {
//			_VkBootstrapSwapchain.swapchain = VK_NULL_HANDLE;
//		}
//		vkb::destroy_swapchain(_VkBootstrapSwapchain);
//
//		_VkBootstrapSwapchain = vkb_swapchain_result.value();
//		_Swapchain = _VkBootstrapSwapchain.swapchain;
//		_SwapchainImages = _VkBootstrapSwapchain.get_images().value();
//		_SwapchainImageViews = _VkBootstrapSwapchain.get_image_views().value();
//		_SwapchainImageFormat = _VkBootstrapSwapchain.image_format;
//		_Width = width;
//		_Height = height;
//		ImGui_RecreateSwapChain();
//	}
//
//
//	void CreateCommands()
//	{
//		//create a command pool for commands submitted to the graphics queue.
//		VkCommandPoolCreateInfo command_pool_create_info = {};
//		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//		command_pool_create_info.pNext = nullptr;
//
//		//the command pool will be one that can submit graphics commands
//		command_pool_create_info.queueFamilyIndex = _GraphicsQueueFamily;
//		//we also want the pool to allow for resetting of individual command buffers
//		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//
//		VK_ASSERT_SUCCESSED(vkCreateCommandPool(_Device, &command_pool_create_info, nullptr, &_CommandPool));
//
//		//allocate the default command buffer that we will use for rendering
//		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
//		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//		command_buffer_allocate_info.pNext = nullptr;
//
//		//commands will be made from our _commandPool
//		command_buffer_allocate_info.commandPool = _CommandPool;
//		//we will allocate 1 command buffer
//		command_buffer_allocate_info.commandBufferCount = 1;
//		// command level is Primary
//		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//
//		VK_ASSERT_SUCCESSED(vkAllocateCommandBuffers(_Device, &command_buffer_allocate_info, &_MainCommandBuffer));
//
//	}
//
//	void CreateRenderPass()
//	{
//		// the renderpass will use this color attachment.
//		VkAttachmentDescription color_attachment = {};
//		//the attachment will have the format needed by the swapchain
//		color_attachment.format = _SwapchainImageFormat;
//		//1 sample, we won't be doing MSAA
//		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
//		// we Clear when this attachment is loaded
//		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//		// we keep the attachment stored when the renderpass ends
//		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		//we don't care about stencil
//		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
//		//we don't know or care about the starting layout of the attachment
//		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//
//		//after the renderpass ends, the image has to be on a layout ready for display
//		color_attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		VkAttachmentReference color_attachment_ref = {};
//		//attachment number will index into the pAttachments array in the parent renderpass itself
//		color_attachment_ref.attachment = 0;
//		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//		//we are going to create 1 subpass, which is the minimum you can do
//		VkSubpassDescription subpass = {};
//		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//		subpass.colorAttachmentCount = 1;
//		subpass.pColorAttachments = &color_attachment_ref;
//
//		VkRenderPassCreateInfo render_pass_info = {};
//		render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//
//		//connect the color attachment to the info
//		render_pass_info.attachmentCount = 1;
//		render_pass_info.pAttachments = &color_attachment;
//		//connect the subpass to the info
//		render_pass_info.subpassCount = 1;
//		render_pass_info.pSubpasses = &subpass;
//
//
//		VK_ASSERT_SUCCESSED(vkCreateRenderPass(_Device, &render_pass_info, nullptr, &_RenderPass));
//	}
//
//	void CreateFrameBuffers()
//	{
//		//create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
//		VkFramebufferCreateInfo fb_info = {};
//		fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//		fb_info.pNext = nullptr;
//
//		fb_info.renderPass = _RenderPass;
//		fb_info.attachmentCount = 1;
//		fb_info.width = _Width;
//		fb_info.height = _Height;
//		fb_info.layers = 1;
//
//		//grab how many images we have in the swapchain
//		const uint32_t swapchain_imagecount = _SwapchainImages.size();
//		_Framebuffers = std::vector<VkFramebuffer>(swapchain_imagecount);
//
//		//create framebuffers for each of the swapchain image views
//		for (int i = 0; i < swapchain_imagecount; i++) {
//
//			fb_info.pAttachments = &_SwapchainImageViews[i];
//			VK_ASSERT_SUCCESSED(vkCreateFramebuffer(_Device, &fb_info, nullptr, &_Framebuffers[i]));
//		}
//	}
//
//	void CreateSyncObject()
//	{
//		//create synchronization structures
//
//		VkFenceCreateInfo fence_create_info = {};
//		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//		fence_create_info.pNext = nullptr;
//
//		//we want to create the fence with the Create Signaled flag, so we can wait on it before using it on a GPU command (for the first frame)
//		fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//		VK_ASSERT_SUCCESSED(vkCreateFence(_Device, &fence_create_info, nullptr, &_RenderFence));
//
//		//for the semaphores we don't need any flags
//		VkSemaphoreCreateInfo semaphore_create_info = {};
//		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//		semaphore_create_info.pNext = nullptr;
//		semaphore_create_info.flags = 0;
//
//		VK_ASSERT_SUCCESSED(vkCreateSemaphore(_Device, &semaphore_create_info, nullptr, &_PresentSemaphore));
//		VK_ASSERT_SUCCESSED(vkCreateSemaphore(_Device, &semaphore_create_info, nullptr, &_RenderSemaphore));
//	}
//
//	void Draw()
//	{
//		VK_ASSERT_SUCCESSED(vkWaitForFences(_Device, 1, &_RenderFence, true, 1000000000));
//		VK_ASSERT_SUCCESSED(vkResetFences(_Device, 1, &_RenderFence));
//		uint32_t swapchain_image_index;
//		VK_ASSERT_SUCCESSED(vkAcquireNextImageKHR(_Device, _Swapchain, 1000000000, _PresentSemaphore, nullptr, &swapchain_image_index));
//
//		VK_ASSERT_SUCCESSED(vkResetCommandBuffer(_MainCommandBuffer, 0));
//
//		//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
//		VkCommandBufferBeginInfo cmdBeginInfo = {};
//		cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//		cmdBeginInfo.pNext = nullptr;
//
//		cmdBeginInfo.pInheritanceInfo = nullptr;
//		cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//		VK_ASSERT_SUCCESSED(vkBeginCommandBuffer(_MainCommandBuffer, &cmdBeginInfo));
//		VkClearValue clearValue;
//		float flash = abs(sin(_FrameNumber / 120.f));
//		clearValue.color = { { 0.0f, 0.0f, flash, 1.0f } };
//
//		//start the main renderpass.
//		//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
//		VkRenderPassBeginInfo rpInfo = {};
//		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
//		rpInfo.pNext = nullptr;
//
//		rpInfo.renderPass = _RenderPass;
//		rpInfo.renderArea.offset.x = 0;
//		rpInfo.renderArea.offset.y = 0;
//		rpInfo.renderArea.extent = VkExtent2D(_Width, _Height);
//		rpInfo.framebuffer = _Framebuffers[swapchain_image_index];
//
//		//connect clear values
//		rpInfo.clearValueCount = 1;
//		rpInfo.pClearValues = &clearValue;
//
//		vkCmdBeginRenderPass(_MainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
//		//finalize the render pass
//		vkCmdEndRenderPass(_MainCommandBuffer);
//		//finalize the command buffer (we can no longer add commands, but it can now be executed)
//		VK_ASSERT_SUCCESSED(vkEndCommandBuffer(_MainCommandBuffer));
//
//
//		VkSubmitInfo submit = {};
//		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//		submit.pNext = nullptr;
//
//		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//
//		submit.pWaitDstStageMask = &waitStage;
//
//		submit.waitSemaphoreCount = 1;
//		submit.pWaitSemaphores = &_PresentSemaphore;
//
//		submit.signalSemaphoreCount = 1;
//		submit.pSignalSemaphores = &_RenderSemaphore;
//
//		submit.commandBufferCount = 1;
//		submit.pCommandBuffers = &_MainCommandBuffer;
//
//		//submit command buffer to the queue and execute it.
//		// _renderFence will now block until the graphic commands finish execution
//		VK_ASSERT_SUCCESSED(vkQueueSubmit(_GraphicsQueue, 1, &submit, _RenderFence));
//
//		// this will put the image we just rendered into the visible window.
//		// we want to wait on the _renderSemaphore for that,
//		// as it's necessary that drawing commands have finished before the image is displayed to the user
//		VkPresentInfoKHR presentInfo = {};
//		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//		presentInfo.pNext = nullptr;
//		presentInfo.pSwapchains = &_Swapchain;
//		presentInfo.swapchainCount = 1;
//		presentInfo.pWaitSemaphores = &_RenderSemaphore;
//		presentInfo.waitSemaphoreCount = 1;
//		presentInfo.pImageIndices = &swapchain_image_index;
//		VK_ASSERT_SUCCESSED(vkQueuePresentKHR(_GraphicsQueue, &presentInfo));
//
//		//increase the number of frames drawn
//		_FrameNumber++;
//	}
//
//
//	void ImGui_CreateDescriptorPool()
//	{
//		VkDescriptorPoolSize pool_sizes[] =
//		{
//			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
//			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
//			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
//		};
//		VkDescriptorPoolCreateInfo pool_info = {};
//		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
//		pool_info.maxSets = 1000 * IM_ARRAYSIZE(pool_sizes);
//		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
//		pool_info.pPoolSizes = pool_sizes;
//		VK_ASSERT_SUCCESSED(vkCreateDescriptorPool(_Device, &pool_info, nullptr, &_ImGuiDescriptorPool));
//	}
//	void ImGui_CreateRenderPass()
//	{
//		VkAttachmentDescription attachment = {};
//		attachment.format = _SwapchainImageFormat;
//		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
//		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
//		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//		VkAttachmentReference color_attachment = {};
//		color_attachment.attachment = 0;
//		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//		VkSubpassDescription subpass = {};
//		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//		subpass.colorAttachmentCount = 1;
//		subpass.pColorAttachments = &color_attachment;
//
//		VkSubpassDependency dependency = {};
//		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//		dependency.dstSubpass = 0;
//		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
//		dependency.srcAccessMask = 0;  // or VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
//
//		VkRenderPassCreateInfo info = {};
//		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//		info.attachmentCount = 1;
//		info.pAttachments = &attachment;
//		info.subpassCount = 1;
//		info.pSubpasses = &subpass;
//		info.dependencyCount = 1;
//		info.pDependencies = &dependency;
//		VK_ASSERT_SUCCESSED(vkCreateRenderPass(_Device, &info, nullptr, &_ImGuiRenderPass));
//	}
//	void ImGui_RecreateSwapChain()
//	{
//		ImGui_ImplVulkan_SetMinImageCount(2);
//	}
//	void ImGui_Init()
//	{
//		ImGui_CreateDescriptorPool();
//		ImGui_CreateRenderPass();
//		// Setup Dear ImGui context
//		IMGUI_CHECKVERSION();
//		ImGui::CreateContext();
//		ImGuiIO& io = ImGui::GetIO(); (void)io;
//		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
//		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//
//		// Setup Dear ImGui style
//		ImGui::StyleColorsDark();
//		//ImGui::StyleColorsClassic();
//
//		// Setup Platform/Renderer bindings
//		ImGui_ImplGlfw_InitForVulkan(GP_Application->GLFWwindowPtr(), true);
//		ImGui_ImplVulkan_InitInfo init_info = {};
//		init_info.Instance = _Instance;
//		init_info.PhysicalDevice = _PhysicalDevice;
//		init_info.Device = _Device;
//		init_info.QueueFamily = _GraphicsQueueFamily;
//		init_info.Queue = _GraphicsQueue;
//		init_info.PipelineCache = VK_NULL_HANDLE;
//		init_info.DescriptorPool = _ImGuiDescriptorPool;
//		init_info.Allocator = nullptr;
//		init_info.MinImageCount = 2;
//		init_info.ImageCount = _VkBootstrapSwapchain.image_count;
//		init_info.CheckVkResultFn = nullptr;
//		ImGui_ImplVulkan_Init(&init_info, _ImGuiRenderPass);
//
//		// Upload Fonts
//		{
//			//// Use any command queue
//			//VkCommandPool command_pool = wd->Frames[wd->FrameIndex].CommandPool;
//			//VkCommandBuffer command_buffer = wd->Frames[wd->FrameIndex].CommandBuffer;
//			//VK_ASSERT_SUCCESSED(vkResetCommandPool(_Device, command_pool, 0));
//			//VkCommandBufferBeginInfo begin_info = {};
//			//begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//			//begin_info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//			//VK_ASSERT_SUCCESSED(vkBeginCommandBuffer(command_buffer, &begin_info));
//			//ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
//			//VkSubmitInfo end_info = {};
//			//end_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//			//end_info.commandBufferCount = 1;
//			//end_info.pCommandBuffers = &command_buffer;
//			//VK_ASSERT_SUCCESSED(vkEndCommandBuffer(command_buffer));
//			//VK_ASSERT_SUCCESSED(vkQueueSubmit(_GraphicsQueue, 1, &end_info, VK_NULL_HANDLE));
//			//VK_ASSERT_SUCCESSED(vkDeviceWaitIdle(_Device));
//			//ImGui_ImplVulkan_DestroyFontUploadObjects();
//		}
//	}
//
//	void ImGui_Draw()
//	{
//		ImGui_ImplVulkan_NewFrame();
//		ImGui_ImplGlfw_NewFrame();
//		ImGui::NewFrame();
//		ImGui::ShowDemoWindow();
//		ImGui::Render();
//	}
//
//	void ImGui_Cleanup()
//	{
//		ImGui_ImplVulkan_Shutdown();
//		ImGui_ImplGlfw_Shutdown();
//		ImGui::DestroyContext();
//	}
//
//	VkDescriptorPool _ImGuiDescriptorPool;
//	VkRenderPass _ImGuiRenderPass;
//
//	VkInstance _Instance = VK_NULL_HANDLE;
//	VkDebugUtilsMessengerEXT _DebugUtilsMessenger = VK_NULL_HANDLE;
//	VkPhysicalDevice _PhysicalDevice = VK_NULL_HANDLE;
//	VkDevice _Device = VK_NULL_HANDLE;
//	VkSurfaceKHR _Surface = VK_NULL_HANDLE;
//
//	VkSwapchainKHR _Swapchain = VK_NULL_HANDLE;
//	VkFormat _SwapchainImageFormat;
//	std::vector<VkImage> _SwapchainImages;
//	std::vector<VkImageView> _SwapchainImageViews;
//
//	//
//	VkQueue _GraphicsQueue = VK_NULL_HANDLE;
//	uint32_t _GraphicsQueueFamily;
//	VkCommandPool _CommandPool = VK_NULL_HANDLE;
//	VkCommandBuffer _MainCommandBuffer = VK_NULL_HANDLE;
//
//	VkRenderPass _RenderPass = VK_NULL_HANDLE;
//	std::vector<VkFramebuffer> _Framebuffers;
//
//	VkSemaphore _PresentSemaphore = VK_NULL_HANDLE;
//	VkSemaphore	_RenderSemaphore = VK_NULL_HANDLE;
//	VkFence _RenderFence = VK_NULL_HANDLE;
//
//	int64_t _FrameNumber = 0;
//
//	uint32_t _Width;
//	uint32_t _Height;
//
//private:
//	vkb::Instance _VkBootstrapInstance;
//	vkb::PhysicalDevice _VkBootstrapPhysicalDevice;
//	vkb::Device _VkBootstrapDevice;
//	vkb::Swapchain _VkBootstrapSwapchain;
//};
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
//
