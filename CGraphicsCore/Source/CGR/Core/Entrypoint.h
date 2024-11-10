#pragma once

extern Cgr::Application* Cgr::CreateApplication();

int main(int argc, char** argv)
{
	Cgr::Log::Init();

	auto app = Cgr::CreateApplication();

	app->Run();
}
