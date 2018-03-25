#ifndef _KETRANSFORM_H__
#define _KETRANSFORM_H__
#include "EngineConstans.h"
#define GLM_FORCE_AVX2
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm\glm.hpp"
#include "glm\matrix.hpp"
#include "glm\gtx\matrix_operation.hpp"
#include "glm\gtc\matrix_transform.hpp"


struct KETransformComponent
{
public:
	enum { TRIPLE_BUFFER_SIZE = 3 };

	KETransformComponent() :m_transform() {
	
	};
	~KETransformComponent() {
	
	};
	//KETransformComponent(const KETransformComponent&) = delete;
	//KETransformComponent& operator =(const KETransformComponent&) = delete;

	void SetLocalTransform(const glm::mat4& p_transform, uint8_t p_current_buffer = 0) {
		m_transform[p_current_buffer] = p_transform;
	}
	void SetLocalTranslation(const glm::vec3& p_translation, uint8_t p_current_buffer = 0) {
		m_transform[p_current_buffer] = glm::translate(m_transform[p_current_buffer], p_translation);
	}
	void SetScale(const glm::vec3& p_translation, uint8_t p_current_buffer = 0) {
		m_transform[p_current_buffer] = glm::scale(m_transform[p_current_buffer], p_translation);
	}
	glm::mat4 m_transform[TRIPLE_BUFFER_SIZE];

private:
	//Local Transform
};




#endif