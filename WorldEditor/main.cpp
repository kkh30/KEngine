#include "KEEntity.h"
#include "KEWindow.h"
#include "KEVulkanRHI\VulkanRHI.h"
#include "KELog.h"
#include "KEComponent.h"

int main() {


	auto& window = KEWindow::GetWindow();
	window.CreateKEWindow(2560,100,800,600,SDL_WindowFlags::SDL_WINDOW_VULKAN);

	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));

	auto& entity_manager = EntityManager::GetEntityManager();

	auto entity0 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity0);

	auto entity1 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity1);

	auto entity2 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity2);

	entity_manager.DestoryEntity(entity1);
	KELog::Log("Destory Entity %d\n", entity1);


	auto entity3 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity3);

	auto component_manager = ComponentManager<float>();

	component_manager.AddEntityComponent<float>(entity0,1.0);
	

	auto value = component_manager.GetEntityComponent<float>(entity0);


	window.Show();


	
	



	return 0;
}