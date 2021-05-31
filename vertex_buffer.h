#pragma once
#include <GL/glew.h>

#include "defines.h"

struct VertexBuffer
{
	VertexBuffer(void* data, uint32 numVertices)
	{
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ARRAY_BUFFER, numVertices * sizeof(Vertex), data, GL_STATIC_DRAW);
		num_vertices = numVertices;

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(struct Vertex, position.x));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*) offsetof(struct Vertex, color.r));

		glBindVertexArray(0);
	}

	virtual ~VertexBuffer()
	{
		glDeleteBuffers(1, &bufferId);
	}

	void bind()
	{
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
	}

	void unbind()
	{
		glBindVertexArray(0);
	}

	void update(std::vector<Vertex> vertices)
	{
		bind();
		glBufferSubData(GL_ARRAY_BUFFER, 0, num_vertices * sizeof(Vertex), vertices.data());
	}

	uint64 getNum_vertices()
	{
		return num_vertices;
	}

private:
	uint64 num_vertices;
	GLuint bufferId;
	GLuint vao;
};