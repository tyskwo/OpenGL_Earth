#include <tlocCore/tloc_core.h>
#include <tlocGraphics/tloc_graphics.h>
#include <tlocMath/tloc_math.h>

#include <iostream>

#include <gameAssetsPath.h>

using namespace tloc;

class Level
{
public:
	enum LevelInfo
	{
		EMPTY		 = 0,
		DIRT		 = 1,
		EMERALD		 = 2,
		MONEY		 = 3,
		DIGGER_SPAWN = 4,
		ENEMY_SPAWN  = 5
	};



	struct Tile
	{
		bool			isDug;
		math_t::Vec2f32 position;
		//TODO: add graphic field
	};

public:
	Level();
	~Level() { };

	int getGridWidth() { return GRID_WIDTH; };
	int getGridHeight() { return GRID_HEIGHT; };

	bool loadLevel(core_io::Path levelPath);

private:
	int GRID_WIDTH = 15;
	int GRID_HEIGHT = 10;
	tl_size NUM_TILES = GRID_WIDTH * GRID_HEIGHT;

	//TODO front texture
	//TODO backTexture

	core_conts::Array<Tile>* grid;
};