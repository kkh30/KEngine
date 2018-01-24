#include "KEEntity.h"
#include "KEWindow.h"
#include "KEVulkanRHI\VulkanRHI.h"
#include "KELog.h"
#include "KEComponent.h"
#include "KERenderComponent.h"

int main() {


	auto& window = KEWindow::GetWindow();
	window.CreateKEWindow(2560,100,800,600,SDL_WindowFlags::SDL_WINDOW_VULKAN);
	auto& entity_manager = EntityManager::GetEntityManager();

	auto entity0 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity0);

	auto entity1 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity1);

	auto entity2 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity2);

	//entity_manager.DestoryEntity(entity1);
	//KELog::Log("Destory Entity %d\n", entity1);


	auto entity3 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity3);

	auto& component_system = System<float>::GetSystem();

	component_system.AddEntityComponent(entity0, 1.0);


	auto value = component_system.GetEntityComponent(entity0);

	auto& render_system = System<KRenderComponent>::GetSystem();
	// Setup vertices

	render_system.AddEntityComponent(entity0, KRenderComponent({
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
		}));

	render_system.AddEntityComponent(entity1, KRenderComponent({
		{ { 1.0f,  -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
		}));

	render_system.AddEntityComponent(entity2, KRenderComponent({
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { 1.0f,  -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		}));

	
	render_system.AddEntityComponent(entity3, KRenderComponent({
		{ { -1.0f,  -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		}));


	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));

	


	window.Show();


	
	



	return 0;
}