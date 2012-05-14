#include "sensor.h"
#include "sensor_callbacks.h"
#include "trace.h"
#include "exception.h"

#include <assert.h>

namespace kinect_app
{

NuiSensor::NuiSensor(SensorCallbacks& callbacks)
	: m_callbacks(callbacks)
{
	KINECT_TRACE_INF("NuiSensor: create");

	KINECT_APP_EXPECT_HR(::NuiCreateSensorByIndex(0, &m_sensor), 1);

	m_events[0] = m_stopEvent.GetWaitableHandle();
	m_events[1] = m_nextDepthFrameEvent.GetWaitableHandle();
	m_events[2] = m_nextColorFrameEvent.GetWaitableHandle();
	m_events[3] = m_skeletonTrackingEvent.GetWaitableHandle();
	
	KINECT_APP_EXPECT_HR(m_sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH_AND_PLAYER_INDEX | NUI_INITIALIZE_FLAG_USES_SKELETON | NUI_INITIALIZE_FLAG_USES_COLOR), 1);

	KINECT_TRACE_INF("NuiSensor: created");
}

NuiSensor::~NuiSensor()
{
	KINECT_TRACE_INF("NuiSensor destroy");

	m_sensor->NuiShutdown();
	m_sensor->Release();

	KINECT_TRACE_INF("NuiSensor destroyed");
}

void NuiSensor::EnableSkeletonTracking()
{
	KINECT_TRACE_INF("Enable skeleton tracking..");

	KINECT_APP_EXPECT_HR(
		m_sensor->NuiSkeletonTrackingEnable(
			m_skeletonTrackingEvent.GetWaitableHandle(),
			0
		), 3);

	KINECT_TRACE_INF("Skeleton tracking enabled");
}

void NuiSensor::OpenImageColorStream()
{
	KINECT_TRACE_INF("Enable color tracking..");

	KINECT_APP_EXPECT_HR(
		m_sensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_COLOR,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			m_nextColorFrameEvent.GetWaitableHandle(),
			&m_streamHandle
		), 4);

	KINECT_TRACE_INF("Color tracking enabled");
}

void NuiSensor::OpenImageDepthStream()
{
	KINECT_TRACE_INF("Enable depth tracking..");

	KINECT_APP_EXPECT_HR(
		m_sensor->NuiImageStreamOpen(
			NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX,
			NUI_IMAGE_RESOLUTION_640x480,
			0,
			2,
			m_nextDepthFrameEvent.GetWaitableHandle(),
			&m_depthStreamHandle
		), 5);

	KINECT_TRACE_INF("Depth tracking enabled");
}

bool NuiSensor::WaitEvents(event_mask& kind)
{
	m_index = ::WaitForMultipleObjects(sizeof(m_events)/sizeof(m_events[0]), m_events, FALSE, 100);
	
	switch (m_index)
	{
    case WAIT_TIMEOUT:
		kind = Timeout;
        break;
			
	case WAIT_OBJECT_0:
		assert(false); // @todo
		break;

    case WAIT_OBJECT_0 + 1:
		kind = Depth;
        break;

    case WAIT_OBJECT_0 + 2:
		kind = Color;
        break;

    case WAIT_OBJECT_0 + 3:
		kind = Skeleton;
        break;
	}

    return m_index != WAIT_OBJECT_0;
}
	
bool NuiSensor::DispatchEvents()
try
{
	switch (m_index)
	{
    case WAIT_TIMEOUT:
        break;
			
	case WAIT_OBJECT_0:
		return false;
		break;

    case WAIT_OBJECT_0 + 1:
		this->DispatchDepthAlert();
        break;

    case WAIT_OBJECT_0 + 2:
        this->DispatchColorAlert();
        break;

    case WAIT_OBJECT_0 + 3:
		this->DispatchSkeletonAlert();
        break;
	}

	return true;
}
catch (unsigned short)
{
	KINECT_TRACE_ERR("Dispatch failed"); // @todo: clarify details

	return true; // however we continue our adventure anyway :)
}

void NuiSensor::DispatchDepthAlert()
{
	NUI_IMAGE_FRAME imageFrame;

	KINECT_APP_EXPECT_HR(m_sensor->NuiImageStreamGetNextFrame(m_depthStreamHandle, 0, &imageFrame), 10);

	m_callbacks.ConsumeDepthInput(imageFrame);

	m_sensor->NuiImageStreamReleaseFrame(m_depthStreamHandle, &imageFrame);
}
	
void NuiSensor::DispatchColorAlert()
{
	NUI_IMAGE_FRAME imageFrame;

	KINECT_APP_EXPECT_HR(m_sensor->NuiImageStreamGetNextFrame(m_streamHandle, 0, &imageFrame), 11);

	m_callbacks.ConsumeColorInput(imageFrame);

	m_sensor->NuiImageStreamReleaseFrame(m_streamHandle, &imageFrame);
}

void NuiSensor::DispatchSkeletonAlert()
{
	NUI_SKELETON_FRAME skeletonFrame = {0};

	if (SUCCEEDED(m_sensor->NuiSkeletonGetNextFrame(0, &skeletonFrame)))
	{
		m_callbacks.ConsumeSkeletonInput(skeletonFrame);
	}
	else
	{
		KINECT_TRACE_ERR("Error on skeleton alert");
		 // @todo: decide
	}
}

void NuiSensor::Shutdown()
{
	KINECT_TRACE_INF("NuiSensor shutdown");

	m_stopEvent.Set();
}

}
