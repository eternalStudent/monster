#define STATE_IDLE 3
#define STATE_RUN  2
#define STATE_JUMP 1
#define STATE_FALL 0

struct HitBox {
    union {
        Position2 pos;
        struct {pixels x, y;};
    };
    pixels radius, height;
};

struct Entity {
    union {
        HitBox hitBox;
        struct {Position2 pos; pixels radius, height;};
        struct {pixels x, y;   pixels radius, height;};
    };

    // renderable
    TextureHandle texture;

    // can move
    Velocity2 velocity;
    bool onGround;

    // changing sprite/animation
    milliseconds timeSpentInState;
    byte direction;
    byte stateRow;
    byte stateColumn;
};

static Entity player;

// GUI stuff
#define ElementCapacity 9

static GUI gui = {};
static UIElement elements[ElementCapacity+1] = {};
static int32 renderOrder[ElementCapacity] = {};

static float32* ui_accelaration;
static float32* ui_jumpForce;
static float32* ui_maxSpeed;
static float32* ui_gravity;
static Position2* ui_text_box;

void GameInit() {
	bool load = LoadStream("level.dat", grid, sizeof(grid));
	if (!load){
		grid[4][6] = 1;
		grid[6][8] = 1;
		grid[7][8] = 1;
		grid[8][8] = 1;
		grid[5][3] = 1;
		grid[0][3] = 1;
		grid[1][3] = 1;
	}

	player = {};
	player.pos = {32.0f, 64.0f*4};
	player.hitBox.height = 58.0f;
	player.hitBox.radius = 10.0f;
	player.texture = GenerateTextureFromFile("adventurer.bmp", Pixelated);

	gui.elements = elements;
	gui.renderOrder = renderOrder;

	UIElement* container = GetNewElement(&gui);
	container->p0 = {8.0f, 900.0f};
	container->p1 = Move(container->p0, 290.0f, 162.0f);
	container->parent = 0;
	container->flags = 1;
	container->background = GenerateBrush(0x22ffffff);
	renderOrder[8] = container->index;
	ui_text_box = &container->p0;

	UIElement* slider1 = GetNewElement(&gui);
	slider1->p0 = {6.0f, 30.0f+3*36.0f};
	slider1->p1 = Move(slider1->p0, 264.0f, 8.0f);
	slider1->parent = container->index;
	slider1->flags = 0;
	slider1->background = GenerateBrush(0x88c1daff);
	renderOrder[7]=slider1->index;

	UIElement* sliderPos1 = GetNewElement(&gui);
	sliderPos1->p0 = {128.0f, 0.0f};
	sliderPos1->p1 = Move(sliderPos1->p0, 8.0f, 8.0f);
	sliderPos1->parent = slider1->index;
	sliderPos1->flags = 1;
	sliderPos1->background = blackBrush;
	renderOrder[6]=sliderPos1->index;
	ui_accelaration = &(sliderPos1->x0);

	UIElement* slider2 = GetNewElement(&gui);
	slider2->p0 = {6.0f, 30.0f+2*36.0f};
	slider2->p1 = Move(slider2->p0, 264.0f, 8.0f);
	slider2->parent = container->index;
	slider2->flags = 0;
	slider2->background = GenerateBrush(0x88c1daff);
	renderOrder[5]=slider2->index;

	UIElement* sliderPos2 = GetNewElement(&gui);
	sliderPos2->p0 = {128.0f, 0.0f};
	sliderPos2->p1 = Move(sliderPos2->p0, 8.0f, 8.0f);
	sliderPos2->parent = slider2->index;
	sliderPos2->flags = 1;
	sliderPos2->background = blackBrush;
	renderOrder[4]=sliderPos2->index;
	ui_maxSpeed = &(sliderPos2->x0);

	UIElement* slider3 = GetNewElement(&gui);
	slider3->p0 = {6.0f, 30.0f+36.0f};
	slider3->p1 = Move(slider3->p0, 264.0f, 8.0f);
	slider3->parent = container->index;
	slider3->flags = 0;
	slider3->background = GenerateBrush(0x88c1daff);
	renderOrder[3]=slider3->index;

	UIElement* sliderPos3 = GetNewElement(&gui);
	sliderPos3->p0 = {128.0f, 0.0f};
	sliderPos3->p1 = Move(sliderPos3->p0, 8.0f, 8.0f);
	sliderPos3->parent = slider3->index;
	sliderPos3->flags = 1;
	sliderPos3->background = blackBrush;
	renderOrder[2]=sliderPos3->index;
	ui_jumpForce = &sliderPos3->x0;

	UIElement* slider4 = GetNewElement(&gui);
	slider4->p0 = {6.0f, 30.0f};
	slider4->p1 = Move(slider4->p0, 264.0f, 8.0f);
	slider4->parent = container->index;
	slider4->flags = 0;
	slider4->background = GenerateBrush(0x88c1daff);
	renderOrder[1]=slider4->index;

	UIElement* sliderPos4 = GetNewElement(&gui);
	sliderPos4->p0 = {128.0f, 0.0f};
	sliderPos4->p1 = Move(sliderPos4->p0, 8.0f, 8.0f);
	sliderPos4->parent = slider4->index;
	sliderPos4->flags = 1;
	sliderPos4->background = blackBrush;
	renderOrder[0]=sliderPos4->index;
	ui_gravity = &sliderPos4->x0;
}

