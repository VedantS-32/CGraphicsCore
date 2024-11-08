#pragma once

#ifdef CGR_PLATFORM_WINDOWS

extern Cgr::Application* Cgr::CreateApplication();

int main(int argc, char** argv)
{
	Cgr::Log::Init();

	auto app = Cgr::CreateApplication();

	app->Run();
}

#endif
