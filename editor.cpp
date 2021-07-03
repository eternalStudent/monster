#define ElementCapacity 50

static GUI gui = {};
static UIElement elements[ElementCapacity+1] = {};
static int32 renderOrder[ElementCapacity] = {};

static int32 selectedIndex = 0;

TextureHandle* minimapTexture;
Position2* minicameraPos;

void Select(int32 boxId) {
	selectedIndex = boxId;
}

void Save(int32 boxId) {
	SaveStream("level.dat", grid, sizeof(grid));
}

void EditorInit() {
	gui.elements = elements;
	gui.renderOrder = renderOrder;

	TextureHandle neutral = GenerateTextureFromRGBA(0x88c1daff);
	TextureHandle placeholder = GenerateTextureFromRGBA(0xff998888);
	TextureHandle grey1 = GenerateTextureFromRGBA(0xff999999);
	TextureHandle grey2 = GenerateTextureFromRGBA(0x88bbbbbb);
	TextureHandle grey3 = GenerateTextureFromRGBA(0xffaaaaaa);

	UIElement* tabControl = GetNewElement(&gui);
	tabControl->p0 = {1636.0f, 50.0f};
	tabControl->p1 = Move(tabControl->p0, 200.0f, 800.0f);
	tabControl->parent = 0;
	tabControl->flags = 1;
	tabControl->texture = neutral;
	renderOrder[40] = tabControl->index;

	UIElement* tabHead2 = GetNewElement(&gui);
	tabHead2->p0 = {100.0f, 752.0f};
	tabHead2->p1 = Move(tabHead2->p0, 88.0f, 36.0f);
	tabHead2->parent = tabControl->index;
	tabHead2->flags = 0;
	tabHead2->texture = grey3;
	renderOrder[39] = tabHead2->index;

	UIElement* tab2 = GetNewElement(&gui);
	tab2->p0 = {-88.0f, -740.0f};
	tab2->p1 = Move(tab2->p0, 176.0f, 740.0f);
	tab2->parent = tabHead2->index;
	tab2->flags = 0;
	tab2->texture = grey3;
	renderOrder[38] = tab2->index;

	for(int32 i = 1; i<=16; i++){
		UIElement* element = GetNewElement(&gui);
		element->x0 = (i % 2) ? 90.0f : 22.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->p1 = Move(element->p0, 64.0f, 64.0f);
		element->parent = tab2->index;
		element->tileId = i+16;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[21+i]=element->index;
	}

	UIElement* tabHead1 = GetNewElement(&gui);
	tabHead1->p0 = {12.0f, 752.0f};
	tabHead1->p1 = Move(tabHead1->p0, 88.0f, 36.0f);
	tabHead1->parent = tabControl->index;
	tabHead1->flags = 0;
	tabHead1->texture = grey1;
	renderOrder[21] = tabHead1->index;

	UIElement* tab1 = GetNewElement(&gui);
	tab1->p0 = {0.0f, -740.0f};
	tab1->p1 = Move(tab1->p0, 176.0f, 740.0f);
	tab1->parent = tabHead1->index;
	tab1->flags = 0;
	tab1->texture = grey1;
	renderOrder[20] = tab1->index;

	for(int32 i = 1; i<=16; i++){
		UIElement* element = GetNewElement(&gui);
		element->x0 = (i % 2) ? 90.0f : 22.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->p1 = Move(element->p0, 64.0f, 64.0f);
		element->parent = tab1->index;
		element->tileId = i;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[3+i]=element->index;
	}

	UIElement* buttonParent = GetNewElement(&gui);
	buttonParent->box = BOX2(80.0f, 950.0f, 220.0f, 1030.0f);
	buttonParent->parent = 0;
	buttonParent->flags = 1;
	buttonParent->texture = neutral;
	renderOrder[3] = buttonParent->index;

	UIElement* button = GetNewElement(&gui);
	button->box = BOX2(10.0f, 10.0f, 130.0f, 70.0f);
	button->parent = buttonParent->index;
	button->flags = 4;
	button->texture = placeholder;
	button->onClick = &Save;
	renderOrder[2] = button->index;

	UIElement* minimap = GetNewElement(&gui);
	minimap->p0 = {1600.0f, 887.0f};
	minimap->p1 = Move(minimap->p0, (W_ * 4.0f), (H_ * 4.0f));
	minimap->parent = 0;
	minimap->flags = 1;
	minimapTexture = &(minimap->texture);
	renderOrder[1] = minimap->index;

	UIElement* minicamera = GetNewElement(&gui);
	minicamera->box = BOX2(0.0f, 0.0f, X_ * 4.0f, Y_ * 4.0f);
	minicamera->parent = minimap->index;
	minicamera->flags = 1;
	minicamera->texture = grey2;
	renderOrder[0] = minicamera->index;
	minicameraPos = &(minicamera->p0);

	LoadStream("level.dat", grid, sizeof(grid));
}

void EditorUpdateAndRender(MouseEventQueue* mouseEventQueue, Position2 cursorPos) {
	ClearScreen();

	int32 mouseEvent = UpdateElements(&gui, cursorPos, mouseEventQueue);
	
	Point2 cameraPos = Scale(*minicameraPos, 0.25f);

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
	*minimapTexture = GenerateTextureFromImage(minimap, Pixelated);

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