#define JUMP  0x01
#define LEFT  0x02
#define DOWN  0x04
#define RIGHT 0x08

void UpdatePlayerByInput(int32 keysPressed, milliseconds deltaTime) {
	pixels_per_millisec_2 accelaration = LoadExp(-12) + LoadExp(-20)*(*ui_accelaration);
	pixels_per_millisec_2 jumpForce = LoadExp(-5) + LoadExp(-13)*(*ui_jumpForce);
	pixels_per_millisec maxSpeed = 0.5f + LoadExp(-9)*(*ui_maxSpeed);

	if (keysPressed & JUMP && player.stateRow != STATE_JUMP && player.stateRow != STATE_FALL) {
		player.velocity.y += deltaTime * jumpForce;
		player.stateRow = STATE_JUMP;
	}

	if (keysPressed & LEFT && player.velocity.x <= 0) {
		player.velocity.x -= deltaTime * accelaration;
		player.direction = 1;
	}
	else if (keysPressed & RIGHT && player.velocity.x >= 0) {
		player.velocity.x += deltaTime * accelaration;
		player.direction = 0;
	}
	else
		player.velocity.x = 0;

	if (player.velocity.x > maxSpeed) player.velocity.x = maxSpeed;
	if (player.velocity.x < -maxSpeed) player.velocity.x = -maxSpeed;
}

static void UpdatePlayerByGravity(milliseconds deltaTime) {
	pixels_per_millisec_2 gravity = LoadExp(-9) + LoadExp(-17)*(*ui_gravity);

	if (player.y > 0)
		player.velocity.y -= deltaTime * gravity;
}

void UpdatePlayerStatus(byte previousState, milliseconds deltaTime) {
	if (player.velocity.x == 0 && player.velocity.y == 0 && player.onGround) {
		player.stateRow = STATE_IDLE;
	}
	if (player.velocity.x != 0 && player.velocity.y == 0 && player.onGround) {
		player.stateRow = STATE_RUN;
	}
	if (player.velocity.y < 0) {
		player.stateRow = STATE_FALL;
	}

	if (player.stateRow != previousState) {
		player.timeSpentInState = 0;
		player.stateColumn = 0;
	}
	else {
		player.timeSpentInState += deltaTime;
		if (player.timeSpentInState > 200) {
			player.stateColumn++;
			player.timeSpentInState -= 200;
		}

		if (player.stateRow == STATE_FALL && player.stateColumn > 1)
			player.stateColumn = 1;

		else if (player.stateColumn == 4)
			player.stateColumn = 0;
	}
}

inline float32 SafeDivNoNeg(float32 a, float32 b) {
    if (a == 0) return 0;
    if (b == 0) return INF32();
    float32 result = a/b;
    return result < 0 ? 0 : result;
}

