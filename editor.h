float buildDepth = 250;
void placeTerrain(ws::Window& window)
{
    static bool lastSpace = false;
	static bool lastInc = false;
	static bool lastDec = false;
    bool space = ws::Global::getButton(ws::Key::Space);
	bool inc,dec;
	inc = ws::Global::getButton(ws::Key::Up);
	dec = ws::Global::getButton(ws::Key::Down);
	

    if (space && !lastSpace)
    {
        int centerX = window.getView().getSize().x / 2;
        int centerY = window.getView().getSize().y / 2;

		float scaleAtDepth = (float)camera.perspective / buildDepth;
		float screenRadius = map.TERRAIN_RADIUS * scaleAtDepth;
		
		ws::Vec3f worldPos = camera.unproject(centerX + screenRadius, centerY + screenRadius, buildDepth);

        Render r;
        r.x = worldPos.x;
        r.y = worldPos.y;
        r.z = worldPos.z;
        r.shapeID = shapes.size();
        
		ws::Radial shape;
		shape.setFillColor(ws::Hue::white);
		shape.setBorderWidth(0);
		shape.setRadius(50);
		shape.setOrigin(shape.getRadius(),shape.getRadius());
		shape.setPointCount(20);
		r.shapeID = shapes.size();
		shapes.push_back(shape);
		renders.push_back(r); 

        grid.build(renders);
    }
    lastSpace = space;
	
	
	if(inc != lastInc)
		buildDepth += 10;
	inc = lastInc;
	if(dec != lastDec)
		buildDepth -= 10;
	dec = lastDec;
}