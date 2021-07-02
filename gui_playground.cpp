#define ElementCount 20

static GUI gui = {};
static UIElement elements[ElementCount+1] = {};
static int32 renderOrder[ElementCount] = {};

void PlaygroundInit() {
	gui.elementCount = 1;
	gui.elements = elements;
	gui.renderOrder = renderOrder;

	TextureHandle neutral = GenerateTextureFromRGBA(0x88c1daff);
	TextureHandle grey1 = GenerateTextureFromRGBA(0xFF999999);
	TextureHandle grey2 = GenerateTextureFromRGBA(0xFF777777);

	UIElement* tabControl = GetNewElement(&gui);
	tabControl->p0 = {50.0f, 50.0f};
	tabControl->p1 = Move(tabControl->p0, 240, 396);
	tabControl->parent = 0;
	tabControl->flags = 1;
	tabControl->texture = neutral;
	renderOrder[6] = tabControl->index;

	UIElement* tabHead2 = GetNewElement(&gui);
	tabHead2->p0 = {120.0f, 348.0f};
	tabHead2->p1 = Move(tabHead2->p0, 108.0f, 36.0f);
	tabHead2->parent = tabControl->index;
	tabHead2->flags = 0;
	tabHead2->texture = grey2;
	renderOrder[5] = tabHead2->index;

	UIElement* tab2 = GetNewElement(&gui);
	tab2->p0 = {-108.0f, -336.0f};
	tab2->p1 = Move(tab2->p0, 216.0f, 336.0f);
	tab2->parent = tabHead2->index;
	tab2->flags = 0;
	tab2->texture = grey2;
	renderOrder[4] = tab2->index;

	UIElement* tabHead1 = GetNewElement(&gui);
	tabHead1->p0 = {12.0f, 348.0f};
	tabHead1->p1 = Move(tabHead1->p0, 108.0f, 36.0f);
	tabHead1->parent = tabControl->index;
	tabHead1->flags = 0;
	tabHead1->texture = grey1;
	renderOrder[3] = tabHead1->index;	

	UIElement* tab1 = GetNewElement(&gui);
	tab1->p0 = {0.0f, -336.0f};
	tab1->p1 = Move(tab1->p0, 216.0f, 336.0f);
	tab1->parent = tabHead1->index;
	tab1->flags = 0;
	tab1->texture = grey1;
	renderOrder[2] = tab1->index;

	UIElement* slider = GetNewElement(&gui);
	slider->p0 = {80.0f, 916.0f};
	slider->p1 = Move(slider->p0, 264.0f, 8.0f);
	slider->parent = 0;
	slider->flags = 1;
	slider->texture = neutral;
	renderOrder[1]=slider->index;

	UIElement* sliderPos = GetNewElement(&gui);
	sliderPos->p0 = {128.0f, 0.0f};
	sliderPos->p1 = Move(sliderPos->p0, 8.0f, 8.0f);
	sliderPos->parent = slider->index;
	sliderPos->flags = 1;
	sliderPos->texture = blackTexture;
	renderOrder[0]=sliderPos->index;
}

void PlaygroundUpdateAndRender(MouseEventQueue* mouseEventQueue, Position2 cursorPos) {
	ClearScreen();
	UpdateElements(&gui, cursorPos, mouseEventQueue);
	RenderElements(gui);

	// Debug text
	char buffer[256];
	char* str;

	str = buffer;
	str += CopyString("cursor: (", 9, str); str += float32ToDecimal(cursorPos.x, 0, str); 
	str += CopyString(", ", 2, str); str += float32ToDecimal(cursorPos.y, 0, str); memcpy(str, ")", 2);
	DebugPrintText(16, 32.0 * 28, buffer);

	if (gui.elementIndex != 0){
		UIElement* element = &elements[gui.elementIndex];
		str = buffer;
		str += CopyString("element", 7, str);
		str += int32ToDecimal(gui.elementIndex, str);
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