#include <iostream>
#include <vector>
#include <fstream>
#include <GL/glew.h>
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glm.hpp"
#include "ext/matrix_transform.hpp"
#include "gtc/matrix_transform.hpp"
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#pragma comment(lib, "SDL2.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(lib, "opengl32.lib")

#define FEET_TO_METER 3.28084

#include "curl_noise.h"
#include "defines.h"
#include "vertex_buffer.h"
#include "shader.h"
#include "fps_camera.h"
#include "floating_camera.h"
#include "index_buffer.h"
#include "mesh.h"


int readModel(std::vector<Vertex>* vertices, uint64* num_vertices, std::vector<uint32>* indices, uint64* num_indices, const std::string path)
{
	std::ifstream input = std::ifstream(path, std::ios::in | std::ios::binary);
	if (!input.is_open())
	{
		std::cout << "Error reading model file." << std::endl;
		return 1;
	}
	input.read(reinterpret_cast<char*>(num_vertices), sizeof(uint64));
	input.read(reinterpret_cast<char*>(num_indices), sizeof(uint64));

	for (uint64 i = 0; i < *num_vertices; i++)
	{
		Vertex vertex;
		input.read(reinterpret_cast<char*>(&vertex.position.x), sizeof(float));
		input.read(reinterpret_cast<char*>(&vertex.position.y), sizeof(float));
		input.read(reinterpret_cast<char*>(&vertex.position.z), sizeof(float));
		vertex.color.r = 1.0f;
		vertex.color.g = 1.0f;
		vertex.color.b = 1.0f;
		vertex.color.a = 1.0f;
		(*vertices).push_back(vertex);
	}

	for (uint64 i = 0; i < *num_indices; i++)
	{
		uint32 index;
		input.read(reinterpret_cast<char*>(&index), sizeof(uint32));
		(*indices).push_back(index);
	}
	return 0;
}

void openGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param)
{
	//std::cout << "[OpenGL Error] " << message << std::endl;
}

void _GLGetError(const char* file, int line, const char* call)
{
	while (GLenum error = glGetError())
		std::cout << "[OpenGL Error] " << glewGetErrorString(error) << " in " << file << ":" << line << " Call: " << call << std::endl;
}

