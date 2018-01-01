#ifndef __KEWINDOW_H__
#define __KEWINDOW_H__


#define SDL_MAIN_HANDLED  //Make Sure SDL Does Not Mess Up the "main" Func
#include <SDL\SDL.h>
#include <SDL\SDL_syswm.h>
#include <functional>
#include "EngineConstans.h"


#ifdef VULKAN_RENDERER
#include "KEVulkanRHI\vulkan\vulkan.h"
#endif // VULKAN_RENDERER



//KEWindow Implemented In A Singleton Way,
//As Engine Assumes There Will Use Only One Window.
class KEWindow
{
public:

	static KEWindow& GetWindow() {
		static KEWindow l_window;
		return l_window;
	}


	void CreateKEWindow(int p_x , int p_y , int p_width,int p_height,SDL_WindowFlags p_flags);
	void Show();
	inline void SetRendererFunc(std::function<void(void)> p_renderer_func) { m_renderer_update_func = p_renderer_func; }
	~KEWindow();

private:
	KEWindow();
	SDL_Window * m_SDLWindow;
	int m_width;
	int m_height;
	bool is_running;
	std::function<void(void)> m_renderer_update_func;
#ifdef VULKAN_RENDERER


#endif // VULKAN_RENDERER


};


#endif
