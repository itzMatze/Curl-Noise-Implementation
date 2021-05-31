#pragma once
#include <iostream>

#include "index_buffer.h"
#include "shader.h"
#include "vertex_buffer.h"
#include "glm.hpp"

struct Material
{
	glm::vec3 diffuse;
	glm::vec3 specular;
	glm::vec3 emissive;
	float shininess;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex>& vertices, uint64 num_vertices, std::vector<uint32>& indices, uint64 num_indices, Material material, Shader* shader)
	{
		this->material = material;
		this->shader = shader;

		index_buffer = new IndexBuffer(indices.data(), num_indices, sizeof(indices[0]));
		vertex_buffer = new VertexBuffer(vertices.data(), num_vertices);
	}
	~Mesh()
	{
		delete vertex_buffer;
		delete index_buffer;
	}

	void render()
	{
		vertex_buffer->bind();
		index_buffer->bind();
		shader->bind();
		glUniform3fv(shader->get_location("color"), 1, &material.diffuse.x);
		//glUniform3fv(shader->get_location("u_specular"), 1, &material.specular.x);
		//glUniform3fv(shader->get_location("u_emissive"), 1, &material.emissive.x);
		//glUniform1f(shader->get_location("u_shininess"), material.shininess);
		glDrawElements(GL_TRIANGLES, index_buffer->getNum_indices(), GL_UNSIGNED_INT, 0);
	}

private:
	VertexBuffer* vertex_buffer;
	IndexBuffer* index_buffer;
	Shader* shader;
	Material material;
};

class Model
{
public:
	void init(const char* filename, Shader* shader)
	{
		std::ifstream input = std::ifstream(filename, std::ios::in | std::ios::binary);
		if (!input.is_open())
		{
			std::cout << "Could not open file!" << std::endl;
			return;
		}
		uint64 num_meshes = 0;
		input.read(reinterpret_cast<char*>(&num_meshes), sizeof(uint64));

		for (unsigned int i = 0; i < num_meshes; i++)
		{
			Material m;
			std::vector<Vertex> vertices;
			uint64 num_vertices = 0;
			std::vector<uint32> indices;
			uint64 num_indices = 0;

			input.read(reinterpret_cast<char*>(&m), sizeof(Material));
			input.read(reinterpret_cast<char*>(&num_vertices), sizeof(uint64));
			input.read(reinterpret_cast<char*>(&num_indices), sizeof(uint64));

			for (unsigned int j = 0; j < num_vertices; j++)
			{
				Vertex vertex;
				input.read(reinterpret_cast<char*>(&vertex.position.x), sizeof(float));
				input.read(reinterpret_cast<char*>(&vertex.position.y), sizeof(float));
				input.read(reinterpret_cast<char*>(&vertex.position.z), sizeof(float));
				input.read(reinterpret_cast<char*>(&vertex.normal.x), sizeof(float));
				input.read(reinterpret_cast<char*>(&vertex.normal.y), sizeof(float));
				input.read(reinterpret_cast<char*>(&vertex.normal.z), sizeof(float));
				vertex.position.x /= FEET_TO_METER;
				vertex.position.y /= FEET_TO_METER;
				vertex.position.z /= FEET_TO_METER;
				vertex.color.r = m.diffuse.r;
				vertex.color.g = m.diffuse.g;
				vertex.color.b = m.diffuse.b;
				vertex.color.a = 1.0f;
				vertices.push_back(vertex);
			}

			for (unsigned int j = 0; j < num_indices; j++)
			{
				uint32 index;
				input.read(reinterpret_cast<char*>(&index), sizeof(uint32));
				indices.push_back(index);
			}

			Mesh* mesh = new Mesh(vertices, num_vertices, indices, num_indices, m, shader);
			meshes.push_back(mesh);
		}
		input.close();
	}

	void render()
	{
		for (Mesh* mesh : meshes)
		{
			mesh->render();
		}
	}

	~Model()
	{
		for (Mesh* mesh : meshes)
		{
			delete mesh;
		}
	}
private:
	std::vector<Mesh*> meshes;
};