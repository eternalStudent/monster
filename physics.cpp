typedef float32 pixels;
typedef float32 milliseconds;
typedef float32 pixels_per_millisec;
typedef float32 pixels_per_millisec_2;

typedef Point2  Vector2;
typedef Vector2 Position2;
typedef Vector2 Velocity2;
typedef Vector2 Acceleration2;

struct HitBox {
	pixels height; 
	pixels radius; 
	// pixels altitude;
};

struct PhysicalBody {
	union {
		HitBox hitBox;
		struct {pixels height, radius;};
	};
	Velocity2 velocity;
    bool onGround;
    bool isFalling;
};

struct {
	int32* accelaration;
	int32* jumpForce;
	int32* maxSpeed;
	int32* gravity;
} physics;

void PhysicsInit(int32* accelaration, int32* jumpForce, int32* maxSpeed, int32* gravity) {
	physics.accelaration = accelaration;
	physics.jumpForce = jumpForce;
	physics.maxSpeed = maxSpeed;
	physics.gravity = gravity;
}

void PhysicsInitPlayer(PhysicalBody* player) {
	player->hitBox.radius = 10.0f;
	player->hitBox.height = 58.0f;
}

inline float32 SafeDivNoNeg(float32 a, float32 b) {
    if (a == 0) return 0;
    if (b == 0) return INF32();
    float32 result = a/b;
    return result < 0 ? 0 : result;
}

