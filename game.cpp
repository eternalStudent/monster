#include "physics.cpp"
#include "animation.cpp"

struct Entity {
	union {
		Position2 pos;
		struct {float32 x, y;};
	};
	PhysicalBody body;
	RenderObject object;
};

struct {
	bool isPlayerExists;
	Entity player;
} game;

void GameInit(Arena* arena, int32* accelaration, int32* jumpForce, int32* maxSpeed, int32* gravity) {
	game.player = {};
	PhysicsInit(accelaration, jumpForce, maxSpeed, gravity);
	PhysicsInitPlayer(&game.player.body);
	AnimationInitPlayer(arena, &game.player.object);
}

void GameAddPlayer(int32 x) {
	if (game.isPlayerExists) return;
	Point2 p = ScreenPosToWorldPos(CursorPosToScreenPos(x, 0).x, 0);
	game.player.pos = {p.x, 500};
	game.isPlayerExists = true;
}

void GameRemovePlayer() {
	game.isPlayerExists = false;
}

void GameUpdate(milliseconds deltaTime) {
	if (!game.isPlayerExists) return;
	PhysicsUpdate(&game.player.body, &game.player.pos, deltaTime);
	AnimationUpdate(&game.player.object, game.player.body, deltaTime);
}

void GameDraw() {
	if (!game.isPlayerExists) return;
	AnimationDraw(game.player.object, game.player.pos);
}