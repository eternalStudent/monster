struct Arena {
	ssize ptr;			// base-address realtive to the buffer
	ssize prev_ptr;		// the previous base-address relative to the buffer
	ssize capacity;
	void* buffer;
};

void ArenaInit(Arena* arena, void* buffer, ssize capacity) {
	arena->ptr = 0;
	arena->prev_ptr = 0;
	arena->buffer = buffer;
	arena->capacity = capacity;
}

void* ArenaAlloc(Arena* arena, ssize size, bool zero) {
	if (size == 0) return NULL;

	// compute divisibility of base-address by size
	int32 index = LowBit((uint32)size); //xxxx1->0, xxx10->1, xx100->3, x1000->4, x00000->5  
	if (index > 5) index = 5;
	uint32 divisibility = 1 << index;

	// align base-adress to divisibilty
	uint32 modulo = arena->ptr & (divisibility - 1);
	ssize new_ptr = arena->ptr;
	if (modulo != 0) new_ptr += (divisibility - modulo);

	// do NOT change anything before verifying you have enough space.
	if (new_ptr+size > arena->capacity) return NULL;

	arena->prev_ptr = new_ptr; //this is now the relative base-address of the new data
	arena->ptr = new_ptr + size;

	void* data = (void*)((uintptr_t)arena->buffer + (uintptr_t)arena->prev_ptr);
	if (zero) memset(data, 0, size);
	return data;
}

void* ArenaResize(Arena* arena, void* data, ssize old_size, ssize new_size, bool zero) {
	if (old_size == new_size) return data;
	// if no space was allocated to begin with, just allocate
	if (data == NULL || old_size == 0)
		return ArenaAlloc(arena, new_size, zero);

	ssize ptr = (ssize)((uintptr_t)data - (uintptr_t)arena->buffer);

	// if this is not the last space to be allocated, just allocate new space and copy the data
	if (ptr != arena->prev_ptr) {
		void* new_data = ArenaAlloc(arena, new_size, zero);
		if (new_data == NULL) return NULL;
		memmove(new_data, data, old_size);
		return new_data;
	}
	// assuming now that the space to be resized is the last space that was allocated

	ssize allocated_size = arena->ptr - arena->prev_ptr;
	// if the new size if bigger than what was already allocated, allocate the remaining space
	if (new_size > allocated_size) {
		if (ArenaAlloc(arena, new_size - allocated_size, zero))
			arena->prev_ptr = ptr;
	}
	// if the new size is smaller than what was already allocated, move the pointer back
	else if (new_size < old_size) {
		arena->ptr = (arena->prev_ptr + new_size);
	}
	return data;
}

void ArenaFreeAll(Arena* arena) {
	arena->ptr = 0;
	arena->prev_ptr = 0;
}