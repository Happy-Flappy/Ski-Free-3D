#ifndef MAP_H
#define MAP_H

class Map
{
	public:
	
	ws::Texture image;
	int width = 8192;
	int depth = 8192;
	int density = 40;
	int yDensity = 5;

	float TERRAIN_RADIUS = 50;	
	
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
				shape.setRadius(TERRAIN_RADIUS);
				shape.setOrigin(shape.getRadius(),0);
				shape.setPointCount(20);
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
		ws::Hue foggedColor = getLighting(ws::Hue::white,normal,{d.x,d.y,d.z},d.depth,camera.visible);

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

#endif