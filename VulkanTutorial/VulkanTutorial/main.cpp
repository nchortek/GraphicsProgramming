#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// TODO: Investigate if I need to be using the MINGW64 Binaries since that's what my git bash is.
// (I don't think so since I use visual studio for editing and compiling, and MINGW64 is just my bash shell)

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

// TODO:
// After following this tutorial, you could implement automatic resource management by writing C++
// classes that acquire Vulkan objects in their constructor and release them in their destructor,
// or by providing a custom deleter to either std::unique_ptr or std::shared_ptr, depending on your
// ownership requirements. RAII is the recommended model for larger Vulkan programs, but for learning
// purposes it's always good to know what's going on behind the scenes.
//
// Potentially accomplish this via RAII object/class management

// NOTE: The following dimensions are in screen coordinates, **not** pixels.
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

#pragma region Debug Proxy Functions

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

#pragma endregion

class HelloTriangleApplication
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
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	// Note that the VkPhysicalDevice object is implicitly destroyed when the VkInstance is destroyed,
	// which is we why don't need to specifically handle it during cleanup.
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	// Device queues are implicitly cleaned up when the device is destroyed, so we don't need to do anything in cleanup
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	// These images get created by the implementation for the swap chain and they will be automatically cleaned up once
	// the swap chain has been destroyed, therefore we don't need to add any cleanup code.
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		// NOTE: The dimensions of the window are specified in screen coordinates here.
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		// The window surface needs to be created right after the instance creation, because it can actually influence
		// the physical device selection
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
	}

