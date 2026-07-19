
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
	ws::Vec2f target = {0,0};
	ws::Vec2f wanderVec = {0,0};
	
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



class MintKillers
{
	public:
	ws::Wav roar;
	
	ws::IntRect currentRect = {0,0,60,60};
	std::string state = "wander";
	ws::Timer timer;
	ws::Timer reproduceTimer;
	int renderID = 0;
	
	float gravity = 0.5;
    ws::Vec3f velocity = {0,0,0};
    float maxSpeed = 3.0f;//1.7          // maximum speed
    float wanderSpeed = 0.5f;
	float steerStrength = 0.5f;     // how quickly it turns (lower = more sluggish)
    float noiseAmount = 0.2f;       // random perturbation for inaccuracy
	
	float scale = 1;
	float jumpTime = 0; 
	float hillHeight = 0;
	ws::Vec2f target = {0,0};
	ws::Vec2f wanderVec = {0,0};
	
	float sightDist = 1000;
	
	MintKillers()
	{
		roar.open("ASSETS//roar" + std::to_string(1) + ".wav",20);					
	}
	
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
		
		currentRect = {60,0,60,60};
		state = "jump";		
	}
	void jump(Render &r)
	{
 		if(state != "jump")
			return;
		
		if(r.y >= hillHeight)//touching the ground or close to it.
		{
			state = "chase";
			currentRect = {0,0,60,60};
			jumpTime = ((rand()%10) / 8) + 0.2;
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
			currentRect = {0,0,60,60};
			sprite.setTexture(mintKillerTex,true);
			sprite.setTextureRect(currentRect);
			sprite.setOrigin(sprite.getTextureRect().width / 2,sprite.getTextureRect().height);
			r.scale = {scale,scale};
		}

		sprite.setTextureRect(currentRect);

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
		
		
 		if(state == "jump")
			jump(r);
		if(state == "chase")
			chase(r);
		if(state == "wander")
			wander(r);
		
	}
	
};

std::vector<MintKillers> mintKillers;


class Actor
{
	public:
	
	ws::Wav wav;
	std::string state = "";
	std::vector<std::string> events;

	ws::Timer timer;
	ws::ShiftData currentShift;
	int renderID = 0;
	
	float gravity = 0.5;
    ws::Vec3f velocity = {0,0,0};
    float maxSpeed = 2.0f;
    float wanderSpeed = 0.2f;
	float steerStrength = 0.05f;     // how quickly it turns (lower = more sluggish)
    float noiseAmount = 0.2f;       // random perturbation for inaccuracy
	
	float jumpTime = 0; 
	float hillHeight = 0;
	ws::Vec2f target = {0,0};
	ws::Vec2f wanderVec = {0,0};
	ws::Vec2f mapSize = {4,4};
	
	float sightDist = 200;
	int channel = -1;
	
	Actor()
	{
		channel = wav.getFreeChannel();
		wav.channel = channel;
	}
};


class Candivore : public Actor
{
	public:
	ws::Timer waitTimer;
	float maxWaitTime = 2;
	float fleeTimeElapsed = 0;
	
	
	bool updateCandivore()
	{
		if(renderID >= renders.size() || renderID < 0)
			return false;
		
		Render &r = renders[renderID];
		if(r.spriteID >= sprites.size() || r.spriteID < 0)
			return false;
		
		r.y = hillHeight;		
		
		if(state == "wander")
			wander();
		if(state == "flee")
			flee(r);
		
		float dx = target.x - r.x;
		float dz = target.y - r.z;
		float dist = std::sqrt(dx*dx + dz*dz);	
		if(dist < sightDist)
			state = "flee";
		
		if(waitTimer.getSeconds() > maxWaitTime)
			startWander();
		
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
		velocity.x *= 0.9;
		velocity.z *= 0.9;
		
		clampToMap(r);
		
		return true;
	}
	
	void clampToMap(Render &r)
	{
		if(r.x < 0)
		{
			r.x = 0;
			velocity.x = 0;
			velocity.z = 0;
			startWander();
		}
		if(r.y < 0)
		{
			r.y = 0;
			velocity.x = 0;
			velocity.z = 0;
			startWander();
		}		
		if(r.x >= mapSize.x)
		{
			r.x = mapSize.x-1;
			velocity.x = 0;
			velocity.z = 0;
			startWander();
		}		
		if(r.y >= mapSize.y)
		{
			r.y = mapSize.y-1;
			velocity.x = 0;
			velocity.z = 0;
			startWander();
		}		
	}
	
