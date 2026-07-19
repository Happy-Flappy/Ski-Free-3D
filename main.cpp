#include "winsimple.hpp"
#include "winsimple-audio.hpp"
#include "winsimple-animation.hpp"
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <cmath>

ws::Texture actorTex;
ws::Texture exploTex;
ws::Texture lampTex;
ws::Texture mintKillerTex;
ws::Texture melloTex;
void loadTextures()
{
	actorTex.loadFromFile("ASSETS//actors.png");
	actorTex.setScaleMode(ws::Texture::ScaleMode::NearestNeighbor);
	exploTex.loadFromFile("ASSETS//explo.png");
	exploTex.setScaleMode(ws::Texture::ScaleMode::NearestNeighbor);	
	lampTex.loadFromFile("ASSETS//lamp.png");
	lampTex.setScaleMode(ws::Texture::ScaleMode::NearestNeighbor);	
	mintKillerTex.loadFromFile("ASSETS//MintKiller.png");
	mintKillerTex.setScaleMode(ws::Texture::ScaleMode::NearestNeighbor);	
	melloTex.loadFromFile("ASSETS//UnMelloFellow-2.png");
	melloTex.setScaleMode(ws::Texture::ScaleMode::NearestNeighbor);	
	
}

#include "vec3fStuff.h"
#include "light.h"
#include "camera.h"



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

#include "spatialGrid.h"
#include "actor.h"
#include "map.h"
#include "populate.h"


float gravity = 1;
#include "player.h"

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
	
	map.makeTerrain("ASSETS//converted.png");
	camera = Camera(map.width/2,0,100);
	camera.visible = 1000;
	populate("ASSETS//map.bmp");
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
	
	/* Render mintRender;
	mintRender.x = camera.x;
	mintRender.z = camera.z + 200;
	mintRender.y = map.getHillHeight(camera.x,camera.z) - 50;
	mintRender.spriteID = sprites.size();
	sprites.push_back(ws::Sprite());
	mintKillers.push_back(MintKillers());
	mintKillers.back().renderID = renders.size();
	renders.push_back(mintRender);
	mintKillers.back().update(map.getHillHeight(renders.back().x,renders.back().z + 200),{camera.x,camera.z});
	 */
	
	for(int a=0;a<100;a++)
	{
		Render r;
		r.x = std::rand() % map.width;
		r.z = std::rand() % map.depth;
		r.y = camera.y;
		r.spriteID = sprites.size();
		sprites.push_back(ws::Sprite());
		mellos.push_back(Mello());
		mellos.back().renderID = renders.size();
		renders.push_back(r);
		mellos.back().init(map.width,map.depth);
	}
	
	
	ws::Texture backTex;
	makeSky(backTex,window.getSize().x,window.getSize().y);
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
			for(auto& m : mintKillers)
			{
				if(m.renderID < renders.size() && m.renderID >= 0)
				{
					Render &r = renders[m.renderID];
					m.update(map.getHillHeight(r.x,r.z),{camera.x,camera.z});
				}
			}
			for(auto& m : mellos)
			{
				if(m.renderID < renders.size() && m.renderID >= 0)
				{
					Render &r = renders[m.renderID];
					m.update(map.getHillHeight(r.x,r.z),{camera.x,camera.z});
				}
			}
			

		}
		
		
		//window.getView().setPortSize({960/8,540/8});
		window.clear(ws::Hue::white);
		window.draw(backSprite);
		
		float floor = map.getHillHeight(camera.x,camera.z) - 50;
		
		//incrementally reach that position for smooth movement.
		camera.y += (floor - camera.y) * 0.6;


		
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
		
		auto addObject = [&](int idx){
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
		};
		for(int idx : nearby)
			addObject(idx);
		addObject(snowman.renderID);
		for(auto& d : dogs)
			addObject(d.renderID);
		for(auto& m : mintKillers)
			addObject(m.renderID);
		for(auto& m : mellos)
			addObject(m.renderID);


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
