#include <WinSock2.h>
#include <Windows.h>
#include <process.h>

#include "network_service.h"
#include "api.h"
#include "event.h"
#include "trace.h"

#include <libwebsockets.h>

#include <memory>
#include <sstream>

#include <map>

namespace network
{

// Application class binds lws pure C callback with c++ object idioma :)
class Application
{
public:
	Application();

	~Application();
	
	void Initialize(ServiceCallback* cb);

	void Deinitialize();

	static int WebSocketCallback(
		libwebsocket_context* context,
		libwebsocket* wsi,
		libwebsocket_callback_reasons reason,
		void* user,
		void* in,
		size_t len);

private:
	int Callback(
		libwebsocket_context* context,
		libwebsocket* wsi,
		libwebsocket_callback_reasons reason,
		void* user,
		void* in,
		size_t len);

private:
	ServiceCallback* m_callback;
};


Application App;


Application::Application(): m_callback(0)
{
}

Application::~Application()
{
}
	
void Application::Initialize(ServiceCallback* cb)
{
	m_callback = cb;
}

void Application::Deinitialize()
{
}

int Application::WebSocketCallback(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* user, void* in, size_t len)
{
	return App.Callback(context, wsi, reason, user, in, len);
}

int Application::Callback(libwebsocket_context* context, libwebsocket* wsi, libwebsocket_callback_reasons reason, void* /*user*/, void* in, size_t len)
{
	KINECT_TRACE_DBG("WebSocketCallback");

	switch (reason)
	{
	case LWS_CALLBACK_ESTABLISHED:
		KINECT_TRACE_INF("Connection establised");
		break;

	case LWS_CALLBACK_BROADCAST:
		KINECT_TRACE_INF("Broadcast");
		break;

	case LWS_CALLBACK_RECEIVE:
		{
			KINECT_TRACE_INF("Data received");
			
			static const size_t PaddingsSize      = LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING;
			static const size_t DefaultBufferSize = PaddingsSize + 1024*1024;

			struct ResponseImpl : public network::Response
			{
				ResponseImpl(libwebsocket* wsi): m_wsi(wsi), m_buffer(DefaultBufferSize)
				{
				}

				virtual void Send(const char* data, size_t size)
				{
					const size_t fullSize = size + PaddingsSize;

					if (m_buffer.size() < fullSize)
					{
						m_buffer.resize(fullSize);
					}

					memcpy(&m_buffer[LWS_SEND_BUFFER_PRE_PADDING], data, size);

					const int ret = ::libwebsocket_write(
							m_wsi,
							&m_buffer[LWS_SEND_BUFFER_PRE_PADDING],
							size,
							LWS_WRITE_TEXT
							); // @todo: handle ret
				}

				libwebsocket* m_wsi;
				std::vector<unsigned char> m_buffer;
			};

			ResponseImpl response(wsi);

			m_callback->OnDataReceived(reinterpret_cast<const char*>(in), len, response);

			KINECT_TRACE_INF("Data handled");

			break;
		}

	case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
		KINECT_TRACE_DBG("Filter protocol");
		break;

	default:
		break;
	}
	return 0;
}
	


class WebsocketService : public Service
{
public:
	WebsocketService(unsigned short port);
	~WebsocketService();

	virtual void Start(ServiceCallback& cb);
	virtual void Stop();

private:
	static unsigned int __stdcall WorkerStub(void* arg);
	void Worker();

private:
	unsigned short m_port;
	volatile bool m_stop;
	kinect_app::Event m_listeningEvent;
	HANDLE m_acceptingThread;
};


WebsocketService::WebsocketService(unsigned short port): m_port(port), m_stop(false)
{
	KINECT_TRACE_INF("Create service..");

	WSADATA wsaData;
	int err = ::WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (err != NO_ERROR)
		throw err;

	KINECT_TRACE_INF("Service created");
}

WebsocketService::~WebsocketService()
{
	KINECT_TRACE_INF("Destroy service..");

    WSACleanup();

	KINECT_TRACE_INF("Service destroyed");
}

void WebsocketService::Start(ServiceCallback& cb)
{
	KINECT_TRACE_INF("Starting service..");

	App.Initialize(&cb);

	m_stop = false;

	unsigned int threadId;
	if (!(m_acceptingThread = (HANDLE)_beginthreadex(NULL, 0, &WebsocketService::WorkerStub, this, 0, &threadId)))
	{
		KINECT_TRACE_ERR("Failed to start working thread");
		throw ::GetLastError();
	}

	m_listeningEvent.Wait();

	KINECT_TRACE_INF("Service started");
}

void WebsocketService::Stop()
{
	KINECT_TRACE_INF("Stop service..");

	m_stop = true;

	::WaitForSingleObject(m_acceptingThread, INFINITE);

	KINECT_TRACE_INF("Service stopped");
}

unsigned int WebsocketService::WorkerStub(void* arg)
{
	try
	{
		(reinterpret_cast<WebsocketService*>(arg))->Worker();
	}
	catch (...) // wtf?
	{
	}
	return 0;
}

static struct libwebsocket_protocols protocols[] =
{
	{
		"kinect_app",
		&Application::WebSocketCallback,
		0
	},
	{
		NULL, NULL, 0
	}
};

void WebsocketService::Worker()
{
	KINECT_TRACE_INF("Worker started");

	m_listeningEvent.Set();

	char interface_name[128] = "";
	const char* iface = NULL;

	libwebsocket_context* context = libwebsocket_create_context(int(m_port), iface, protocols, libwebsocket_internal_extensions, 0, 0, -1, -1, 0);

	if (context == NULL)
	{
		KINECT_TRACE_ERR("libwebsocket init failed");

		return;
	}

	while (!m_stop)
	{
		libwebsocket_service(context, 50);
	}

	libwebsocket_context_destroy(context);

	KINECT_TRACE_INF("Worker finished");
}

std::auto_ptr<Service> CreateWebsocketService(unsigned short port)
{
	return std::auto_ptr<Service>(new WebsocketService(port));
}

}
