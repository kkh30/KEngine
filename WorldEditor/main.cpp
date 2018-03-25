#include "Camera\KECamera.h"
#include "ECS/KEEntity.h"
#include "KEWindow.h"
#include "KEVulkanRHI\VulkanRHI.h"
#include "KELog.h"
#include "ECS/KESystem.h"
#include "ECS/KERenderComponent.h"
#include <fbxsdk.h>
#include "fbximporter\DisplayMesh.h"
#include "fbximporter\Common.h"
#include "ECS\KETransformComponent.h"
void static DisplayContent(FbxNode* pNode, FbxManager* manager = nullptr)
{
	FbxNodeAttribute::EType lAttributeType;
	int i;

	if (pNode->GetNodeAttribute() == NULL)
	{
		FBXSDK_printf("NULL Node Attribute\n\n");
	}
	else
	{
		lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

		switch (lAttributeType)
		{
		default:
			break;
			//case FbxNodeAttribute::eMarker:  
			//    DisplayMarker(pNode);
			//    break;
			//
			//case FbxNodeAttribute::eSkeleton:  
			//    DisplaySkeleton(pNode);
			//    break;

		case FbxNodeAttribute::eMesh:
			auto e = EntityManager::GetEntityManager().CreateEntity();
			DisplayMesh(pNode,e,manager);
			break;

			//case FbxNodeAttribute::eNurbs:      
			//    DisplayNurb(pNode);
			//    break;
			//
			//case FbxNodeAttribute::ePatch:     
			//    DisplayPatch(pNode);
			//    break;
			//
			//case FbxNodeAttribute::eCamera:    
			//    DisplayCamera(pNode);
			//    break;
			//
			//case FbxNodeAttribute::eLight:     
			//    DisplayLight(pNode);
			//    break;
			//
			//case FbxNodeAttribute::eLODGroup:
			//    DisplayLodGroup(pNode);
			//    break;
		}
	}

	//DisplayUserProperties(pNode);
	//DisplayTarget(pNode);
	//DisplayPivotsAndLimits(pNode);
	//DisplayTransformPropagation(pNode);
	//DisplayGeometricTransform(pNode);

	for (i = 0; i < pNode->GetChildCount(); i++)
	{
		DisplayContent(pNode->GetChild(i));
	}
}

void static DisplayContent(FbxScene* pScene, FbxManager* manager = nullptr)
{
	int i;
	FbxNode* lNode = pScene->GetRootNode();

	if (lNode)
	{
		for (i = 0; i < lNode->GetChildCount(); i++)
		{
			DisplayContent(lNode->GetChild(i),manager);
		}
	}
}

int main(int argc, char **argv) {


	auto& window = KEWindow::GetWindow();
	window.CreateKEWindow(2560,0,1920,1080,SDL_WindowFlags::SDL_WINDOW_VULKAN);
	auto& entity_manager = EntityManager::GetEntityManager();

	auto entity0 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity0);

	auto entity1 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity1);
	//
	//auto entity2 = entity_manager.CreateEntity();
	//KELog::Log("Create Entity %d\n", entity2);
	//
	//entity_manager.DestoryEntity(entity1);
	//KELog::Log("Destory Entity %d\n", entity1);
	//
	//
	//auto entity3 = entity_manager.CreateEntity();
	//KELog::Log("Create Entity %d\n", entity3);

	auto& component_system = System<float>::GetSystem();

	component_system.AddEntityComponent(entity0, 1.0);


	auto value = component_system.GetEntityComponent(entity0);

	auto& render_system = System<KERenderComponent>::GetSystem();
	auto& transform_system = TransformSystem::GetSystem();



	FbxManager* lSdkManager = nullptr;
	FbxScene* lScene = NULL;
	bool lResult;



	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);
	lResult = LoadScene(lSdkManager, lScene, argv[1]);
	DisplayContent(lScene);

	auto floor = KERenderComponent({
		{ { -100.0f, 0.0f, 100.0f, },{ 0.0f, 1.0f,  0.0f } },
		{ { -100.0f, 0.0f, -100.0f, },{ 0.0f, 1.0f,  0.0f } },
		{ { 100.0f,  0.0f, -100.0f, },{ 0.0f, 1.0f,  0.0f } },
		{ { 100.0f,  0.0f, 100.0f, },{ 0.0f, 1.0f,  0.0f } },
		}, { 0,2,1,0,3,2 });
	auto floor_transform = KETransformComponent();
	floor_transform.SetScale(glm::vec3(0.1f));
	//floor.SetScale();
	floor.material[0] = { 0.15f  };
	floor.material[1] = { 0.47f };
	floor.material[2] = { 0.75f };
	floor.material[3] = { 1.0f };

	render_system.AddEntityComponent(entity1, floor);
	transform_system.AddEntityComponent(entity1, floor_transform);
	System<KERenderComponent>& render_component_system = System<KERenderComponent>::GetSystem();
	auto& all_entites = EntityManager::GetEntityManager().GetAllEntities();

	for (auto& entity : all_entites) {
		auto& render_component = render_component_system.GetEntityComponent(entity);
		auto& transform_component = transform_system.GetSystem().GetEntityComponent(entity);

		transform_component.SetScale(glm::vec3(1.0f));
		//render_component.SetLocalTranslation(
		//	glm::vec3(
		//		float(rand()) / RAND_MAX * 0.1f,
		//		float(rand()) / RAND_MAX * 0.1f, 
		//		float(rand()) / RAND_MAX * 0.1f
		//	));
	}
	//render_component_system.GetEntityComponent(*all_entites.begin()).SetLocalTranslation(glm::vec3(2.0f));
	

	auto& camera = KECamera::GetCamera();


	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));

	


	window.Show();


	
	



	return 0;
}