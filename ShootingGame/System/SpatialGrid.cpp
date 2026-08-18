#include "SpatialGrid.h"
#include <algorithm>

void SpatialGrid::Init(int mapWidth, int mapHeight, int cellSize)
{
	this->mapWidth = mapWidth;
	this->mapHeight = mapHeight;
	this->cellSize = max(1, cellSize);

	gridCols = (mapWidth + this->cellSize - 1) / this->cellSize;
	gridRows = (mapHeight + this->cellSize - 1) / this->cellSize;

	cells.resize(static_cast<size_t>(gridCols) * gridRows);
}

void SpatialGrid::Clear()
{
	for (auto& cell : cells)
	{
		cell.clear();
	}
}

void SpatialGrid::Insert(const std::shared_ptr<Craft::Actor>& actor)
{
	if (!actor || !actor->IsActive()) return;

	int idx = GetCellIndex(actor->GetPosition().x, actor->GetPosition().y);
	if (idx < 0) return;

	cells[idx].push_back(actor);
}

std::vector<std::shared_ptr<Craft::Actor>> SpatialGrid::Query(int x, int y) const
{
	std::vector<std::shared_ptr<Craft::Actor>> result;

	int col = x / cellSize;
	int row = y / cellSize;

	// 자신 + 인접 8방향 = 최대 9셀 검사.
	for (int dy = -1; dy <= 1; ++dy)
	{
		for (int dx = -1; dx <= 1; ++dx)
		{
			int c = col + dx;
			int r = row + dy;

			if (c < 0 || c >= gridCols || r < 0 || r >= gridRows)
			{
				continue;
			}

			int idx = r * gridCols + c;
			const auto& cell = cells[idx];
			result.insert(result.end(), cell.begin(), cell.end());
		}
	}

	return result;
}

int SpatialGrid::GetCellIndex(int x, int y) const
{
	if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight) return -1;

	int col = x / cellSize;
	int row = y / cellSize;

	return row * gridCols + col;
}
