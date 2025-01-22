#pragma once
#include <string>

namespace Cgr
{
	namespace Utils
	{
		static std::string ExtractNameFromFilepath(const std::string& filePath)
		{
			auto lastSlash = filePath.find_last_of("/\\");
			lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
			auto lastDot = filePath.rfind('.');
			auto count = lastDot == std::string::npos ? filePath.size() - lastSlash : lastDot - lastSlash;
			
			return filePath.substr(lastSlash, count);
		}
	}
}