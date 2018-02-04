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

void static DisplayContent(FbxNode* pNode, Entity e)
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
			DisplayMesh(pNode,e);
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
		DisplayContent(pNode->GetChild(i),e);
	}
}

void static DisplayContent(FbxScene* pScene,Entity e)
{
	int i;
	FbxNode* lNode = pScene->GetRootNode();

	if (lNode)
	{
		for (i = 0; i < lNode->GetChildCount(); i++)
		{
			DisplayContent(lNode->GetChild(i),e);
		}
	}
}

int main(int argc, char **argv) {


	auto& window = KEWindow::GetWindow();
	window.CreateKEWindow(2560,100,800,600,SDL_WindowFlags::SDL_WINDOW_VULKAN);
	auto& entity_manager = EntityManager::GetEntityManager();

	auto entity0 = entity_manager.CreateEntity();
	KELog::Log("Create Entity %d\n", entity0);

	//auto entity1 = entity_manager.CreateEntity();
	//KELog::Log("Create Entity %d\n", entity1);
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



	FbxManager* lSdkManager = nullptr;
	FbxScene* lScene = NULL;
	bool lResult;

	// Prepare the FBX SDK.
	InitializeSdkObjects(lSdkManager, lScene);

	lResult = LoadScene(lSdkManager, lScene, argv[1]);

	//if (lResult == false)
	//{
	//	FBXSDK_printf("\n\nAn error occurred while loading the scene...");
	//}
	//
	DisplayContent(lScene,entity0);

	auto& camera = KECamera::GetCamera();


	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));

	


	window.Show();


	
	



	return 0;
}