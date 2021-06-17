struct BakedChar {
   uint16 x0,y0,x1,y1; // coordinates of bbox in bitmap
   float32 xoff,yoff,xadvance;
};

struct BakedFont {
	BakedChar chardata[96];
	TextureHandle texture;
};

// TODO: accept RGB as argument, accept pixel_height as argument
BakedFont BakeFont(const char* filePath){
	BakedFont font;
	byte* data = (byte*)LoadStream(filePath);
	byte* mono_bitmap = (byte*)Alloc(512*512);
	float32 pixel_height = 24.0f;
	int32 pw = 512;
	int32 ph = 512;
	int32 first_char = 32;
	int32 num_chars = 96;
	float32 scale;
   	int32 x,y,bottom_y, i;
   	FontInfo f;
   	if (!GetFontInfo(&f, data, 0))
    	 return {};
   	memset(mono_bitmap, 0, pw*ph); // background of 0 around pixels
   	x=y=1;
   	bottom_y = 1;

   	scale = ScaleForPixelHeight(&f, pixel_height);

   	for (i=0; i < num_chars; ++i) {
   		int32 advance, lsb, x0,y0,x1,y1,gw,gh;
      	int32 g = FindGlyphIndex(&f, first_char + i);
      	GetGlyphHMetrics(&f, g, &advance, &lsb);
      	GetGlyphBitmapBox(&f, g, scale,scale, &x0,&y0,&x1,&y1);
      	gw = x1-x0;
      	gh = y1-y0;
      	if (x + gw + 1 >= pw)
        	y = bottom_y, x = 1; // advance to next row
      	if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         	return {};
      	Assert(x+gw < pw);
      	Assert(y+gh < ph);
      	MakeGlyphBitmap(&f, mono_bitmap+x+y*pw, gw,gh,pw, scale,scale, g);
      	font.chardata[i].x0 = (int16) x;
      	font.chardata[i].y0 = (int16) y;
      	font.chardata[i].x1 = (int16) (x + gw);
      	font.chardata[i].y1 = (int16) (y + gh);
      	font.chardata[i].xadvance = scale * advance;
      	font.chardata[i].xoff     = (float32) x0;
      	font.chardata[i].yoff     = (float32) y0;
      	x = x + gw + 1;
      	if (y+gh+1 > bottom_y)
         	bottom_y = y+gh+1;
   	}

	byte* bitmap = ExpandChannels(mono_bitmap, 512 * 512);

	font.texture = GenerateTextureFromImage(Image{512, 512, 4, bitmap}, Smooth);

	return font;
}

// NOTE: adopted from stbtt_GetBakedQuad
void PrintText(BakedFont font, float32 x, float32 y, char* text){
	Box2 crop;
	while (*text) {
		if (*text >= 32 && *text < 128) {
			float32 ipw = 1.0f / 512, iph = 1.0f / 512; // NOTE: division takes longer than multipication, this runs division at compile time.
			const BakedChar* bakedchar = font.chardata + (*text - 32);

			int32 round_y = (int32)floorf(y + bakedchar->yoff + 0.5f); // NOTE: floor rounds towards -infinty, and casting rounds toward zero.
			float32 y1 = (float32)(round_y + bakedchar->y1 - bakedchar->y0);
			float32 printy = 2 * y - y1;

			int32 round_x = (int32)floorf(x + bakedchar->xoff + 0.5f);
			float32 printx = round_x + 0.5f * (bakedchar->x1 - bakedchar->x0);

			crop.x0 = bakedchar->x0 * ipw;
			crop.y0 = bakedchar->y0 * iph;
			crop.x1 = bakedchar->x1 * ipw;
			crop.y1 = bakedchar->y1 * iph;

			x += bakedchar->xadvance;

			Render(
				font.texture, crop,
				Vector2{ printx, printy },
				(pixels)(bakedchar->x1 - bakedchar->x0),
				(pixels)(bakedchar->y1 - bakedchar->y0),
				0);
		}
		++text;
	}
}