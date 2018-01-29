#include "Camera\KECamera.h"
#include "ECS/KEEntity.h"
#include "KEWindow.h"
#include "KEVulkanRHI\VulkanRHI.h"
#include "KELog.h"
#include "ECS/KESystem.h"
#include "ECS/KERenderComponent.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

//static bool TestLoadObj(const char* filename, const char* basepath = NULL,
//	bool triangulate = true) {
//	std::cout << "Loading " << filename << std::endl;
//
//	tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materials;
//
//	//timerutil t;
//	//t.start();
//	std::string err;
//	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename,
//		basepath, triangulate);
//	//t.end();
//	//printf("Parsing time: %lu [msecs]\n", t.msec());
//
//	if (!err.empty()) {
//		std::cerr << err << std::endl;
//	}
//
//	if (!ret) {
//		printf("Failed to load/parse .obj.\n");
//		return false;
//	}
//
//	//PrintInfo(attrib, shapes, materials);
//
//	return true;
//}

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
	// Setup vertices
	const char* basepath = "models/";
	if (argc > 2) {
		basepath = argv[2];
	}
	//assert(true == TestLoadObj(argv[1], basepath));

	std::cout << "Loading " << argv[1] << std::endl;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	//timerutil t;
	//t.start();
	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, argv[1],
		basepath, true);
	
	std::vector<KEVertex> vertices;  // pos(3float), normal(3float), color(3float)
	std::vector<uint32_t> indices;


	for (auto i = 0; i < attrib.vertices.size() ; i+=3) {
		KEVertex l_vertex = {};
		l_vertex.position[0] = attrib.vertices[i + 0];
		l_vertex.position[1] = attrib.vertices[i + 1];
		l_vertex.position[2] = attrib.vertices[i + 2];
		l_vertex.color[0] = 0.5f;
		l_vertex.color[1] = 0.5f;
		l_vertex.color[2] = 0.75f;
		vertices.push_back(l_vertex);

	}

	for (auto& shape : shapes) {
		for (auto& index : shape.mesh.indices) {
			indices.push_back(index.vertex_index);
		}
	}


	render_system.AddEntityComponent(entity0, KERenderComponent(std::move(vertices), std::move(indices)));
	

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!ret) {
		printf("Failed to load/parse .obj.\n");
		return false;
	}

	//PrintInfo(attrib, shapes, materials);
	
	//render_system.AddEntityComponent(entity0, KERenderComponent({
	//	{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	//	{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	//	{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	//	}, { 0,1,2 }));
	//
	//render_system.AddEntityComponent(entity1, KERenderComponent({
	//	{ { 1.0f,  -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	//	{ { -1.0f,  -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	//	{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	//	}, { 0,1,2 }));
	//
	//render_system.AddEntityComponent(entity2, KERenderComponent({
	//	{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	//	{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
	//	{ { 1.0f,  -1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	//	}, {0,1,2}));
	//
	//
	//render_system.AddEntityComponent(entity3, KERenderComponent({
	//	{ { -1.0f,  -1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
	//	{ { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
	//	{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
	//	}, { 0,1,2 }));
	

	auto& camera = KECamera::GetCamera();


	auto renderer = KEVulkanRHI::VulkanRHI();
	renderer.Init();

	window.SetRendererFunc(std::bind(&KEVulkanRHI::VulkanRHI::Update,renderer));

	


	window.Show();


	
	



	return 0;
}