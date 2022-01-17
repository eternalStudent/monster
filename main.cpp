#include "basic/basic.cpp"
#include "graphics/graphics.cpp"
#  include "gui.cpp"
#  include "world.cpp"
#    include "game.cpp"

static byte persistsTileId = 0;
static bool isFullScreen;
static UIImage* minimap;
static UIElement* fullButton;
static String* debugString;
static byte debugBuffer[256];

void PickTile(UIElement* e) {
	persistsTileId = (byte)e->context.i;
}

void ExitFullScreen(UIElement* e);

void EnterFullScreen(UIElement* e) {
	OsEnterFullScreen();
	isFullScreen = true;
	e->onClick = ExitFullScreen;
	e->image->crop = {0.25, 0.5, 0.5, 0.75};
}

void ExitFullScreen(UIElement* e) {
	OsExitFullScreen();
	isFullScreen = false;
	e->onClick = EnterFullScreen;
	e->image->crop = {0, 0.5, 0.25, 0.75};
}

void LoadWorld(UIElement* e) {
	char path[MAX_PATH_SIZE];
	if (OsOpenFileDialog(path, MAX_PATH_SIZE)) {
		WorldReadData(path, minimap->atlas);
	}
}

void SaveWorld(UIElement* e) {
	char path[MAX_PATH_SIZE];
	if (OsSaveFileDialog(path, MAX_PATH_SIZE)) {
		WorldWriteData(path);
	}
}

void ClearWorld(UIElement* e) {
	WorldClearData(minimap->atlas);
	GameRemovePlayer();
}

void MoveCamera(UIElement* e) {
	CameraMoveByMinimap(e->x, e->height-e->y);
}

