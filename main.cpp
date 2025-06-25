#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<iostream>
#include<stdexcept>
#include<cstdlib>
#include<vector>
#include<string.h>

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

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtension;
		glfwExtension = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtension;

		if (enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		//开始实例化
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
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


private:
	GLFWwindow* window = nullptr;
	VkInstance instance;
};

int main()
{
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