struct Box {
    union{
        HitBox hitBox;
        struct {
            union {
                Position2f pos;
                struct { pixels x, y; };
            };
            pixels radius, height;            
        };
    };

    int32 tileId;
    TextureHandle solid;

    int32 parent;

    uint32 flags; // 1-movable, 2-resizable, 4-clickable
    void (*onClick)(int32);
};

#define TileCount 	22
#define BoxCount 	TileCount+5  // 27
#define I 			TileCount+1  // 23
#define MM 			I+3			 // 26

static Box boxes[BoxCount+1] = {};
static int32 renderOrder[BoxCount] = {};
static int32 boxIndex = 0;
static int32 selectedIndex = 0;

static HitBox originalPos;
static Position2f grabPos;
// TODO: these should be flags
static bool isGrabbing = false;
static bool isResizing = false;
static bool isPressed = false;

static TextureHandle black;
static TextureHandle placeholder;
static TextureHandle neutral;
static TextureHandle grey1;
static TextureHandle grey2;

static Position2f cameraPos;

void Win32SetCursorToMove();
void Win32SetCursorToArrow();
void Win32SetCursorToResize();
void Win32SetCursorToHand();

void Select(int32 boxId){
	selectedIndex = boxId;
}

void Save(int32 boxId){
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
		Box* box = &boxes[i];
		box->pos.x = (i % 2) ? 34.0f : -34.0f;
		box->pos.y = (((i-1)/2) * 68.0f) + 24.0f;
		box->radius = 32.0f;
		box->height = 64.0f;
		box->parent = I;
		box->tileId = i > 16 ? 0 : i;
		box->solid = placeholder;
		box->flags = 4;
		box->onClick = &Select;
		renderOrder[i-1]=i;
	}

	boxes[I].pos.x = 150.0f;
	boxes[I].pos.y = 50.0f;
	boxes[I].radius = 100.0f;
	boxes[I].height = 800.0f;
	boxes[I].parent = 0;
	boxes[I].flags = 1;
	boxes[I].solid = neutral;
	renderOrder[I-1]=I;

	boxes[I+1].pos.x = 150.0f;  // 24
	boxes[I+1].pos.y = 950.0f;
	boxes[I+1].radius = 70.0f;
	boxes[I+1].height = 80.0f;
	boxes[I+1].parent = 0;
	boxes[I+1].flags = 1;
	boxes[I+1].solid = neutral;
	renderOrder[I+1]=I+1;

	boxes[I+2].pos.x = 0.0f;  // 25
	boxes[I+2].pos.y = 10.0f;
	boxes[I+2].radius = 60.0f;
	boxes[I+2].height = 60.0f;
	boxes[I+2].parent = I+1;
	boxes[I+2].flags = 4;
	boxes[I+2].solid = placeholder;
	boxes[I+2].onClick = &Save;
	renderOrder[I]=I+2;

	boxes[I+3].pos.x = 1700.0f;  // 26
	boxes[I+3].pos.y = 850.0f;
	boxes[I+3].radius = W_ * 2.0f;
	boxes[I+3].height = H_ * 4.0f;
	boxes[I+3].parent = 0;
	boxes[I+3].flags = 1;
	boxes[I+3].solid = grey1;
	renderOrder[I+3]=I+3;

	// NOTE: 27 minimap
	boxes[I+4].pos.x = -(W_-X_) * 2.0f;
	boxes[I+4].pos.y = 0.0f;
	boxes[I+4].radius = X_ * 2.0f;
	boxes[I+4].height = Y_ * 4.0f;
	boxes[I+4].parent = I+3;
	boxes[I+4].flags = 1;
	boxes[I+4].solid = grey2;
	renderOrder[I+2]=I+4;
}

inline bool IsInsideHitBox(HitBox hitBox, Position2f pos){
	return (hitBox.x-hitBox.radius <= pos.x && pos.x <= hitBox.x+hitBox.radius) &&
		(hitBox.y <= pos.y && pos.y <= hitBox.y+hitBox.height);
}

inline HitBox GetAbsolutePosition(Box box){
	if (box.parent == 0)
		return box.hitBox;

	Position2f parentPos = GetAbsolutePosition(boxes[box.parent]).pos;
	HitBox hitBox = box.hitBox;
	hitBox.pos = Position2f{parentPos.x + box.x, parentPos.y + box.y};
	return hitBox;
}

inline Position2f GetRelativePosition(Position2f pos, Box box){
	Position2f base = GetAbsolutePosition(boxes[box.parent]).pos;
	return Position2f{pos.x - base.x, pos.y - base.y};
}

inline bool IsInsideBox(Box box, Position2f pos){
	return IsInsideHitBox(GetAbsolutePosition(box), pos);
}

inline bool IsInBottomRight(HitBox hitBox, Position2f pos){
	return hitBox.x+hitBox.radius-4 <= pos.x && pos.y <= hitBox.y+4;
}

inline bool IsInBottomRight(Box box, Position2f pos){
	HitBox hitBox = GetAbsolutePosition(box);
	return IsInBottomRight(hitBox, pos);
}

inline int32 GetBoxIndexByPos(Position2f cursorPos){
	for(int32 i = 0; i < BoxCount; i++){
		if (IsInsideBox(boxes[renderOrder[i]], cursorPos)) {
			return renderOrder[i];
		}	
	}
	return 0;
}

void SetPosition(Box* box, Position2f newPos){
	if (!box->parent){ box->pos = newPos; return; } 
	Box parent = boxes[box->parent];

	if (newPos.x < -parent.radius+box->radius) box->x = -parent.radius+box->radius;
	else if (-box->radius+parent.radius < newPos.x) box->x = -box->radius+parent.radius;
	else box->x = newPos.x;

	if (newPos.y < 0.0f) box->y = 0;
	else if (parent.height-box->height < newPos.y) box->y = parent.height-box->height;
	else box->y = newPos.y;
}

