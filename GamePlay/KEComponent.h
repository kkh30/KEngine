#ifndef _COMPONENT_H__
#define _COMPONENT_H__
#include <array>
#include <unordered_map>
#include "KEEntity.h"

using ComponentID = uint32_t;



template<typename T>
class ComponentManager
{
public:
	ComponentManager() {
	
	};

	virtual ~ComponentManager() {
	
	};

	template<typename T>
	__forceinline void AddEntityComponent(Entity e,T p_t) {
		m_components.insert(std::pair<Entity, T>(e,p_t));
	};

	template<typename T>
	__forceinline T GetEntityComponent(Entity e) {
		return m_components[e];
	};

private:

	std::unordered_map<Entity, T> m_components;

};

#endif // !_COMPONENT_H__
