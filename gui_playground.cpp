#define ElementCount 20

static GUI gui = {};
static UIElement elements[ElementCount+1] = {};
static int32 renderOrder[ElementCount] = {};

void PlaygroundInit() {
	gui.elements = elements;
	gui.renderOrder = renderOrder;
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

	if (gui.active) {
		UIElement* element = gui.active;
		str = buffer;
		str += CopyString("element", 7, str);
		str += int32ToDecimal(gui.active->index, str);
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