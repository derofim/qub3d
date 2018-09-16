#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <viking/SDLWindow.hpp>
#include <viking/IRenderer.hpp>
#include <viking/IUniformBuffer.hpp>
#include <viking/IComputePipeline.hpp>
#include <viking/IComputeProgram.hpp>

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>

#include "camera.hpp"
#include "chunk.hpp"
#include "worldRenderer.hpp"

using namespace viking;

#define RESOLVE_ASSET_PATH(path) ASSETS_PATH path

int main(int argc, char *argv[])
{
	const RenderingAPI renderingAPI = RenderingAPI::GL3;
	const WindowingAPI windowAPI = WindowingAPI::SDL;

	IWindow *window = IWindow::createWindow(WindowDescriptor("Sandblox Client", 1280, 720), windowAPI, renderingAPI);

	viking::IRenderer* renderer = IRenderer::createRenderer(renderingAPI);
	renderer->start();

	qub3d::Camera cameraController((SDL_Window*) window->getNativeWindowHandle());
	cameraController.projectionMatrix = glm::perspective(45.f, 1280.f / 720.f, 0.1f, 100.f);

	qub3d::WorldRenderer worldRenderer;
	worldRenderer.initalize(renderer, &cameraController);
	worldRenderer.createChunk();

	bool wasEscapeJustPressed = false,
		 freeCursor = false;

	while (window->isRunning())
	{
		window->poll();
		
		bool currentEscapeValue = SDL_GetKeyboardState(nullptr)[SDL_SCANCODE_ESCAPE];
		if (currentEscapeValue && !wasEscapeJustPressed) 
		{
			freeCursor = !freeCursor;
			SDL_ShowCursor(freeCursor);
		}
		wasEscapeJustPressed = currentEscapeValue;

		if(!freeCursor)
			cameraController.update(1000.f / 60.f);

		worldRenderer.render();
		renderer->render();

		window->swapBuffers();
	}

	delete window;
	delete renderer;

	return 0;
}