#pragma region Swap Chains and Surfaces

	void createSwapChain()
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

		VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

		// Request at least one more image than the minimum to avoid having to wait on the driver to complete internal
		// operations before we can acquire another image to render to.
		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

		// if capabilities.maxImageCount is 0 it means there is no max. If a max exists, make sure we don't exceed it.
		if (swapChainSupport.capabilities.maxImageCount > 0
			&& imageCount > swapChainSupport.capabilities.maxImageCount)
		{
			// TODO: Why wouldn't we always want to take the maxImageCount if a max exists? 
			// Are there ideal image counts for rendering? Is higher not necessarily always better?
			imageCount = swapChainSupport.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		if (indices.graphicsFamily != indices.presentFamily)
		{
			// We prefer EXCLUSIVE mode for performance reasons, but we need both graphics and present support,
			// so if we don't have a queue family that supports both we have to go with CONCURRENT since it supports
			// multiple distinct queue families.
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			// An image is owned by one queue family at a time and ownership must be explicitly transferred before
			// using it in another queue family. This option offers the best performance.
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		// We can specify that a certain transform should be applied to images in the swap chain if it is supported
		// (supportedTransforms in capabilities), like a 90 degree clockwise rotation or horizontal flip. To specify
		// that we do not want any transformation, we specify the current transformation.
		createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

		// The compositeAlpha field specifies if the alpha channel should be used for blending with other windows in
		// the window system. We almost always want to simply ignore the alpha channel, hence VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR.
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;

		// clipped specifies whether the Vulkan implementation is allowed to discard rendering operations that affect
		// regions of the surface that are not visible. (e.g. another window obscuring the window we're drawing to)
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create swapchain.");
		}

		// Retrieve the image objects that the swapChain implementation created.
		// Note that we need to re-fetch the imageCount since we only specified a minimum.
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		// Store our surfaceFormat and extent selections for future reference as well.
		swapChainImageFormat = surfaceFormat.format;
		swapChainExtent = extent;
	}

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0)
		{
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}
	

	// Quote from the tutorial:
	// 
	// --------------------------------------------------------------------------------------------------------------------
	// The swap extent is the resolution of the swap chain images and it's almost always exactly equal to the resolution
	// of the window that we're drawing to in pixels (more on that in a moment). The range of the possible resolutions is
	// defined in the VkSurfaceCapabilitiesKHR structure. Vulkan tells us to match the resolution of the window by setting
	// the width and height in the currentExtent member. However, some window managers do allow us to differ here and this
	// is indicated by setting the width and height in currentExtent to a special value: the maximum value of uint32_t.
	// In that case we'll pick the resolution that best matches the window within the minImageExtent and maxImageExtent
	// bounds. But we must specify the resolution in the correct unit.
	// 
	// GLFW uses two units when measuring sizes : pixels and screen coordinates.For example, the resolution{ WIDTH, HEIGHT }
	// that we specified earlier when creating the window is measured in screen coordinates.But Vulkan works with pixels, 
	// so the swap chain extent must be specified in pixels as well.Unfortunately, if you are using a high DPI display
	// (like Apple's Retina display), screen coordinates don't correspond to pixels.Instead, due to the higher pixel density,
	// the resolution of the window in pixel will be larger than the resolution in screen coordinates.So if Vulkan doesn't
	// fix the swap extent for us, we can't just use the original {WIDTH, HEIGHT}. Instead, we must use glfwGetFramebufferSize
	// to query the resolution of the window in pixel before matching it against the minimum and maximum image extent
	// --------------------------------------------------------------------------------------------------------------------
	// 
	// TODO Things I don't currently understand:
	//
	// 1. What does the maximimum value of uint32_t indicate within currentExtent? The spec says it indicates "that the surface
	//    size will be determined by the extent of a swapchain targeting the surface", but what precisely does that mean? 
	//    When and why do we fall into that case vs the non-max case?
	//
	// 2. What exactly is happening with the clamping? glfwGetFramebufferSize gives dimensions in pixels, which is what
	//    Vulkan needs. Then we clamp that value to the surface's minimum and maximum image extent values. But why would the
	//    window have dimensions outside the valid range of the surface? Is it because the window size is programmatically
	//    defined rather than being tied to hardware? So we want to make sure the window we specified for GLFW in screen
	//    coordinates does not result in a pixel dimension outside the range our surface supports?
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
		{
			return capabilities.currentExtent;
		}
		else
		{
			// Width and height here are in pixels, **not** screen coordinates
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D actualExtent =
			{
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return actualExtent;
		}
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
	{
		for (const auto& availablePresentMode : availablePresentModes)
		{
			// VK_PRESENT_MODE_MAILBOX_KHR is a very nice trade-off if energy usage is not a concern.
			// It allows us to avoid tearing while still maintaining a fairly low latency by rendering
			// new images that are as up-to-date as possible right until the vertical blank. On mobile
			// devices, where energy usage is more important, you will probably want to use
			// VK_PRESENT_MODE_FIFO_KHR instead.
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availablePresentMode;
			}
		}

		// FIFO is mode is guaranteed to be available so its a good default
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
				&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	void createSurface()
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create window surface.");
		}
	}

#pragma endregion

#pragma region Logical Devices

	void createLogicalDevice()
	{
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

		if (enableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create logical device.");
		}

		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
		vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
	}

#pragma endregion

#pragma region Physical Devices

	void pickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		// Grab just the device count first
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		
		if (deviceCount == 0)
		{
			throw std::runtime_error("Failed to find a GPU that supports Vulkan.");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}
		}

		if (physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("Failed to find a GPU suitable for this application.");
		}
	}

	bool checkDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : requiredExtensions)
		{
			if (!requiredExtensions.contains(extension))
			{
				return false;
			}
		}

		return true;
	}

	bool isDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);

		bool extensionsSupported = checkDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported)
		{
			SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.isComplete() && extensionsSupported && swapChainAdequate;
	}

#pragma endregion

#pragma region Queues

	struct QueueFamilyIndices
	{
		// Any value of uint32_t could potentially be a valid queue family index (including 0),
		// so we wrap it into std::optional in order to tell if a queue family was actually found.
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value() && presentFamily.has_value(); 
		}
	};

	// TODO: if this depends on the physical device being initialized, do we need some error checking?
	// Should that be the responsibility of this method or its callers?
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		// Grab the count first
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			// Determine if this queue family of our physical device supports presentation to our surface
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentFamily = i;
			}

			// TODO: Why not just break as soon as we find the first queue family?
			// Thought: Maybe because we may want "isComplete" to check for the presence
			// of multiple different queue families with distinct features, so we might not want
			// to break after finding just one queue family that satisfies one (but not necessarily all)
			// of our requirements
			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

