#define TileCount 	 22
#define ElementCount TileCount+7  // 29
#define I 			 TileCount+1  // 23
#define MM 			 I+3		  // 26

static UIElement elements[ElementCount+1] = {};
static int32 renderOrder[ElementCount] = {};
static int32 elementIndex = 0;
static int32 selectedIndex = 0;

static Box2 originalPos;
static Position2 grabPos;
// TODO: these should be flags
static bool isGrabbing = false;
static bool isResizing = false;
static bool isPressed = false;

static TextureHandle black;
static TextureHandle placeholder;
static TextureHandle neutral;
static TextureHandle grey1;
static TextureHandle grey2;

void Win32SetCursorToMove();
void Win32SetCursorToArrow();
void Win32SetCursorToResize();
void Win32SetCursorToHand();

void Select(int32 boxId) {
	selectedIndex = boxId;
}

void Save(int32 boxId) {
	SaveStream("level.dat", grid, sizeof(grid));
}

void EditorInit() {
	LoadStream("level.dat", grid, sizeof(grid));

	neutral = GenerateTextureFromRGBA(0x88c1daff);
	black = GenerateTextureFromRGBA(0xff000000);
	placeholder = GenerateTextureFromRGBA(0xff998888);
	grey1 = GenerateTextureFromRGBA(0x88999999);
	grey2 = GenerateTextureFromRGBA(0x88bbbbbb);

	for(int32 i=1; i<=TileCount; i++){
		UIElement* element = &elements[i];
		element->x0 = (i % 2) ? 102.0f : 34.0f;
		element->y0 = (((i-1)/2) * 68.0f) + 24.0f;
		element->x1 = (i % 2) ? 166.0f : 98.0f;
		element->y1 = element->y0 + 64.0f;
		element->parent = I;
		element->tileId = i > 16 ? 0 : i;
		element->solid = placeholder;
		element->flags = 4;
		element->onClick = &Select;
		renderOrder[i-1]=i;
	}

	elements[I].x0 = 50.0f;
	elements[I].y0 = 50.0f;
	elements[I].x1 = 250.0f;
	elements[I].y1 = 850.0f;
	elements[I].parent = 0;
	elements[I].flags = 1;
	elements[I].solid = neutral;
	renderOrder[I-1]=I;

	elements[I+1].x0 = 80.0f;
	elements[I+1].y0 = 950.0f;
	elements[I+1].x1 = 220.0f;
	elements[I+1].y1 = 1030.0f;
	elements[I+1].parent = 0;
	elements[I+1].flags = 1;
	elements[I+1].solid = neutral;
	renderOrder[I+1]=I+1;

	elements[I+2].x0 = 10.0f;
	elements[I+2].y0 = 10.0f;
	elements[I+2].x1 = 130.0f;
	elements[I+2].y1 = 70.0f;
	elements[I+2].parent = I+1;
	elements[I+2].flags = 4;
	elements[I+2].solid = placeholder;
	elements[I+2].onClick = &Save;
	renderOrder[I]=I+2;

	elements[I+3].x0 = 1700.0f - (W_ * 2.0f);
	elements[I+3].y0 = 850.0f;
	elements[I+3].x1 = 1700.0f + (W_ * 2.0f);
	elements[I+3].y1 = 850.0f + (H_ * 4.0f);
	elements[I+3].parent = 0;
	elements[I+3].flags = 1;
	elements[I+3].solid = grey1;
	renderOrder[I+3]=I+3;

	elements[I+4].x0 = 0.0f;
	elements[I+4].y0 = 0.0f;
	elements[I+4].x1 = X_ * 4.0f;
	elements[I+4].y1 = Y_ * 4.0f;
	elements[I+4].parent = I+3;
	elements[I+4].flags = 1;
	elements[I+4].solid = grey2;
	renderOrder[I+2]=I+4;

	elements[I+5].box = BOX2(80.0f, 916.0f, 344.0f, 924.0f);
	elements[I+5].parent = 0;
	elements[I+5].flags = 1;
	elements[I+5].solid = neutral;
	renderOrder[I+5]=I+5;

	elements[I+6].box = BOX2(128.0f, 0.0f, 136.0f, 8.0f);
	elements[I+6].parent = I+5;
	elements[I+6].flags = 1;
	elements[I+6].solid = black;
	renderOrder[I+4]=I+6;
}

inline int32 GetElementIndexByPos(Position2 pos) {
	for(int32 i = 0; i < ElementCount; i++) {
		if (IsInsideBox(elements[renderOrder[i]], pos, elements)) {
			return renderOrder[i];
		}	
	}
	return 0;
}

