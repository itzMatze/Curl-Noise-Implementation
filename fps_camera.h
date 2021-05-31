#pragma once
#include "camera.h"

class FPSCamera : public Camera
{
public:
	FPSCamera(float fov, float width, float height) : Camera(fov, width, height)
	{
		up = glm::vec3(0.0f, 1.0f, 0.0f);
		yaw = 90.0f;
		pitch = 0.0f;
		onMouseMove(0.0f, 0.0f);
		update();
	}

	void onMouseMove(float xRel, float yRel)
	{
		yaw += xRel * mouse_sensitivity;
		pitch -= yRel * mouse_sensitivity;
		if (pitch > 89.0f)
		{
			pitch = 89.0f;
		}
		if (pitch < -89.0f)
		{
			pitch = -89.0f;
		}
		glm::vec3 front;
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		look_at = glm::normalize(front);
		update();
	}

	void update() override
	{
		view = glm::lookAt(position, position + look_at, up);
		vp = projection * view;
	}

	void moveFront(float amount)
	{
		translate(glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f) * look_at) * amount);
		update();
	}

	void moveSideways(float amount)
	{
		translate(glm::normalize(glm::cross(look_at, up)) * amount);
		update();
	}

protected:
	float yaw;
	float pitch;
	const float mouse_sensitivity = 0.25f;
	glm::vec3 look_at;
	glm::vec3 up;
};