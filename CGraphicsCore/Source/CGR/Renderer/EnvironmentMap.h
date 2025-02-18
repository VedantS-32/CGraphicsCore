#pragma once

#include "CGR/Core/Core.h"

namespace Cgr
{
	class Camera;

	enum class CubeMapSide
	{
		PositiveX,
		NegativeX,
		NegativeY,
		PositiveY,
		PositiveZ,
		NegativeZ,
	};

	class CGR_API CubeMap
	{
	public:
		virtual ~CubeMap() = default;

		virtual void Bind() = 0;
		virtual void UnBind() = 0;

		virtual void Render(Camera& camera) = 0;

		static Ref<CubeMap> Create();
	};
}