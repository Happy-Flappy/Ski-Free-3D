#ifndef PLAYER_H
#define PLAYER_H


class Player
{
	public:
	
	float forwardVel = 0;
	ws::Vec3f velocity = {0,0,0};
	
	Player() {}
	
	ws::Wav sledSnd;
 	bool sndLoaded = false;
	void update(Camera &camera)	
	{
		if(!sndLoaded)
		{
			sledSnd.open("ASSETS//sspsurvival-sledding-on-snow-sliding-on-snow-snow-and-sledding-16590.mp3",3);
			sndLoaded = true;
		}
		static bool startedSpeed = false;
		if(velocity.z > 3 || velocity.x > 3)
		{
			if(!startedSpeed)
			{
				ws::Wav::PlayFree("ASSETS/alexzavesa-swoosh-long-463613.mp3",2);
				startedSpeed = true;
			}
			
			if(ws::Wav::getChannelStatus(sledSnd.channel) == "stopped" || sledSnd.isFinished())
			{
				sledSnd.setProgress(0);
				sledSnd.play();				
			}
		}
		else
		{
			sledSnd.stop();
			startedSpeed = false;
		}	
		
		const float delta = 3.0f;
		
		float heightLeft  = map.getHillHeight(camera.x - delta, camera.z);
		float heightRight = map.getHillHeight(camera.x + delta, camera.z);
		float heightBack  = map.getHillHeight(camera.x, camera.z - delta);
		float heightFront = map.getHillHeight(camera.x, camera.z + delta);		

		float slopeX = (heightLeft - heightRight) / (2.0f * delta);
		float slopeZ = (heightBack - heightFront) / (2.0f * delta);

		float forwardSlope = slopeX * std::sin(camera.yaw) + slopeZ * std::cos(camera.yaw);
		
		const float SLOPE_ACCEL    = 0.3f;
		const float FRICTION_DECEL = 0.05f;

		velocity.x -= forwardSlope * SLOPE_ACCEL * std::sin(camera.yaw);
		velocity.z -= forwardSlope * SLOPE_ACCEL * std::cos(camera.yaw);
		
		float speed = std::sqrt(velocity.x * velocity.x + velocity.z * velocity.z);
		const float EPSILON = 1e-6f;
		if(speed > EPSILON) {
			float decel = std::min(FRICTION_DECEL, speed);
			velocity.x -= decel * (velocity.x / speed);
			velocity.z -= decel * (velocity.z / speed);
			if(std::sqrt(velocity.x*velocity.x + velocity.z*velocity.z) < EPSILON) {
				velocity.x = 0.0f;
				velocity.z = 0.0f;
			}
		}
	}
};

#endif