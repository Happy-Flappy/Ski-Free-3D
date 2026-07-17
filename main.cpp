#include "winsimple.hpp"
#include "winsimple-audio.hpp"
#include "winsimple-animation.hpp"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

//ws::Vec3f sunLight = {0.5, 0.8, 0.5};
ws::Vec3f lightColor = {1.0, 1.0, 1.0};
ws::Vec3f sunLight = {0.5, 0.8, 0.5};

void normalize(ws::Vec3f &v)
{
	double length = std::sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
	if(length > 0)
	{
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}
}

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
};
Camera camera(0,0,0);



std::vector<ws::Sprite> sprites;
std::vector<ws::Radial> shapes;

struct Render
{
	int shapeID = -1;
	int spriteID = -1;
	ws::Vec2f scale = {1,1};
	float x,y,z;
	float depth = 0;
	
};
std::vector<Render> renders;

struct SpatialGrid
{
    float cellSize = 200.0f;
    std::unordered_map<long long, std::vector<int>> cells;

    long long key(float x, float z)
    {
        int cx = (int)std::floor(x / cellSize);
        int cz = (int)std::floor(z / cellSize);
        return ((long long)cx << 32) ^ (unsigned int)cz;
    }

    void build(std::vector<Render>& allRenders)
    {
        cells.clear();
        for(int i = 0; i < (int)allRenders.size(); i++)
            cells[key(allRenders[i].x, allRenders[i].z)].push_back(i);
    }

    void query(float x, float z, float radius, std::vector<int>& out)
    {
        int cellRadius = (int)std::ceil(radius / cellSize);
        int cx = (int)std::floor(x / cellSize);
        int cz = (int)std::floor(z / cellSize);

        for(int dx = -cellRadius; dx <= cellRadius; dx++)
        for(int dz = -cellRadius; dz <= cellRadius; dz++)
        {
            long long k = ((long long)(cx+dx) << 32) ^ (unsigned int)(cz+dz);
            auto it = cells.find(k);
            if(it != cells.end())
                out.insert(out.end(), it->second.begin(), it->second.end());
        }
    }
};
SpatialGrid grid;

#include "actor.h"

class Map
{
	public:
	
	ws::Texture image;
	int width = 8192;
	int depth = 8192;
	int density = 40;
	int yDensity = 5;
	
	float getHillHeight(float x, float z) 
	{
		return (float)image.getPixel(x,z).r * yDensity;
	}
	
	void makeTerrain(std::string path) 
	{
		image.loadFromFile(path);
		width = image.getSize().x;
		depth = image.getSize().y;
		
		shapes.reserve(shapes.size() + ((width*depth)/density));
		
	    for(float x = 0; x < image.getSize().x; x += density) 
		{
			for(float z = 0; z < image.getSize().y; z += density) 
			{	
 				Render r;
	            r.x = x;
	            r.z = z;
	            r.y = getHillHeight(x, z);
				
				ws::Radial shape;
				shape.setFillColor(ws::Hue::white);
				shape.setBorderWidth(0);
				shape.setRadius(50);
				shape.setOrigin(shape.getRadius(),0);
				shape.setPointCount(5);
				r.shapeID = shapes.size();
				shapes.push_back(shape);
				renders.push_back(r); 
	        }
	    }
		
		makeLighting();
	}
	
	void makeLighting(Render &d)
	{
		if(d.shapeID < 0 || d.shapeID >= shapes.size())
			return;
		ws::Vec3f normal = getTerrainNormal(d.x, d.z);
		float diffuse = std::max(0.0f, normal.x * sunLight.x + 
									  normal.y * sunLight.y + 
									  normal.z * sunLight.z);
		
		float ambient = 0.25f;
		float brightness = ambient + (1.0f - ambient) * diffuse;

		ws::Hue base = ws::Hue::grey;
		
		
		ws::Vec3f tinted = {brightness * lightColor.x, brightness * lightColor.y, brightness * lightColor.z};
		ws::Hue litColor = ws::Hue(
			(int)(base.r * tinted.x),
			(int)(base.g * tinted.y),
			(int)(base.b * tinted.z)
		);				


		float fogFactor = 1.0f - std::clamp(d.depth / camera.visible, 0.0f, 1.0f);
		ws::Hue foggedColor = ws::Hue(
			(int)(litColor.r * fogFactor),
			(int)(litColor.g * fogFactor),
			(int)(litColor.b * fogFactor)
		);

		shapes[d.shapeID].setFillColor(foggedColor);
		shapes[d.shapeID].setBorderColor(foggedColor);				
	}
	
