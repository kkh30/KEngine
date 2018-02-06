#include "KECamera.h"




KECamera::KECamera():m_current_buffer(0)
{
	auto width = KEWindow::GetWindow().GetWidth();
	auto height = KEWindow::GetWindow().GetHeight();

	for (auto& mvp:mvp_buffers) {
		mvp.proj = glm::perspective(glm::radians(60.0f), (float)width / (float)height, 0.1f, 256.0f);

		mvp.view = glm::lookAt(
			glm::vec3(50, 80, 0.0),  // Camera is at (-5,3,-10), in World Space
			glm::vec3(0, 1, 0),     // and looks at the origin
			glm::vec3(0, 1, 0)     // Head is up (set to 0,-1,0 to look upside-down)
		);
		mvp.model = glm::scale(mvp.model, glm::vec3(0.01));
		// Vulkan clip space has inverted Y and half Z.
		// Todo Move this logic into vulkanRHI
		mvp.proj = VK_CLIP_CORRECTION_MAT * mvp.proj;
	}
		
	

}

void KECamera::Update(uint8_t current_buffer) {
	//Translate(glm::vec3(0.0001,0.0,0.0));
	m_current_buffer = current_buffer;
	Rotate(glm::vec3(0, 1, 0), 0.01);

}
void KECamera::Rotate(const glm::vec3& axis, float angle) {
	mvp_buffers[m_current_buffer].model = glm::rotate(mvp_buffers[m_current_buffer].model,angle,glm::vec3(axis));

}


void KECamera::Translate(const glm::vec3& delta) {
	mvp_buffers[m_current_buffer].model = glm::translate(mvp_buffers[m_current_buffer].model,delta);
}



KECamera::~KECamera()
{
}