void Collision(milliseconds deltaTime) {
	milliseconds remainingTime = deltaTime;
	for (uint32 i = 0; i < 4; i++) {

		pixels_per_millisec speedEpsilon = 0.00001f;
		bool atEpsilonSpeed = -speedEpsilon <= player.velocity.x && player.velocity.x <= speedEpsilon;

		if (remainingTime <= 0 || atEpsilonSpeed && player.velocity.y == 0)
			break;

		pixels projectedX = atEpsilonSpeed  
			? player.x : 
			player.x + player.velocity.x * remainingTime;
		pixels projectedY = player.y + player.velocity.y * remainingTime;
		if (projectedY < 0)
			projectedY = 0;

		int32 left   = (int32)((projectedX - player.hitBox.radius + 32.0f)/64.0f);
		int32 right  = (int32)((projectedX + player.hitBox.radius + 32.0f)/64.0f);
		int32 top    = (int32)((projectedY + player.hitBox.height)/64.0f);
		int32 bottom = (int32)((projectedY)/64.0f);

		bool hitsTop = false, hitsBottom = false, hitsLeft = false, hitsRight = false;
		if (0 <= left   && left   < W_) for (int32 y = bottom; y <= top; y++) {
			uint32 flags = tiles[grid[left][y]].flags;
			if (flags & 1) hitsLeft = true;
			if (flags & 2) grid[left][y] = 0;
		}
		if (0 <= right  && right  < W_) for (int32 y = bottom; y <= top; y++) {
			uint32 flags = tiles[grid[right][y]].flags;
			if (flags & 1) hitsRight = true;
			if (flags & 2) grid[right][y] = 0;
		}
		if (0 <= top    && top    < H_) for (int32 x = left; x <= right; x++) {
			int32 flags = tiles[grid[x][top]].flags;
			if (flags & 1) hitsTop = true;
			if (flags & 2) grid[x][top] = 0;
		}
		if (0 <= bottom && bottom < H_) for (int32 x = left; x <= right; x++) {
			uint32 flags = tiles[grid[x][bottom]].flags;
			uint32 oneTileUpFlags = tiles[grid[x][bottom+1]].flags;
			// NOTE: you can not stand on a block if one tile above it is blocked
			if ((flags & 1) && !(oneTileUpFlags & 1)) {
				hitsBottom = true;
			}
			if (flags & 2) grid[x][bottom] = 0;
		}

		pixels pointLeft   = (left   + 0.5f)*64.0f + player.hitBox.radius + 1;
		pixels pointRight  = (right  - 0.5f)*64.0f - player.hitBox.radius - 1;
		pixels pointTop    = (top    - 0.0f)*64.0f - player.hitBox.height - 1;
		pixels pointBottom = (bottom + 1.0f)*64.0f;

		milliseconds timeToTop = SafeDivNoNeg(pointTop - player.y, player.velocity.y);
		milliseconds timeToBottom = SafeDivNoNeg(pointBottom - player.y, player.velocity.y);
		milliseconds timeToLeft = SafeDivNoNeg(pointLeft - player.x, player.velocity.x);
		milliseconds timeToRight = SafeDivNoNeg(pointRight - player.x, player.velocity.x);

		if (!hitsTop && !hitsBottom && !hitsLeft && !hitsRight) {
			player.x = projectedX;
			player.y = projectedY;
			break;
		}
		else {
			if (atEpsilonSpeed && player.velocity.y <= 0 && hitsBottom) {
				if (projectedY < pointBottom)
					player.y = pointBottom;
				player.velocity.y = 0;
				player.onGround = true;
				break;
			}
			else if (!atEpsilonSpeed && player.velocity.y <= 0) {
				if (hitsBottom && player.velocity.y != 0) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToBottom <= timeToRight) || hitsLeft && timeToBottom <= timeToLeft) {
						if (projectedY < pointBottom)
							player.y = pointBottom;
						player.velocity.y = 0;
						player.onGround = true;
						remainingTime -= timeToBottom;
						continue;
					}
				}
				player.y = projectedY;
				remainingTime -= timeToBottom;
				continue;
			}
			else if (atEpsilonSpeed && player.velocity.y > 0) {
				if (projectedY > pointTop)
					player.y = pointTop;
				player.velocity.y = 0;
				break;
			}
			else if (!atEpsilonSpeed && player.velocity.y > 0) {
				if (hitsLeft) {
					if (!hitsTop || (hitsTop && timeToLeft <= timeToTop)) {
						if (projectedX < pointLeft && pointLeft < player.x)
							player.x = pointLeft;
						player.velocity.x = -speedEpsilon;
						remainingTime -= timeToLeft;
						continue;
					}
				}
				if (hitsRight) {
					if (!hitsTop || (hitsTop && timeToRight <= timeToTop)) {
						if (player.x < pointRight && pointRight < projectedX)
							player.x = pointRight;
						player.velocity.x = speedEpsilon;
						remainingTime -= timeToRight;
						continue;
					}
				}
				if (hitsTop) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToTop < timeToRight) || hitsLeft && timeToTop < timeToLeft) {
						if (projectedY > pointTop)
							player.y = pointTop;
						player.velocity.y = 0;
						remainingTime -= timeToTop;
						continue;
					}
				}
				Assert(0);
			}
			else {
				Assert(0);
			}
		}
	}
}

