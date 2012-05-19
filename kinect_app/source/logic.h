#ifndef __kinect_app_logic_h__
#define __kinect_app_logic_h__

#include "sensor_callbacks.h"
#include "network_service.h"
#include "critical_section.h"

#include <Windows.h>
#include <NuiApi.h>

#include <string>

namespace kinect_app
{

class Engine : public SensorCallbacks, network::ServiceCallback
{
public:
	Engine();
	~Engine();

	void StartNetworkService(unsigned short port);

	// SensorCallbacks
	virtual void ConsumeDepthInput(const NUI_IMAGE_FRAME& frame);
	virtual void ConsumeSkeletonInput(const NUI_SKELETON_FRAME& frame);
	virtual void ConsumeColorInput(const NUI_IMAGE_FRAME& frame);

	// ServiceCallback
	virtual void OnDataReceived(const char* data, size_t size, network::Response& response);

private:
	void OnSkeletonRequest(network::Response& response);
	void OnDepthRequest(network::Response& response);
	
	void ComposeDepthString(const BYTE* image, size_t imageSize, std::string& out);
	void ComposeSkeletonString(const NUI_SKELETON_FRAME& frame, std::string& out);

private:
	std::auto_ptr<network::Service> m_ns;

	std::string m_skeletonAsString;
	std::string m_depthAsString;

	std::string m_skeletonOut;
	std::string m_depthOut;

	CriticalSection m_skeletonGuard;
	CriticalSection m_depthGuard;

	ConditionVariable m_skeletonReady;
	ConditionVariable m_depthReady;

	enum RequestProcessingMode
	{
		BlockingMode,    // wait until frame is ready
		NonBlockingMode  // poll frame, get empty response in no frame ready yet
	};
	RequestProcessingMode m_processingMode;
};

}
#endif //__kinect_app_logic_h__
