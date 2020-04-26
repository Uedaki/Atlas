#pragma once

#define GLFW_INCLUDE_VULKAN
//#define GLFW_EXPOSE_NATIVE_WIN32
//#define VK_USE_PLATFORM_WIN32_KHR
#include <glfw/glfw3.h>
#include <vulkan/vulkan.h>

namespace vk
{
	class Window
	{
	public:
		Window(int width, int height);
		~Window();

		bool startFrame();
		void *getCurrentBuffer();
		void render();
		bool isWindowOpen();

	private:
		static constexpr uint16_t MAX_FRAMES_IN_FLIGHT = 2;

		int width;
		int height;

		uint32_t currentImage = 0;
		uint32_t currentFrame = 0;
		void *currentBuffer = nullptr;

		GLFWwindow *win = nullptr;
		VkInstance instance = VK_NULL_HANDLE;
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
		VkDevice device = VK_NULL_HANDLE;
		VkSurfaceKHR surface = VK_NULL_HANDLE;

		uint32_t swapchainSize = 0;
		VkSwapchainKHR swapchain = VK_NULL_HANDLE;
		VkImage *swapchainImage = nullptr;
		VkFormat swapchainFormat = {};

		VkBuffer *imageBuffers = VK_NULL_HANDLE;
		VkDeviceMemory *imageBuffersMemory = VK_NULL_HANDLE;

		VkCommandPool commandPool = VK_NULL_HANDLE;
		VkCommandBuffer *commandBuffers = nullptr;

		uint32_t transferQueueFamily = -1;
		VkQueue transferQueue = VK_NULL_HANDLE;

		uint16_t maxFrames = MAX_FRAMES_IN_FLIGHT;
		VkSemaphore imageAvailableSemaphore[MAX_FRAMES_IN_FLIGHT];
		VkSemaphore renderFinishedSemaphore[MAX_FRAMES_IN_FLIGHT];
		VkFence	inFlightFences[MAX_FRAMES_IN_FLIGHT];

		VkAllocationCallbacks *allocator = nullptr;

		void createGlfwWindow();
		void initializeVulkan();

		void createInstance();
		void selectPhysicalDevice();
		void selectQueueFamily();
		void createLogicalDevice();
		void createSurface();
		void createSwapchain();
		void getSwapchainImages();
		void createImageBuffers();
		void createCommandPool();
		void createCommandBuffers();
		void createSyncObjects();

		VkSurfaceFormatKHR selectSurfaceFormat();
		VkPresentModeKHR selectPresentMode();
		uint32_t selectMemoryType(uint32_t filter, VkMemoryPropertyFlags flags);
	};
}