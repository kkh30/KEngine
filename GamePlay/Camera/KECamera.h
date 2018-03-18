#ifndef __KE_CAMERA_H__
#define __KE_CAMERA_H__
#define GLM_FORCE_AVX2
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"
#include "glm\matrix.hpp"
#include "glm\gtx\matrix_operation.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "KEWindow.h"

static const glm::mat4 VK_CLIP_CORRECTION_MAT = glm::mat4(
	1.0f, 0.0f, 0.0f, 0.0f, 
	0.0f,-1.0f, 0.0f, 0.0f,
	0.0f, 0.0f,0.5f, 0.0f, 
	0.0f, 0.0f, 0.5f, 1.0f);

class KECamera
{
public:
	static KECamera& GetCamera() {
		static KECamera l_camera;
		return l_camera;
	}
	~KECamera();
	KECamera(const KECamera&) = delete;
	KECamera& operator=(const KECamera&) = delete;

	enum {TRIPLE_BUFFER_SIZE = 3};

	struct MVP{
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
		glm::mat4 shadow_map_view;
	};

	MVP mvp_buffers[TRIPLE_BUFFER_SIZE];
	uint8_t m_current_buffer;
	KECamera();
	void Update();
	void Translate(const glm::vec3&);
	void Rotate(const glm::vec3&,float);

};






#endif // !__KE_CAMERA_H__
