#pragma once
#include <cstdint>
#include "glm.hpp"

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Vertex
{
	glm::vec3 position;
	glm::vec4 color;
	glm::vec3 normal;
};