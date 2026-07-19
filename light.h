#ifndef LIGHT_H
#define LIGHT_H

//the sun is not one of the Light objects because it would require a distance of infinity and a radius of infinity. 
//Better to just not take distance into account for the sun and only focus on diretcion normals.
ws::Vec3f sunLight = {0.5, 0.8, 0.5};

std::vector<ws::Vec3f> lightColors = 
{
	{0.05, 0.05, 0.15},//night 0
	{0.08, 0.06, 0.12},//pre-dawn 1
	{1.0, 0.5, 0.2},//dawn 2
	{1.0, 0.85, 0.6},//Morning	Soft golden‑yellow 3	
	{1.0, 0.95, 0.8},//Mid‑morning	Slightly warm white 4
	{0.95, 0.95, 1.0},//Noon Pure white with a hint of blue 5
	{1.0, 0.9, 0.7},//Afternoon	Warmer white 6
	{1.0, 0.7, 0.3},//Golden Hour (late afternoon)	Rich orange‑gold 7
	{1.0, 0.4, 0.1},//Sunset	Deep red‑orange 8
	{0.6, 0.3, 0.7},//Dusk / Twilight	Purple‑blue with pink remnants 9
	{0.2, 0.2, 0.6}//Blue hour (after sunset)	Deep blue‑purple	 10
};
ws::Vec3f lightColor = lightColors[7];


struct PointLight 
{
    ws::Vec3f position;   
    ws::Vec3f color;
    float intensity;
    float radius;         // distance at which light fades to zero
};
std::vector<PointLight> pointLights;



void makeSky(ws::Texture &tex,int width,int height)
{
	tex.create(width,height);
	
    ws::Vec3f topColor = {
        lightColor.x * 0.3f,
        lightColor.y * 0.3f,
        lightColor.z * 0.5f   // slightly more blue at zenith
    };
    ws::Vec3f bottomColor = {
        std::min(lightColor.x * 1.3f, 1.0f),
        std::min(lightColor.y * 1.3f, 1.0f),
        std::min(lightColor.z * 1.3f, 1.0f)
    };

    // Clamp to [0,1] just in case
    auto clamp01 = [](float v) { return v < 0 ? 0 : (v > 1 ? 1 : v); };
    topColor.x = clamp01(topColor.x);
    topColor.y = clamp01(topColor.y);
    topColor.z = clamp01(topColor.z);
    bottomColor.x = clamp01(bottomColor.x);
    bottomColor.y = clamp01(bottomColor.y);
    bottomColor.z = clamp01(bottomColor.z);



    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            // t = 0 at top, 1 at bottom
            float t = (float)y / (float)height;
            ws::Vec3f color = lerpVec3(topColor, bottomColor, t);

            // Convert to 0-255 and set pixel
            ws::Hue pixel(
                (int)(color.x * 255),
                (int)(color.y * 255),
                (int)(color.z * 255)
            );
            tex.setPixel(x, y, pixel);
        }
    }	
}


ws::Hue getLighting(ws::Hue base,ws::Vec3f normal,ws::Vec3f position,float depth,float camVisible)
{

    float sunDiffuse = std::max(0.0f, normal.x * sunLight.x + 
                                     normal.y * sunLight.y + 
                                     normal.z * sunLight.z);
    float ambient = 0.25f;
    float sunBrightness = ambient + (1.0f - ambient) * sunDiffuse;
    ws::Vec3f sunColor = {
        sunBrightness * lightColor.x,
        sunBrightness * lightColor.y,
        sunBrightness * lightColor.z
    };

	ws::Vec3f totalLightColor = sunColor; 
	
    for(auto& pl : pointLights) 
	{
        ws::Vec3f dirToLight = pl.position - position;
        float dist = getDistance(dirToLight);
        if (dist < 0.0001f) continue;       // skip if light is exactly at the point

        ws::Vec3f dir = dirToLight / dist;
		
        // linear falloff inside the radius
        float t = std::clamp(dist / pl.radius, 0.0f, 1.0f);
		// quadratic falloff feels more natural
        float attenuation = 1.0f - t * t;    
        
        float pointDiffuse = std::max(0.0f, normal.x * dir.x + 
                                            normal.y * dir.y + 
                                            normal.z * dir.z);
        //add the diffuse contribution form this light to the color.
		ws::Vec3f pointContrib = pointDiffuse * pl.intensity * attenuation * pl.color;
        totalLightColor += pointContrib;
    }	
	
    totalLightColor.x = std::min(totalLightColor.x, 1.0f);
    totalLightColor.y = std::min(totalLightColor.y, 1.0f);
    totalLightColor.z = std::min(totalLightColor.z, 1.0f);
	
	
    ws::Hue litColor = ws::Hue(
        (int)(base.r * totalLightColor.x),
        (int)(base.g * totalLightColor.y),
        (int)(base.b * totalLightColor.z)
    );	


	ws::Hue::HSV hsv = litColor.toHSV();

	float fogFactor = 1.0f - std::clamp(depth / camVisible, 0.0f, 1.0f);
	
	//desaturate
	hsv.s *= fogFactor;
    
/* 	//darkening fog
	const float darkTarget = 0.05f; // minimum brightness when fully fogged
    hsv.v = hsv.v * fogFactor + darkTarget * (1.0f - fogFactor);
	 */
	ws::Hue finalColor = hsv.toHue();
	return finalColor;
}


#endif