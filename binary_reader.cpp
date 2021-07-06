inline byte ReadByte(byte** data) {
	return *(*data)++;
}

inline byte PeekByte(byte** data) {
	return **data;
}

inline void Skip(byte** data, ssize distance) {
	*data+=distance;
}

inline uint16 ReadUint16LittleEndian(byte** data) {
    uint16 z = ReadByte(data);
    return ((uint16)(ReadByte(data)) << 8) | z;
}

inline int16 ReadInt16LittleEndian(byte** data) {
    return (int16)ReadUint16LittleEndian(data);
}

inline uint32 ReadUint32LittleEndian(byte** data) {
    uint32 z = ReadUint16LittleEndian(data);
    return (((uint32)ReadUint16LittleEndian(data)) << 16) | z;
}

inline int32 ReadInt32LittleEndian(byte** data) {
    return (int32)ReadUint32LittleEndian(data);
}

inline uint64 ReadUint64LittleEndian(byte** data) {
    uint64 z = ReadUint32LittleEndian(data);
    return ((uint64)(ReadUint32LittleEndian(data)) << 32) | z;
}

inline int64 ReadInt64LittleEndian(byte** data) {
    return (int64)ReadUint64LittleEndian(data);
}

inline uint16 ReadUint16BigEndian(byte** data) {
    uint16 z = ReadByte(data);
    return (z << 8) | (ReadByte(data));
}

inline int16 ReadInt16BigEndian(byte** data) {
	return (int16)ReadUint16BigEndian(data);
}

inline uint32 ReadUint32BigEndian(byte** data) {
    uint32 z = ReadUint16BigEndian(data);
    return (z << 16) | (ReadUint16BigEndian(data));
}

inline int32 ReadInt32BigEndian(byte** data) {
	return (int32)ReadUint32BigEndian(data);
}

inline uint64 ReadUint64BigEndian(byte** data) {
    uint64 z = ReadUint32BigEndian(data);
    return (z << 32) | (ReadUint32BigEndian(data));
}

inline int64 ReadInt64BigEndian(byte** data) {
	return (int64) ReadUint64BigEndian(data);
}

inline float32 ReadFloat32LittleEndian(byte** data) {
    union {uint32 u; float32 f;} temp;
    temp.u = ReadUint32LittleEndian(data);
    return temp.f;
}

inline float64 ReadFloat64LittleEndian(byte** data) {
    union {uint64 u; float64 f;} temp;
    temp.u = ReadUint64LittleEndian(data);
    return temp.f;
}

inline float32 ReadFloat32BigEndian(byte** data) {
    union {uint32 u; float32 f;} temp;
    temp.u = ReadUint32BigEndian(data);
    return temp.f;
}

inline float64 ReadFloat64BigEndian(byte** data) {
    union {uint64 u; float64 f;} temp;
    temp.u = ReadUint64BigEndian(data);
    return temp.f;
}

