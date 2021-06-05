/*
	TODO:
	- grid should be world grid
*/

struct Sprite{
	union{
		TextureHandle spritesheet;
		TextureHandle tileset;
		TextureHandle solid;
		TextureHandle texture;
	};
	Rectanglef crop;
};

// TODO: probably Tile would have to be distinct from Sprite
// in the future
typedef Sprite Tile;

static Sprite sprites[1024] = {};

#define X_ 31
#define Y_ 17
#define W_ 62
#define H_ 34
static int32 grid[W_][H_] = {};

void SpritesInit(){
	TextureHandle tileset = GenerateTextureFromFile("grass_tileset.bmp", Pixelated);
	for (int i=0; i<16; i++){
		int32 x = i / 4;
		int32 y = i % 4;
		Sprite* tile = &sprites[i+1];
		tile->tileset = tileset;
		tile->crop = {x*0.25f, y*0.25f, (x+1)*0.25f, (y+1)*0.25f};
	}
}