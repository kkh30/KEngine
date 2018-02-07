#include "KEWindow.h"
#include <assert.h>
#include <vector>






KEWindow::KEWindow():
	m_width(0),
	m_height(0),
	is_running(false),
	m_SDLWindow(nullptr),
	m_renderer_update_func(std::function<void(void)>()),
	m_title("KEngine-")
{

}

HWND KEWindow::GetNativeWindowHandle() {
	SDL_SysWMinfo window_info;
	SDL_GetWindowWMInfo(m_SDLWindow, &window_info);
	return window_info.info.win.window;
}

void KEWindow::CreateKEWindow(int p_x , int p_y ,int p_width , int p_height, SDL_WindowFlags p_flags)
{

	m_width = p_width;
	m_height = p_height;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		assert(0);
	}

	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
		assert(0);

	}

	
	if ((m_SDLWindow = SDL_CreateWindow(
		m_title.c_str(), p_x, p_y,
		p_width, p_height, p_flags)
		) == NULL) {
		assert(0);

	}

	is_running = true;





}
void KEWindow::Show() {


	while (is_running) {

		//Call Renderer Func
		KECamera::GetCamera().Update();
		m_renderer_update_func();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			switch (event.type) {

			case SDL_QUIT:
				is_running = false;
				break;

			default:
				// Do nothing.
				break;
			}
		}

	}

}



KEWindow::~KEWindow()
{
}



