#ifndef PLAYER_H
#define PLAYER_H


class Player
{
	public:
	ws::Vec3f velocity = {0,0,0};
	
	
	Player() {}
	
	void update(Camera &camera,ws::Window &window)	
	{
		float floor = map.getHillHeight(camera.x,camera.z) - 50;
		//incrementally reach that position for smooth movement.
		camera.y += (floor - camera.y) * 0.6;
		
		camera.z += velocity.z;
		camera.x += velocity.x;
		camera.y += velocity.y;
		
		velocity.x *= 0.95;
		velocity.z *= 0.95;
		
		
		float constSpeed = 2;
		
		if (ws::Global::getButton('W'))
		{
			velocity.x =  camera.sinYaw * constSpeed;
			velocity.z =  camera.cosYaw * constSpeed;
		}
		if (ws::Global::getButton('S'))
		{
			velocity.x = -camera.sinYaw * constSpeed;
			velocity.z = -camera.cosYaw * constSpeed;
		}
		if (ws::Global::getButton('A'))
		{
			velocity.x = -camera.cosYaw * constSpeed;
			velocity.z =  camera.sinYaw * constSpeed;
		}
		if (ws::Global::getButton('D'))
		{
			velocity.x =  camera.cosYaw * constSpeed;
			velocity.z = -camera.sinYaw * constSpeed;
		}	


		static const float mouseSensitivityYaw = 0.005f;
		static const float mouseSensitivityPitch = 0.005f;
		static ws::Vec2i lastMousePos = ws::Global::getMousePos(window);	

		ws::Vec2i currentMousePos = ws::Global::getMousePos(window);
		int deltaX = currentMousePos.x - lastMousePos.x;
		int deltaY = currentMousePos.y - lastMousePos.y;

		camera.yaw   += deltaX * mouseSensitivityYaw;
		camera.pitch += deltaY * mouseSensitivityPitch;
		camera.pitch = std::clamp(camera.pitch, -1.57f, 1.57f);

		lastMousePos = currentMousePos;			
		
	}
};

#endif