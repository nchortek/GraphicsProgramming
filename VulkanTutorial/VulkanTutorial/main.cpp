#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// TODO: Investigate if I need to be using the MINGW64 Binaries since that's what my git bash is.
// (I don't think so since I use visual studio for editing and compiling, and MINGW64 is just my bash shell)

#include <cstdlib>
#include <iostream>
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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

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

	// Note that the VkPhysicalDevice object is implicitly destroyed when the VkInstance is destroyed,
	// which is we why don't need to specifically handle it during cleanup.
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	void initWindow()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan()
	{
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
	}

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
			if (isGraphicsDeviceSuitable(device))
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

	bool isGraphicsDeviceSuitable(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices = findQueueFamilies(device);
		return indices.isComplete();
	}

#pragma endregion

#pragma region Queues

	struct QueueFamilyIndices
	{
		// Any value of uint32_t could potentially be a valid queue family index (including 0),
		// so we wrap it into std::optional in order to tell if a queue family was actually found.
		std::optional<uint32_t> graphicsFamily;

		bool isComplete()
		{
			return graphicsFamily.has_value();
		}
	};

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
		if (enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}

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