	void startWander()
	{
		waitTimer.restart();
		maxWaitTime = (std::rand()%10) + 1;			
		wanderVec = {float(std::rand()%10)+0.01,float(std::rand()%10)+0.01};
		wanderVec.x /= 5.0f;
		wanderVec.y /= 5.0f;
		
		wanderVec.x -= 1.0f;
		wanderVec.y -= 1.0f;
		state = "wander";		
	}
	void wander()
	{
		velocity.x = wanderVec.x * wanderSpeed;
		velocity.z = wanderVec.y * wanderSpeed;
	}
	void flee(Render &r)
	{
		waitTimer.restart();
		float dx = target.x - r.x;
		float dz = target.y - r.z;
		float dist = std::sqrt(dx*dx + dz*dz);	
		if(dist < 0.01)
			return;
		
		float desiredX = (dx / dist) * maxSpeed;
		float desiredZ = (dz / dist) * maxSpeed;	
		
		desiredX += (rand()%100 - 50) * 0.01f * noiseAmount;
		desiredZ += (rand()%100 - 50) * 0.01f * noiseAmount;

		//reverse direction so flees instead of chases.
		velocity.x -= (desiredX - velocity.x) * steerStrength;
		velocity.z -= (desiredZ - velocity.z) * steerStrength;

		float speed = std::sqrt(velocity.x*velocity.x + velocity.z*velocity.z);
		if(speed > maxSpeed) 
		{
			velocity.x = (velocity.x / speed) * maxSpeed;
			velocity.z = (velocity.z / speed) * maxSpeed;
		}					

		if(dist > sightDist)
			fleeTimeElapsed += (1.f/60.f);
		if(fleeTimeElapsed > 5)
		{
			fleeTimeElapsed = 0;
			startWander();
		}
	}
};


class Mello : public Candivore
{
	public:
	
	ws::ShiftData currentShift;
	std::string lastState = "";
	
	struct Shifts
	{
		ws::ShiftData stand,walk,panic;
		Shifts()
		{
			stand.add({0,0,60,60});
			walk.add({60,0,60,60});
			walk.add({120,0,60,60});
			panic.delay = 0.05;
			panic.add({180,0,60,60});
			panic.add({180+60,0,60,60});
			
		}
	}shifts;
	
	
	void init(float mapWidth,float mapDepth)
	{
		startWander();
		mapSize.x = mapWidth;
		mapSize.y = mapDepth;
		if(!updateCandivore())
			return;
		Render &r = renders[renderID];
		ws::Sprite &sprite = sprites[r.spriteID];
		sprite.setTexture(melloTex);
		currentShift = shifts.stand;
		sprite.setTextureRect(ws::Shift(currentShift));
		sprite.setOrigin(sprite.getTextureRect().width/2,sprite.getTextureRect().height * 0.85);
		r.scale = {1,1};
	}
	
	void update(float hillHeight,ws::Vec2f target)
	{
		this->hillHeight =  hillHeight;
		this->target = target;
		
		if(!updateCandivore())
			return;
		Render &r = renders[renderID];
		ws::Sprite &sprite = sprites[r.spriteID];
		
		
		sprite.setTextureRect(ws::Shift(currentShift));
		
		if(currentShift.ended)
		{
			currentShift.ended = false;
			currentShift.currentframe = 0;
			currentShift.start = true;
		}		

		if(lastState != state)
		{
			if(state == "wait")
			{
				currentShift = shifts.stand;
				currentShift.currentframe = 0;
				currentShift.start = true;
			}
			if(state == "wander")
			{
				currentShift = shifts.walk;
				currentShift.currentframe = 0;
				currentShift.start = true;
			}
			if(state == "flee")
			{
				static bool snd1 = false;
				if(snd1)
				{
					snd1 = false;
					wav.open("ASSETS//melloScared0.wav",channel);
					wav.play();
				}
				else
				{
					snd1 = true;
					wav.open("ASSETS//melloScared1.wav",channel);
					wav.play();					
				}
				currentShift = shifts.panic;
				currentShift.currentframe = 0;
				currentShift.start = true;
			}
			
			lastState = state;
		}
	}	
};
std::vector<Mello> mellos;


class Carnivore
{
	public:
	
	
};





