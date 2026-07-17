#ifndef LIGHT_H
#define LIGHT_H

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
ws::Vec3f lightColor = lightColors[5];


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

#endif