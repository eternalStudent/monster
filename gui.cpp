struct UIElement {
    int32 index;

    // Why am I using floating points here anyway??
    union {
    	Box2 box;
    	struct {Position2 p0, p1;};
    	struct {pixels x0, y0, x1, y1;};
    };

    // NOTE: either a tile, or a texture
    int32 tileId;
    TextureHandle texture;

    int32 parent;

    uint32 flags; // 1-movable, 2-resizable, 4-clickable
    void (*onClick)(int32);
};

struct GUI {
    UIElement* elements;
    int32* renderOrder; // NOTE: the front is 0
    int32 elementCount;
    int32 elementIndex;
    Position2 originalPos;
    Position2 grabPos;

    bool isGrabbing;
    bool isResizing;
    bool isPressed;
    bool isBottomRight;
};

inline UIElement* GetNewElement(GUI* gui) {
    int32 index = gui->elementCount;
    UIElement* element = &(gui->elements[index]);
    element->index = index;
    gui->elementCount++;
    return element;
}

inline Box2 GetAbsolutePosition(UIElement element, UIElement* elements) {
    if (element.parent == 0)
        return element.box;

    Box2 parentPos = GetAbsolutePosition(elements[element.parent], elements);
    Box2 absolute;
    absolute.x0 = parentPos.x0 + element.x0;
    absolute.y0 = parentPos.y0 + element.y0;
    absolute.x1 = parentPos.x0 + element.x1;
    absolute.y1 = parentPos.y0 + element.y1;
    return absolute;
}

inline bool IsInsideBox(UIElement element, Position2 pos, UIElement* elements) {
    return IsInsideBox(GetAbsolutePosition(element, elements), pos);
}

inline bool IsInBottomRight(UIElement element, Position2 pos, UIElement* elements) {
    Box2 box = GetAbsolutePosition(element, elements);
    return box.x1-4 <= pos.x && pos.y <= box.y1+4;
}

void SetPosition(UIElement* element, pixels x0, pixels y0, UIElement* elements) {
    pixels width = element->x1 - element->x0;
    pixels height = element->y1 - element->y0;
    Position2 p0;

    if (!element->parent) {
        p0 = {x0,y0};
    }
    else {
        UIElement parent = elements[element->parent];

        if (x0 < 0) p0.x = 0;
        else if (x0+width > parent.x1-parent.x0) p0.x = parent.x1-parent.x0-width;
        else p0.x = x0;

        if (y0 < 0) p0.y = 0;
        else if (y0+height > parent.y1-parent.y0) p0.y = parent.y1-parent.y0-height;
        else p0.y = y0;
    }
    
    element->p0 = p0;
    element->x1 = p0.x + width;
    element->y1 = p0.y + height;
}

inline int32 GetElementIndexByPos(Position2 pos, GUI gui) {
    for(int32 i = 0; i < gui.elementCount; i++) {
        if (IsInsideBox(gui.elements[gui.renderOrder[i]], pos, gui.elements)) {
            return gui.renderOrder[i];
        }   
    }
    return 0;
}

void MoveToFront(int32 index, GUI gui) {
    int32 orderIndex = 0;
    for(int32 i=0; i<gui.elementCount; i++) 
        if (gui.renderOrder[i] == index){orderIndex=i; break;}

    if (orderIndex == 0) return;
    memmove(&(gui.renderOrder[1]), gui.renderOrder, sizeof(int32)*orderIndex);
    gui.renderOrder[0] = index;

    for (int32 i=1; i<=gui.elementCount; i++)
        if (gui.elements[i].parent == index) MoveToFront(i, gui);
}

