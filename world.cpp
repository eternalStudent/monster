#include "camera.cpp"
#define TILE_TERRAIN 		1
#define TILE_OBJECT			2

struct Tile {
	TextureId atlas;
	Box2 crop;
	uint32 flags;
};

#define WORLD_BACKGROUND			RGBA_BLUE
#define WORLD_WIDTH					62
#define WORLD_HEIGHT				32	
#define DATA(x, y)					world.data[y*WORLD_WIDTH + x]			

struct {
	byte* data;
	Tile tiles[256];
	Arena* arena;
} world;

void WorldInit(Arena* persist, Arena* scratch) {
	world.arena = scratch;
	world.data = (byte*)ArenaAlloc(persist, WORLD_WIDTH*WORLD_HEIGHT);
	memset(world.data, 0, WORLD_WIDTH*WORLD_HEIGHT);

	byte tileId = 1;
	TextureId grass = LoadTexture(scratch, "data/grass_tileset.bmp", GRAPHICS_PIXELATED);
	for (int32 i=0; i<4; i++) for (int32 j=0; j<4; j++) {
		Tile* tile = &(world.tiles[tileId]);
		tile->atlas = grass;
    	tile->crop = {i*0.25f, j*0.25f, (i+1)*0.25f, (j+1)*0.25f};
    	tile->flags = TILE_TERRAIN;
    	tileId++;
	}

	TextureId stuff = LoadTexture(scratch, "data/stuff_tileset.bmp", GRAPHICS_PIXELATED);
	for (int32 i=0; i<4; i++) for (int32 j=0; j<4; j++) {
		Tile* tile = &(world.tiles[tileId]);
		tile->atlas = stuff;
    	tile->crop = {i*0.25f, j*0.25f, (i+1)*0.25f, (j+1)*0.25f};
    	tile->flags = TILE_OBJECT;
    	tileId++;
	}
}

void WorldDrawTiles() {
	for (int32 x = 0; x < WORLD_WIDTH; x++) for (int32 y = 0; y < WORLD_HEIGHT; y++) {
		byte tileId = DATA(x, y);
		if (tileId == 0) continue;
		Tile tile = world.tiles[tileId];

		Point2 worldPos0 = TilePosToWorldPos(x, y);
		Point2 worldPos1 = TilePosToWorldPos(x + 1, y + 1);
		Point2 p0 = WorldPosToScreenPos(worldPos0.x, worldPos0.y);
		Point2 p1 = WorldPosToScreenPos(worldPos1.x, worldPos1.y);

		Box2 pos = {p0.x, p0.y, p1.x, p1.y};
		DrawImage(tile.atlas, tile.crop, pos);
	}
}

void WorldSetTile(Point2i cursorPos, byte tileId, TextureId minimap) {
	Point2 screenPos = CursorPosToScreenPos(cursorPos.x, cursorPos.y);
	Point2 worldPos = ScreenPosToWorldPos(screenPos.x, screenPos.y);
	Point2i tilePos = WorldPosToTilePos(worldPos.x, worldPos.y);

	DATA(tilePos.x, tilePos.y) = tileId;

	__m128i v = tileId ? _mm_set1_epi32(RGBA_GREY) : _mm_set1_epi32(0x22413830);
	__m128i data[4] = {v, v, v, v};
	UpdateTextureData(minimap, 4*tilePos.x, 4*tilePos.y, 4, 4, data);
}

void WorldReadData(const char* filePath, TextureId minimap) {
	OsReadToBuffer(world.data, filePath);

	__m128i* data = (__m128i*)ArenaAlloc(world.arena, (WORLD_WIDTH*4)*(WORLD_HEIGHT*4)*4);
	for (int32 x=0; x<WORLD_WIDTH; x++) for (int32 i=0; i<4; i++) for (int32 y=0; y<WORLD_HEIGHT; y++) {
		data[(y*4 + i)*WORLD_WIDTH + x] = DATA(x, y) 
			? _mm_set1_epi32(RGBA_GREY) 
			: _mm_set1_epi32(0x22413830);
	}

	UpdateTextureData(minimap, 0, 0, (WORLD_WIDTH*4), (WORLD_HEIGHT*4), data);
}

void WorldWriteData(const char* filePath) {
	OsWriteAll({world.data, WORLD_WIDTH*WORLD_HEIGHT}, filePath);
}

void* WorldGetCleanMinimap() {
	__m128i* data = (__m128i*)ArenaAlloc(world.arena, (WORLD_WIDTH*4)*(WORLD_HEIGHT*4)*4);
	for (int32 i = 0; i<WORLD_WIDTH*WORLD_HEIGHT*4; i++) data[i] = _mm_set1_epi32(0x22413830);
	return data;
}

void WorldClearData(TextureId minimap) {
	memset(world.data, 0, WORLD_WIDTH*WORLD_HEIGHT);

	void* data = WorldGetCleanMinimap();
	UpdateTextureData(minimap, 0, 0, (WORLD_WIDTH*4), (WORLD_HEIGHT*4), data);
}

uint32 WorldGetFlags(int32 x, int32 y) {
	return world.tiles[DATA(x, y)].flags;
}