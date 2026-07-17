auto resolveSphereCollision = [](ws::Vec3f& vel)
{
	float playerRadius = 0.0f;
    const float radius = map.TERRAIN_RADIUS;
    const float combinedRadius = radius + playerRadius;
	
	const float epsilon = 0.1f;          // small push‑out margin
    const int maxIter = 3;               // few iterations for stability
	
    for(int iter = 0; iter < maxIter; iter++)
    {
        bool any = false;
        std::vector<int> nearby;
        grid.query(camera.x, camera.z, combinedRadius + 10.0f, nearby);

        for(int idx : nearby)
        {
            Render& d = renders[idx];
            if(d.shapeID == -1) continue;   
			
            ws::Vec3f center = { d.x, d.y, d.z };
            ws::Vec3f diff = {
                camera.x - center.x,
                camera.y - center.y,
                camera.z - center.z
            };
            float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);

            if(dist < combinedRadius)
            {
                any = true;
                // If exactly at center, push upward
                if (dist < 0.0001f) {
                    diff = { 0.0f, 1.0f, 0.0f };
                    dist = 1.0f;
                }
				if(diff.x == 0)
					diff.x = 0.001;
				if(diff.y == 0)
					diff.y = 0.001;
				if(diff.z == 0)
					diff.z = 0.001;
				ws::Vec3f normal = {diff.x / dist,diff.y/dist,diff.z/dist};

                // Move camera outside sphere
                float overlap = combinedRadius - dist + epsilon;
                camera.x += normal.x * overlap;
                camera.y += normal.y * overlap;
                camera.z += normal.z * overlap;

                // Cancel velocity component that moves into the sphere
                float velDot = vel.x*normal.x + vel.y*normal.y + vel.z*normal.z;
                if (velDot < 0.0f) {
                    vel.x -= velDot * normal.x;
                    vel.y -= velDot * normal.y;
                    vel.z -= velDot * normal.z;
                }
            }
        }
        if(!any) 
			break;
    }
};