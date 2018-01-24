#ifndef _COMPONENT_H__
#define _COMPONENT_H__
#include <array>
#include <unordered_map>
#include "KEEntity.h"
#include "KERenderComponent.h"

using ComponentID = uint32_t;



template<typename T>
class System
{
	
	

private:
	System() {
	
	};

public:

	static System& GetSystem() {
		static System l_system;
		return l_system;
	}

	System(const System&) = delete;
	System operator=(const System&) = delete;

	virtual ~System() {
	
	};

	//template<typename T>
	__forceinline void AddEntityComponent(Entity e,T p_t) {
		m_components.insert(std::pair<Entity, T>(e,p_t));
	};

	//template<typename T>
	__forceinline T GetEntityComponent(Entity e) {
		return m_components[e];
	};

private:

	std::unordered_map<Entity, T> m_components;

};

using RenderSystem = System<KRenderComponent>;

#endif // !_COMPONENT_H__
