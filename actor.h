

class Dog
{
	public:
	std::string state = "idle";
	ws::Timer timer;
	ws::ShiftData currentShift;
	int renderID = 0;
	float goX=0,goZ=0;
	struct Shifts
	{
		ws::ShiftData idle;
		ws::ShiftData bark;
		Shifts()
		{
			idle.add({0,157,21,19});
			idle.add({23,157,21,19});
			bark.add({45,157,21,19});
			bark.add({66,157,21,19});
			
		}
	}shifts;
	
	
	
	void update(float hillHeight)
	{
		if(renderID >= renders.size() || renderID < 0)
			return;
		Render &r = renders[renderID];
		if(r.spriteID < 0 || r.spriteID >= sprites.size())
			return;
		ws::Sprite &sprite = sprites[r.spriteID];
		
		r.y = hillHeight;
		if(!sprite.hasTexture())
		{
			sprite.setTexture(actorTex);
			currentShift = shifts.idle;
			goX = (std::rand()%10) - 5;
			goZ = (std::rand()%10) - 5;
			
			goX *= 0.5;
			goZ *= 0.5;
		}
		if(currentShift.ended)
		{
			currentShift.ended = false;
			currentShift.currentframe = 0;
			currentShift.start = true;
		}
		sprite.setTextureRect(ws::Shift(currentShift));
		
		if(timer.getSeconds() > 1 && state == "bark")
		{
			state = "idle";
			currentShift = shifts.idle;
			timer.restart();
			goX = (std::rand()%10) - 5;
			goZ = (std::rand()%10) - 5;

			goX *= 0.1;
			goZ *= 0.1;			
		}
		
		if(timer.getSeconds() > 3 && state == "idle")
		{
			state = "bark";
			currentShift = shifts.bark;
			timer.restart();
		}			
		
		if(state == "idle")
		{
			r.x += goX;
			r.z += goZ;
		}
	}
	
};
std::vector<Dog> dogs;



class Snowman
{
	public:
	ws::Wav roar;
	
	
	std::string state = "wander";
	ws::Timer timer;
	ws::ShiftData currentShift;
	int renderID = 0;
	
	float gravity = 0.5;
    ws::Vec3f velocity = {0,0,0};
    float maxSpeed = 3.0f;//1.7          // maximum speed
    float wanderSpeed = 0.5f;
	float steerStrength = 0.5f;     // how quickly it turns (lower = more sluggish)
    float noiseAmount = 0.2f;       // random perturbation for inaccuracy
	
	float scale = 3;
	float jumpTime = 0; 
	float hillHeight = 0;
	ws::Vec2f target;
	ws::Vec2f wanderVec;
	
	float sightDist = 1000;
	
	Snowman()
	{
		roar.open("ASSETS//roar" + std::to_string(1) + ".wav",20);					
	}
	
	struct Shifts
	{
		ws::ShiftData jump;
		ws::ShiftData chase;
		
		Shifts()
		{
			jump.add({0,71,32,43});
			jump.add({33,71,32,43});
			
			chase.add({62,71,29,43});
			chase.add({91,71,30,43});
			
		}
	}shifts;
	
