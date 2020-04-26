#include "pch.h"
#include "ext/Oidn.h"

#include <OpenImageDenoise/oidn.hpp>

atlas::ext::Oidn::Oidn(size_t width, size_t height)
	: width(width), height(height), outputImage(width *height)
{}

void atlas::ext::Oidn::launch(Buffer &buffer)
{
	denoisingTime.start();

	oidn::DeviceRef device = oidn::newDevice();
	device.commit();

	oidn::FilterRef filter = device.newFilter("RT");
	filter.setImage("color", buffer.image.data(), oidn::Format::Float3, width, height);
	filter.setImage("albedo", buffer.albedo.data(), oidn::Format::Float3, width, height);
	filter.setImage("normal", buffer.normal.data(), oidn::Format::Float3, width, height);
	filter.setImage("output", outputImage.data(), oidn::Format::Float3, width, height);
	filter.commit();

	filter.execute();

	denoisingTime.stop();
}

void atlas::ext::Oidn::asyncLaunch(Buffer &buffer)
{
	inProgress = true;
	denoisingTime.start();

}