#pragma once

#include <string>
#include <vector>
#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u8 = std::uint8_t;
using f32 = float;

inline void* AddOffset(const void* ptr, intptr_t offset) {
	return reinterpret_cast<void*>(uintptr_t(ptr) + offset);
}

template <typename T>
[[nodiscard]] T SwapEndian(const T value) {
	const T newValue = value;
	u8* data = (u8*)&newValue;
	u8 temp;

	for (u32 i = 0; i < sizeof(T) / 2; i++) {
		temp = data[i];
		data[i] = data[sizeof(T) - i - 1];
		data[sizeof(T) - i - 1] = temp;
	}

	return newValue;
}

// https://github.com/Zenith-Team/smevo/blob/main/include/sme/carterra/map.h#L8
class MapData {
public:
	enum {
		MAGIC = 0x41324C53,
		VERSION = 1
	};

public:
	struct Header {
		u32 magic;
		u32 version;
		u32 mapID;
	};

	struct WorldInfo {
		u32 worldID;
		char name[32];
		u32 accentColor;
	};

	struct Node {
		enum Type {
			Normal,
			Passthrough,
			Level
		};

		Type type;
		char boneName[32];
		union {
			struct {

			} normal, passthrough;
			struct {
				u32 levelID;
				u32 unlocksMapID;
			} level;
		};
	};

	struct Path {
		enum Animation : u32 {
			Walk = 0,
			WalkSand = 1, 
			WalkSnow = 2, 
			WalkWet = 3,
			Jump = 4, 
			JumpSand = 5, 
			JumpSnow = 6,
			JumpWet = 7,
			Climb = 8, 
			ClimbLeft = 9, 
			ClimbRight = 10, 
			Fall = 11,
			Swim = 12, 
			Run = 13, 
			Pipe = 14, 
			Door = 15
		};

		u32 startingNodeIndex;
		u32 endingNodeIndex;
		f32 speed;
		Animation animation;
		u32 unlockCriteriaOffs;

		u8* unlockCriteriaData;
	};

public:
	explicit MapData(u8* data);
	~MapData();

	template <typename T>
	void fixRef(T*& indexAsPtr) {
		u64 index = reinterpret_cast<std::uintptr_t>(indexAsPtr);
		if (index == 0xFFFFFFFF || index == 0) {
			indexAsPtr = 0;
		} else {
			indexAsPtr = (T*)(((u8*)this) + index);
		}
	}
	
	Header header;
	WorldInfo worldInfo;
	u32 nodeCount;
	u32 nodes; // Node**
	u32 pathCount;
	u32 paths; // Path**

	Node* nodesPtr;
	Path* pathsPtr;
};

class Map {
public:
	Map();
	explicit Map(const std::string& filePath);
	~Map();

	MapData::Header header;
	MapData::WorldInfo worldInfo;
	std::vector<MapData::Node> nodes;
	std::vector<MapData::Path> paths;
};
