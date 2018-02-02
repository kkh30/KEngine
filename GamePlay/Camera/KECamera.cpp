#include "KECamera.h"




KECamera::KECamera()
{
	auto width = KEWindow::GetWindow().GetWidth();
	auto height = KEWindow::GetWindow().GetHeight();

	//m_mvp.proj = glm::perspective(fov, static_cast<float>(width) / static_cast<float>(height), 0.1f, -100.0f);
	
	m_mvp.proj = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);
	
	m_mvp.view = glm::lookAt(glm::vec3(30, 60, 0.0),  // Camera is at (-5,3,-10), in World Space
		glm::vec3(0, 0, 0),     // and looks at the origin
		glm::vec3(0, 1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
	);
	m_mvp.model = glm::scale(m_mvp.model,glm::vec3(0.1));
	// Vulkan clip space has inverted Y and half Z.
	// Todo Move this logic into vulkanRHI
	for (auto& mvp: mvp_buffers) {
		mvp = VK_CLIP_CORRECTION_MAT * m_mvp.proj * m_mvp.view * m_mvp.model;

	}
	//m_mvp.mvp = glm::mat4();

}

void KECamera::Update(uint8_t current_buffer) {
	//Translate(glm::vec3(0.0001,0.0,0.0));
	Rotate(glm::vec3(0, 1, 0), 0.001);
	mvp_buffers[current_buffer] = VK_CLIP_CORRECTION_MAT * m_mvp.proj * m_mvp.view * m_mvp.model;

}
void KECamera::Rotate(const glm::vec3& axis, float angle) {
	m_mvp.model = glm::rotate(m_mvp.model,angle,glm::vec3(axis));

}


void KECamera::Translate(const glm::vec3& delta) {
	m_mvp.model = glm::translate(m_mvp.model,delta);
}



KECamera::~KECamera()
{
}
