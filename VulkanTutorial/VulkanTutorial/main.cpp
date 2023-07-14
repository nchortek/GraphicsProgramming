#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// TODO: Investigate if I need to be using the MINGW64 Binaries since that's what my git bash is.
// (I don't think so since I use visual studio for editing and compiling, and MINGW64 is just my bash shell)

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <set>

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

class HelloTriangleApplication {
public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	GLFWwindow* window;
	VkInstance instance;

	void initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	}

	void initVulkan() {
		createInstance();
	}

	// TODO: Would be good for this to provide some information about the missing extensions
	bool checkRequiredExtensions(const char** requiredExtensions, uint32_t numRequiredExtensions) {
		uint32_t numExtensions = 0;

		// Grab just the number of available extensions first so we can allocate an appropriately
		// sized vector to hold them.
		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(numExtensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, availableExtensions.data());

		std::set<std::string> extensionNames = {};

		for (const auto& availableExtension : availableExtensions) {
			std::string strExtension(availableExtension.extensionName);
			extensionNames.insert(strExtension);
		}

		for (uint32_t i = 0; i < numRequiredExtensions; i++) {
			const char* requiredExtension = requiredExtensions[i];
			std::string strReqExtension(requiredExtension);

			if (!extensionNames.contains(strReqExtension)) {
				return false;
			}
		}

		return true;
	}

	void createInstance() {
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

		// TODO: Do a refresher on pointers and references

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		if (!checkRequiredExtensions(glfwExtensions, glfwExtensionCount)) {
			throw std::runtime_error("Missing one or more Vulkan extensions required by GLFW.");
		}

		createInfo.enabledExtensionCount = glfwExtensionCount;

		// TODO: ppEnabledExtensionNames is defined as type "const char* const*" -- What exactly does that mean? Both the double pointer and the double const.
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create VkInstance!");
		}
		else {
			std::cout << "VkInstance successfully created." << std::endl;
		}
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	// Note that we are careful to free resources in the reverse order that
	// they were allocated in our run() function
	void cleanup() {
		vkDestroyInstance(instance, nullptr);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	HelloTriangleApplication app;

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}