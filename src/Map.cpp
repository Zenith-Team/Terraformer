#include "Map.h"
#include <assert.h>
#include <string>
#include <fstream>

Map Map::instance;

void Map::Load(string filePath) {
	if (!Map::GetInstance().mapLoaded) assert(false);

	string buffer;

	std::ifstream file(filePath, std::ios::binary);

	if (!file.is_open()) assert(false);
	
	file.seekg(0, std::ios::end);
	buffer.resize(file.tellg());
	
	file.seekg(0);
	file.read(buffer.data(), buffer.size());
	file.close();

	Map::GetInstance().map = MapData((u8*)buffer.data());
	Map::GetInstance().mapLoaded = true;

}

void Map::Unload() {
	
}

void Map::Save() {
	
}

void Map::SaveAs() {

}

static void findUnlockCriteriaSize(u8* unlockCriteria, u32& idx) {
	u8 controlByte = unlockCriteria[idx++];
	u8 conditionType = controlByte >> 6;

	if (conditionType == 0) {
		u8 subConditionType = (controlByte & 0x3F);
		if (subConditionType < 4) {
			idx += 2;
		}
	}
	else if (conditionType == 1) {
		++idx;
	}
	else if (conditionType == 2 || conditionType == 3) {
		u8 termCount = (controlByte & 0x3F) + 1;
		for (int i = 0; i < termCount; i++) {
			findUnlockCriteriaSize(unlockCriteria, idx);
		}
	}
}

MapData::MapData(u8* data) {
	this->nodeCount = 0;
	this->nodes = nullptr;
	this->pathCount = 0;
	this->paths = nullptr;

	MapData* xdata = reinterpret_cast<MapData*>(data);

	this->header.magic = xdata->header.magic;
	this->header.version = xdata->header.version;
	this->header.mapID = xdata->header.mapID;

	if (this->header.magic != MapData::MAGIC) assert(false && "Invalid MAGIC");
	if (this->header.version != MapData::VERSION) assert(false && "Invalid VERSION");

	this->worldInfo.worldID = xdata->worldInfo.worldID;
	memcpy(this->worldInfo.name, xdata->worldInfo.name, 32);
	this->worldInfo.accentColor = xdata->worldInfo.accentColor;

	this->nodeCount = xdata->nodeCount;
	this->pathCount = xdata->pathCount;

	xdata->fixRef(this->nodes);
	xdata->fixRef(this->paths);

	this->nodes = new Node*[this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		xdata->fixRef(this->nodes[i]);

		this->nodes[i] = new Node();
		this->nodes[i]->type = xdata->nodes[i]->type;
		memcpy(this->nodes[i]->boneName, xdata->nodes[i]->boneName, 32);

		if (this->nodes[i]->type == MapData::Node::Type::Level) {
			this->nodes[i]->level.levelID = xdata->nodes[i]->level.levelID;
			this->nodes[i]->level.unlocksMapID = xdata->nodes[i]->level.unlocksMapID;
		}
	}

	this->paths = new Path*[this->pathCount];

	for (u32 i = 0; i < this->pathCount; i++) {
		xdata->fixRef(this->paths[i]);

		this->paths[i] = new Path();
		xdata->fixRef(this->paths[i]->startingNode);
		xdata->fixRef(this->paths[i]->endingNode);

		this->paths[i]->startingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->startingNode == xdata->nodes[j]) {
				this->paths[i]->startingNode = this->nodes[j];
			}
		}

		this->paths[i]->endingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->endingNode == xdata->nodes[j]) {
				this->paths[i]->endingNode = this->nodes[j];
			}
		}

		this->paths[i]->speed = xdata->paths[i]->speed;
		this->paths[i]->animation = xdata->paths[i]->animation;

		xdata ->fixRef(this->paths[i]->unlockCriteria);

		u32 ucSize = 0;
		u8* unlockCriteria = xdata->paths[i]->unlockCriteria;
		findUnlockCriteriaSize(unlockCriteria, ucSize);

		this->paths[i]->unlockCriteria = new u8[ucSize];
		memcpy(this->paths[i]->unlockCriteria, unlockCriteria, ucSize);

	}
}