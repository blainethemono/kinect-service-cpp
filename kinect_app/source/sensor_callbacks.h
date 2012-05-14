#ifndef __kinect_app_sensor_callbacks_h__
#define __kinect_app_sensor_callbacks_h__

#include <Windows.h>
#include <NuiApi.h>

namespace kinect_app
{

struct SensorCallbacks
{
	virtual void ConsumeColorInput(const NUI_IMAGE_FRAME& frame) {}
	virtual void ConsumeDepthInput(const NUI_IMAGE_FRAME& frame)  {}
	virtual void ConsumeSkeletonInput(const NUI_SKELETON_FRAME& frame) {}

	~SensorCallbacks() {}
};

}

#endif //__kinect_app_sensor_callbacks_h__

