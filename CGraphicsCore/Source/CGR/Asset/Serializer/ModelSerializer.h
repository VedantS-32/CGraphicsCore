#pragma once

#include "CGR/Renderer/Model.h"

namespace Cgr
{
	class CGR_API ModelSerializer
	{
	public:
		ModelSerializer(Ref<Model> Model);

		void Serialize(const std::filesystem::path& path);
		void Deserialize(const std::filesystem::path& path);

	private:
		Ref<Model> m_Model;
	};
}