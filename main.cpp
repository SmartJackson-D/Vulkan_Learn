#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<iostream>
#include<stdexcept>
#include<cstdlib>
#include<vector>
#include<string.h>
#include<optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

//所有标准验证层都包含在VK_LAYER_KHRONOS_validation
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//根据是否启动debug模式决定是否启用验证层
#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif // NDEBUG

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

//物理设备的队列结构体
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value();
	}
};

class HelloTriangleApplication{
public:
	void run()
	{
		initWindow();
		initVulcan();
		mainLoop();
		cleanup();
	}

private:
	void initWindow()
	{
		//先初始化glfw库
		glfwInit();

		//关闭OPGL环境
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//先设置窗口不可调整大小
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//创建窗口
		window = glfwCreateWindow(WIDTH,HEIGHT,"Vulkan",nullptr,nullptr);

	}

	void initVulcan() 
	{
		createInstance();
		setupDebugMessenger();
		pickPhysicalDevice();
		createLogicalDevice();
	}

	void mainLoop()
	{
		while (!glfwWindowShouldClose(window))
		{
			//拉取窗口消息
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		//清除逻辑设备
		vkDestroyDevice(device, nullptr);

		//清除消息
		if (enableValidationLayers) {
			DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
		}
		//清除VK实例
		vkDestroyInstance(instance, nullptr);

		//清除窗口
		glfwDestroyWindow(window);


		//结束GLFW自身相关
		glfwTerminate();
	}

private:
	void createInstance()
	{
		//检查验证层 开启检验但是检验层可能不存在
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		//创建APP信息（非必要）
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  //标注这些信息属于哪种信息，这里标记为是软件信息
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//创建VK实例的信息（必要）
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		/*uint32_t glfwExtensionCount = 0;
		const char** glfwExtension;
		glfwExtension = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		这部分使用后面自定义的getRequiredExtensions函数替代了，在内部重写了一便又加了内容*/

		auto extensions = getRequiredExtensions();

		createInfo.enabledExtensionCount = static_cast<uint32_t> (extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();

			populateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		//开始实例化
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
		
	}

	void pickPhysicalDevice()
	{
		//列出所有可用物理设备
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		
		//检查是否符合需求
		for (const auto& device:devices)
		{
			if (isDeviceSuitable(device))
			{
				physicalDevice = device;
				break;
			}	
		}

		if (physicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("failed to find a suitable GPU!");
	}

	void createLogicalDevice()
	{
		//指定队列族
		QueueFamilyIndices indices = findQueueFamily(physicalDevice);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		//指定使用物理设备哪些功能
		VkPhysicalDeviceFeatures deviceFeatures{};

		//创建逻辑设备信息
		VkDeviceCreateInfo deviceCreateInfo{};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.queueCreateInfoCount = 1;

		deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

		deviceCreateInfo.enabledExtensionCount = 0;
		if (enableValidationLayers)
		{
			deviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			deviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
		{
			deviceCreateInfo.enabledLayerCount = 0;
		}
		//创建逻辑设备
		if (vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device) != VK_SUCCESS)
			throw std::runtime_error("failed to create logical device!");

		//找回队列句柄
		vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	}

private:
	//填充消息结构体信息
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr; // Optional
	}

	//开启debug消息传送
	void setupDebugMessenger()
	{
		if (!enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		populateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//检查所需检验层是否都有
	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//检查可用检验层中是否存在所需检验层 
		//check if all of the layers in validationLayers exist in the availableLayers list
		//这部分是B中是否存在A，只要有一个没有就返回false
		for (const char* layerName : validationLayers)
		{
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layerFound = true;
					break;
				}
			}
			
			if (!layerFound)
				return false;
		}

		return true;
	}

	//获取所需的拓展
	std::vector<const char*> getRequiredExtensions() 
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	//debug回调函数，完成自定义debug返回信息
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageSeverityFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {
		switch (messageSeverity)
		{
		/*case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			std::cout << "Diagnostic message: " << std::endl
				<<pCallbackData->pMessage << std::endl;
			break;*/
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			std::cout << "Informational message like the creation of a resource: " << std::endl
				<< pCallbackData->pMessage << std::endl;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			std::cout << "Message about behavior that is not necessarily an error,"<< 
				"but very likely a bug in your application: " << std::endl
				<< pCallbackData->pMessage << std::endl;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			std::cout << "Message about behavior that is invalid and may cause crashes: " << std::endl
				<< pCallbackData->pMessage << std::endl;
			break;
		}

		//std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	//检查物理设备是否符合需求
	bool isDeviceSuitable(VkPhysicalDevice device) {
		VkPhysicalDeviceProperties  deviceProperties;
		VkPhysicalDeviceFeatures	deviceFeatures;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
		
		//检查物理设备的队列
		QueueFamilyIndices indice=findQueueFamily(device);

		return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
			deviceFeatures.geometryShader && indice.isComplete();
	}

	//寻找物理设备里面的队列
	QueueFamilyIndices findQueueFamily(VkPhysicalDevice device)
	{
		//列出该设备对应的所有队列
		QueueFamilyIndices indice;
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamily(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamily.data());

		//找所需的队列
		int i = 0;
		for (const auto& queue : queueFamily)
		{
			if (queue.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indice.graphicsFamily = i;
			}
			if (indice.isComplete())
				break;
			i++;

		}

		return indice;
	}

private:
	GLFWwindow* window = nullptr;
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
	VkSurfaceKHR surface = VK_NULL_HANDLE;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;   //物理设备
	VkDevice device = VK_NULL_HANDLE;					//逻辑设备
	VkQueue  graphicsQueue = VK_NULL_HANDLE;			//队列句柄
};

int main2()
{
	std::cout << "原版" << std::endl;
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