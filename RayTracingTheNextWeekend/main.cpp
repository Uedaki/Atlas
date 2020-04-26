#include <glm/glm.hpp>

#include <iostream>
#include <stdexcept>

#include "Atlas/Buffer.h"
#include "Atlas/Camera.h"
#include "Atlas/ext/Oidn.h"
#include "Atlas/Rendering/Session.h"
#include "Atlas/Scene.h"
#include "Atlas/Sphere.h"
#include "Atlas/Tools.h"
#include "Atlas/Rect.h"
#include "Atlas/Box.h"

#include "NoiseTexture.h"
#include "ImageTexture.h"
#include "CheckerTexture.h"
#include "Materials.h"
#include "Window.h"

constexpr uint32_t WIDTH = 720;
constexpr uint32_t HEIGHT = 480;
constexpr uint32_t NBR_SAMPLE = 1024;

NoiseTexture::Noise NoiseTexture::noise;

struct Color
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

atlas::Scene create_scene()
{
	atlas::Scene scene;

	atlas::Material &red = scene.defineMaterial<Lambert>(atlas::MaterialInput(0.65, 0.05, 0.05));
	atlas::Material &white = scene.defineMaterial<Lambert>(atlas::MaterialInput(0.73, 0.73, 0.73));
	atlas::Material &green = scene.defineMaterial<Lambert>(atlas::MaterialInput(0.12, 0.45, 0.15));
	atlas::Material &light = scene.defineMaterial<Light>(atlas::MaterialInput(20, 20, 20));

	scene.defineShape<atlas::RectYZ>(0, 555, 0, 555, 555, &green);
	scene.defineShape<atlas::RectYZ>(0, 555, 0, 555, 0, &red);
	scene.defineShape<atlas::RectXZ>(213, 343, 227, 332, 554, &light);
	scene.defineShape<atlas::RectXZ>(0, 555, 0, 555, 0, &white);
	scene.defineShape<atlas::RectXZ>(0, 555, 0, 555, 555, &white);
	scene.defineShape<atlas::RectXY>(0, 555, 0, 555, 555, &white);

	scene.defineShape<atlas::Box>(glm::vec3(130, 0, 65), glm::vec3(295, 165, 230), &white);
	scene.defineShape<atlas::Box>(glm::vec3(265, 0, 295), glm::vec3(430, 330, 460), &white);

	return (scene);
}

void copyImageToBackBuffer(Color *dst, const std::vector<glm::vec3> &src)
{
	for (size_t i = 0; i < WIDTH * HEIGHT; i++)
	{
		size_t x = i % WIDTH;
		size_t vy = i / WIDTH; // Vulkan image buffer index
		size_t py = HEIGHT - i / WIDTH - 1; // Pathtracing image buffer index

		float r = src[x + py * WIDTH].r > 1 ? 1 : src[x + py * WIDTH].r;
		float g = src[x + py * WIDTH].g > 1 ? 1 : src[x + py * WIDTH].g;
		float b = src[x + py * WIDTH].b > 1 ? 1 : src[x + py * WIDTH].b;

		dst[x + vy * WIDTH].a = 255;
		dst[x + vy * WIDTH].r = static_cast<unsigned char>(r * 255);
		dst[x + vy * WIDTH].g = static_cast<unsigned char>(g * 255);
		dst[x + vy * WIDTH].b = static_cast<unsigned char>(b * 255);
	}
}

int main()
{
	try
	{
		bool isDenoised = false;
		atlas::Scene scene = create_scene();
		atlas::rendering::Session session(scene, WIDTH, HEIGHT, NBR_SAMPLE);
		session.setFar(10000.f);

		atlas::ext::Oidn denoiser(WIDTH, HEIGHT);

		glm::vec3 pos(278, 278, -800);
		glm::vec3 target(278, 278, 0);
		glm::vec3 up(0, 1, 0);
		float focusDistance = 10;
		float aperture = 0;
		atlas::Camera camera(pos, target, up, 40, session.getOutputImageRatio());
		camera.enableDefocusBlur(aperture, focusDistance);

		atlas::Buffer output(WIDTH * HEIGHT);

		vk::Window window(WIDTH, HEIGHT);
		session.launch(camera, 3);
		while (window.isWindowOpen())
		{
			if (window.startFrame())
			{
				Color *pixels = reinterpret_cast<Color *>(window.getCurrentBuffer());
				if (!isDenoised)
				{
					if (session.isWorking())
					{
						session.fetchResult(output);
						copyImageToBackBuffer(pixels, output.image);
					}
					else
					{
						denoiser.launch(output);
						copyImageToBackBuffer(pixels, denoiser.getDenoisedOutput());
						isDenoised = true;
					}
				}
				else
					copyImageToBackBuffer(pixels, denoiser.getDenoisedOutput());
				window.render();
			}
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "[Error] " << e.what() << std::endl;
		return (1);
	}
	return (0);
}