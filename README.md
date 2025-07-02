# Vulkan学习过程
## 这是一个学习vulkan的学习过程记录
跟随https://vulkan-tutorial.com 学习
## 05.7.3 物理设备学习
找到所有所有的物理设备，然后筛选除能满足功能的设备，渲染简单图形的话找到VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU类型，具有图形渲染功能的设备。然后检查该设备是否含有所需功能的队列族，并记录队列族的索引。