void GameUpdateAndRender(uint32 keysPressed, milliseconds deltaTime, MouseEventQueue* mouseEventQueue, Position2 cursorPos) {
	if (deltaTime > 17.0f) deltaTime = 100.0f / 6.0f;

	ClearScreen();
	UpdateElements(&gui, cursorPos, mouseEventQueue);

	byte previousState = player.stateRow;
	player.onGround = false;
	UpdatePlayerByInput(keysPressed, deltaTime);
	UpdatePlayerByGravity(deltaTime);
	Collision(deltaTime);

	if (player.y <= 0) {
		player.pos = {32.0f, 64.0f*4};
		player.velocity = {};
		player.stateRow = STATE_IDLE;
	}
	UpdatePlayerStatus(previousState, deltaTime);

	pixels half_width = windowDim.width / 2.0f;
	pixels half_height = windowDim.height / 2.0f;
	pixels camerax = player.x > half_width ? player.x - half_width : 0;
	pixels cameray = player.y > half_height ? player.y - half_height : 0;
	// TODO: this is iterating over more than necessary tiles
	for (int x = 0; x < W_; x++) for (int y = 0; y < H_; y++) {
		int32 tileId = grid[x][y];
		if (tileId == 0)
			continue;

		Tile tile = tiles[tileId];
		Render(
			tile.tileset, tile.crop,
			{x*64.0f-camerax, y*64.0f-cameray},
			64.0f, 64.0f,
			0);
	}

	Box2 crop;
	crop.x0 = player.stateColumn*0.25f;
	crop.y0 = player.stateRow*0.25f;
	crop.x1 = (player.stateColumn + 1)*0.25f;
	crop.y1 = (player.stateRow + 1)*0.25f;
	Render(
		player.texture, crop,
		{player.x-camerax, player.y-cameray},
		100.0f, 74.0f,
		player.direction);

	RenderElements(gui);


	// print debug info on screen
	char buffer[256];
	char* str = buffer; 
	str += CopyString("player.x: ", 10, str); float32ToDecimal(player.x, 2, str);
	DebugPrintText(16, 32.0*27, buffer);
	str = buffer;
	str += CopyString("player.y: ", 10, str); float32ToDecimal(player.y, 2, str);
	DebugPrintText(16, 32.0*26, buffer);
	str = buffer;
	str += CopyString("velocity.x: ", 12, str); float32ToDecimal(player.velocity.x, 2, str);
	DebugPrintText(16, 32.0*25, buffer);
	str = buffer;
	str += CopyString("velocity.y: ", 12, str); float32ToDecimal(player.velocity.y, 2, str);
	DebugPrintText(16, 32.0*24, buffer);
	str = buffer;
	str += CopyString("on ground: ", 11, str); int32ToDecimal((int32)player.onGround, str);
	DebugPrintText(16, 32.0*23, buffer);
	str = buffer;
	str += CopyString("delta-time: ", 12, str); str += float32ToDecimal(deltaTime, 2, str); memcpy(str, "ms", 3);
	DebugPrintText(16, 32.0*22, buffer);


	str = buffer;
	float32 accelaration = LoadExp(-12) + LoadExp(-20)*(*ui_accelaration);
	str += CopyString("accelaration: ", 14, str); float32ToDecimal(accelaration, 8, str);
	DebugPrintText(ui_text_box->x+12.0f, ui_text_box->y+12+3*36.0f, buffer);

	str = buffer;
	float32 max_speed = 0.5f + LoadExp(-9)*(*ui_maxSpeed);
	str += CopyString("max speed: ", 11, str); float32ToDecimal(max_speed, 8, str);
	DebugPrintText(ui_text_box->x+12.0f, ui_text_box->y+12+2*36.0f, buffer);

	str = buffer;
	float32 jumpForce = LoadExp(-5) + LoadExp(-13)*(*ui_jumpForce);
	str += CopyString("jump force: ", 12, str); float32ToDecimal(jumpForce, 8, str);
	DebugPrintText(ui_text_box->x+12.0f, ui_text_box->y+12+36.0f, buffer);

	str = buffer;
	float32 gravity = LoadExp(-9) + LoadExp(-17)*(*ui_gravity);
	str += CopyString("gravity: ", 9, str); float32ToDecimal(gravity, 8, str);
	DebugPrintText(ui_text_box->x+12.0f, ui_text_box->y+12.0f, buffer);
}