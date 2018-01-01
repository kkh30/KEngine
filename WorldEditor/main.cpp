#include "KEWindow.h"
#include "KEVulkanRHI\VulkanRHI.h"

int main() {


	auto& window = KEWindow::GetWindow();
	window.CreateKEWindow(2560,100,800,600,SDL_WindowFlags::SDL_WINDOW_VULKAN);

	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));


	window.Show();



	return 0;
}