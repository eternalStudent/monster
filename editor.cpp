static int32 tileIds[ElementCapacity] = {};
static int32 selectedTileId = 0;

TextureHandle* minimapTexture;
Position2* minicameraPos;

static bool mouseIsDown = false;

void Select(UIElement* element) {
	selectedTileId = tileIds[element->index];
}

void Save(UIElement* element) {
	SaveStream("level.dat", grid, sizeof(grid));
}

void Run(UIElement* element) {
	GameInit();
}

void EditorInit() {
	Brush strawYellow = GenerateBrush(0x88c1daff);
	Brush grey1 = GenerateBrush(0xff999999);
	Brush grey2 = GenerateBrush(0x88bbbbbb);
	Brush grey3 = GenerateBrush(0xffaaaaaa);

	UIElement* tabControl = GetNewElement(&gui);
	tabControl->p0 = {1636.0f, 50.0f};
	tabControl->p1 = Move(tabControl->p0, 200.0f, 800.0f);
	tabControl->parent = 0;
	tabControl->flags = 1;
	tabControl->background = strawYellow;
	renderOrder[41] = tabControl->index;

	UIElement* tabHead2 = GetNewElement(&gui);
	tabHead2->p0 = {100.0f, 752.0f};
	tabHead2->p1 = Move(tabHead2->p0, 88.0f, 36.0f);
	tabHead2->parent = tabControl->index;
	tabHead2->flags = 0;
	tabHead2->background = grey3;
	renderOrder[40] = tabHead2->index;

	UIElement* tab2 = GetNewElement(&gui);
	tab2->p0 = {-88.0f, -740.0f};
	tab2->p1 = Move(tab2->p0, 176.0f, 740.0f);
	tab2->parent = tabHead2->index;
	tab2->flags = 0;
	tab2->background = grey3;
	renderOrder[39] = tab2->index;

	for(int32 i = 1; i<=16; i++) {
		UIElement* element = GetNewElement(&gui);
		element->x0 = (i % 2) ? 90.0f : 22.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->p1 = Move(element->p0, 64.0f, 64.0f);
		element->parent = tab2->index;
		element->background = tiles[i+16].sprite;
		tileIds[element->index] = i+16;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[22+i]=element->index;
	}

	UIElement* tabHead1 = GetNewElement(&gui);
	tabHead1->p0 = {12.0f, 752.0f};
	tabHead1->p1 = Move(tabHead1->p0, 88.0f, 36.0f);
	tabHead1->parent = tabControl->index;
	tabHead1->flags = 0;
	tabHead1->background = grey1;
	renderOrder[22] = tabHead1->index;

	UIElement* tab1 = GetNewElement(&gui);
	tab1->p0 = {0.0f, -740.0f};
	tab1->p1 = Move(tab1->p0, 176.0f, 740.0f);
	tab1->parent = tabHead1->index;
	tab1->flags = 0;
	tab1->background = grey1;
	renderOrder[21] = tab1->index;

	for(int32 i = 1; i<=16; i++) {
		UIElement* element = GetNewElement(&gui);
		element->x0 = (i % 2) ? 90.0f : 22.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->p1 = Move(element->p0, 64.0f, 64.0f);
		element->parent = tab1->index;
		element->background = tiles[i].sprite;
		tileIds[element->index] = i;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[4+i]=element->index;
	}

	UIElement* buttonParent = GetNewElement(&gui);
	buttonParent->box = BOX2(80.0f, 950.0f, 370.0f, 1030.0f);
	buttonParent->parent = 0;
	buttonParent->flags = 1;
	buttonParent->background = strawYellow;
	renderOrder[4] = buttonParent->index;

	UIElement* saveButton = GetNewElement(&gui);
	saveButton->box = BOX2(10.0f, 10.0f, 130.0f, 70.0f);
	saveButton->parent = buttonParent->index;
	saveButton->flags = 4;
	saveButton->background = GenerateBrush(0xff998888);
	saveButton->onClick = &Save;
	renderOrder[3] = saveButton->index;

	UIElement* runButton = GetNewElement(&gui);
	runButton->box = BOX2(150.0f, 10.0f, 280.0f, 70.0f);
	runButton->parent = buttonParent->index;
	runButton->flags = 4;
	runButton->background = GenerateBrush(0xff998888);
	runButton->onClick = &Run;
	renderOrder[2] = runButton->index;

	UIElement* minimap = GetNewElement(&gui);
	minimap->p0 = {1600.0f, 887.0f};
	minimap->p1 = Move(minimap->p0, (W_ * 4.0f), (H_ * 4.0f));
	minimap->parent = 0;
	minimap->flags = 1;
	minimap->crop = BOX2_UNIT();
	minimapTexture = &(minimap->texture);
	renderOrder[1] = minimap->index;

	UIElement* minicamera = GetNewElement(&gui);
	minicamera->box = BOX2(0.0f, 0.0f, X_ * 4.0f, Y_ * 4.0f);
	minicamera->parent = minimap->index;
	minicamera->flags = 1;
	minicamera->background = grey2;
	renderOrder[0] = minicamera->index;
	minicameraPos = &(minicamera->p0);

	LoadStream("level.dat", grid, sizeof(grid));
}

void EditorUpdateAndRender(int32 mouseEvent, Position2 cursorPos) {
	ClearScreen();

	UpdateElements(&gui, cursorPos, mouseEvent);
	
	Point2 cameraPos = Scale(*minicameraPos, 0.25f);

	if (mouseEvent == LDN) mouseIsDown = true;
	if (mouseEvent == LUP) mouseIsDown = false;

	// Update grid
	if (!gui.active && selectedTileId) {
		int32 tileX = (int32)((cursorPos.x + 32.0f) / 64.0f + cameraPos.x);
		int32 tileY = (int32)(cursorPos.y / 64.0f + cameraPos.y);
		if (mouseIsDown){
			grid[tileX][tileY] = selectedTileId;
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
	for (int32 x=0; x<W_; x++) for (int32 i=0; i<4; i++) for (int32 y=0; y<H_; y++) {
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

		Sprite sprite = tiles[tileId].sprite;
		Render(
			sprite.tileset, sprite.crop,
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

	if (gui.active) {
		UIElement* element = gui.active;
		str = buffer;
		str += CopyString("element", 7, str);
		str += int32ToDecimal(gui.active->index, str);
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