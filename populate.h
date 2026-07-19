#ifndef POP_H
#define POP_H

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
	for(int a=0;a<100;a++)
	{
		int x = std::rand()% map.width;
		int z = std::rand()% map.depth;
		int y = map.getHillHeight(x,z) + 50;
		
		PointLight lamp;
		lamp.position = {x,y,z};
		float cr = (float(std::rand()%255)+1.0f) / 255;
		float cg = (float(std::rand()%255)+1.0f) / 255;
		float cb = (float(std::rand()%255)+1.0f) / 255;
		
		
		lamp.color = {cr,cg,cb};       // warm yellow
		lamp.intensity = 0.8f;                 
		lamp.radius = 300.0f;                    
		pointLights.push_back(lamp);
		
		Render r;
		r.x = x;
		r.y = y;
		r.z = z;
		r.spriteID = sprites.size();
		ws::Sprite spr;
		spr.setTexture(lampTex);
		spr.setOrigin(lampTex.getSize().x/2,lampTex.getSize().y * 0.9);
		r.scale = {1.2,1.2};
		sprites.push_back(spr);
		renders.push_back(r);		
	}
	
}


#endif