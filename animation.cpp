#define STATE_IDLE 0.75f;
#define STATE_RUN  0.5f
#define STATE_JUMP 0.25f
#define STATE_FALL 0.0f

struct RenderObject {
	TextureId spritesheet;
	Box2 crop;
	pixels height; 
	pixels radius;

	milliseconds timeSpentInState;
};

void AnimationInitPlayer(Arena* arena, RenderObject* player) {
	player->radius = 50.0f;
	player->height = 74.0f;
	player->spritesheet = LoadTexture(arena, "data/adventurer.bmp", GRAPHICS_PIXELATED);
	player->crop = {0.0f, 0.75, 0.25f, 1.0f};
}

void Switch(float32* x0, float32* x1) {
	float32 temp = *x0;
	*x0 = *x1;
	*x1 = temp;
}

void AnimationUpdate(RenderObject* player, PhysicalBody body, milliseconds deltaTime) {
	bool left = player->crop.x0 > player->crop.x1;
	player->crop.x0 = MIN(player->crop.x0, player->crop.x1);

	float32 previousState = player->crop.y0;
	if (body.velocity.x == 0.0f && body.velocity.y == 0.0f && body.onGround) {
		player->crop.y0 = STATE_IDLE;
	}
	if (body.velocity.x != 0.0f && body.velocity.y == 0.0f && body.onGround) {
		player->crop.y0 = STATE_RUN;
	}
	if (body.velocity.y < 0) {
		player->crop.y0 = STATE_FALL;
	}
	if (body.velocity.y > 0) {
		player->crop.y0 = STATE_JUMP;
	}

	if (player->crop.y0 != previousState) {
		player->timeSpentInState = 0;
		player->crop.x0 = 0.0f;
	}
	else {
		player->timeSpentInState += deltaTime;
		if (player->timeSpentInState > 200) {
			player->crop.x0 += 0.25f;
			player->timeSpentInState -= 200;
		}

		if (player->crop.y0 == STATE_FALL && player->crop.x0 > 0.25) {
			player->crop.x0 = 0.25f;
		}

		else if (player->crop.x0 == 1.0f) {
			player->crop.x0 = 0.0f;
		}
	}

	player->crop.x1 = player->crop.x0 + 0.25f;
	player->crop.y1 = player->crop.y0 + 0.25f;

	if (body.velocity.x < 0.0f || (body.velocity.x == 0.0f && left)) {
		Switch(&player->crop.x1, &player->crop.x0);
	}
}

void AnimationDraw(RenderObject object, Position2 pos) {
	Point2 p0 = WorldPosToScreenPos(pos.x - object.radius, pos.y);
	Point2 p1 = WorldPosToScreenPos(pos.x + object.radius, pos.y + object.height);
	DrawImage(object.spritesheet, object.crop, {p0.x, p0.y, p1.x, p1.y});
}