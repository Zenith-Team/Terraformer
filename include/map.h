#pragma once

#include <string>
#include <vector>
#include <cstdint>

using u64 = std::uint64_t;
using u32 = std::uint32_t;
using u8 = std::uint8_t;
using f32 = float;

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
		enum Animation {
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

		Node* startingNode;
		Node* endingNode;
		f32 speed;
		Animation animation;
		u8* unlockCriteria;
	};

public:
	explicit MapData(u8* data);

	template <typename T>
	void fixRef(T*& indexAsPtr) {
		u64 index = reinterpret_cast<std::uintptr_t>(indexAsPtr);
		if (index == 0xFFFFFFFF || index == 0) {
			indexAsPtr = 0;
		} else {
			indexAsPtr = (T*)(((u8*)this) + index);
		}
	}

	template <typename T>
	T fixRefPtrConv(u32 index) {
		u64 ptr = 0;
		ptr += index;
		if (index == 0xFFFFFFFF || index == 0) {
			ptr = 0;
		} else {
			//ptr = reinterpret_cast<T>(reinterpret_cast<std::uintptr_t>(this) + index);
			ptr += reinterpret_cast<std::uintptr_t>(this);
		}

		return reinterpret_cast<T>(ptr);
	}
	
	Header header;
	WorldInfo worldInfo;
	u32 nodeCount;
	u32 nodes;
	u32 pathCount;
	u32 paths;

	Node** nodesPtr;
	Path** pathsPtr;
};

class Map {
public:
	Map(const std::string& filePath);

	MapData::Header header;
	MapData::WorldInfo worldInfo;
	std::vector<MapData::Node> nodes;
	std::vector<MapData::Path> paths;
};


class DataBE {
public:
	template <typename T>
	[[nodiscard]] static T swapEndian(const T value) {
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
};
