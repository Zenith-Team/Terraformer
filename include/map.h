#pragma once

#include <string>
#include <vector>
#include <cstdint>

using u8 = std::uint8_t;
using u32 = std::uint32_t;
using s32 = std::int32_t;
using u64 = std::uint64_t;
using f32 = float;

inline void* AddOffset(const void* ptr, intptr_t offset) {
	return reinterpret_cast<void*>(uintptr_t(ptr) + offset);
}

template <typename T>
[[nodiscard]] T SwapEndian(const T value) {
	T newValue = value;
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
				s32 unlocksMapID;
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

		// extra
		u8* unlockCriteriaData;
	};

private:
	MapData() = delete;
};

class Map {
public:
	Map();
	explicit Map(const std::string& filePath);
	~Map();

	void AddNode();
	void AddPath();

	void RemoveNode(u32 index);
	void RemovePath(u32 index);

	void Save(const std::string& filePath);

	MapData::Header header;
	MapData::WorldInfo worldInfo;
	std::vector<MapData::Node> nodes;
	std::vector<MapData::Path> paths;
};
