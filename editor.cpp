#define TileCount 	 22
#define ElementCount TileCount+7  // 29
#define I 			 TileCount+1  // 23
#define MM 			 I+3		  // 26

static GUI gui = {};
static UIElement elements[ElementCount+1] = {};
static int32 renderOrder[ElementCount] = {};

static int32 selectedIndex = 0;
static TextureHandle black;

void Select(int32 boxId) {
	selectedIndex = boxId;
}

void Save(int32 boxId) {
	SaveStream("level.dat", grid, sizeof(grid));
}

void EditorInit() {
	gui.elementCount = 1;
	gui.elements = elements;
	gui.renderOrder = renderOrder;

	LoadStream("level.dat", grid, sizeof(grid));

	TextureHandle neutral = GenerateTextureFromRGBA(0x88c1daff);
	TextureHandle placeholder = GenerateTextureFromRGBA(0xff998888);
	TextureHandle grey1 = GenerateTextureFromRGBA(0x88999999);
	TextureHandle grey2 = GenerateTextureFromRGBA(0x88bbbbbb);

	for(int32 i = 1; i<=TileCount; i++){
		UIElement* element = GetNewElement(&gui);
		element->x0 = (i % 2) ? 102.0f : 34.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->x1 = (i % 2) ? 166.0f : 98.0f;
		element->y1 = element->y0 + 64.0f;
		element->parent = I;
		element->tileId = i > 16 ? 0 : i;
		element->texture = placeholder;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[i-1]=i;
	}

	UIElement* tab = GetNewElement(&gui);
	tab->box = BOX2(50.0f, 50.0f, 250.0f, 850.0f);
	tab->parent = 0;
	tab->flags = 1;
	tab->texture = neutral;
	renderOrder[I-1]=I;

	UIElement* buttonParent = GetNewElement(&gui);
	buttonParent->box = BOX2(80.0f, 950.0f, 220.0f, 1030.0f);
	buttonParent->parent = 0;
	buttonParent->flags = 1;
	buttonParent->texture = neutral;
	renderOrder[I+1]=I+1;

	UIElement* button = GetNewElement(&gui);
	button->box = BOX2(10.0f, 10.0f, 130.0f, 70.0f);
	button->parent = I+1;
	button->flags = 4;
	button->texture = placeholder;
	button->onClick = &Save;
	renderOrder[I]=I+2;

	UIElement* minimap = GetNewElement(&gui);
	minimap->x0 = 1600.0f;
	minimap->y0 = 850.0f;
	minimap->x1 = 1600.0f + (W_ * 4.0f);
	minimap->y1 = 850.0f + (H_ * 4.0f);
	minimap->parent = 0;
	minimap->flags = 1;
	minimap->texture = grey1;
	renderOrder[I+3]=I+3;

	UIElement* minicamera = GetNewElement(&gui);
	minicamera->box = BOX2(0.0f, 0.0f, X_ * 4.0f, Y_ * 4.0f);
	minicamera->parent = I+3;
	minicamera->flags = 1;
	minicamera->texture = grey2;
	renderOrder[I+2]=I+4;

	UIElement* slider = GetNewElement(&gui);
	slider->box = BOX2(80.0f, 916.0f, 344.0f, 924.0f);
	slider->parent = 0;
	slider->flags = 1;
	slider->texture = neutral;
	renderOrder[I+5]=I+5;

	UIElement* sliderPos = GetNewElement(&gui);
	sliderPos->box = BOX2(128.0f, 0.0f, 136.0f, 8.0f);
	sliderPos->parent = I+5;
	sliderPos->flags = 1;
	sliderPos->texture = black;
	renderOrder[I+4]=I+6;
}

void EditorUpdateAndRender(MouseEventQueue* mouseEventQueue, Position2 cursorPos) {
	ClearScreen();

	int32 mouseEvent = UpdateElements(&gui, cursorPos, mouseEventQueue);
	
	Point2 cameraPos = Scale(elements[I+4].p0, 0.25f);

	// Update grid
	if (gui.elementIndex == 0 && selectedIndex != 0) {
		int32 tileX = (int32)((cursorPos.x + 32.0f) / 64.0f + cameraPos.x);
		int32 tileY = (int32)(cursorPos.y / 64.0f + cameraPos.y);
		if (mouseEvent == LDN){
			grid[tileX][tileY] = elements[selectedIndex].tileId;
		}
		else if (mouseEvent == RDN){
			grid[tileX][tileY] = 0;
		}
	}

	// Update minimap according to grid
	Image minimap;
	minimap.dimensions = {W_*4, H_*4};
	minimap.channels = 4;
	minimap.data = (byte*)Alloc((W_*4)*(H_*4)*4);
	for (int32 x=0; x<W_; x++) for (int32 i=0; i<4; i++) for (int32 y=0; y<H_; y++){
		((__m128i*)minimap.data)[y*4*W_ + i*W_ + x] = grid[x][y] 
			? _mm_set1_epi32(0xff000000) 
			: _mm_set1_epi32(0x88999999);
	}
	elements[MM].tileId = 0;
	elements[MM].texture = GenerateTextureFromImage(minimap, Pixelated);

	// Render grid
	for (int32 x = (int32)cameraPos.x; x < (int32)cameraPos.x+X_; x++) 
	for (int32 y = (int32)cameraPos.y; y < (int32)cameraPos.y+Y_; y++) {
		int32 tileId = grid[x][y];
		if (tileId == 0)
			continue;

		Tile tile = sprites[tileId];
		Render(
			tile.tileset, tile.crop,
			{(x-cameraPos.x)*64.0f, (y-cameraPos.y)*64.0f},
			64.0f, 64.0f,
			0);
	}

	RenderElements(gui);

	// Debug text
	char buffer[256];
	char* str;

	str = buffer;
	str += CopyString("cursor: (", 9, str); str += float32ToDecimal(cursorPos.x, 0, str); 
	str += CopyString(", ", 2, str); str += float32ToDecimal(cursorPos.y, 0, str); memcpy(str, ")", 2);
	DebugPrintText(16, 32.0 * 28, buffer);

	str = buffer;
	str += CopyString("camera: (", 9, str); str += float32ToDecimal(cameraPos.x, 0, str); 
	str += CopyString(", ", 2, str); str += float32ToDecimal(cameraPos.y, 0, str); memcpy(str, ")", 2);
	DebugPrintText(16, 32.0 * 27, buffer);

	if (gui.elementIndex != 0){
		UIElement* element = &elements[gui.elementIndex];
		str = buffer;
		str += CopyString("element", 7, str);
		str += int32ToDecimal(gui.elementIndex, str);
		str += CopyString(": {x0=", 6, str);
		str += float32ToDecimal(element->x0, 1, str);
		str += CopyString(" y0=", 4, str);
		str += float32ToDecimal(element->y0, 1, str);
		str += CopyString(" x1=", 4, str);
		str += float32ToDecimal(element->x1, 0, str);
		str += CopyString(" y1=", 4, str);
		str += float32ToDecimal(element->y1, 0, str);
		str += CopyString(" flags=", 7, str);
		str += uint32ToDecimal(element->flags, str);
		memcpy(str, "}", 2);
		DebugPrintText(16, 32.0 * 25, buffer);
	}
}