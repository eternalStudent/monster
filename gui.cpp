struct UIElement {
    int32 index;
    int32 parent;

    // TODO: maybe Box2 is not right for this, p0 and dimensions is a better fit
    union {
    	Box2 box;
    	struct {Position2 p0, p1;};
    	struct {pixels x0, y0, x1, y1;};
    };

    union {
        Sprite background;
        struct {TextureHandle texture; Box2 crop;};
    };

    uint32 flags; // 1-movable, 2-resizable, 4-clickable
    void (*onClick)(UIElement*);
};

struct GUI {
    UIElement* elements;
    UIElement* active;
    int32* renderOrder; // NOTE: the front is 0
    int32 elementCount;
    Position2 originalPos;
    Position2 grabPos;

    bool isGrabbing;
    bool isResizing;
    bool isPressed;
    bool isBottomRight;
};

inline UIElement* GetNewElement(GUI* gui) {
    int32 index = gui->elementCount+1;
    UIElement* element = &(gui->elements[index]);
    element->index = index;
    gui->elementCount++;
    return element;
}

inline UIElement GetParent(GUI gui, UIElement element) {
    return gui.elements[element.parent];
}

inline Box2 GetAbsolutePosition(GUI gui, UIElement element) {
    if (element.parent == 0)
        return element.box;


    Box2 parentPos = GetAbsolutePosition(gui, GetParent(gui, element));
    Box2 absolute;
    absolute.x0 = parentPos.x0 + element.x0;
    absolute.y0 = parentPos.y0 + element.y0;
    absolute.x1 = parentPos.x0 + element.x1;
    absolute.y1 = parentPos.y0 + element.y1;
    return absolute;
}

inline bool IsInsideBox(GUI gui, UIElement element, Position2 pos) {
    return IsInsideBox(GetAbsolutePosition(gui, element), pos);
}

inline bool IsInBottomRight(GUI gui, UIElement element, Position2 pos) {
    Box2 box = GetAbsolutePosition(gui, element);
    return box.x1-4 <= pos.x && pos.y <= box.y1+4;
}

void SetPosition(GUI gui, UIElement* element, pixels x0, pixels y0) {
    pixels width = element->x1 - element->x0;
    pixels height = element->y1 - element->y0;
    Position2 p0;

    if (!element->parent) {
        p0 = {x0,y0};
    }
    else {
        UIElement parent = GetParent(gui, *element);

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

inline UIElement* GetElementByPos(GUI gui, Position2 pos) {
    for(int32 i = 0; i < gui.elementCount; i++) {
        int32 elementIndex = gui.renderOrder[i];
        if (IsInsideBox(gui, gui.elements[elementIndex], pos)) {
            return &gui.elements[elementIndex];
        }   
    }
    return NULL;
}

void MoveToFront(GUI gui, int32 index) {
    int32 orderIndex = 0;
    for(int32 i=0; i<gui.elementCount; i++) 
        if (gui.renderOrder[i] == index){orderIndex=i; break;}

    if (orderIndex == 0) return;
    memmove(&(gui.renderOrder[1]), gui.renderOrder, sizeof(int32)*orderIndex);
    gui.renderOrder[0] = index;

    for (int32 i=1; i<=gui.elementCount; i++)
        if (gui.elements[i].parent == index) MoveToFront(gui, i);
}

inline void MoveToFront(GUI gui, UIElement* element) {
    return MoveToFront(gui, element->index);
}

void UpdateActiveElement(GUI gui, Position2 cursorPos) {
    UIElement* element = gui.active;
    // Handle grabbing
    if (gui.isGrabbing && 
            (cursorPos.x != gui.grabPos.x || cursorPos.y != gui.grabPos.y)){
        pixels newx0  = gui.originalPos.x + cursorPos.x - gui.grabPos.x;
        pixels newy0 = gui.originalPos.y + cursorPos.y - gui.grabPos.y;
        SetPosition(gui, element, newx0, newy0);
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

void HandleCursorPosition(GUI* gui, Position2 cursorPos){
    gui->isBottomRight = false;
    if (!gui->isResizing && !gui->isGrabbing)
        gui->active = GetElementByPos(*gui, cursorPos);
    UIElement* element = gui->active;
    Win32SetCursorToArrow();
    if (element) {
        if(IsInBottomRight(*gui, *element, cursorPos) && (element->flags & 2)){
            Win32SetCursorToResize();
            gui->isBottomRight = true;
        }
        else{
            if (element->flags & 4) Win32SetCursorToHand();
            else if (element->flags & 1) Win32SetCursorToMove();        
        }
    }
}

#define LDN  1
#define LUP  2
#define RDN  3
#define RUP  4

void HandleMouseEvent(GUI* gui, 
                        int mouseEvent, 
                        Position2 cursorPos) {
    if(!mouseEvent)
        return;
    Assert(mouseEvent < 5)
    UIElement* element = gui->active;
    if (mouseEvent == LDN && gui->active){
        MoveToFront(*gui, element);
        if (element->flags & 4){
            gui->isPressed = true;
            if (element->onClick)
                element->onClick(element);
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
}

void UpdateElements(GUI* gui, Position2 cursorPos, int32 mouseEvent){
    HandleCursorPosition(gui, cursorPos);
    HandleMouseEvent(gui, mouseEvent, cursorPos);
    UpdateActiveElement(*gui, cursorPos);
}

void RenderElements(GUI gui) {
    for(int32 i=gui.elementCount-1; i>=0; i--){
        int32 index = gui.renderOrder[i];
        UIElement element = gui.elements[index];
        Box2 pos = GetAbsolutePosition(gui, element);
        Sprite sprite = element.background;
        if (gui.active && gui.active->index == index && gui.isPressed){
            RenderSprite(blackBrush, pos);
            RenderSprite(sprite, Box_MoveBy(pos, {2, -2}));
        }
        else RenderSprite(sprite, pos);
    }
}