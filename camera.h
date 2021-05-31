#pragma once

#include "glm.hpp"
#include "ext/matrix_clip_space.hpp"
#include "ext/matrix_transform.hpp"

class Camera
{
public:

	Camera(float fov, float width, float height)
	{
		projection = glm::perspective(fov / 2.0f, width / height, 0.1f, 1000.0f);
		view = glm::mat4(1.0f);
		position = glm::vec3(0.0f);
		update();
	}
	
	glm::mat4 getVP()
	{
		return vp;
	}

	virtual void update()
	{
		vp = projection * view;
	}

	virtual void translate(glm::vec3 v)
	{
		position += v;
		// when the camera is moving to the left, the world is moving to the right, v is camera movement but we are moving the world
		view = glm::translate(view, -1.0f * v);
		update();
	}

protected:
	glm::vec3 position;
	glm::mat4 projection;
	glm::mat4 view;
	glm::mat4 vp;
};