#pragma endregion

#pragma region Extensions

	// TODO: Would be good for this to provide some information about the missing extensions
	bool checkRequiredExtensions(const std::vector<const char*> requiredExtensions)
	{
		uint32_t numExtensions = 0;

		// Grab just the number of available extensions first so we can allocate an appropriately
		// sized vector to hold them.
		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(numExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, availableExtensions.data());

		// Build a set of the available extensions so we avoid a nested loop when iterating through
		// our required extensions to check that they are available
		std::set<std::string> extensionNames = {};

		for (const auto& availableExtension : availableExtensions)
		{
			std::string strExtension(availableExtension.extensionName);
			extensionNames.insert(strExtension);
		}

		for (const auto& requiredExtension : requiredExtensions)
		{
			std::string strReqExtension(requiredExtension);

			if (!extensionNames.contains(strReqExtension))
			{
				return false;
			}
		}

		return true;
	}

	std::vector<const char*> getRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		// Pointer arithmetic in both C and C++ is a logical addition, not a numeric addition. 
		// Adding x to a pointer means "produce a pointer to the object that comes in memory x places after this one," 
		// which means that the compiler automatically scales up whatever you're incrementing the pointer with by the 
		// size of the object being pointed at.
		std::vector<const char*> requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		if (!checkRequiredExtensions(requiredExtensions))
		{
			throw std::runtime_error("Missing one or more required Vulkan extensions.");
		}

		return requiredExtensions;
	}

#pragma endregion


#pragma region Validation

	bool checkValidationLayerSupport()
	{
		uint32_t numLayers;

		// Grab just the number of available layers first
		vkEnumerateInstanceLayerProperties(&numLayers, nullptr);

		std::vector<VkLayerProperties> availableLayers(numLayers);
		vkEnumerateInstanceLayerProperties(&numLayers, availableLayers.data());

		// Build a set of the available validation layers so we avoid a nested loop when iterating
		// through our requested layers to check that they are available
		std::set<std::string> layerNames = {};

		for (const auto& availableLayer : availableLayers)
		{
			std::string strLayer(availableLayer.layerName);
			layerNames.insert(strLayer);
		}

		for (const char* requestedLayer : validationLayers)
		{
			std::string strRequestedLayer(requestedLayer);

			if (!layerNames.contains(strRequestedLayer))
			{
				return false;
			}
		}

		return true;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	void populateMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;
	}

	void setupDebugMessenger()
	{
		if (!enableValidationLayers)
		{
			return;
		}

		VkDebugUtilsMessengerCreateInfoEXT createInfo;
		populateMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to set up debug messenger.");
		}
	}

#pragma endregion

	void createInstance() {
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("Missing one or more requested validation layers.");
		}

		// Note that delcaring this struct with empty braces causes it to be initialized via
		// "value initialization".
		VkApplicationInfo appInfo{};
		// Question: Why does vulkan need to store a structure type enum? Are struct types
		// not inherent / programmatically checkable?
		// Answer (I think): Vulkan was written in C, which does not support runtime type checking.
		// C++ can support runtime type checks, though, assuming RTTI is enabled
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_API_VERSION_1_0;
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_API_VERSION_1_0;
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		std::vector<const char*> glfwExtensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		createInfo.ppEnabledExtensionNames = glfwExtensions.data();
		
		// The debugCreateInfo variable is placed outside the if statement to ensure that it is not destroyed before the vkCreateInstance call
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create VkInstance!");
		}
		else
		{
			std::cout << "VkInstance successfully created." << std::endl;
		}
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}

	// Note that we are careful to free resources in the reverse order that
	// they were allocated
	void cleanup()
	{
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroyDevice(device, nullptr);

		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main()
{
	HelloTriangleApplication app;

	try
	{
		app.run();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}