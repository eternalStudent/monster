#define STATE_IDLE 0
#define STATE_RUN  1
#define STATE_JUMP 2
#define STATE_FALL 3

struct HitBox {
    union {
        Position2 pos;
        struct { pixels x, y; };
    };
    pixels radius, height;
};

struct Entity {
    union{
        HitBox hitBox;
        struct {
            union {
                Position2 pos;
                struct { pixels x, y; };
            };
            pixels radius, height;          
        };
    };

    // renderable
    TextureHandle texture;

    // can move
    Velocity2 velocity;

    // changing sprite/animation
    milliseconds timeSpentInState;
    byte direction;
    byte stateRow;
    byte stateColumn;
};

static Entity player;

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
	player.pos = { 32.0f, 64.0f*4 };
	player.hitBox.height = 58.0f;
	player.hitBox.radius = 10.0f;
	player.texture = GenerateTextureFromFile("adventurer.bmp", Pixelated);
}

#define JUMP  0x01
#define LEFT  0x02
#define DOWN  0x04
#define RIGHT 0x08

void UpdatePlayerByInput(int32 keysPressed, milliseconds deltaTime) {
	pixels_per_millisec_2 accelaration = 0.0005f;
	pixels_per_millisec_2 jumpForce = 0.06f;
	pixels_per_millisec maxSpeed = 0.8f;

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
	pixels_per_millisec_2 gravity = 0.002f;

	if (player.y > 0)
		player.velocity.y -= deltaTime * gravity;
}

void UpdatePlayerStatus(byte previousState, milliseconds deltaTime) {
	if (player.velocity.x == 0 && player.velocity.y == 0) {
		player.stateRow = STATE_IDLE;
	}
	if (player.velocity.x != 0 && player.velocity.y == 0) {
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

void Collision(milliseconds deltaTime) {
	milliseconds remainingTime = deltaTime;
	for (uint32 i = 0; i < 4; i++) {

		pixels_per_millisec speedEpsilon = 0.00001f;
		bool atEpsilonSpeed = -speedEpsilon <= player.velocity.x && player.velocity.x <= speedEpsilon;

		if (remainingTime <= 0 || atEpsilonSpeed && player.velocity.y == 0)
			break;

		pixels projectedX = player.x + player.velocity.x * remainingTime;
		pixels projectedY = player.y + player.velocity.y * remainingTime;
		if (projectedY < 0)
			projectedY = 0;

		int32 left = (int32)((projectedX - player.hitBox.radius + 32.0f) / 64.0f);
		int32 right = (int32)((projectedX + player.hitBox.radius + 32.0f) / 64.0f);
		int32 top = (int32)((projectedY + player.hitBox.height) / 64.0f);
		int32 bottom = (int32)((projectedY) / 64.0f);

		bool hitsTop = false, hitsBottom = false, hitsLeft = false, hitsRight = false;
		if (0 <= left && left < 31) for (int32 y = bottom; y <= top; y++) if (grid[left][y] != 0) { hitsLeft = true; break; }
		if (0 <= right && right < 31) for (int32 y = bottom; y <= top; y++) if (grid[right][y] != 0) { hitsRight = true; break; }
		if (0 <= top && top < 17) for (int32 x = left; x <= right; x++) if (grid[x][top] != 0) { hitsTop = true; break; }
		if (0 <= bottom && bottom < 17) for (int32 x = left; x <= right; x++) if (grid[x][bottom] != 0) { hitsBottom = true; break; }

		pixels pointLeft = (left + 0.5f) * 64.0f + player.hitBox.radius;
		pixels pointRight = (right - 0.5f) * 64.0f - player.hitBox.radius;
		pixels pointTop = (top - 0) * 64.0f - player.hitBox.height;
		pixels pointBottom = (bottom + 1) * 64.0f;

		milliseconds timeToTop = player.velocity.y != 0 && player.y <= pointTop ?
			(pointTop - player.y) / player.velocity.y : (float32)HUGE_VAL;
		milliseconds timeToBottom = player.velocity.y != 0 && player.y >= pointBottom ?
			(pointBottom - player.y) / player.velocity.y : (float32)HUGE_VAL;
		milliseconds timeToLeft = player.velocity.x != 0 && player.x >= pointLeft ?
			(pointLeft - player.x) / player.velocity.x : (float32)HUGE_VAL;
		milliseconds timeToRight = player.velocity.x != 0 && player.x <= pointRight ?
			(pointRight - player.x) / player.velocity.x : (float32)HUGE_VAL;

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
				break;
			}
			else if (!atEpsilonSpeed && player.velocity.y <= 0) {
				if (hitsLeft && player.velocity.x < 0) {
					if (!hitsBottom || (hitsBottom && timeToLeft < timeToBottom)) {
						if (projectedX < pointLeft)
							player.x = pointLeft;
						player.velocity.x = -speedEpsilon;
						remainingTime -= timeToLeft;
					}
				}
				if (hitsRight && player.velocity.x > 0) {
					if (!hitsBottom || (hitsBottom && timeToRight < timeToBottom)) {
						if (projectedX > pointRight)
							player.x = pointRight;
						player.velocity.x = speedEpsilon;
						remainingTime -= timeToRight;
					}
				}
				if (hitsBottom && player.velocity.y != 0) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToBottom <= timeToRight) || hitsLeft && timeToBottom <= timeToLeft) {
						if (projectedY < pointBottom)
							player.y = pointBottom;
						player.velocity.y = 0;
						remainingTime -= timeToBottom;
					}
				}
			}
			else if (atEpsilonSpeed && player.velocity.y > 0) {
				if (projectedY > pointTop)
					player.y = pointTop;
				player.velocity.y = -0.001f;
				break;
			}
			else if (!atEpsilonSpeed && player.velocity.y > 0) {
				if (hitsLeft) {
					if (!hitsBottom || (hitsTop && timeToLeft <= timeToTop)) {
						if (projectedX < pointLeft)
							player.x = pointLeft;
						player.velocity.x = -speedEpsilon;
						remainingTime -= timeToLeft;
					}
				}
				if (hitsRight) {
					if (!hitsTop || (hitsTop && timeToRight <= timeToTop)) {
						if (projectedX > pointRight)
							player.x = pointRight;
						player.velocity.x = speedEpsilon;
						remainingTime -= timeToRight;
					}
				}
				if (hitsTop) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToTop < timeToRight) || hitsLeft && timeToTop < timeToLeft) {
						if (projectedY > pointTop)
							player.y = pointTop;
						player.velocity.y = -0.001f;
						remainingTime -= timeToBottom;
					}
				}
			}
			else {
				// weird edge cases
				player.x = projectedX;
				player.y = projectedY;
				break;
			}
		}
	}
}