void MoveToFront(int32 boxId) {
	int32 orderIndex = 0;
	for(int32 i=0; i<ElementCount; i++) 
		if (renderOrder[i] == boxId){orderIndex=i; break;}

	if (orderIndex == 0) return;
	memmove(&(renderOrder[1]), renderOrder, sizeof(int32)*orderIndex);
	renderOrder[0] = boxId;

	for (int32 i=1; i<=ElementCount; i++)
		if (elements[i].parent == boxId) MoveToFront(i);
}

void EditorUpdateAndRender(MouseEventQueue* mouseEventQueue, Position2 cursorPos) {
	ClearScreen();

	Point2 cameraPos = Scale(elements[I+4].p0, 0.25f);
	
	bool isBottomRight = false;
	if (!isResizing && !isGrabbing)
	elementIndex = GetElementIndexByPos(cursorPos);
	UIElement* element = &(elements[elementIndex]);
	if (elementIndex == 0) {
		Win32SetCursorToArrow();
	}
	else{
		if(IsInBottomRight(*element, cursorPos, elements) && (element->flags & 2)){
			Win32SetCursorToResize();
			isBottomRight = true;
		}
		else{
			if (element->flags & 4) Win32SetCursorToHand();
			else if (element->flags & 1) Win32SetCursorToMove();		
		}
	}

	int32 tileX = (int32)((cursorPos.x + 32.0f) / 64.0f + cameraPos.x);
	int32 tileY = (int32)(cursorPos.y / 64.0f + cameraPos.y);

	while(mouseEventQueue->size){
		MouseEvent mouseEvent = Dequeue(mouseEventQueue);
		Assert(mouseEvent.type < 5)
		if (mouseEvent.type == LDN && elementIndex != 0){
			MoveToFront(elementIndex);
			if (element->flags & 4){
				isPressed = true;
				if (element->onClick)
					element->onClick(elementIndex);
			}
			else{
				if (isBottomRight && (element->flags & 2)) isResizing = true;
				else isGrabbing = true;
				grabPos = cursorPos;
				originalPos = element->box;
			}
		}
		else if (mouseEvent.type == LDN && elementIndex == 0 && selectedIndex != 0){
			grid[tileX][tileY] = elements[selectedIndex].tileId;
		}
		else if (mouseEvent.type == RDN && elementIndex == 0 && selectedIndex != 0){
			grid[tileX][tileY] = 0;
		}
		else if (mouseEvent.type == LUP){
			isGrabbing = false;
			isResizing = false;
			isPressed = false;
		}
	}

	if (isGrabbing && 
			(cursorPos.x != grabPos.x || cursorPos.y != grabPos.y)){
		pixels newx0  = originalPos.x0 + cursorPos.x - grabPos.x;
		pixels newy0 = originalPos.y0 + cursorPos.y - grabPos.y;
		SetPosition(element, newx0, newy0, elements);
	}

	if (isResizing) {
		// TODO: should I re-introduce resizing, just for completion sake?

		/*Position2 relativeCursorPos = GetRelativePosition(cursorPos, *element);
		element->y = relativeCursorPos.y;
		pixels left = originalPos.x-originalPos.radius;
		element->x = (relativeCursorPos.x + left)*0.5f;
		pixels top = originalPos.y + originalPos.height;
		element->height = top - relativeCursorPos.y;
		element->radius = fabsf((relativeCursorPos.x - left)*0.5f);

		if (element->height < 0){
			element->y = originalPos.y+originalPos.height;
			element->height = -element->height;
		}*/
	}

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
	elements[MM].solid = GenerateTextureFromImage(minimap, Pixelated);

	for(int32 i=ElementCount-1; i>=0; i--){
		int32 itBoxId = renderOrder[i];
		UIElement itBox = elements[itBoxId];
		Box2 pos = GetAbsolutePosition(itBox, elements);
		int32 tileId = itBox.tileId;
		Tile tile;
		if (itBox.tileId){
			tile = sprites[tileId];
		}
		else{
			tile.crop = BOX2_UNIT();
			tile.solid = itBox.solid; 
		}
		if (itBoxId == elementIndex && isPressed){
			RenderBox2(
				black, BOX2_UNIT(),
				pos);
			RenderBox2(
				tile.texture, tile.crop,
				Box_MoveBy(pos, {2, -2}));
		}
		else RenderBox2(
			tile.texture, tile.crop,
			pos);
	}

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

	// print debug-info on screen
	str = buffer;
	str += CopyString("tile: (", 7, str); str += int32ToDecimal(tileX, str); 
	str += CopyString(", ", 2, str); str += int32ToDecimal(tileY, str); memcpy(str, ")", 2);
	DebugPrintText(16, 32.0 * 26, buffer);

	if (elementIndex != 0){
		str = buffer;
		str += CopyString("element", 7, str);
		str += int32ToDecimal(elementIndex, str);
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