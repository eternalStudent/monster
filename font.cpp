struct PreRenderedFont {
	stbtt_bakedchar cdata[96];
	TextureHandle texture;
};

// TODO: accept RGB as argument, accept pixel_height as argument
PreRenderedFont PreRenderFont(const char* filePath){
	PreRenderedFont font;
	void* ttf_buffer = LoadStream(filePath);
	byte* mono_bitmap = (byte*)Alloc(512*512);
	stbtt_BakeFontBitmap((byte*)ttf_buffer, 0, 24.0, mono_bitmap, 512, 512, 32, 96, font.cdata); // no guarantee this fits!
	byte* bitmap = ExpandChannels(mono_bitmap, 512 * 512);

	font.texture = GenerateTextureFromImage(Image{512, 512, 4, bitmap}, Smooth);

	return font;
}

// NOTE: adopted from stbtt_GetBakedQuad
void PrintText(PreRenderedFont font, float32 x, float32 y, char* text){
	Box2 crop;
	while (*text) {
		if (*text >= 32 && *text < 128) {
			float32 ipw = 1.0f / 512, iph = 1.0f / 512; // NOTE: division takes longer than multipication, this runs division at compile time.
			const stbtt_bakedchar* bakedchar = font.cdata + (*text - 32);

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