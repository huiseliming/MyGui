#include "VulkanGuiApplication.h"
#include "../third_party/imgui/backends/imgui_impl_vulkan.h"
#include "../third_party/imgui/backends/imgui_impl_glfw.h"
#include <Poco/File.h>

#ifdef _WIN32
#include <MinHook.h>
#endif // _WIN32

static VulkanGuiApplication* S_VulkanGuiApplication;

VkSurfaceFormatKHR ImGui_ImplVulkanH_SelectSurfaceFormat_HookFunction(VkPhysicalDevice physical_device, VkSurfaceKHR surface, const VkFormat* request_formats, int request_formats_count, VkColorSpaceKHR request_color_space)
{
	return S_VulkanGuiApplication->SurfaceFormatRef().operator VkSurfaceFormatKHR &();
}

static void HookImGuiFunction(VulkanGuiApplication* vulkan_gui_application)
{
	S_VulkanGuiApplication = vulkan_gui_application;
#ifdef _WIN32
	MH_CreateHook(&ImGui_ImplVulkanH_SelectSurfaceFormat, &ImGui_ImplVulkanH_SelectSurfaceFormat_HookFunction, nullptr);
	MH_EnableHook(&ImGui_ImplVulkanH_SelectSurfaceFormat);
#endif // _WIN32
}

VulkanGuiApplication::VulkanGuiApplication()
{
	addSubsystem(_TaskQueueSubsystem = new TaskQueueSubsystem());
}

static void UnhookImGuiFunction()
{
#ifdef _WIN32
	MH_DisableHook(&ImGui_ImplVulkanH_SelectSurfaceFormat);
#endif // _WIN32
}

void VulkanGuiApplication::initialize(Poco::Util::Application& self)
{
	MH_Initialize();
	HookImGuiFunction(this);
	glfwInit();
	Super::initialize(self);
}

void VulkanGuiApplication::reinitialize(Poco::Util::Application& self)
{
	Super::reinitialize(self);
}

void VulkanGuiApplication::uninitialize()
{
	Super::uninitialize();
	glfwTerminate();
	UnhookImGuiFunction();
	MH_Uninitialize();
}

void VulkanGuiApplication::defineOptions(Poco::Util::OptionSet& options)
{
	Super::defineOptions(options);
	options.addOption(
		Poco::Util::Option("help", "h", "display help information on command line arguments")
		.required(false)
		.repeatable(false)
		.callback(Poco::Util::OptionCallback<VulkanGuiApplication>(this, &VulkanGuiApplication::HandleHelp)));
}

void VulkanGuiApplication::handleOption(const std::string& name, const std::string& value)
{
	Super::handleOption(name, value);
}

