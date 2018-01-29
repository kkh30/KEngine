#ifndef __KE_CAMERA_H__
#define __KE_CAMERA_H__
#define GLM_FORCE_INLINE 
#define GLM_FORCE_AVX2
#include "glm\glm.hpp"
#include "glm\matrix.hpp"
#include "glm\gtx\matrix_operation.inl"
#include "glm\gtc\matrix_transform.inl"
#include "KEWindow.h"

static const glm::mat4 VK_CLIP_CORRECTION_MAT = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

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
	struct MVP{
		glm::mat4 proj;
		glm::mat4 view;
		glm::mat4 model;
		glm::mat4 mvp;
	}m_mvp;

	KECamera();

};






#endif // !__KE_CAMERA_H__
