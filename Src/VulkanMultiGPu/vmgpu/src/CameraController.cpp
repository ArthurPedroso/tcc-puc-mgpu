#include "CameraController.h"


glm::vec3 CameraController::quatTransform(const glm::quat& q, const glm::vec3& v)
{
	glm::quat tmp = q * glm::quat(0.f, v) * conjugate(q);
	return glm::vec3(tmp.x, tmp.y, tmp.z);
}

void CameraController::update(float delta)
{
	
	glm::vec3 cameraDirection = quatTransform(
		node->getOrientation(), glm::vec3(0.f, 0.f, -1.f));
	glm::vec3 cameraRight = quatTransform(
		node->getOrientation(), glm::vec3(1.f, 0.f, 0.f));

	bool moved = false;
	glm::vec3 movement;

	if (forwardPressed && !backwardPressed)
	{
		movement += cameraDirection;
		moved = true;
	} else if (backwardPressed && !forwardPressed)
	{
		movement -= cameraDirection;
		moved = true;
	}
	if (leftPressed && !rightPressed)
	{
		movement -= cameraRight;
		moved = true;
	} else if (rightPressed && !leftPressed)
	{
		movement += cameraRight;
		moved = true;
	}
	if (downPressed && !upPressed)
	{
		movement -= glm::vec3(0.f, 1.f, 0.f);
		moved = true;
	} else if (upPressed && !downPressed)
	{
		movement += glm::vec3(0.f, 1.f, 0.f);
		moved = true;
	}

	if (moved)
	{
		movement = glm::normalize(movement) * delta
			   * speed;
		node->translate(movement);
	}

	node->setRotation(yaw, glm::vec3(0.f, 1.f, 0.f));
	node->rotate(pitch, glm::vec3(1.f, 0.f, 0.f));	
	node->update();
}

void CameraController::motion(float yaw, float pitch)
{
	CameraController::yaw -= yaw * sensitivity;
	CameraController::pitch -= pitch * sensitivity;
}