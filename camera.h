#ifndef CAMERA_H
#define CAMERA_H

class Camera
{
	public:
	
	double x=0,y=0,z=0;
	float yaw = 0;
	float pitch = 0;
	
	int perspective = 300;
	int visible = 800;
	
	double sinYaw = 0;
	double cosYaw = 0;
	double sinPitch = 0;
	double cosPitch = 0;


	
	Camera(double x,double y,double z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
	
	void getTrigs()
	{
		sinYaw = std::sin(yaw);
		cosYaw = std::cos(yaw);
		sinPitch = std::sin(pitch);
		cosPitch = std::cos(pitch);				
	}
	
	bool project(float x,float y,float z,float &screenX,float &screenY,float &scale)
	{
		float relativeZ = z - this->z;
		float relativeX = x - this->x;
		float relativeY = y - this->y;
		
		float yawY = relativeY;
        float yawX = relativeX * cosYaw - relativeZ * sinYaw;
        float yawZ = relativeX * sinYaw + relativeZ * cosYaw;

		float pitchX = yawX;
		float pitchY = yawY * cosPitch - yawZ * sinPitch;
		float pitchZ = yawY * sinPitch + yawZ * cosPitch;
		
		float rotX = pitchX;
		float rotY = pitchY;
		float rotZ = pitchZ;
		
		relativeX = rotX;
		relativeZ = rotZ;
		relativeY = rotY;
		
		if(relativeZ < 0 || relativeZ > visible || relativeX < -visible || relativeX > visible || relativeY < -visible || relativeY > visible) return false;
		
		scale = perspective / relativeZ;
		
		screenX = (relativeX * scale) + 960/2;
		screenY = (relativeY * scale) + 540/2;	

		return true;		
	}
	
	ws::Vec3f unproject(float screenX, float screenY, float depth)
	{
		float relX = (screenX - 960.0f/2.0f) * depth / perspective;
		float relY = (screenY - 540.0f/2.0f) * depth / perspective;
		float relZ = depth;

		// Inverse pitch rotation
		float yawX = relX;
		float yawY = relY * cosPitch + relZ * sinPitch;
		float yawZ = -relY * sinPitch + relZ * cosPitch;

		// Inverse yaw rotation
		float worldRelX = yawX * cosYaw + yawZ * sinYaw;
		float worldRelY = yawY;
		float worldRelZ = -yawX * sinYaw + yawZ * cosYaw;

		return { x + worldRelX, y + worldRelY, z + worldRelZ };
	}	
};
Camera camera(0,0,0);

#endif