void Init(Arena* persist, Arena* scratch) {
	ArenaAlign(persist, alignof(Font));
	Font* font = (Font*)ArenaAlloc(persist, sizeof(Font));
	*font = LoadFont(scratch, "data/AzeretMono-Regular.ttf", 18, 0xffffff);

	UIElement* control = UICreateTabControl(NULL, {298, 322});
	control->pos = {1400, 150};

	byte tileId = 1;
    UIElement* tab1 = UICreateTab(control, {72, 24}, STR("grass"), font);
    for (int32 i=0; i<4; i++) for (int32 j=0; j<4; j++) {
    	UIElement* tileElement = UICreateElement(tab1);
    	tileElement->flags = UI_CLICKABLE;
    	tileElement->pos = {i*72+8, j*72+8};
    	tileElement->dim = {64, 64};
    	tileElement->context.i = (int64)tileId;
    	tileElement->onClick = PickTile;
    	UIImage* image = UICreateImage(tileElement);
    	image->dim = {64, 64};
    	Tile* tile = &(world.tiles[tileId]);
    	image->atlas = tile->atlas;
    	image->crop = tile->crop;
    	tileId++;
	}

    UIElement* tab2 = UICreateTab(control, {72, 24}, STR("stuff"), font);
    for (int32 i=0; i<4; i++) for (int32 j=0; j<4; j++) {
    	UIElement* tileElement = UICreateElement(tab2);
    	tileElement->flags = UI_CLICKABLE;
    	tileElement->pos = {i*72+8, j*72+8};
    	tileElement->dim = {64, 64};
    	tileElement->context.i = (int64)tileId;
    	tileElement->onClick = PickTile;
    	UIImage* image = UICreateImage(tileElement);
    	image->dim = {64, 64};
    	Tile* tile = &(world.tiles[tileId]);
    	image->atlas = tile->atlas;
    	image->crop = tile->crop;
    	tileId++;
	}

	UISetActiveTab(tab1);

	TextureId iconAtlas = LoadTexture(scratch, "data/icons.bmp", GRAPHICS_SMOOTH);

	UIElement* loadButton = UICreateElement(NULL);
	loadButton->pos = {12, 12};
	loadButton->dim = {24, 24};
	loadButton->flags = UI_CLICKABLE;
	loadButton->onClick = LoadWorld;
	loadButton->name = STR("load");
	UIImage* loadIcon = UICreateImage(loadButton);
	loadIcon->atlas = iconAtlas;
	loadIcon->crop = {0, 0.75, 0.25, 1};

	UIElement* saveButton = UICreateElement(NULL);
	saveButton->pos = {48, 12};
	saveButton->dim = {24, 24};
	saveButton->flags = UI_CLICKABLE;
	saveButton->onClick = SaveWorld;
	saveButton->name = STR("save");
	UIImage* SaveIcon = UICreateImage(saveButton);
	SaveIcon->atlas = iconAtlas;
	SaveIcon->crop = {0.25, 0.75, 0.5, 1};

	UIElement* clearButton = UICreateElement(NULL);
	clearButton->pos = {84, 12};
	clearButton->dim = {24, 24};
	clearButton->flags = UI_CLICKABLE;
	clearButton->onClick = ClearWorld;
	clearButton->name = STR("clear");
	UIImage* clearIcon = UICreateImage(clearButton);
	clearIcon->atlas = iconAtlas;
	clearIcon->crop = {0.5, 0.75, 0.75, 1};

	fullButton = UICreateElement(NULL);
	fullButton->pos = {120, 12};
	fullButton->dim = {24, 24};
	fullButton->flags = UI_CLICKABLE;
	fullButton->onClick = EnterFullScreen;
	fullButton->name = STR("enter/exit full screen");
	UIImage* fullIcon = UICreateImage(fullButton);
	fullIcon->atlas = iconAtlas;
	fullIcon->crop = {0, 0.5, 0.25, 0.75};

    Image minimapImage = {62*4, 32*4, 4, (byte*)WorldGetCleanMinimap()};
    UIElement* minimapElement = UICreateElement(NULL);
    minimapElement->pos = {1400, 12};
    minimapElement->dim = minimapImage.dimensions;
    minimapElement->flags = UI_MOVABLE;
    minimap = UICreateImage(minimapElement);
    minimapElement->borderColor = RGBA_WHITE;
	minimapElement->borderWidth = 1;
    minimap->atlas = GenerateTexture(minimapImage, GRAPHICS_PIXELATED);
    minimap->crop = {0, 0, 1, 1};

    UIElement* minicamera = UICreateElement(minimapElement);
    minicamera->dim = {31*4, 16*4};
    minicamera->y = 16*4;
    minicamera->background = 0x887e7872;
    minicamera->flags = UI_MOVABLE;
    minicamera->onMove = MoveCamera;

    UIElement* sliders = UICreateElement(NULL);
    sliders->background = 0x22413830;
    sliders->dim = {276, 162};
    sliders->pos = {6, 48};
    sliders->flags = UI_MOVABLE;
    sliders->borderColor = RGBA_WHITE;
    sliders->borderWidth = 1;

    UIElement* slider1 = UICreateSlider(sliders, 264);
    slider1->pos = {6, 30};
    UIText* text1 = UICreateText(slider1);
    text1->y = -24;
    text1->font = font;
    text1->string = STR("accelaration:");

    UIElement* slider2 = UICreateSlider(sliders, 264);
    slider2->pos = {6, 66};
    UIText* text2 = UICreateText(slider2);
    text2->y = -24;
    text2->font = font;
    text2->string = STR("max speed:");

    UIElement* slider3 = UICreateSlider(sliders, 264);
    slider3->pos = {6, 102};
    UIText* text3 = UICreateText(slider3);
    text3->y = -24;
    text3->font = font;
    text3->string = STR("jump force:");

    UIElement* slider4 = UICreateSlider(sliders, 264);
    slider4->pos = {6, 138};
    UIText* text4 = UICreateText(slider4);
    text4->y = -24;
    text4->font = font;
    text4->string = STR("gravity:");

    UIElement* debugContainer = UICreateElement(NULL);
    debugContainer->flags = UI_MOVABLE;
    debugContainer->borderColor = RGBA_WHITE;
    debugContainer->borderWidth = 1;
    debugContainer->dim = {276, 162+24};
    debugContainer->pos = {6, 240};
    UIText* label = UICreateText(debugContainer);
    label->font = font;
    label->string = STR("debug log");
    UIElement* debugLog = UICreateElement(debugContainer);
    debugLog->background = 0x22413830;
    debugLog->dim = {276, 162};
    debugLog->pos = {0, 24};
    debugLog->borderColor = RGBA_WHITE;
    debugLog->borderWidth = 1;
    UIText* debugText = UICreateText(debugLog);
    debugText->font = font;
    debugText->string.data = debugBuffer;
    debugText->string.length = 0;

    debugString = &(debugText->string);

    CameraInit(&(minicamera->box));
    GameInit(scratch, &slider1->first->x, &slider2->first->x, &slider3->first->x, &slider4->first->x);
}

