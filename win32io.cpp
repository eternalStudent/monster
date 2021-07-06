// NOTE: unused
void Win32Skip(HANDLE hFile, LONG distance) {
    DWORD dwPtrLow = SetFilePointer(hFile, distance, NULL, FILE_CURRENT);
    if (dwPtrLow == INVALID_SET_FILE_POINTER || GetLastError() != NO_ERROR) {
        Log("failed to set file pointer");
    }
}

DWORD Win32ReadFile(HANDLE hFile, void* buffer, DWORD size) {
    DWORD bytesRead;
    if (FALSE == ReadFile(hFile, buffer, size, &bytesRead, 0)) {
        Log("failed to read file");
    }
    return bytesRead;
}

HANDLE Win32OpenFile(const char* filePath) {
    HANDLE hFile = CreateFileA(filePath, GENERIC_READ, 
        FILE_SHARE_READ,
        NULL, // securtiy mode
        OPEN_EXISTING, 
        0,   // attributes
        NULL // template file 
        );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Log("failed to open file");
        CloseHandle(hFile);
        return NULL;
    }
    return hFile;
}

HANDLE Win32CreateFile(const char* filePath){
    HANDLE hFile = CreateFileA(filePath, GENERIC_WRITE, 
        0,    // sharing mode
        NULL, // securtiy mode
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
        NULL // template file 
        );
    if (hFile == INVALID_HANDLE_VALUE)
    {
        Log("failed to create file");
        CloseHandle(hFile);
        return NULL;
    }
    return hFile;
}

// TODO: I should have a type for data+size
void* Win32LoadStream(const char* filePath, int32 extraBytes, LARGE_INTEGER* size) {
    HANDLE hFile = Win32OpenFile(filePath);
    if (!hFile)
        return NULL;

    GetFileSizeEx(hFile, size);

    void* buffer = Alloc(size->LowPart+extraBytes);
    if (buffer == NULL) {
        CloseHandle(hFile);
        Log("failed to allocate");
        return NULL;
    }

    // NOTE: will only work for files up to INT32_MAX in size
    DWORD bytesRead = Win32ReadFile(hFile, buffer, size->LowPart);
    if (bytesRead == 0) {
        CloseHandle(hFile);
        Log("failed to read file");
        return NULL;
    }

    CloseHandle(hFile);

    return buffer;
}

bool Win32LoadStream(const char* filePath, void* buffer, int32 size) {
    HANDLE hFile = Win32OpenFile(filePath);
    if (!hFile)
        return false;

    DWORD bytesRead = Win32ReadFile(hFile, buffer, size);
    if (bytesRead == 0) {
        CloseHandle(hFile);
        Log("failed to read file");
        return false;
    }

    CloseHandle(hFile);

    return true;
}

void* Win32LoadStream(const char* filePath){
    LARGE_INTEGER size;
    return Win32LoadStream(filePath, 0, &size);
}

char* Win32LoadText(const char* filePath) {
    LARGE_INTEGER size;
    char* text = (char*)Win32LoadStream(filePath, 1, &size);
    text[size.LowPart] = '\0';
    return text;
}

bool Win32SaveStream(const char* filePath, void* data, int32 bytesToWrite){
    HANDLE file = Win32CreateFile(filePath);
    if (!file) return false;

    DWORD bytesWritten;
    BOOL success = WriteFile(file, data, bytesToWrite, &bytesWritten, NULL);
    if (!success) return Fail("failed to write to the file");
    return true;
}