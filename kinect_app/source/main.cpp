#include "app.h"

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

struct LeaksReport
{
	~LeaksReport()
	{
		_CrtDumpMemoryLeaks();
	}
};

LeaksReport Leaks;

namespace
{

class ConsoleHandler
{
public:
	void Init(kinect_app::ServiceApp& app)
	{
		m_app = &app;
	}

	BOOL OnEvent(DWORD eventType)
	{
		switch (eventType)
		{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			m_app->Stop();
			break;
		}

		return TRUE;
	}

private:
	kinect_app::ServiceApp* m_app;
};

ConsoleHandler ConsoleCtrl;

BOOL WINAPI HandlerRoutine(DWORD eventType)
{
	return ConsoleCtrl.OnEvent(eventType);
}

}

int main(int argc, char** argv)
{
	if (strcmp(argv[1], "install") == 0)
	{
		kinect_app::InstallService();
		return 0;
	}

	if (strcmp(argv[1], "uninstall") == 0)
	{
		kinect_app::UninstallService();
		return 0;
	}

	if (strcmp(argv[1], "console") == 0)
	{
		kinect_app::app_flags::Mask settings(0);

		if (argc > 2)
		{
			if (strstr(argv[2], "d"))
			{
				settings |= kinect_app::app_flags::Depth;
			}
			if (strstr(argv[2], "c"))
			{
				settings |= kinect_app::app_flags::Color;	
			}
			if (strstr(argv[2], "s"))
			{
				settings |= kinect_app::app_flags::Skeleton;
			}
		}
		else
		{
			settings = kinect_app::app_flags::Default;
		}

		std::auto_ptr<kinect_app::ServiceApp> app(new kinect_app::App(settings));

		app->Init();

		ConsoleCtrl.Init(*app);

		SetConsoleCtrlHandler(&HandlerRoutine, TRUE);

		app->Run();

		app->Deinit();

		return 0;
	}

	std::auto_ptr<kinect_app::ServiceApp> app(new kinect_app::App(kinect_app::app_flags::Default)); // @todo:
	kinect_app::StartService(app);

	return 0;
}
