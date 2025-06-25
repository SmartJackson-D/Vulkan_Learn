#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include<iostream>
#include<stdexcept>
#include<cstdlib>
#include<vector>
#include<string.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

//���б�׼��֤�㶼������VK_LAYER_KHRONOS_validation
const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

//�����Ƿ�����debugģʽ�����Ƿ�������֤��
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
		//�ȳ�ʼ��glfw��
		glfwInit();

		//�ر�OPGL����
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//�����ô��ڲ��ɵ�����С
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		//��������
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
			//��ȡ������Ϣ
			glfwPollEvents();
		}
	}

	void cleanup()
	{
		//���VKʵ��
		vkDestroyInstance(instance, nullptr);

		//�������
		glfwDestroyWindow(window);


		//����GLFW�������
		glfwTerminate();
	}
private:
	void createInstance()
	{
		//�����֤�� �������鵫�Ǽ������ܲ�����
		if (enableValidationLayers && !checkValidationLayerSupport())
		{
			throw std::runtime_error("validation layers requested, but not available!");
		}

		//����APP��Ϣ���Ǳ�Ҫ��
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;  //��ע��Щ��Ϣ����������Ϣ��������Ϊ�������Ϣ
		appInfo.pApplicationName = "Hello Triangle";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		//����VKʵ������Ϣ����Ҫ��
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

		//��ʼʵ����
		if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instance!");
		}
		
	}

	//������������Ƿ���
	bool checkValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		//�����ü�������Ƿ������������ 
		//check if all of the layers in validationLayers exist in the availableLayers list
		//�ⲿ����B���Ƿ����A��ֻҪ��һ��û�оͷ���false
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