// Sprites
//----------------------- 

struct Sprite {
	union {
		TextureHandle spritesheet;
		TextureHandle tileset;
		TextureHandle texture;
	};
	Box2 crop;
};

typedef Sprite Brush;

inline Brush GenerateBrush(uint32 rgba) {
	Sprite brush;
	brush.texture = GenerateTextureFromRGBA(rgba);
	brush.crop = BOX2_UNIT();
	return brush;
}

inline void RenderSprite(Sprite sprite, Box2 pos) {
	RenderBox2(sprite.texture, sprite.crop, pos);
}

static Brush blackBrush;

void TilesInit() {
	blackBrush = GenerateBrush(0xff000000);
}


// Tiles  
//-----------------------             

struct Tile {
	int32 index;
	union {
		Sprite sprite;
		struct {TextureHandle tileset; Box2 crop;};
	};
	uint32 flags; // 1 - blocking, 2 - disapper on touch
};

static Tile tiles[1024] = {};

#define W_ 62
#define H_ 34

// TODO: either these should be computed, or the tile-size
// probably should be moved
#define X_ 31
#define Y_ 17

static int32 grid[W_][H_] = {};

void SpritesInit() {
	TextureHandle tileset = GenerateTextureFromFile("grass_tileset.bmp", Pixelated);
	for (int32 i=0; i<16; i++){
		int32 x = i / 4;
		int32 y = i % 4;
		Tile* tile = &tiles[i+1];
		tile->tileset = tileset;
		tile->crop = {x*0.25f, y*0.25f, (x+1)*0.25f, (y+1)*0.25f};
		tile->flags = 1;
	}

	tileset = GenerateTextureFromFile("stuff_tileset.bmp", Pixelated);
	for (int32 i=0; i<16; i++){
		int32 x = i / 4;
		int32 y = i % 4;
		Tile* tile = &tiles[i+17];
		tile->tileset = tileset;
		tile->crop = {x*0.25f, y*0.25f, (x+1)*0.25f, (y+1)*0.25f};
		tile->flags = 2;
	}
}