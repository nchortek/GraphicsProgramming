#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

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

		// TODO: Need to create a VkInstanceCreateInfo object
	}

	void mainLoop() {
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
		}
	}

	void cleanup() {
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