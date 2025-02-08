#pragma once

#include "CGR/Core/Core.h"
#include <glm/glm.hpp>

namespace Cgr::Math
{
	CGR_API bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}