#ifndef SPGRID_H
#define SPGRID_H

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

#endif