	void makeLighting()
	{
		for(auto& d : renders)
		{
			makeLighting(d);
		}
	}

	ws::Vec3f getTerrainNormal(float x, float z) 
	{
		const float eps = density*4;
		
		float hL = getHillHeight(x - eps, z);
		float hR = getHillHeight(x + eps, z);
		float hD = getHillHeight(x, z - eps);
		float hU = getHillHeight(x, z + eps);

		float gradX = (hR - hL) / (2.0f * eps);
		float gradZ = (hU - hD) / (2.0f * eps);

		ws::Vec3f normal = { -gradX, 1.0f, -gradZ };
		normalize(normal);
		return normal;
	}
	
}map;

void populate(std::string path)
{
	ws::Texture tex;
	tex.loadFromFile(path);
	
	for(int x=0;x<tex.getSize().x;x++)
	{
		for(int z=0;z<tex.getSize().y;z++)
		{
			Render r;
			r.x = x * map.density;
			r.z = z * map.density;
			r.y = map.getHillHeight(r.x, r.z);
			
			ws::Hue c = tex.getPixel(x,z);
			if(c == ws::Hue::white)
				continue;
			if(c == ws::Hue::green)
			{
				ws::Sprite sprite;
				sprite.setTexture(exploTex,true);
				sprite.setTextureRect({0,1361,92,125});
				sprite.setOrigin(sprite.getTextureRect().width/2, sprite.getTextureRect().height - 20);
				r.spriteID = sprites.size();
				sprites.push_back(sprite);
			}
			if(c == ws::Hue(31,31,31))
			{
				Dog d;
				d.renderID = renders.size();
				dogs.push_back(d);
				dogs.back().update(r.y);
				r.spriteID = sprites.size();
				sprites.push_back(ws::Sprite());
				sprites.back().setOrigin(sprites.back().getTextureRect().width/2, sprites.back().getTextureRect().height + 10);
			}
			renders.push_back(r);
		}
	}
	
/*  	for(int a=0;a<200;a++)
	{
		int x = std::rand()% map.width;
		int z = std::rand()% map.depth;
		Render r;
		r.x = x;
		r.z = z;
		r.y = map.getHillHeight(r.x,r.z);
		r.spriteID = sprites.size();
		ws::Sprite spr;
		spr.setTexture(exploTex);
		spr.setTextureRect({0,1613,88,127});
		spr.setOrigin(88/2,127);
		sprites.push_back(spr);
		renders.push_back(r);
	} 
 */	
 
	for(int x =0;x < map.width;x+=map.density)
	for(int z =0;z < map.depth;z+=map.density)
	{
		Render r;
		r.x = x;
		r.z = z;
		r.y = map.getHillHeight(x,z) - 300;
		
		ws::Radial shape;
		shape.setFillColor(ws::Hue::white);
		shape.setBorderWidth(0);
		shape.setRadius(50);
		shape.setOrigin(shape.getRadius(),0);
		shape.setPointCount(5);
		r.shapeID = shapes.size();
		shapes.push_back(shape);
		renders.push_back(r); 		
		
	}
	
	for(int a=0;a<100;a++)
	{
		int x = std::rand()% map.width;
		int z = std::rand()% map.depth;
		Render r;
		r.x = x;
		r.z = z;
		r.y = map.getHillHeight(r.x,r.z);
		r.spriteID = sprites.size();
		ws::Sprite spr;
		spr.setTexture(caveStuffTex);
		spr.setTextureRect({46,142,410,808});
		spr.setOrigin(spr.getTextureRect().width/2,spr.getTextureRect().height * 0.9);
		sprites.push_back(spr);
		renders.push_back(r);		
	}
	
}