bool VulkanGuiApplication::Startup(int32_t ArgC, const char* ArgV[]) {
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	_GLFWwindow = glfwCreateWindow(_Width, _Height, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(_GLFWwindow, this);
	glfwSetFramebufferSizeCallback(_GLFWwindow, &FramebufferResizeCallback);
	CreateInstance();
	CreateSurface();
	CreateDevice();
	CreateSwapchain();
	CreateRenderPass();
	CreateFramebuffers();
	CreateCommandPoolAndAllocateCommandBuffer();
	CreateSyncObjects();
	ImGui_Startup();
	return true;
}

void VulkanGuiApplication::MainLoop() {
	glfwPollEvents();
	int width = 0, height = 0;
	glfwGetFramebufferSize(_GLFWwindow, &width, &height);
	if (width != 0 || height != 0)
	{
		ImGui_RenderFrame();
		DrawFrame();
	}
	_TaskQueueSubsystem->ProcessMainThreadTasks();
}

void VulkanGuiApplication::Cleanup() {
	if (_Instance)
	{
		if (_Device)
		{
			_Device.waitIdle();
			ImGui_Cleanup();
			DestroySyncObjects();
			if (_CommandPool)
			{
				_Device.freeCommandBuffers(_CommandPool, _CommandBuffers);
				_Device.destroyCommandPool(_CommandPool);
			}
			CleanupSwapChain();
			_Device.destroy();
		}
		if (_Surface)
		{
			_Instance.destroySurfaceKHR(_Surface);
		}
		_Instance.destroy();
	}
	glfwDestroyWindow(_GLFWwindow);
}

bool VulkanGuiApplication::IsRequestExit()
{
	return glfwWindowShouldClose(_GLFWwindow);
}

void VulkanGuiApplication::SetRequestExit()
{
	glfwSetWindowShouldClose(_GLFWwindow, GLFW_TRUE);
}

void VulkanGuiApplication::ImGui_CreateDescriptorPool()
{
	vk::DescriptorPoolSize descriptor_pool_sizes[] =
	{
		{ vk::DescriptorType::eSampler , 1000 },
		{ vk::DescriptorType::eCombinedImageSampler, 1000 },
		{ vk::DescriptorType::eSampledImage, 1000 },
		{ vk::DescriptorType::eStorageImage, 1000 },
		{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
		{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
		{ vk::DescriptorType::eUniformBuffer, 1000 },
		{ vk::DescriptorType::eStorageBuffer, 1000 },
		{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
		{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
		{ vk::DescriptorType::eInputAttachment, 1000 },
	};
	vk::DescriptorPoolCreateInfo descriptor_pool_create_info;
	descriptor_pool_create_info.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
	descriptor_pool_create_info.maxSets = 1000 * std::size(descriptor_pool_sizes);
	descriptor_pool_create_info.poolSizeCount = std::size(descriptor_pool_sizes);
	descriptor_pool_create_info.pPoolSizes = descriptor_pool_sizes;
	_ImGuiDescriptorPool = _Device.createDescriptorPool(descriptor_pool_create_info);
}

void VulkanGuiApplication::ImGui_CreateCommandPoolAndAllocateCommandBuffer()
{
	vk::CommandPoolCreateInfo command_pool_create_info;
	command_pool_create_info.queueFamilyIndex = _GraphicsQueueFamily;
	command_pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	_ImGuiCommandPool = _Device.createCommandPool(command_pool_create_info);

	vk::CommandBufferAllocateInfo command_buffer_allocate_info;
	command_buffer_allocate_info.commandPool = _ImGuiCommandPool;
	command_buffer_allocate_info.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_allocate_info.commandBufferCount = MaxFramesInFlight;
	_ImGuiCommandBuffers = _Device.allocateCommandBuffers(command_buffer_allocate_info);
}

void VulkanGuiApplication::ImGui_CreateRenderPass()
{
	vk::AttachmentDescription attachment = {};
	attachment.format = _SurfaceFormat.format;
	attachment.samples = vk::SampleCountFlagBits::e1;
	attachment.loadOp = vk::AttachmentLoadOp::eLoad;
	attachment.storeOp = vk::AttachmentStoreOp::eStore;
	attachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	attachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
	attachment.initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
	attachment.finalLayout = vk::ImageLayout::ePresentSrcKHR;

	vk::AttachmentReference color_attachment = {};
	color_attachment.attachment = 0;
	color_attachment.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass = {};
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment;

	vk::SubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	dependency.srcAccessMask = vk::AccessFlagBits(0);
	dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo render_pass_create_info = {};
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	render_pass_create_info.dependencyCount = 1;
	render_pass_create_info.pDependencies = &dependency;
	_ImGuiRenderPass = _Device.createRenderPass(render_pass_create_info);
}

void VulkanGuiApplication::ImGui_CreateFramebuffer()
{
	VkFramebufferCreateInfo framebuffer_create_info = {};
	framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebuffer_create_info.renderPass = _ImGuiRenderPass;
	framebuffer_create_info.width = _SurfaceExtent.width;
	framebuffer_create_info.height = _SurfaceExtent.height;
	framebuffer_create_info.layers = 1;
	_ImGuiFramebuffers.resize(_SwapchainImageViews.size());
	for (uint32_t i = 0; i < _SwapchainImageViews.size(); i++)
	{
		VkImageView attachments[] = { _SwapchainImageViews[i] };
		framebuffer_create_info.attachmentCount = std::size(attachments);
		framebuffer_create_info.pAttachments = attachments;
		_ImGuiFramebuffers[i] = _Device.createFramebuffer(framebuffer_create_info);
	}
}

void VulkanGuiApplication::ImGui_CleanupSwapChain()
{
	for (size_t i = 0; i < _ImGuiFramebuffers.size(); i++)
	{
		_Device.destroyFramebuffer(_ImGuiFramebuffers[i]);
	}
	_ImGuiFramebuffers.clear();
	_Device.destroyRenderPass(_ImGuiRenderPass);
}

void VulkanGuiApplication::ImGui_RecordCommand(vk::CommandBuffer& command_buffer, uint32_t image_index)
{
	vk::ClearValue clear_value[] = {
		vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
	};
	vk::CommandBufferBeginInfo command_buffer_begin_info;
	command_buffer.begin(command_buffer_begin_info);
	vk::RenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.renderPass = _ImGuiRenderPass;
	render_pass_begin_info.framebuffer = _ImGuiFramebuffers[image_index];
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = _SurfaceExtent;
	render_pass_begin_info.clearValueCount = 1;
	render_pass_begin_info.pClearValues = clear_value;
	command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), command_buffer);
	command_buffer.endRenderPass();
	command_buffer.end();
}

void VulkanGuiApplication::ImGui_Startup()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#if _WIN32
	std::string system_msyh_font_path = "C:\\Windows\\Fonts\\msyh.ttc";
	if (Poco::File(system_msyh_font_path).exists())
	{
		io.Fonts->AddFontFromFileTTF(system_msyh_font_path.c_str(), 15, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	}
#endif

	ImGui::StyleColorsDark();
	ImGui_CreateDescriptorPool();
	ImGui_CreateRenderPass();
	ImGui_CreateFramebuffer();
	ImGui_CreateCommandPoolAndAllocateCommandBuffer();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForVulkan(_GLFWwindow, true);
	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = _Instance;
	init_info.PhysicalDevice = _PhysicalDevice;
	init_info.Device = _Device;
	init_info.QueueFamily = _GraphicsQueueFamily;
	init_info.Queue = _GraphicsQueue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = _ImGuiDescriptorPool;
	init_info.Allocator = nullptr;
	init_info.MinImageCount = _MinImageCount;
	init_info.ImageCount = _ImageCount;
	init_info.CheckVkResultFn = nullptr;
	ImGui_ImplVulkan_Init(&init_info, _ImGuiRenderPass);

	ExecuteGraphicsCommandAndWaitComplete([](vk::CommandBuffer& command_buffer) {
		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);
		});

}

void VulkanGuiApplication::ImGui_RenderFrame()
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui_Draw();
	ImGui::Render();

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void VulkanGuiApplication::ImGui_Cleanup()
{
	if (_ImGuiCommandPool)
	{
		_Device.freeCommandBuffers(_ImGuiCommandPool, _ImGuiCommandBuffers);
		_Device.destroyCommandPool(_ImGuiCommandPool);
	}
	_Device.destroyDescriptorPool(_ImGuiDescriptorPool);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void VulkanGuiApplication::ImGui_Draw()
{
	for (auto& Drawable : _Drawables)
	{
		Drawable->Draw();
	}
}

bool VulkanGuiApplication::CheckValidationLayerSupport() {
	auto available_layer_properties = vk::enumerateInstanceLayerProperties();
	for (const char* layerName : _ValidationLayers) {
		bool layerFound = false;

		for (const auto& layer_properties : available_layer_properties) {
			if (strcmp(layerName, layer_properties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			return false;
		}
	}

	return true;
}

bool VulkanGuiApplication::FindQueueFamily(std::vector<vk::QueueFamilyProperties, std::allocator<vk::QueueFamilyProperties>> queue_family_properties)
{
	_GraphicsQueueFamily = UINT32_MAX;
	_PresentQueueFamily = UINT32_MAX;
	std::vector<uint32_t> graphics_queue_family_indices;
	std::set<uint32_t> present_queue_family_indices;
	for (size_t i = 0; i < queue_family_properties.size(); i++)
	{
		if (_GraphicsQueueFamily == UINT32_MAX)
		{
			if (queue_family_properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
			{
				graphics_queue_family_indices.push_back(i);
			}
		}
		if (_PhysicalDevice.getSurfaceSupportKHR(i, _Surface) == VK_TRUE)
		{
			present_queue_family_indices.insert(i);
		}
	}
	if (graphics_queue_family_indices.empty()) return false;
	if (present_queue_family_indices.empty()) return false;
	for (size_t i = 0; i < graphics_queue_family_indices.size(); i++)
	{
		if (present_queue_family_indices.contains(i))
		{
			_GraphicsQueueFamily = i;
			_PresentQueueFamily = i;
			break;
		}
	}

	if (_GraphicsQueueFamily == UINT32_MAX || _PresentQueueFamily == UINT32_MAX)
	{
		_GraphicsQueueFamily = graphics_queue_family_indices[0];
		_PresentQueueFamily = *present_queue_family_indices.begin();
	}
	if (_PresentQueueFamily == UINT32_MAX) return false;
	return true;
}

void VulkanGuiApplication::ExecuteGraphicsCommandAndWaitComplete(std::function<void(vk::CommandBuffer&)> record_command_callback)
{
	vk::CommandBufferAllocateInfo command_buffer_allocate_info;
	command_buffer_allocate_info.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_allocate_info.commandPool = _CommandPool;
	command_buffer_allocate_info.commandBufferCount = 1;
	auto command_buffers = _Device.allocateCommandBuffers(command_buffer_allocate_info);
	vk::CommandBufferBeginInfo command_buffer_begin_info;
	command_buffer_begin_info.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;
	command_buffers[0].begin(command_buffer_begin_info);
	record_command_callback(command_buffers[0]);
	command_buffers[0].end();
	vk::SubmitInfo submit_info;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = command_buffers.data();
	_GraphicsQueue.submit(submit_info);
	_GraphicsQueue.waitIdle();
	_Device.freeCommandBuffers(_CommandPool, command_buffers);
}

void VulkanGuiApplication::CreateInstance() {
	if (_EnableValidationLayers && !CheckValidationLayerSupport()) [[unlikely]] throw std::runtime_error("CheckValidationLayerSupport FAILED");
	// create vulkan instance 
	vk::InstanceCreateInfo instance_create_info;

	vk::ApplicationInfo application_info;
	application_info.pApplicationName = "VulkanGuiApplication";
	application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.pEngineName = "No Engine";
	application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	application_info.apiVersion = VK_API_VERSION_1_3;

	instance_create_info.setPApplicationInfo(&application_info);

	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	instance_create_info.enabledExtensionCount = glfw_extension_count;
	instance_create_info.ppEnabledExtensionNames = glfw_extensions;

	instance_create_info.enabledLayerCount = 0;

	if (_EnableValidationLayers) {
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(_ValidationLayers.size());
		instance_create_info.ppEnabledLayerNames = _ValidationLayers.data();
	}
	else {
		instance_create_info.enabledLayerCount = 0;
	}

	_Instance = vk::createInstance(instance_create_info);

	//auto instance_extension_properties = vk::enumerateInstanceExtensionProperties();
	//for (const auto& instance_extension_property : instance_extension_properties) {
	//	spdlog::info("{:s}", instance_extension_property.extensionName);
	//}
}

void VulkanGuiApplication::CreateSurface()
{
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(_Instance, _GLFWwindow, nullptr, &surface) != VK_SUCCESS)
		[[unlikely]] throw std::runtime_error("glfwCreateWindowSurface FAILED");
	_Surface = surface;
}

void VulkanGuiApplication::CreateDevice()
{
	auto physical_devices = _Instance.enumeratePhysicalDevices();
	for (auto physical_device : physical_devices)
	{
		auto physical_device_properties = physical_device.getProperties();
		if (physical_device_properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			_PhysicalDevice = physical_device;
		}
	}

	if (!_PhysicalDevice && !physical_devices.empty())
	{
		_PhysicalDevice = physical_devices[0];
	}

	if (!_PhysicalDevice) [[unlikely]] throw std::runtime_error("NOT FIND AVAILABLE PHYSICAL DEVICE");
	auto queue_family_properties = _PhysicalDevice.getQueueFamilyProperties();
	if (!FindQueueFamily(queue_family_properties)) [[unlikely]] throw std::runtime_error("NOT FIND AVAILABLE QUEUE FAMILY");

	float queue_priority = 1.0f;
	std::vector<vk::DeviceQueueCreateInfo> device_queue_create_info;
	device_queue_create_info.emplace_back();
	device_queue_create_info[0].queueFamilyIndex = _GraphicsQueueFamily;
	device_queue_create_info[0].queueCount = 1;
	device_queue_create_info[0].pQueuePriorities = &queue_priority;

	if (_PresentQueueFamily != _GraphicsQueueFamily)
	{
		device_queue_create_info.emplace_back();
		device_queue_create_info[0].queueFamilyIndex = _PresentQueueFamily;
		device_queue_create_info[0].queueCount = 1;
		device_queue_create_info[0].pQueuePriorities = &queue_priority;
	}

	vk::DeviceCreateInfo device_create_info;
	device_create_info.pQueueCreateInfos = device_queue_create_info.data();
	device_create_info.queueCreateInfoCount = device_queue_create_info.size();
	device_create_info.pEnabledFeatures = &_PhysicalDeviceFeatures;

	if (_EnableValidationLayers)
	{
		device_create_info.enabledLayerCount = _ValidationLayers.size();
		device_create_info.ppEnabledLayerNames = _ValidationLayers.data();
	}
	else
	{
		device_create_info.enabledLayerCount = 0;
	}
	std::vector<const char*> device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	device_create_info.enabledExtensionCount = device_extensions.size();
	device_create_info.ppEnabledExtensionNames = device_extensions.data();

	_Device = _PhysicalDevice.createDevice(device_create_info);

	_GraphicsQueue = _Device.getQueue(_GraphicsQueueFamily, 0);
	_PresentQueue = _Device.getQueue(_PresentQueueFamily, 0);
}

void VulkanGuiApplication::CreateSwapchain()
{
	auto surface_formats = _PhysicalDevice.getSurfaceFormatsKHR(_Surface);
	if (surface_formats.empty()) [[unlikely]] throw std::runtime_error("NOT FIND AVAILABLE SURFACE FORMAT");
	_SurfaceFormat = surface_formats[0];
	for (const auto& surface_format : surface_formats)
	{
		if (surface_format.format == vk::Format::eB8G8R8A8Srgb && surface_format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			_SurfaceFormat = surface_format;
		}
	}

	auto surface_present_modes = _PhysicalDevice.getSurfacePresentModesKHR(_Surface);
	_SurfacePresentMode = vk::PresentModeKHR::eFifo;
	for (const auto& surface_present_mode : surface_present_modes)
	{
		if (surface_present_mode == vk::PresentModeKHR::eMailbox)
		{
			_SurfacePresentMode = surface_present_mode;
		}
	}
	auto surface_capabilities = _PhysicalDevice.getSurfaceCapabilitiesKHR(_Surface);
	if (surface_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		_SurfaceExtent = surface_capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(_GLFWwindow, &width, &height);
		VkExtent2D actual_extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		actual_extent.width = std::clamp(actual_extent.width, surface_capabilities.minImageExtent.width, surface_capabilities.maxImageExtent.width);
		actual_extent.height = std::clamp(actual_extent.height, surface_capabilities.minImageExtent.height, surface_capabilities.maxImageExtent.height);
		_SurfaceExtent = actual_extent;
	}
	_MinImageCount = surface_capabilities.minImageCount;
	_ImageCount = _MinImageCount + 1;
	if (surface_capabilities.maxImageCount > 0 && _ImageCount > surface_capabilities.maxImageCount) {
		_ImageCount = surface_capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR swapchain_craete_info;
	swapchain_craete_info.surface = _Surface;
	swapchain_craete_info.minImageCount = _MinImageCount;
	swapchain_craete_info.imageFormat = _SurfaceFormat.format;
	swapchain_craete_info.imageColorSpace = _SurfaceFormat.colorSpace;
	swapchain_craete_info.imageExtent = _SurfaceExtent;
	swapchain_craete_info.imageArrayLayers = 1;
	swapchain_craete_info.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
	if (_GraphicsQueueFamily != _GraphicsQueueFamily)
	{
		uint32_t queue_family_indices[] = { _GraphicsQueueFamily, _PresentQueueFamily };
		swapchain_craete_info.imageSharingMode = vk::SharingMode::eConcurrent;
		swapchain_craete_info.queueFamilyIndexCount = 2;
		swapchain_craete_info.pQueueFamilyIndices = queue_family_indices;
	}
	else
	{
		swapchain_craete_info.imageSharingMode = vk::SharingMode::eExclusive;
		swapchain_craete_info.queueFamilyIndexCount = 0;
		swapchain_craete_info.pQueueFamilyIndices = nullptr;
	}
	swapchain_craete_info.preTransform = surface_capabilities.currentTransform;
	swapchain_craete_info.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
	swapchain_craete_info.presentMode = _SurfacePresentMode;
	swapchain_craete_info.clipped = VK_TRUE;
	swapchain_craete_info.oldSwapchain = VK_NULL_HANDLE;
	_Swapchain = _Device.createSwapchainKHR(swapchain_craete_info);
	_SwapchainImages = _Device.getSwapchainImagesKHR(_Swapchain);
	_SwapchainImageViews.resize(_SwapchainImages.size());
	for (size_t i = 0; i < _SwapchainImages.size(); i++)
	{
		vk::ImageViewCreateInfo image_view_create_info;
		image_view_create_info.image = _SwapchainImages[i];
		image_view_create_info.viewType = vk::ImageViewType::e2D;
		image_view_create_info.format = _SurfaceFormat.format;
		image_view_create_info.components.r = vk::ComponentSwizzle::eIdentity;
		image_view_create_info.components.g = vk::ComponentSwizzle::eIdentity;
		image_view_create_info.components.b = vk::ComponentSwizzle::eIdentity;
		image_view_create_info.components.a = vk::ComponentSwizzle::eIdentity;
		image_view_create_info.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;
		image_view_create_info.subresourceRange.baseArrayLayer = 0;
		image_view_create_info.subresourceRange.layerCount = 1;
		_SwapchainImageViews[i] = _Device.createImageView(image_view_create_info);
	}
}

void VulkanGuiApplication::CreateSyncObjects()
{
	vk::SemaphoreCreateInfo semaphore_create_info;
	vk::FenceCreateInfo fence_create_info;
	fence_create_info.flags = vk::FenceCreateFlagBits::eSignaled;
	for (size_t i = 0; i < MaxFramesInFlight; i++)
	{
		_ImageAvailableSemaphores.push_back(_Device.createSemaphore(semaphore_create_info));
		_RenderFinishedSemaphores.push_back(_Device.createSemaphore(semaphore_create_info));
		_InFlightFences.push_back(_Device.createFence(fence_create_info));
	}
}

void VulkanGuiApplication::DestroySyncObjects()
{

	for (auto& image_available_semaphores : _ImageAvailableSemaphores)
	{
		_Device.destroySemaphore(image_available_semaphores);
	}
	_ImageAvailableSemaphores.clear();
	for (auto& render_finished_semaphore : _RenderFinishedSemaphores)
	{
		_Device.destroySemaphore(render_finished_semaphore);
	}
	_RenderFinishedSemaphores.clear();
	for (auto& in_flight_fences : _InFlightFences)
	{
		_Device.destroyFence(in_flight_fences);
	}
	_InFlightFences.clear();
}

void VulkanGuiApplication::CreateRenderPass()
{
	vk::AttachmentDescription color_attachment;
	color_attachment.format = _SurfaceFormat.format;
	color_attachment.samples = vk::SampleCountFlagBits::e1;
	color_attachment.loadOp = vk::AttachmentLoadOp::eClear;
	color_attachment.storeOp = vk::AttachmentStoreOp::eStore;
	color_attachment.stencilLoadOp = vk::AttachmentLoadOp::eClear;
	color_attachment.stencilStoreOp = vk::AttachmentStoreOp::eStore;
	color_attachment.initialLayout = vk::ImageLayout::eUndefined;
	color_attachment.finalLayout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::AttachmentReference color_attachment_ref;
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

	vk::SubpassDescription subpass;
	subpass.pipelineBindPoint = vk::PipelineBindPoint::eGraphics;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	//vk::SubpassDependency subpass_dependency;
	//subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	//subpass_dependency.dstSubpass = 0;
	//subpass_dependency.srcStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	//subpass_dependency.srcAccessMask = vk::AccessFlagBits(0);
	//subpass_dependency.dstStageMask = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	//subpass_dependency.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

	vk::RenderPassCreateInfo render_pass_create_info;
	render_pass_create_info.attachmentCount = 1;
	render_pass_create_info.pAttachments = &color_attachment;
	render_pass_create_info.subpassCount = 1;
	render_pass_create_info.pSubpasses = &subpass;
	//render_pass_create_info.dependencyCount = 1;
	//render_pass_create_info.pDependencies = &subpass_dependency;

	_RenderPass = _Device.createRenderPass(render_pass_create_info);
}

void VulkanGuiApplication::CreateFramebuffers()
{
	for (size_t i = 0; i < _SwapchainImageViews.size(); i++)
	{
		vk::FramebufferCreateInfo framebuffer_create_info;
		framebuffer_create_info.renderPass = _RenderPass;
		framebuffer_create_info.attachmentCount = 1;
		framebuffer_create_info.pAttachments = &_SwapchainImageViews[i];
		framebuffer_create_info.width = _SurfaceExtent.width;
		framebuffer_create_info.height = _SurfaceExtent.height;
		framebuffer_create_info.layers = 1;
		_SwapchainFramebuffers.push_back(_Device.createFramebuffer(framebuffer_create_info));
	}
}

void VulkanGuiApplication::CreateCommandPoolAndAllocateCommandBuffer()
{
	vk::CommandPoolCreateInfo command_pool_create_info;
	command_pool_create_info.queueFamilyIndex = _GraphicsQueueFamily;
	command_pool_create_info.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
	_CommandPool = _Device.createCommandPool(command_pool_create_info);

	vk::CommandBufferAllocateInfo command_buffer_allocate_info;
	command_buffer_allocate_info.commandPool = _CommandPool;
	command_buffer_allocate_info.level = vk::CommandBufferLevel::ePrimary;
	command_buffer_allocate_info.commandBufferCount = MaxFramesInFlight;
	_CommandBuffers = _Device.allocateCommandBuffers(command_buffer_allocate_info);
}

void VulkanGuiApplication::CleanupSwapChain()
{
	ImGui_CleanupSwapChain();
	for (size_t i = 0; i < _SwapchainFramebuffers.size(); i++)
	{
		_Device.destroyFramebuffer(_SwapchainFramebuffers[i]);
	}
	_SwapchainFramebuffers.clear();
	if (_RenderPass)
	{
		_Device.destroyRenderPass(_RenderPass);
	}
	for (size_t i = 0; i < _SwapchainImageViews.size(); i++)
	{
		_Device.destroyImageView(_SwapchainImageViews[i]);
	}
	_SwapchainImageViews.clear();
	if (_Swapchain)
	{
		_Device.destroySwapchainKHR(_Swapchain);
	}
}

void VulkanGuiApplication::RecreateSwapchain()
{
	_Device.waitIdle();
	CleanupSwapChain();
	CreateSwapchain();
	CreateRenderPass();
	CreateFramebuffers();
	ImGui_CreateRenderPass();
	ImGui_CreateFramebuffer();
	ImGui_ImplVulkan_SetMinImageCount(_MinImageCount);
}

void VulkanGuiApplication::RecordCommand(vk::CommandBuffer& command_buffer, uint32_t image_index)
{
	vk::ClearValue clear_value[] = {
		vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
	};
	vk::CommandBufferBeginInfo command_buffer_begin_info;
	command_buffer.begin(command_buffer_begin_info);
	vk::RenderPassBeginInfo render_pass_begin_info;
	render_pass_begin_info.renderPass = _RenderPass;
	render_pass_begin_info.framebuffer = _SwapchainFramebuffers[image_index];
	render_pass_begin_info.renderArea.offset.x = 0;
	render_pass_begin_info.renderArea.offset.y = 0;
	render_pass_begin_info.renderArea.extent = _SurfaceExtent;
	render_pass_begin_info.clearValueCount = std::size(clear_value);
	render_pass_begin_info.pClearValues = clear_value;
	command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);
	command_buffer.endRenderPass();
	command_buffer.end();
}

void VulkanGuiApplication::DrawFrame()
{
	(void)_Device.waitForFences(_InFlightFences[_CurrentFrame], VK_TRUE, UINT64_MAX);
	if (_FramebufferResized)
	{
		_FramebufferResized = false;
		RecreateSwapchain();
		return;
	}
	auto image_index_result_value = _Device.acquireNextImageKHR(_Swapchain, UINT64_MAX, _ImageAvailableSemaphores[_CurrentFrame], VK_NULL_HANDLE);
	if (image_index_result_value.result == vk::Result::eSuboptimalKHR)
	{
		RecreateSwapchain();
		return;
	}
	else if (image_index_result_value.result != vk::Result::eSuccess) [[unlikely]]
	{
		throw std::runtime_error("ACQUIRE SWAPCHAIN IMAGE FAILED");
	}
	uint32_t image_index = image_index_result_value.value;
	(void)_Device.resetFences(1, &_InFlightFences[_CurrentFrame]);

	vk::Semaphore wait_semaphores[] = { _ImageAvailableSemaphores[_CurrentFrame] };
	vk::Semaphore signal_semaphores[] = { _RenderFinishedSemaphores[_CurrentFrame] };
	vk::PipelineStageFlags wait_dst_stage_mask[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
	_CommandBuffers[_CurrentFrame].reset();
	RecordCommand(_CommandBuffers[_CurrentFrame], image_index);
	_ImGuiCommandBuffers[_CurrentFrame].reset();
	ImGui_RecordCommand(_ImGuiCommandBuffers[_CurrentFrame], image_index);
	vk::CommandBuffer command_buffers[] = { _CommandBuffers[_CurrentFrame], _ImGuiCommandBuffers[_CurrentFrame] };
	vk::SubmitInfo submit_info;
	submit_info.waitSemaphoreCount = std::size(wait_semaphores);
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_dst_stage_mask;
	submit_info.commandBufferCount = std::size(command_buffers);
	submit_info.pCommandBuffers = command_buffers;
	submit_info.signalSemaphoreCount = std::size(signal_semaphores);
	submit_info.pSignalSemaphores = signal_semaphores;
	_GraphicsQueue.submit(submit_info, _InFlightFences[_CurrentFrame]);

	vk::SwapchainKHR swapchains[] = { _Swapchain };

	vk::PresentInfoKHR present_info;
	present_info.waitSemaphoreCount = std::size(signal_semaphores);
	present_info.pWaitSemaphores = signal_semaphores;
	present_info.swapchainCount = std::size(swapchains);
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;
	present_info.pResults = nullptr;
	auto present_result = _PresentQueue.presentKHR(present_info);

	if (present_result == vk::Result::eSuboptimalKHR)
	{
		RecreateSwapchain();
	}
	else if (image_index_result_value.result != vk::Result::eSuccess) [[unlikely]]
	{
		throw std::runtime_error("ACQUIRE SWAPCHAIN IMAGE FAILED");
	}
	_CurrentFrame = (_CurrentFrame + 1) % MaxFramesInFlight;
}
