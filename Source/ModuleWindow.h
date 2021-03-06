#ifndef __MODULE_WINDOW_H__
#define __MODULE_WINDOW_H__

#include "Module.h"
#include "SDL/include/SDL.h"

class ModuleWindow : public Module
{
public:

	ModuleWindow(bool start_enabled = true);
	~ModuleWindow();

	bool Init(JSON_Object* jObject);
	bool CleanUp();

	void SetTitle(const char* title) const;

	void SetScreenScale(uint scale);
	uint GetScreenScale() const;
	void SetWindowWidth(uint width);
	uint GetWindowWidth() const;
	void SetWindowHeight(uint height);
	uint GetWindowHeight() const;
	void UpdateWindowSize() const;
	void GetScreenSize(uint& width, uint& height) const;
	uint GetRefreshRate() const;

	void SetFullscreenWindow(bool fullscreen);
	bool GetFullscreenWindow() const;
	void SetFullDesktopWindow(bool fullDesktop);
	bool GetFullDesktopWindow() const;
	void SetResizableWindow(bool resizable);
	bool GetResizableWindow() const;
	void SetBorderlessWindow(bool borderless);
	bool GetBorderlessWindow() const;
	void SetWindowBrightness(float brightness) const;
	float GetWindowBrightness() const;

	void SaveStatus(JSON_Object*) const;
	void LoadStatus(const JSON_Object*);

public:
	
	SDL_Window* window = nullptr; // The window we'll be rendering to	
	SDL_Surface* screen_surface = nullptr; // The surface contained by the window

private:

	uint width = 0;
	uint height = 0;
	uint scale = 1;

	bool fullscreen = false;
	bool resizable = false;
	bool borderless = false;
	bool fullDesktop = false;
};

#endif