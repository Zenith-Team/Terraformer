#pragma once

#include <string>

using u32 = unsigned int;
using u8 = unsigned char;
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
	MapData(u8* data);

	template <typename T>
	T* fixRef(T*& indexAsPtr) {
		u32 index = (u32)indexAsPtr;
		if (index == 0xFFFFFFFF || index == 0) {
			indexAsPtr = 0;
		}
		else {
			indexAsPtr = (T*)(((u8*)this) + index);
		}

		return indexAsPtr;
	}
	
	Header header;
	WorldInfo worldInfo;
	u32 nodeCount;
	Node** nodes;
	u32 pathCount;
	Path** paths;
};


using std::string;

class Map {
public:
	Map(const Map&) = delete;

	static Map& GetInstance() {
		return instance;
	}

private:
	Map() = default;

	static Map instance;

public:
	MapData map;
	bool mapLoaded;

	static void Load(string filePath);
	static void Unload();
	static void Save();
	static void SaveAs();
};
