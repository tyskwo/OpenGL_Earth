#include "Level.h"

Level::Level()
{
	grid = new core_conts::Array<Tile>(NUM_TILES);
};

bool Level::loadLevel(core_io::Path levelPath)
{

	std::cout << "\n";

	float row = 0, col = 0;

	core_io::FileIO_ReadA levelFile(levelPath);

	if (levelFile.Open() != ErrorSuccess)
	{
		TLOC_LOG_CORE_INFO() << "Could not open " << levelPath;
		return false;
	}

	core_str::String levelFileContents;
	levelFile.GetContents(levelFileContents);

	for (auto iter = levelFileContents.begin(); iter != levelFileContents.end(); ++iter)
	{
		if (*iter == '\n')
		{
			std::cout << "\n";
			row++;
		}
		else if (*iter != ' ')
		{
			col++;
			Tile newTile;
			newTile.position = math_t::Vec2f32(col, row);

			switch (*iter)
			{
			case EMPTY:
				newTile.isDug = true;
				break;
			case DIRT:
				newTile.isDug = false;
				break;
			case EMERALD:
				newTile.isDug = false;
				//TODO: Add emerald entity at this location
				break;
			case MONEY:
				newTile.isDug = false;
				//TODO: Add MoneyBag entity at this location
				break;
			case DIGGER_SPAWN:
				newTile.isDug = false;
				//TODO: Add DiggerSpawn entity at this location
				break;
			case ENEMY_SPAWN:
				newTile.isDug = false;
				//TODO: Add EnemySpawn entity at this location
				break;
			default:
				newTile.isDug = false;
				break;
			}

			grid->push_back(newTile);

			std::cout << *iter << " ";
		}

	}

	return true;
};