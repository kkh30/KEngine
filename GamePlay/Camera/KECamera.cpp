#include "KECamera.h"




KECamera::KECamera()
{
	float fov = glm::radians(45.0f);
	auto width = KEWindow::GetWindow().GetWidth();
	auto height = KEWindow::GetWindow().GetHeight();

	if (width > height) {
		fov *= static_cast<float>(height) / static_cast<float>(width);
	}
	m_mvp.proj = glm::perspective(fov, static_cast<float>(width) / static_cast<float>(height), 0.1f, 100.0f);
	m_mvp.view = glm::lookAt(glm::vec3(50, 50, 50),  // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),     // and looks at the origin
		glm::vec3(0, 1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
	);
	m_mvp.model = glm::mat4(0.1);
	// Vulkan clip space has inverted Y and half Z.
	// Todo Move this logic into vulkanRHI
	m_mvp.mvp = VK_CLIP_CORRECTION_MAT * m_mvp.proj * m_mvp.view * m_mvp.model;
	//m_mvp.mvp = glm::mat4();

}

KECamera::~KECamera()
{
}
