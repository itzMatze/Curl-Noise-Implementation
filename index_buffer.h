#pragma once
#include <GL/glew.h>

#include "defines.h"

struct IndexBuffer
{
	IndexBuffer(void* data, uint32 numIndices, uint8 elementSize)
	{
		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * elementSize, data, GL_STATIC_DRAW);
		num_indices = numIndices;
	}

	virtual ~IndexBuffer()
	{
		glDeleteBuffers(1, &bufferId);
	}

	void bind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferId);
	}

	void unbind()
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	uint64 getNum_indices()
	{
		return num_indices;
	}

private:
	uint64 num_indices;
	GLuint bufferId;
};