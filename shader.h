#pragma once
#include "GL/glew.h"
#include <string>
#include <vector>

#include "defines.h"

struct Shader
{
	Shader(const char* vertex_shader_filename, const char* fragment_shader_filename);
	virtual ~Shader();

	void bind();
	void unbind();

	GLuint getShaderId()
	{
		return shaderId;
	}

	GLuint get_location(const GLchar* name)
	{
		return glGetUniformLocation(getShaderId(), name);
	}

private:
	GLuint compile(std::string shader_source, GLenum type);
	std::string parse(const char* filename);
	GLuint createShader(const char* vertex_shader_filename, const char* fragment_shader_filename);
	GLuint shaderId;
	std::vector<GLuint> locations;
};