void GameUpdateAndRender(uint32 keysPressed, milliseconds deltaTime) {
	if (deltaTime > 17.0f) deltaTime = 100.0f / 6.0f;

	ClearScreen();

	byte previousState = player.stateRow;
	UpdatePlayerByInput(keysPressed, deltaTime);
	UpdatePlayerByGravity(deltaTime);
	Collision(deltaTime);

	if (player.y <= 0) {
		player.pos = { 32.0f, 64.0f*4 };
		player.velocity = {};
		player.stateRow = STATE_IDLE;
	}
	UpdatePlayerStatus(previousState, deltaTime);

	for (int x = 0; x < 31; x++) for (int y = 0; y < 17; y++) {
		int32 tileId = grid[x][y];
		if (tileId == 0)
			continue;

		Tile tile = sprites[tileId];
		Render(
			tile.tileset, tile.crop,
			Position2{ x * 64.0f, y * 64.0f },
			64.0f, 64.0f,
			0);
	}

	Box2 crop;
	crop.x0 = player.stateColumn * 0.25f;
	crop.y0 = player.stateRow * 0.25f;
	crop.x1 = (player.stateColumn+1) * 0.25f;
	crop.y1 = (player.stateRow+1) * 0.25f;
	Render(
		player.texture, crop,
		player.pos,
		100.0f, 74.0f,
		player.direction);


	// print debug info on screen
	char buffer[256];
	char* str = buffer; 
	str += CopyString("player.x: ", 10, str); float32ToDecimal(player.x, 2, str);
	DebugPrintText(16, 32.0 * 27, buffer);
	str = buffer;
	str += CopyString("player.y: ", 10, str); float32ToDecimal(player.y, 2, str);
	DebugPrintText(16, 32.0 * 26, buffer);
	str = buffer;
	str += CopyString("velocity.x: ", 12, str); float32ToDecimal(player.velocity.x, 2, str);
	DebugPrintText(16, 32.0 * 25, buffer);
	str = buffer;
	str += CopyString("velocity.y: ", 12, str); float32ToDecimal(player.velocity.y, 2, str);
	DebugPrintText(16, 32.0 * 24, buffer);
	str = buffer;
	str += CopyString("delta-time: ", 12, str); str += float32ToDecimal(deltaTime, 2, str); memcpy(str, "ms", 3);
	DebugPrintText(16, 32.0 * 23, buffer);

	str = buffer;
	str += CopyString("state: (", 8, str); str += uint32ToDecimal(player.stateRow, str);
	str += CopyString(", ", 2, str); str += uint32ToDecimal(player.stateColumn, str); memcpy(str, ")", 2);
	DebugPrintText(16, 32.0 * 22, buffer);
}