void calculate_new_positions(int i, int j_trace_count, int k_trace_count, int l_trace_count, std::vector<Vertex>* vertices, float radius)
{
	for (int j = 0; j < j_trace_count; j++)
	{
		for (int k = 0; k < k_trace_count; k++)
		{
			// move lines as a whole
#if 1
			int index = l_trace_count * 2.0f * k + (l_trace_count * k_trace_count * 2.0f) * j + (l_trace_count * k_trace_count * j_trace_count * 2.0f) * i;
			Vertex v = (*vertices)[index + l_trace_count * 2.0f - 1.0f];
			v.color.r = 0.0f;
			v.color.g = 0.0f;
			v.color.b = 1.0f;
			(*vertices)[index] = v;
			glm::vec3 p = v.position;
			glm::vec3 flow;
#if 0
			for (int l = 1; l < l_trace_count; l++)
			{
				flow = velocity_field(p);
				p += 0.005f * flow;
				vertices[index + l * 2.0f - 1.0f]
					= Vertex{ p, glm::vec4(static_cast<float>(l) / static_cast<float>(l_trace_count), 0.0f, (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count)), 1.0f) };
				vertices[index + l * 2.0f]
					= Vertex{ p, glm::vec4(static_cast<float>(l) / static_cast<float>(l_trace_count), 0.0f, (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count)), 1.0f) };
			}
			flow = velocity_field(p);
			p += 0.005f * flow;
			vertices[index + l_trace_count * 2.0f - 1.0f]
				= Vertex{ p, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
			// move lines step by step
#else
			float pos[3] = { p.x, p.y, p.z };
			float flowarr[3] = { 0.0f, 0.0f, 0.0f };
			float center[3] = { 0.0f, 0.0f, 0.0f };
			for (int l = 1; l < l_trace_count; l++)
			{
				velocity_field(pos, flowarr, center, radius);
				for (int k = 0; k < 3; k++) pos[k] += 0.005f * flowarr[k];
				p += 0.005f * flow;
				(*vertices)[index + l * 2.0f - 1.0f]
					= Vertex{ glm::vec3(pos[0], pos[1], pos[2]), glm::vec4(static_cast<float>(l) / static_cast<float>(l_trace_count), 0.0f, (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count)), 1.0f) };
				(*vertices)[index + l * 2.0f]
					= Vertex{ glm::vec3(pos[0], pos[1], pos[2]), glm::vec4(static_cast<float>(l) / static_cast<float>(l_trace_count), 0.0f, (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count)), 1.0f) };
			}
			velocity_field(pos, flowarr, center, radius);
			for (int k = 0; k < 3; k++) pos[k] += 0.005f * flowarr[k];
			p += 0.005f * flow;
			(*vertices)[index + l_trace_count * 2.0f - 1.0f]
				= Vertex{ glm::vec3(pos[0], pos[1], pos[2]), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
#endif
#else
			int index = l_trace_count * 2.0f * k + (l_trace_count * k_trace_count * 2.0f) * j + (l_trace_count * k_trace_count * j_trace_count * 2.0f) * i;
			vertices[index] = vertices[index + 1.0f];
			vertices[index].color.r = 0.0f;
			vertices[index].color.b = 1.0f;
			for (int l = 1; l < l_trace_count - 1; l++)
			{
				vertices[index + l * 2.0f - 1.0f] = vertices[index + l * 2.0f + 1.0f];
				vertices[index + l * 2.0f - 1.0f].color.r = static_cast<float>(l) / static_cast<float>(l_trace_count);
				vertices[index + l * 2.0f - 1.0f].color.b = (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count));
				vertices[index + l * 2.0f] = vertices[index + l * 2.0f + 2.0f];
				vertices[index + l * 2.0f].color.r = static_cast<float>(l) / static_cast<float>(l_trace_count);
				vertices[index + l * 2.0f].color.b = (1.0f - static_cast<float>(l) / static_cast<float>(l_trace_count));
			}
			vertices[index + l_trace_count * 2.0f - 3.0f] = vertices[index + l_trace_count * 2.0f - 2.0f] = vertices[index + l_trace_count * 2.0f - 1.0f];
			Vertex v = vertices[index + l_trace_count * 2.0f - 1.0f];
			glm::vec3 p = v.position;
			glm::vec3 flow;
			flow = velocity_field(p);
			p += 0.005f * flow;
			vertices[index + l_trace_count * 2.0f - 1.0f]
				= Vertex{ p, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
#endif
		}
	}
}

