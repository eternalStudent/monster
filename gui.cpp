struct UIElement {
    union {
    	Box2 box;
    	struct {Position2 p0, p1;};
    	struct {float32 x0, y0, x1, y1;};
    };

    int32 tileId;
    TextureHandle solid;

    int32 parent;

    uint32 flags; // 1-movable, 2-resizable, 4-clickable
    void (*onClick)(int32);
};

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

// input

#define LDN  1
#define LUP  2
#define RDN  3
#define RUP  4

// NOTE: I'm not actually currently using pos at all
struct MouseEvent {
    int32 type;
    union{
        Position2 pos;
        struct{pixels x, y;};
    };
};

struct MouseEventQueue{
    union{
        void* data;
        MouseEvent* table;
    };
    int32 size;
    int32 capacity;
    int32 front;
    int32 rear;
};