void UpdateElement(UIElement* element, GUI gui, Position2 cursorPos) {
    // Handle grabbing
    if (gui.isGrabbing && 
            (cursorPos.x != gui.grabPos.x || cursorPos.y != gui.grabPos.y)){
        pixels newx0  = gui.originalPos.x + cursorPos.x - gui.grabPos.x;
        pixels newy0 = gui.originalPos.y + cursorPos.y - gui.grabPos.y;
        SetPosition(element, newx0, newy0, gui.elements);
    }

    // Handle resizing
    if (gui.isResizing) {
        /*Position2 relativeCursorPos = GetRelativePosition(cursorPos, *element);
        element->y = relativeCursorPos.y;
        pixels left = originalPos.x-originalPos.radius;
        element->x = (relativeCursorPos.x + left)*0.5f;
        pixels top = originalPos.y + originalPos.height;
        element->height = top - relativeCursorPos.y;
        element->radius = fabsf((relativeCursorPos.x - left)*0.5f);

        if (element->height < 0){
            element->y = originalPos.y+originalPos.height;
            element->height = -element->height;
        }*/
    }
}

void Win32SetCursorToMove();
void Win32SetCursorToArrow();
void Win32SetCursorToResize();
void Win32SetCursorToHand();

UIElement* HandleCursorPosition(GUI* gui, Position2 cursorPos){
    gui->isBottomRight = false;
    if (!gui->isResizing && !gui->isGrabbing)
    gui->elementIndex = GetElementIndexByPos(cursorPos, *gui);
    UIElement* element = &(gui->elements[gui->elementIndex]);
    Win32SetCursorToArrow();
    if (gui->elementIndex != 0) {
        if(IsInBottomRight(*element, cursorPos, gui->elements) && (element->flags & 2)){
            Win32SetCursorToResize();
            gui->isBottomRight = true;
        }
        else{
            if (element->flags & 4) Win32SetCursorToHand();
            else if (element->flags & 1) Win32SetCursorToMove();        
        }
    }
    return element;
}

// input

#define LDN  1
#define LUP  2
#define RDN  3
#define RUP  4

struct MouseEventQueue{
    union{
        void* data;
        int32* table;
    };
    int32 size;
    int32 capacity;
    int32 front;
    int32 rear;
};

int32 HandleMouseEvent(MouseEventQueue* mouseEventQueue, GUI* gui, 
                            UIElement* element, 
                            Position2 cursorPos) {
    if(!(mouseEventQueue->size))
        return {};
    int32 mouseEvent = Dequeue(mouseEventQueue);
    Assert(mouseEvent < 5)
    if (mouseEvent == LDN && gui->elementIndex != 0){
        MoveToFront(gui->elementIndex, *gui);
        if (element->flags & 4){
            gui->isPressed = true;
            if (element->onClick)
                element->onClick(gui->elementIndex);
        }
        else{
            if (gui->isBottomRight && (element->flags & 2)) gui->isResizing = true;
            else if (element->flags & 1) gui->isGrabbing = true;
            gui->grabPos = cursorPos;
            gui->originalPos = element->p0;
        }
    }
    else if (mouseEvent == LUP){
        gui->isGrabbing = false;
        gui->isResizing = false;
        gui->isPressed = false;
    }
    return mouseEvent;
}

int32 UpdateElements(GUI* gui, Position2 cursorPos, MouseEventQueue* mouseEventQueue){
    UIElement* element = HandleCursorPosition(gui, cursorPos);
    int32 mouseEvent = HandleMouseEvent(mouseEventQueue, gui, element, cursorPos);
    UpdateElement(element, *gui, cursorPos);
    return mouseEvent;
}

void RenderElements(GUI gui) {
    for(int32 i=gui.elementCount-1; i>=0; i--){
        int32 index = gui.renderOrder[i];
        UIElement element = gui.elements[index];
        Box2 pos = GetAbsolutePosition(element, gui.elements);
        int32 tileId = element.tileId;
        Tile tile;
        if (element.tileId){
            tile = sprites[tileId];
        }
        else{
            tile.crop = BOX2_UNIT();
            tile.solid = element.texture; 
        }
        if (index == gui.elementIndex && gui.isPressed){
            RenderBox2(
                blackTexture, BOX2_UNIT(),
                pos);
            RenderBox2(
                tile.texture, tile.crop,
                Box_MoveBy(pos, {2, -2}));
        }
        else RenderBox2(
            tile.texture, tile.crop,
            pos);
    }
}