int main() {	
	Arena persist = CreateArena(1024*1024*128);
	Arena scratch = CreateArena(1024*1024*128);
	
	UIInit(&persist, &scratch);
	OsCreateWindow("Monster", 1800, 512);
	GraphicsInit(&scratch);
	WorldInit(&persist, &scratch);
	UISetWindowElement(0);
	
	Init(&persist, &scratch);
	
	OsTimeInit();
	OsTimeStart();
	while(true){
		ArenaFreeAll(&scratch);
		float32 deltaTime = (float32)OsTimeRestart();
		if (deltaTime > 17.0f) deltaTime = 17.0f;
		Point2i cursorPos = OsGetCursorPosition();
		OsHandleWindowEvents();

		CameraUpdateDimensions(OsGetWindowDimensions());

		if (IsKeyDown(KEY_ESC)) {
			if (isFullScreen) ExitFullScreen(fullButton);
			else break;
		}
		if (OsWindowDestroyed()) break;

		GameUpdate(deltaTime);
		UIElement* active = UIUpdateElements(cursorPos);

		// debug string
		StringBuilder builder = {debugBuffer, debugBuffer};
		PushString(&builder, STR("cursor pos: "));
		PushPoint2i(&builder, cursorPos);
		PushNewLine(&builder);
		PushString(&builder, STR("screen pos: "));
		Point2 screenPos = CursorPosToScreenPos(cursorPos.x, cursorPos.y);
		PushPoint2(&builder, screenPos, 1);
		PushNewLine(&builder);
		PushString(&builder, STR("world pos: "));
		Point2 worldPos = ScreenPosToWorldPos(screenPos.x, screenPos.y);
		PushPoint2(&builder, worldPos, 1);
		PushNewLine(&builder);
		PushString(&builder, STR("tile pos: "));
		PushPoint2i(&builder, WorldPosToTilePos(worldPos.x, worldPos.y));
		PushNewLine(&builder);
		PushString(&builder, STR("delta time: "));
		PushFloat32(&builder, deltaTime, 2);
		PushNewLine(&builder);
		PushString(&builder, STR("player pos: "));
		PushPoint2(&builder, game.player.pos, 2);
		if (active && active->name.length) {
			PushNewLine(&builder);
			PushString(&builder, STR("active ui element: "));
			PushString(&builder, active->name);
		}
		*debugString = GetString(&builder);
		
		if (!active) {
			if (IsMouseDown(MOUSE_R)) WorldSetTile(cursorPos, 0, minimap->atlas);
			if (IsMouseDown(MOUSE_L)) {
				WorldSetTile(cursorPos, persistsTileId, minimap->atlas);
				GameAddPlayer(cursorPos.x);
			}
		}
		
		GraphicsClearScreen();
		  CameraCrop();
		    WorldDrawTiles();
		    GameDraw();
		  CameraClearCrop();
		  UIRenderElements();
		GraphicsSwapBuffers();
	}
	return 0;
}