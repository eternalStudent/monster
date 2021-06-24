struct BakedChar {
   union {
   	Box2i box;
   	struct {Point2i p0, p1;};
   	struct {int32 x0, y0, x1, y1;};
   };
   float32 xoff, yoff, xadvance;
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
	FontInfo f;
   if (!GetFontInfo(&f, data, 0))
    	return {};
   memset(mono_bitmap, 0, pw*ph); // background of 0 around pixels
   int32 x = 1;
   int32 y = 1;
   int32 bottom_y = 1;

   float32 scale = ScaleForPixelHeight(&f, pixel_height);

   for (int32 i=0; i < num_chars; ++i) {
   	int32 advance, lsb, x0, y0, x1, y1, gw, gh;
      int32 g = FindGlyphIndex(&f, first_char + i);
      GetGlyphHMetrics(&f, g, &advance, &lsb);
      GetGlyphBitmapBox(&f, g, scale, scale, &x0, &y0, &x1, &y1);
      gw = x1 - x0;
      gh = y1 - y0;
      if (x + gw + 1 >= pw)
      y = bottom_y, x = 1; // advance to next row
      if (y + gh + 1 >= ph) // check if it fits vertically AFTER potentially moving to next row
         return {};
      Assert(x + gw < pw);
      Assert(y + gh < ph);
      MakeGlyphBitmap(&f, mono_bitmap + x + y*pw, gw, gh, pw, scale, scale, g);
      font.chardata[i].x0 = x;
      font.chardata[i].y0 = y;
      font.chardata[i].x1 = x + gw;
      font.chardata[i].y1 = y + gh;
      font.chardata[i].xadvance = scale * advance;
      font.chardata[i].xoff     = (float32) x0;
      font.chardata[i].yoff     = (float32) y0;
      x = x + gw + 1;
      if (y + gh + 1 > bottom_y)
         bottom_y = y + gh + 1;
   }

	byte* bitmap = ExpandChannels(mono_bitmap, pw*ph);

	font.texture = GenerateTextureFromImage(Image{pw, ph, 4, bitmap}, Smooth);

	return font;
}

// NOTE: adopted from stbtt_GetBakedQuad
void PrintText(BakedFont font, float32 x, float32 y, char* text){
	int32 pw = 512;
	int32 ph = 512;
	int32 first_char = 32;
	int32 last_char = 128;

	while (*text) {
		if (*text >= first_char && *text < last_char) {
			float32 ipw = 1.0f / pw, iph = 1.0f / ph; // NOTE: division takes longer than multipication, this runs division at compile time.
			const BakedChar* bakedchar = font.chardata + (*text - first_char);

			int32 round_y = (int32)floorf(y + bakedchar->yoff + 0.5f); // NOTE: floor rounds towards -infinty, and casting rounds toward zero.
			int32 y1 = round_y + bakedchar->y1 - bakedchar->y0;
			int32 printy = 2 * (int32)y - y1;

			int32 round_x = (int32)floorf(x + bakedchar->xoff + 0.5f);

			Box2 pos = fBox2i(Box2_MoveTo(bakedchar->box, {round_x, printy}));

			Box2 crop = BOX2(
				bakedchar->x0 * ipw,
				bakedchar->y0 * iph,
				bakedchar->x1 * ipw,
				bakedchar->y1 * iph);

			RenderBox2(font.texture, crop, pos, 1);

			x += bakedchar->xadvance;
		}
		++text;
	}
}