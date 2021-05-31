#include "shader.h"
#include <fstream>
#include <iostream>

Shader::Shader(const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	shaderId = createShader(vertex_shader_filename, fragment_shader_filename);
}

Shader::~Shader()
{
	glDeleteProgram(shaderId);
}

void Shader::bind()
{
	glUseProgram(shaderId);
}

void Shader::unbind()
{
	glUseProgram(0);
}

GLuint Shader::compile(std::string shader_source, GLenum type)
{
	GLuint id = glCreateShader(type);
	const char* src = shader_source.c_str();
	glShaderSource(id, 1, &src, 0);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
	{
		int length = 0;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = new char[length];
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Shader compilation error: " << message << std::endl;
		delete[] message;
		return 0;
	}
	return id;
}

std::string Shader::parse(const char* filename)
{
	FILE* file;
	if (fopen_s(&file, filename, "rb") != 0)
	{
		std::cout << "File " << filename << " not found" << std::endl;
		return "";
	}
	std::string contents;
	fseek(file, 0, SEEK_END);
	size_t filesize = ftell(file);
	rewind(file);
	contents.resize(filesize);

	fread(&contents[0], 1, filesize, file);
	fclose(file);

	return contents;
}

GLuint Shader::createShader(const char* vertex_shader_filename, const char* fragment_shader_filename)
{
	std::string vertex_shader_source = parse(vertex_shader_filename);
	std::string fragment_shader_source = parse(fragment_shader_filename);

	GLuint program = glCreateProgram();
	GLuint vs = compile(vertex_shader_source, GL_VERTEX_SHADER);
	GLuint fs = compile(fragment_shader_source, GL_FRAGMENT_SHADER);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);

#ifdef _RELEASE
	glDetachShader(program, vs);
	glDetachShader(program, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);
#endif

	return program;
}

