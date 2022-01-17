#define WORLD_BACKGROUND			RGBA_BLUE

struct {
	Dimensions2i dim;   // dimension in screen coordinates
	Point2i screenPos;  // position in screen coordinates
	Point2 worldPos;    // position in world coordinates
	int32 clientHeight;
	UIBox* minimap;
} camera;

void CameraInit(UIBox* minimap) {
	camera.worldPos = {};
	camera.minimap = minimap;
}

void CameraMoveByMinimap(int32 x, int32 y) {
	camera.worldPos = {x*16.0f, y*16.0f};
}

void CameraMoveByPlayer(Point2 p) {
	float32 half_width = 960.0f;
	float32 half_height = 540.0f;
	camera.worldPos.x = p.x > half_width ? p.x - half_width : 0;
	camera.worldPos.y = p.y > half_height ? p.y - half_height : 0;

	camera.minimap->x = (int32)(camera.worldPos.x/16.0f);
	camera.minimap->y = camera.minimap->height - (int32)(camera.worldPos.y/16.0f);
}

void CameraUpdateDimensions(Dimensions2i dimensions) {
	float32 ratio = (float32)dimensions.width/(float32)dimensions.height;
	if (ratio <= 1.875) {
		camera.dim.width = dimensions.width;
		camera.screenPos.x = 0;
		camera.dim.height = (int32)((float32)dimensions.width / 1.875);
		camera.screenPos.y = (dimensions.height - camera.dim.height)/2;
	}
	else {
		camera.dim.height = dimensions.height;
		camera.screenPos.y = 0;
		camera.dim.width = (int32)((float32)dimensions.height * 1.875);
		camera.screenPos.x = (dimensions.width - camera.dim.width)/2;
	}
	camera.clientHeight = dimensions.height;
}

Point2 TilePosToWorldPos(int32 x, int32 y) {
	return {((float32)x - 0.5f)*64.0f, y*64.0f};
}

Point2 WorldPosToScreenPos(float32 x, float32 y) {
	float32 scale = (float32)camera.dim.width/1920.0f;

	return {(float32)camera.screenPos.x + scale*(x - camera.worldPos.x),
			(float32)camera.screenPos.y + scale*(y - camera.worldPos.y)};
}

Point2 CursorPosToScreenPos(int32 x, int32 y) {
	return {(float32)x, (float32)(camera.clientHeight-y-1)};
}

Point2 ScreenPosToWorldPos(float32 x, float32 y) {
	float32 scale = (float32)camera.dim.width/1920.0f;

	return {((x - (float32)camera.screenPos.x)/scale) + camera.worldPos.x,
			((y - (float32)camera.screenPos.y)/scale) + camera.worldPos.y};
}

Point2i WorldPosToTilePos(float32 x, float32 y) {
	return {(int32)((x/64.0f) + 0.5f), (int32)(y/64.0f)};
}

void CameraCrop() {
	DrawBox2(
		WORLD_BACKGROUND, {
			(float32)camera.screenPos.x, 
			(float32)camera.screenPos.y, 
			(float32)(camera.screenPos.x + camera.dim.width), 
			(float32)(camera.screenPos.y + camera.dim.height)
		}
	);

	GraphicsCropScreen(camera.screenPos.x, camera.screenPos.y, camera.dim.width, camera.dim.height);
}

void CameraClearCrop(){
	GraphicsClearCrop();
}