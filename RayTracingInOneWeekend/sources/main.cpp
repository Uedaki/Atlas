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

#include "Materials.h"
#include "Window.h"

#include "../Acheron.h"

constexpr uint32_t WIDTH = 720;
constexpr uint32_t HEIGHT = 480;
constexpr uint32_t NBR_SAMPLE = 64;

struct Color
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;
};

#define ACHERON 0

atlas::Scene create_scene()
{
	atlas::Scene scene;

	atlas::Material &mapMaterial = scene.defineMaterial<Lambert>(glm::vec3(0.5, 0.5, 0.5));
	scene.defineShape<atlas::Sphere>(glm::vec3(0, -1000, 0), 1000.f, &mapMaterial);

	for (int a = -11; a < 11; a++)
	{
		for (int b = -11; b < 11; b++)
		{
			float choose_mat = atlas::Tools::rand();
			glm::vec3 center(a + 0.9 * atlas::Tools::rand(), 0.2, b + 0.9 * atlas::Tools::rand());
			if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9)
			{
				atlas::Material *material = nullptr;
				if (choose_mat < 0.8)
				{
					material = &scene.defineMaterial<Lambert>(glm::vec3(atlas::Tools::rand() * atlas::Tools::rand(),
						atlas::Tools::rand() * atlas::Tools::rand(),
						atlas::Tools::rand() * atlas::Tools::rand()));
				}
				else if (choose_mat < 0.95)
				{
					material = &scene.defineMaterial<Metal>(glm::vec3(0.5f * (1.0f + atlas::Tools::rand()),
						0.5f * (1.0f + atlas::Tools::rand()),
						0.5f * (1.0f + atlas::Tools::rand())), 0.5f * atlas::Tools::rand());
				}
				else
				{
					material = &scene.defineMaterial<Dielectric>(1.5f);
				}
				scene.defineShape<atlas::Sphere>(center, 0.2f, material);
			}
		}
	}

	atlas::Material &dielectric = scene.defineMaterial<Dielectric>(1.3f);
	scene.defineShape<atlas::Sphere>(glm::vec3(0, 1, 0), 1.f, &dielectric);

	atlas::Material &lambert = scene.defineMaterial<Lambert>(glm::vec3(0.3, 0.2, 0.1));
	scene.defineShape<atlas::Sphere>(glm::vec3(-4, 1, 0), 1.f, &lambert);

	atlas::Material &metal = scene.defineMaterial<Metal>(glm::vec3(0.7, 0.6, 0.5), 0.f);
	scene.defineShape<atlas::Sphere>(glm::vec3(4, 1, 0), 1.f, &metal);

	return (scene);
}

void copyImageToBackBuffer(Color *dst, const std::vector<glm::vec3> &src)
{
	for (size_t i = 0; i < WIDTH * HEIGHT; i++)
	{
		size_t x = i % WIDTH;
		size_t vy = i / WIDTH; // Vulkan image buffer index
		size_t py = HEIGHT - i / WIDTH - 1; // Pathtracing image buffer index

		dst[x + vy * WIDTH].a = 255;
		dst[x + vy * WIDTH].r = static_cast<unsigned char>(src[x + py * WIDTH].r * 255.99);
		dst[x + vy * WIDTH].g = static_cast<unsigned char>(src[x + py * WIDTH].g * 255.99);
		dst[x + vy * WIDTH].b = static_cast<unsigned char>(src[x + py * WIDTH].b * 255.99);
	}
}

int main()
{
	try
	{
		bool isDenoised = false;
		atlas::Scene scene = create_scene();
#if ACHERON
		Acheron session(scene, WIDTH, HEIGHT, NBR_SAMPLE);
#else
		atlas::rendering::Session session(scene, WIDTH, HEIGHT, NBR_SAMPLE);
#endif
		//atlas::ext::Oidn denoiser(WIDTH, HEIGHT);

		glm::vec3 pos(13, 2, 3);
		glm::vec3 target(0, 0, 0);
		glm::vec3 up(0, 1, 0);
		float focusDistance = 10;
		float aperture = 0.1f;
		atlas::Camera camera(pos, target, up, 20, session.getOutputImageRatio());
		//camera.enableDefocusBlur(aperture, focusDistance);

		atlas::Buffer output(WIDTH * HEIGHT);

		vk::Window window(WIDTH, HEIGHT);
		
		//session.launch(camera, 3);
#if ACHERON
		session.launch(camera, output);
#else
		session.launch(camera, 4);
#endif
		while (window.isWindowOpen())
		{
			if (window.startFrame())
			{
				Color *pixels = reinterpret_cast<Color *>(window.getCurrentBuffer());
				if (!isDenoised)
				{
					if (session.isWorking())
					{
#if !ACHERON
						session.fetchResult(output);
#endif
						copyImageToBackBuffer(pixels, output.image);
					}
					else
					{
						//denoiser.launch(output);
						copyImageToBackBuffer(pixels, output.image);
						isDenoised = true;
					}
				}
				else
					copyImageToBackBuffer(pixels, output.image);
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