void MoveToFront(int32 boxId){
	int32 orderIndex = 0;
	for(int32 i=0; i<BoxCount; i++) 
		if (renderOrder[i] == boxId){orderIndex=i; break;}

	if (orderIndex == 0) return;
	memmove(&(renderOrder[1]), renderOrder, sizeof(int32)*orderIndex);
	renderOrder[0] = boxId;

	for (int32 i=1; i<=BoxCount; i++)
		if (boxes[i].parent == boxId) MoveToFront(i);
}

void EditorUpdateAndRender(MouseEventQueue* mouseEventQueue, Position2f cursorPos) {
	ClearScreen();

	Rectanglef unit;
	unit.x0 = 0.0f;
	unit.y0 = 0.0f;
	unit.x1 = 1.0f;
	unit.y1 = 1.0f;

	cameraPos.x = (boxes[I+4].pos.x + ((W_-X_) * 2.0f)) * 0.25f;// * 64.0f;
	cameraPos.y = boxes[I+4].pos.y * 0.25f;// * 64.0f;
	
	bool isBottomRight = false;
	if (!isResizing && !isGrabbing)
		boxIndex = GetBoxIndexByPos(cursorPos);
	Box* box = &(boxes[boxIndex]);
	if (boxIndex == 0) {
		Win32SetCursorToArrow();
	}
	else{
		if(IsInBottomRight(*box, cursorPos) && (box->flags & 2)){
			Win32SetCursorToResize();
			isBottomRight = true;
		}
		else{
			if (box->flags & 4) Win32SetCursorToHand();
			else if (box->flags & 1) Win32SetCursorToMove();		
		}
	}

	int32 tileX = (int32)((cursorPos.x + 32.0f) / 64.0f + cameraPos.x);
	int32 tileY = (int32)(cursorPos.y / 64.0f + cameraPos.y);

	while(mouseEventQueue->size){
		MouseEvent mouseEvent = Dequeue(mouseEventQueue);
		Assert(mouseEvent.type < 5)
		if (mouseEvent.type == LDN && boxIndex != 0){
			MoveToFront(boxIndex);
			if (box->flags & 4){
				isPressed = true;
				if (box->onClick)
					box->onClick(boxIndex);
			}
			else{
				if (isBottomRight && (box->flags & 2)) isResizing = true;
				else isGrabbing = true;
				grabPos = cursorPos;
				originalPos = box->hitBox;
			}
		}
		else if (mouseEvent.type == LDN && boxIndex == 0 && selectedIndex != 0){
			grid[tileX][tileY] = boxes[selectedIndex].tileId;
		}
		else if (mouseEvent.type == RDN && boxIndex == 0 && selectedIndex != 0){
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
		Position2f newPos;
		newPos.x = originalPos.x + cursorPos.x - grabPos.x;
		newPos.y = originalPos.y + cursorPos.y - grabPos.y;
		SetPosition(box, newPos);
	}

	if (isResizing){
		Position2f relativeCursorPos = GetRelativePosition(cursorPos, *box);
		box->y = relativeCursorPos.y;
		pixels left = originalPos.x-originalPos.radius;
		box->x = (relativeCursorPos.x + left)*0.5f;
		pixels top = originalPos.y + originalPos.height;
		box->height = top - relativeCursorPos.y;
		box->radius = fabsf((relativeCursorPos.x - left)*0.5f);

		if (box->height < 0){
			box->y = originalPos.y+originalPos.height;
			box->height = -box->height;
		}
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
		((__m128i*)minimap.data)[y*4*W_ + i*W_ + x] = grid[x][H_-y-1] 
			? _mm_set1_epi32(0xff000000) 
			: _mm_set1_epi32(0x88999999);
	}
	boxes[MM].tileId = 0;
	boxes[MM].solid = GenerateTextureFromImage(minimap, Pixelated);

	for(int32 i=BoxCount-1; i>=0; i--){
		int32 itBoxId = renderOrder[i];
		Box itBox = boxes[itBoxId];
		Position2f pos = GetAbsolutePosition(itBox).pos;
		int32 tileId = itBox.tileId;
		Tile tile;
		if (itBox.tileId){
			tile = sprites[tileId];
		}
		else{
			tile.crop = unit;
			tile.solid = itBox.solid; 
		}
		if (itBoxId == boxIndex && isPressed){
			Render(
				black, unit,
				pos,
				2*itBox.radius, itBox.height,
				0);
			Render(
				tile.texture, tile.crop,
				{pos.x+2, pos.y-2},
				2*itBox.radius, itBox.height,
				0);
		}
		else Render(
			tile.texture, tile.crop,
			pos,
			2*itBox.radius, itBox.height,
			0);
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

	if (boxIndex != 0){
		str = buffer;
		str += CopyString("box", 3, str);
		str += int32ToDecimal(boxIndex, str);
		str += CopyString(": {x=", 5, str);
		str += float32ToDecimal(box->x, 1, str);
		str += CopyString(" y=", 3, str);
		str += float32ToDecimal(box->y, 1, str);
		str += CopyString(" radius=", 8, str);
		str += float32ToDecimal(box->radius, 0, str);
		str += CopyString(" height=", 8, str);
		str += float32ToDecimal(box->height, 0, str);
		str += CopyString(" flags=", 7, str);
		str += uint32ToDecimal(box->flags, str);
		memcpy(str, "}", 2);
		DebugPrintText(16, 32.0 * 25, buffer);
	}
}