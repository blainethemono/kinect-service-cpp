#ifndef __network_service_h__
#define __network_service_h__

#include <vector>
#include <memory>

namespace network
{

struct Response
{
	// sends response syncronously
	virtual void Send(
			const char* data,
			size_t size
			) = 0;

	virtual ~Response() {}
};

struct ServiceCallback
{
	// handles data received in network service, response must be called synchronously if any
	virtual void OnDataReceived(
			const char* data,
			size_t size,
			Response& response
			) = 0;

	virtual ~ServiceCallback() {}
};

struct Service
{
	// starts network service in a separate thread
	virtual void Start(
			ServiceCallback& cb
			) = 0;

	// stops network service, must wait working thread return
	virtual void Stop() = 0;
	
	virtual ~Service() {}
};

// creates websocket network service on the specified port
std::auto_ptr<Service> CreateWebsocketService(
		unsigned short port
		);

}

#endif //__network_service_h__