void PhysicsUpdate(PhysicalBody* player, Position2* pos, milliseconds deltaTime) {
	pixels_per_millisec_2 gravity = LoadExp(-9) + LoadExp(-17)*(*physics.gravity);
	pixels_per_millisec_2 jumpForce = LoadExp(-5) + LoadExp(-13)*(*physics.jumpForce);
	pixels_per_millisec_2 accelaration = LoadExp(-12) + LoadExp(-20)*(*physics.accelaration);
	pixels_per_millisec maxSpeed = 0.5f + LoadExp(-9)*(*physics.maxSpeed);

	if (IsKeyDown(KEY_UP)) {
		if (player->onGround) {
			player->velocity.y += deltaTime * jumpForce;
			player->onGround = false;
		}
	}
	else {
		if (player->velocity.y > 0) player->velocity.y = 0;
	}
	if (IsKeyDown(KEY_LEFT) && player->velocity.x <= 0) {
		player->velocity.x -= deltaTime * accelaration;
	}
	else if (IsKeyDown(KEY_RIGHT) && player->velocity.x >= 0) {
		player->velocity.x += deltaTime * accelaration;
	}
	else
		player->velocity.x = 0;

	if (player->velocity.x > maxSpeed) player->velocity.x = maxSpeed;
	if (player->velocity.x < -maxSpeed) player->velocity.x = -maxSpeed;

	if (pos->y > 0)
		player->velocity.y -= deltaTime * gravity;
	
	milliseconds remainingTime = deltaTime;
	for (uint32 i = 0; i < 4; i++) {
		pixels_per_millisec speedEpsilon = 0.00001f;
		bool atEpsilonSpeed = -speedEpsilon <= player->velocity.x && player->velocity.x <= speedEpsilon;

		if (remainingTime <= 0 || atEpsilonSpeed && player->velocity.y == 0)
			break;

		pixels projectedX = atEpsilonSpeed  
			? pos->x : 
			pos->x + player->velocity.x * remainingTime;
		pixels projectedY = pos->y + player->velocity.y * remainingTime;
		if (projectedY < 0)
			projectedY = 0;

		int32 left   = (int32)((projectedX - player->hitBox.radius + 32.0f)/64.0f);
		int32 right  = (int32)((projectedX + player->hitBox.radius + 32.0f)/64.0f);
		int32 top    = (int32)((projectedY + player->hitBox.height)/64.0f);
		int32 bottom = (int32)((projectedY)/64.0f);

		bool hitsTop = false, hitsBottom = false, hitsLeft = false, hitsRight = false;
		if (0 <= left   && left   < WORLD_WIDTH) for (int32 y = bottom; y <= top; y++) {
			uint32 flags = WorldGetFlags(left, y);
			if (flags & 1) hitsLeft = true;
		}
		if (0 <= right  && right  < WORLD_WIDTH) for (int32 y = bottom; y <= top; y++) {
			uint32 flags = WorldGetFlags(right, y);
			if (flags & 1) hitsRight = true;
		}
		if (0 <= top    && top    < WORLD_HEIGHT) for (int32 x = left; x <= right; x++) {
			int32 flags = WorldGetFlags(x, top);
			if (flags & 1) hitsTop = true;
		}
		if (0 <= bottom && bottom < WORLD_HEIGHT) for (int32 x = left; x <= right; x++) {
			uint32 flags = WorldGetFlags(x, bottom);
			uint32 oneTileUpFlags = WorldGetFlags(x, bottom+1);
			// NOTE: you can not stand on a block if one tile above it is blocked
			if ((flags & 1) && !(oneTileUpFlags & 1)) {
				hitsBottom = true;
			}
		}

		pixels pointLeft   = (left   + 0.5f)*64.0f + player->hitBox.radius + 1;
		pixels pointRight  = (right  - 0.5f)*64.0f - player->hitBox.radius - 1;
		pixels pointTop    = (top    - 0.0f)*64.0f - player->hitBox.height - 1;
		pixels pointBottom = (bottom + 1.0f)*64.0f;

		milliseconds timeToTop = SafeDivNoNeg(pointTop - pos->y, player->velocity.y);
		milliseconds timeToBottom = SafeDivNoNeg(pointBottom - pos->y, player->velocity.y);
		milliseconds timeToLeft = SafeDivNoNeg(pointLeft - pos->x, player->velocity.x);
		milliseconds timeToRight = SafeDivNoNeg(pointRight - pos->x, player->velocity.x);

		if (!hitsTop && !hitsBottom && !hitsLeft && !hitsRight) {
			pos->x = projectedX;
			pos->y = projectedY;
			break;
		}
		else {
			if (atEpsilonSpeed && player->velocity.y <= 0 && hitsBottom) {
				if (projectedY < pointBottom)
					pos->y = pointBottom;
				player->velocity.y = 0;
				player->onGround = true;
				break;
			}
			else if (!atEpsilonSpeed && player->velocity.y <= 0) {
				if (hitsBottom && player->velocity.y != 0) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToBottom <= timeToRight) || hitsLeft && timeToBottom <= timeToLeft) {
						if (projectedY < pointBottom)
							pos->y = pointBottom;
						player->velocity.y = 0;
						player->onGround = true;
						remainingTime -= timeToBottom;
						continue;
					}
				}
				pos->y = projectedY;
				remainingTime -= timeToBottom;
				continue;
			}
			else if (atEpsilonSpeed && player->velocity.y > 0) {
				if (projectedY > pointTop)
					pos->y = pointTop;
				player->velocity.y = 0;
				break;
			}
			else if (!atEpsilonSpeed && player->velocity.y > 0) {
				if (hitsLeft) {
					if (!hitsTop || (hitsTop && timeToLeft <= timeToTop)) {
						if (projectedX < pointLeft && pointLeft < pos->x)
							pos->x = pointLeft;
						player->velocity.x = -speedEpsilon;
						remainingTime -= timeToLeft;
						continue;
					}
				}
				if (hitsRight) {
					if (!hitsTop || (hitsTop && timeToRight <= timeToTop)) {
						if (pos->x < pointRight && pointRight < projectedX)
							pos->x = pointRight;
						player->velocity.x = speedEpsilon;
						remainingTime -= timeToRight;
						continue;
					}
				}
				if (hitsTop) {
					if ((!hitsRight && !hitsLeft) || (hitsRight && timeToTop < timeToRight) || hitsLeft && timeToTop < timeToLeft) {
						if (projectedY > pointTop)
							pos->y = pointTop;
						player->velocity.y = 0;
						remainingTime -= timeToTop;
						continue;
					}
				}
				ASSERT(0);
			}
			else {
				ASSERT(0);
			}
		}
	}

	if (pos->y <= 0) {
		pos->y = 500;
		player->velocity = {};
	}
}