int main(int argc, char** argv) {
	// SDL
	SDL_Window* window;
	float width = 1280;
	float height = 800;
	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("Curl Noise Visualisation", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
	// OpenGL
	SDL_GLContext glContext = SDL_GL_CreateContext(window);

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		std::cout << "Error: " << glewGetErrorString(err) << std::endl;
		std::cin.get();
		return -1;
	}
	std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

	glewInit();

	// imgui
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window, glContext);
	ImGui_ImplOpenGL3_Init("#version 450");

	std::vector<Vertex> vertices;
	uint64 num_vertices = 0;

	std::vector<uint32> indices;
	uint64 num_indices = 0;

	readModel(&vertices, &num_vertices, &indices, &num_indices, "models/rotor_blades.bmf");
	for (int i = 0; i < vertices.size(); i++)
	{
		vertices[i].position.x /= FEET_TO_METER;
		vertices[i].position.y /= FEET_TO_METER;
		vertices[i].position.z /= FEET_TO_METER;
	}
	
	IndexBuffer index_buffer_rotor_blades(indices.data(), num_indices, sizeof(indices[0]));
	VertexBuffer vertex_buffer_rotor_blades(vertices.data(), num_vertices);
	vertices.clear();
	indices.clear();
	
	vertices.push_back(Vertex{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(5.95f, 0.0f, 0.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) });
	// debug lines at every unit on the x axis
	/*vertices.push_back(Vertex{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(0.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(1.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(1.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(2.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(2.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(3.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(3.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(4.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(4.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(5.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(5.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(6.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(6.0f, 0.2f, 0.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });

	vertices.push_back(Vertex{ glm::vec3(5.95f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(5.95f, 0.2f, 0.0f), glm::vec4(0.0f, 0.9f, 1.0f, 1.0f) });
	*/
	vertices.push_back(Vertex{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(0.0f, 5.95f, 0.0f), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(0.0f, 0.0f, 0.0f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) });
	vertices.push_back(Vertex{ glm::vec3(0.0f, 0.0f, 5.95f), glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) });
	
	num_vertices = vertices.size();
	VertexBuffer coordinate_vertex_buffer(vertices.data(), num_vertices);
	vertices.clear();
	
	const float line_width = 7.0f;
	const int i_trace_count = 15;
	const int j_trace_count = 15;
	const int k_trace_count = 15;
	const int l_trace_count = 20;
	const float tracing_height = 8.0f;
	const float tracing_width = 15.0f;
	
	for (int i = 0; i < i_trace_count; i++)
	{
		for (int j = 0; j < j_trace_count; j++)
		{
			for (int k = 0; k < k_trace_count; k++)
			{
				for (int l = 0; l < l_trace_count; l++)
				{
					glm::vec3 position((tracing_width / 2.0f) - i * (tracing_width / i_trace_count) + 0.1f,
						(tracing_height / 2.0f) - k * (tracing_height / k_trace_count) - 0.1f * l + 0.1f,
						(tracing_width / 2.0f) - j * (tracing_width / i_trace_count) + 0.1f);
					Vertex top = { position,glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) };
					position.y = (tracing_height / 2.0f) - k * (tracing_height / k_trace_count) - 0.1f * (l + 1.0f) + 0.1f;
					Vertex bottom = { position,glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
					vertices.push_back(top);
					vertices.push_back(bottom);
				}
			}
		}
	}
	num_vertices = vertices.size();

	VertexBuffer tracing_vertex_buffer(vertices.data(), num_vertices);

	Shader shader("shader/basic.vert", "shader/basic.frag");
	shader.bind();

	Model heli_model;
	heli_model.init("models/heli_full.bmf", &shader);

	tracing_vertex_buffer.bind();
	
	glm::mat4 model = glm::mat4(1.0f);

	FloatingCamera camera(90.0f, width, height);
	camera.translate(glm::vec3(0.0f, 0.0f, -20.0f));
	camera.update();
	
	glUniformMatrix4fv(shader.get_location("u_mvp"), 1, GL_FALSE, &model[0][0]);
		
	uint64 perfCounterFrequency = SDL_GetPerformanceFrequency();
	uint64 lastCounter = SDL_GetPerformanceCounter();
	float delta = 0.0f;
	bool close = false;
	bool button_w = false;
	bool button_a = false;
	bool button_s = false;
	bool button_d = false;
	bool button_n = false;
	bool button_h = false;
	bool button_c = false;
	bool button_q = false;
	bool button_shift_l = false;
	bool button_control_l = false;
	bool button_space = false;
	float camera_speed = 10.0f;
	float time = 0.0f;
	float flow_delta = 0.0f;
	float radius = 5.95f;
	//glEnable(GL_CULL_FACE); // rotor blades do not get drawn correctly, their front face is down so the up part is dropped
	glEnable(GL_DEPTH_TEST);
	while (!close) {
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
			{
				close = true;
			}
			else if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					button_w = true;
					break;
				case SDLK_a:
					button_a = true;
					break;
				case SDLK_s:
					button_s = true;
					break;
				case SDLK_d:
					button_d = true;
					break;
				case SDLK_n:
					button_n = true;
					break;
				case SDLK_r:
					vertices.clear();
					for (int i = 0; i < i_trace_count; i++)
					{
						for (int j = 0; j < j_trace_count; j++)
						{
							for (int k = 0; k < k_trace_count; k++)
							{
								for (int l = 0; l < l_trace_count; l++)
								{
									glm::vec3 position((tracing_width / 2.0f) - i * (tracing_width / i_trace_count) + 0.1f,
										(tracing_height / 2.0f) - k * (tracing_height / k_trace_count) - 0.1f * l + 0.1f,
										(tracing_width / 2.0f) - j * (tracing_width / i_trace_count) + 0.1f);
									Vertex top = { position,glm::vec4(0.0f, 0.0f, 1.0f, 1.0f) };
									position.y = (tracing_height / 2.0f) - k * (tracing_height / k_trace_count) - 0.1f * (l + 1.0f) + 0.1f;
									Vertex bottom = { position,glm::vec4(1.0f, 0.0f, 0.0f, 1.0f) };
									vertices.push_back(top);
									vertices.push_back(bottom);
								}
							}
						}
					}
					break;
				case SDLK_c:
					button_c = !button_c;
					break;
				case SDLK_h:
					button_h = !button_h;
					break;
				case SDLK_q:
					button_q = !button_q;
					for (int i = 0; i < i_trace_count; i++)
					{
						for (int j = 0; j < j_trace_count; j++)
						{
							for (int k = 0; k < k_trace_count; k++)
							{
								int index = l_trace_count * 2.0f * k + (l_trace_count * k_trace_count * 2.0f) * j + (l_trace_count * k_trace_count * j_trace_count * 2.0f) * i;
								for (int l = 0; l < l_trace_count; l++)
								{
									vertices[index + l * 2.0f].position.z /= (tracing_width);
									vertices[index + l * 2.0f + 1].position.z /= (tracing_width);
								}
							}
						}
					}
					break;
				case SDLK_SPACE:
					button_space = !button_space;
					break;
				case SDLK_LSHIFT:
					button_shift_l = true;
					break;
				case SDLK_LCTRL:
					button_control_l = true;
					break;
				case SDLK_f:
					SDL_SetRelativeMouseMode(SDL_TRUE);
					break;
				case SDLK_ESCAPE:
					SDL_SetRelativeMouseMode(SDL_FALSE);
					break;
				}
			}
			else if (event.type == SDL_KEYUP)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_w:
					button_w = false;
					break;
				case SDLK_a:
					button_a = false;
					break;
				case SDLK_s:
					button_s = false;
					break;
				case SDLK_d:
					button_d = false;
					break;
				case SDLK_LSHIFT:
					button_shift_l = false;
					break;
				case SDLK_LCTRL:
					button_control_l = false;
					break;
				}
			}
			else if (event.type == SDL_MOUSEMOTION)
			{
				if (SDL_GetRelativeMouseMode())
				{
					camera.onMouseMove(event.motion.xrel, event.motion.yrel);
				}
			}
		}
		if (button_w)
		{
			camera.moveFront(delta * camera_speed);
		}
		if (button_a)
		{
			camera.moveSideways(-delta * camera_speed);
		}
		if (button_s)
		{
			camera.moveFront(-delta * camera_speed);
		}
		if (button_d)
		{
			camera.moveSideways(delta * camera_speed);
		}
		if (button_shift_l)
		{
			camera.moveUp(delta * camera_speed);
		}
		if (button_control_l)
		{
			camera.moveUp(-delta * camera_speed);
		}
		camera.update();

		//model = glm::scale(model, glm::vec3(1.0f));
		//model = glm::rotate(model, 1.0f * delta, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 mvp = camera.getVP() * model;
		glUniformMatrix4fv(shader.get_location("u_mvp"), 1, GL_FALSE, &mvp[0][0]);
		
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// does not run at start, space play/pauses execution, n is one step forward, r resets the particles, q maps to 2D
		if ((flow_delta > 0.1f && button_space) || button_n)
		{
			std::thread threads[i_trace_count];
			button_n = false;
			flow_delta = 0.0f;
			for (int i = 0; i < i_trace_count; i++)
			{
				threads[i] = std::thread(calculate_new_positions, i, j_trace_count, k_trace_count, l_trace_count, &vertices, radius);
			}
			for (int i = 0; i < i_trace_count; i++)
			{
				threads[i].join();
			}
		}
		tracing_vertex_buffer.update(vertices);
		glLineWidth(line_width);
		glDrawArrays(GL_LINES, 0, tracing_vertex_buffer.getNum_vertices());

		if (button_h)
		{
			heli_model.render();
		}
		if (button_c)
		{
			coordinate_vertex_buffer.bind();
			glDrawArrays(GL_LINES, 0, coordinate_vertex_buffer.getNum_vertices());
		}

		// imgui window
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();
		static int counter = 0;

		ImGui::Begin("Controls");
		ImGui::SliderFloat("Vortex Ring", &radius, 0.0f, 11.9f);//8.925f);
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(window);

		flow_delta += delta;
		time += delta;
		uint64 endCounter = SDL_GetPerformanceCounter();
		uint64 counterElapsed = endCounter - lastCounter;
		delta = (float)counterElapsed / (float)perfCounterFrequency;
		uint32 FPS = (uint32)((float)perfCounterFrequency / (float)counterElapsed);
		std::cout << FPS << std::endl;
		lastCounter = endCounter;
	}
	return 0;
}