	void wander(Render &r)
	{
		r.x += wanderVec.x * wanderSpeed;
		r.z += wanderVec.y * wanderSpeed;

		float dx = target.x - r.x;
		float dz = target.y - r.z;
		float dist = std::sqrt(dx*dx + dz*dz);	
		
		if(dist < sightDist)
			startJump(r);

		if(target.x - r.x < 0)
			r.scale.x = -scale;
		else
			r.scale.x = scale;				
	}	
	void startJump(Render &r)
	{
		velocity.y = -8;
		
		int id = (std::rand()%4)+1;
		if(!roar.isFinished())
			roar.stop();
		
		roar.open("ASSETS//roar" + std::to_string(id) + ".wav",20);	
		roar.play();

		
		//make velocity jump side to side.
		float sideStrength = 3.0f;
		float angle = (rand() % 360) * 0.0174533f;//0.0174533 is one degree in radians - this converts to radians.
		velocity.x += std::cos(angle) * sideStrength;
		velocity.z += std::sin(angle) * sideStrength;				
		
		currentShift = shifts.jump;
		sprites[r.spriteID].setTextureRect(ws::Shift(currentShift));
		currentShift.start = true;
		state = "jump";		
	}
	void jump(Render &r)
	{
 		if(state != "jump")
			return;
		
		if(r.y >= hillHeight)//touching the ground or close to it.
		{
			state = "chase";
			currentShift = shifts.chase;
			
			sprites[r.spriteID].setTextureRect(ws::Shift(currentShift));
			currentShift.start = true;
			jumpTime = (rand()%10) + 2;
			timer.restart();
			velocity.y = 0; 
		}
	}
	void chase(Render &r)
	{
		if(state != "chase")
			return;
		
		if(timer.getSeconds() > jumpTime)
			startJump(r);
		float dx = target.x - r.x;
		float dz = target.y - r.z;
		float dist = std::sqrt(dx*dx + dz*dz);	
		
		if(dist > 0.1f)
		{
			// 2. Desired velocity (normalized direction * maxSpeed)
			float desiredX = (dx / dist) * maxSpeed;
			float desiredZ = (dz / dist) * maxSpeed;	
			
			// 3. Add some inaccuracy (random noise) – makes it less perfect
			desiredX += (rand()%100 - 50) * 0.01f * noiseAmount;
			desiredZ += (rand()%100 - 50) * 0.01f * noiseAmount;

			// 4. Steering force: accelerate towards desired velocity
			velocity.x += (desiredX - velocity.x) * steerStrength;
			velocity.z += (desiredZ - velocity.z) * steerStrength;

			// 5. Clamp speed to maxSpeed
			float speed = std::sqrt(velocity.x*velocity.x + velocity.z*velocity.z);
			if(speed > maxSpeed) {
				velocity.x = (velocity.x / speed) * maxSpeed;
				velocity.z = (velocity.z / speed) * maxSpeed;
			}				
		}
		
		if(dist > sightDist)
		{
			state = "wander";
			currentShift.delay = 0.05;
			wanderVec.x = (std::rand()%10);
			wanderVec.y = (std::rand()%10);

			wanderVec.x /= 5.0f;
			wanderVec.y /= 5.0f;
			
			wanderVec.x -= 1.0f;
			wanderVec.y -= 1.0f;
			//now wander is normalized between -1 and 1.
		}
		
		
		if(target.x - r.x < 0)
			r.scale.x = -scale;
		else
			r.scale.x = scale;		
	}
	
	
	
	
	void update(float hillHeight,ws::Vec2f target)
	{	
		
		this->hillHeight = hillHeight;
		this->target = target;
		
		
		if(renderID >= renders.size() || renderID < 0)
			return;
		Render &r = renders[renderID];
		if(r.spriteID < 0 || r.spriteID >= sprites.size())
			return;
		ws::Sprite &sprite = sprites[r.spriteID];
		if(!sprite.hasTexture())
		{
			state = "wander";
			currentShift = shifts.chase;
			currentShift.delay = 0.05;
			sprite.setTexture(actorTex,true);
			sprite.setTextureRect(ws::Shift(currentShift));
			sprite.setOrigin(sprite.getTextureRect().width / 2,sprite.getTextureRect().height);
			r.scale = {scale,scale};
		}

		sprite.setTextureRect(ws::Shift(currentShift));

		r.x += velocity.x;
		r.y += velocity.y;
		r.z += velocity.z;		
		
		if(r.y > hillHeight)
		{
			if(velocity.y < 0) velocity.y = 0;
			r.y = hillHeight;
		}
		else
			velocity.y += gravity;
		
		if(currentShift.ended)
		{
			currentShift.ended = false;
			currentShift.currentframe = 0;
			currentShift.start = true;
		}		
		
		
 		if(state == "jump")
			jump(r);
		if(state == "chase")
			chase(r);
		if(state == "wander")
			wander(r);
		
	}
	
}snowman;


