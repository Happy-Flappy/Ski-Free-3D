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
		float diffuse = std::max(0.0f, normal.x * sunLight.x + 
									  normal.y * sunLight.y + 
									  normal.z * sunLight.z);
		
		float ambient = 0.25f;
		float brightness = ambient + (1.0f - ambient) * diffuse;

		ws::Hue base = ws::Hue::white;
		
		
		ws::Vec3f tinted = {brightness * lightColor.x, brightness * lightColor.y, brightness * lightColor.z};
		ws::Hue litColor = ws::Hue(
			(int)(base.r * tinted.x),
			(int)(base.g * tinted.y),
			(int)(base.b * tinted.z)
		);				


		ws::Hue::HSV hsv = litColor.toHSV();

		float fogFactor = 1.0f - std::clamp(d.depth / camera.visible, 0.0f, 1.0f);
		hsv.s *= fogFactor;
		ws::Hue foggedColor = hsv.toHue();

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