float gravity = 1;
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

int main()
{
	loadTextures();
	
	normalize(sunLight);
	
	ws::Wav music,atmos;
 	//music.open("ASSETS/MUSIC000.midi",0,true);
	//music.play();
	atmos.open("ASSETS/creatorshome-mountain-wind-371074.mp3",1,true);
	atmos.play(); 
	
	srand(time(nullptr));
	
	ws::Window window(960,540,"");
	ws::Vec2f windowSize = {960.0f,540.0f};
	
	map.makeTerrain("converted.png");
	camera = Camera(map.width/2,0,100);
	camera.visible = 1000;
	populate("map.bmp");
	grid.build(renders);	
	
	ws::Timer clock;
	double timesincelastupdate = 0;
	double timeperframe = 1.f/60.f;
	
	Render manRender;
	manRender.x = std::rand() % map.width;
	manRender.z = std::rand() % map.depth;
	manRender.y = camera.y;
	manRender.spriteID = sprites.size();
	sprites.push_back(ws::Sprite());
	snowman.renderID = renders.size();
	renders.push_back(manRender);
	snowman.update(map.getHillHeight(renders.back().x,renders.back().z+200),{camera.x,camera.z});
	
	
	ws::Texture backTex;
	backTex.create(window.getSize().x,window.getSize().y);
	for(int x=0;x<window.getSize().x;x++)
	for(int y=0;y<window.getSize().y;y++)
	{
		int yVal =  float(y) / float(window.getSize().y + 100) * 255;
		yVal = std::abs(yVal - 255);
		if(yVal < 200)
			yVal = 200;
		backTex.setPixel(x,y,ws::Hue(0,yVal,255));
	}
	ws::Sprite backSprite;
	backSprite.setTexture(backTex);
	
	
	Player player;

	float angle = 0;
	ws::Timer moveTimer;

	std::vector<int> renderIndices;
	renderIndices.reserve(2048);
	std::vector<int> nearby;
	nearby.reserve(512);

	ws::Timer fpsTimer;
	int fpsFrameCount = 0;

	while(window.isOpen())
	{
		
/* 		if(music.isFinished())
			music.setProgress(0);
		if(atmos.isFinished())
			atmos.setProgress(0); */
		
		timesincelastupdate += clock.getSeconds();
		clock.restart();
		
		camera.getTrigs();
		float sinYaw = camera.sinYaw;
		float cosYaw = camera.cosYaw;
		float sinPitch = camera.sinPitch;
		float cosPitch = camera.cosPitch;		
		
		// Camera forward direction (world space)
		ws::Vec3f forward = {
			camera.sinYaw * camera.cosPitch,
			 0,
			 camera.cosYaw * camera.cosPitch
		};
		normalize(forward);

		sunLight = forward; // now the light comes from the viewing direction
		
		
		while(timesincelastupdate > timeperframe)
		{
			timesincelastupdate -= timeperframe;
			
			
			
			camera.z += player.velocity.z;
			camera.x += player.velocity.x;
			camera.y += player.velocity.y;
			
			float constSpeed = 2;
			
			if (ws::Global::getButton('W'))
			{
				player.velocity.x =  sinYaw * constSpeed;
				player.velocity.z =  cosYaw * constSpeed;
			}
			if (ws::Global::getButton('S'))
			{
				player.velocity.x = -sinYaw * constSpeed;
				player.velocity.z = -cosYaw * constSpeed;
			}
			if (ws::Global::getButton('A'))
			{
				player.velocity.x = -cosYaw * constSpeed;
				player.velocity.z =  sinYaw * constSpeed;
			}
			if (ws::Global::getButton('D'))
			{
				player.velocity.x =  cosYaw * constSpeed;
				player.velocity.z = -sinYaw * constSpeed;
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
			
			if(moveTimer.getSeconds() > 1)
			{
				angle += 1;
				moveTimer.restart();
			}

			player.update(camera);
			
			for(auto& d : dogs)
			{
				if(d.renderID < renders.size() && d.renderID >= 0)
				{
					Render &r = renders[d.renderID];
					d.update(map.getHillHeight(r.x,r.z));
				}
			}
			if(snowman.renderID < renders.size() && snowman.renderID >= 0)
			{
				Render &r = renders[snowman.renderID];
				snowman.update(map.getHillHeight(r.x,r.z),{camera.x,camera.z});
			}			

		}
		//window.getView().setPortSize({960/8,540/8});
		window.clear(ws::Hue::black);
		//window.draw(backSprite);
		
		float floor = map.getHillHeight(camera.x,camera.z) - 50;
		if(camera.y > floor)
		{
			player.velocity.y = 0;
			camera.y = floor;
		}
		else
			player.velocity.y += gravity;
		
		renderIndices.clear();

		ws::Timer nearbyTimer;
		nearby.clear();
		
		//0.5 pushes the search center forward 
		//0.6 is the padding so that objects dont pop in and out of existence due to that 0.1 buffer.
		float lookAheadDist = camera.visible * 0.5f;
		float queryX = camera.x + sinYaw * lookAheadDist;
		float queryZ = camera.z + cosYaw * lookAheadDist;
		grid.query(queryX, queryZ, camera.visible * 0.6f, nearby);

		double nearbyMs = nearbyTimer.getMilliSeconds();

		renderIndices.clear();
		for(int idx : nearby)
		{
			Render& d = renders[idx];
			float wx = d.x - camera.x;
			float wz = d.z - camera.z;
			float cz = wx * sinYaw + wz * cosYaw;
			float cx = wx * cosYaw - wz * sinYaw;
			if(cz > 0 && cz < camera.visible && cx > -camera.visible && cx < camera.visible)
			{
				d.depth = cz;
				renderIndices.push_back(idx);
			}
		}
		
		renderIndices.push_back(snowman.renderID);
		for(auto& d : dogs)
			renderIndices.push_back(d.renderID);

	
		ws::Timer sortTimer;
		std::sort(renderIndices.begin(), renderIndices.end(), [](int a, int b){
			return renders[a].depth > renders[b].depth;
		});
		double sortMs = sortTimer.getMilliSeconds();

		ws::Timer drawTimer;
		for(int idx : renderIndices)
		{
			Render& d = renders[idx];
			float screenX, screenY, scale;
			if(!camera.project(d.x, d.y, d.z, screenX, screenY, scale)) continue;
			
			map.makeLighting(d);
			
			if(d.shapeID != -1 && d.shapeID >= 0 && d.shapeID < shapes.size())
			{
				
				ws::Radial &rad = shapes[d.shapeID];
				rad.setPosition(screenX, screenY);
				rad.setScale(scale, scale);
				window.draw(rad);
			}		
			if(d.spriteID != -1 && d.spriteID >= 0 && d.spriteID < sprites.size())
			{
				ws::Sprite &spr = sprites[d.spriteID];
				if(spr.hasTexture() && spr.getTexture().isValid())
				{
					spr.setPosition(screenX, screenY);
					spr.setScale(d.scale.x * scale, d.scale.y * scale);
					spr.drawBlend(window.backBuffer);					
				}
				
				
			}
			
		}
		float drawMs = drawTimer.getMilliSeconds();

		window.display();

		fpsFrameCount++;
		if(fpsTimer.getSeconds() > 1.0)
		{
		 	std::cout << "Draw Delay: " << drawMs << std::endl;
			std::cout << "Query Delay: " << nearbyMs << std::endl;
			std::cout << "Sort Delay: " << sortMs << std::endl;
			std::cout << "FPS: " << fpsFrameCount << std::endl;
			fpsFrameCount = 0;
			fpsTimer.restart();
		}
	}
	
	return 0;
}
