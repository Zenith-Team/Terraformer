#include "Map.h"
#include <string>
#include <fstream>
#include <iostream>

Map* Map::Load(std::string filePath) {
	std::string buffer;

	std::ifstream file(filePath, std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Failed to open file");

	file.seekg(0, std::ios::end);
	buffer.resize(file.tellg());

	file.seekg(0);
	file.read(buffer.data(), buffer.size());
	file.close();

	u8* data = (u8*)buffer.data();

	return new Map(data);
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
	DataBE::swapEndian(this->header.magic);
	this->header.version = xdata->header.version;
	DataBE::swapEndian(this->header.version);
	this->header.mapID = xdata->header.mapID;
	DataBE::swapEndian(this->header.mapID);

	if (this->header.magic != MapData::MAGIC) throw std::runtime_error("Invalid magic");
	if (this->header.version != MapData::VERSION) throw std::runtime_error("Invalid version");

	this->worldInfo.worldID = xdata->worldInfo.worldID;
	DataBE::swapEndian(this->worldInfo.worldID);
	memcpy(this->worldInfo.name, xdata->worldInfo.name, 32);

	this->worldInfo.accentColor = xdata->worldInfo.accentColor;
	DataBE::swapEndian(this->worldInfo.accentColor);

	this->nodeCount = xdata->nodeCount;
	DataBE::swapEndian(this->nodeCount);
	this->pathCount = xdata->pathCount;
	DataBE::swapEndian(this->pathCount);

	DataBE::swapEndian(xdata->nodes);
	xdata->fixRef(xdata->nodes);
	DataBE::swapEndian(xdata->paths);
	xdata->fixRef(xdata->paths);

	this->nodes = new Node* [this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		DataBE::swapEndian(xdata->nodes[i]);
		xdata->fixRef<MapData::Node>(xdata->nodes[i]);

		this->nodes[i] = new Node();
		this->nodes[i]->type = xdata->nodes[i]->type;
		DataBE::swapEndian(this->nodes[i]->type);
		memcpy(&this->nodes[i]->boneName, &xdata->nodes[i]->boneName, 32);

		if (this->nodes[i]->type == MapData::Node::Type::Level) {
			this->nodes[i]->level.levelID = xdata->nodes[i]->level.levelID;
			DataBE::swapEndian(this->nodes[i]->level.levelID);
			this->nodes[i]->level.unlocksMapID = xdata->nodes[i]->level.unlocksMapID;
			DataBE::swapEndian(this->nodes[i]->level.unlocksMapID);
		}
	}

	this->paths = new Path* [this->pathCount];
	for (u32 i = 0; i < this->pathCount; i++) {
		DataBE::swapEndian(xdata->paths[i]);
		xdata->fixRef(xdata->paths[i]);

		this->paths[i] = new Path();
		xdata->fixRef(xdata->paths[i]->startingNode);
		xdata->fixRef(xdata->paths[i]->endingNode);

		this->paths[i]->startingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->startingNode == xdata->nodes[j]) {
				this->paths[i]->startingNode = this->nodes[j];
				DataBE::swapEndian(this->paths[i]->startingNode);
			}
		}

		this->paths[i]->endingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdata->paths[i]->endingNode == xdata->nodes[j]) {
				this->paths[i]->endingNode = this->nodes[j];
				DataBE::swapEndian(this->paths[i]->endingNode);
			}
		}

		this->paths[i]->speed = xdata->paths[i]->speed;
		DataBE::swapEndian(this->paths[i]->speed);
		this->paths[i]->animation = xdata->paths[i]->animation;
		DataBE::swapEndian(this->paths[i]->animation);

		DataBE::swapEndian(xdata->paths[i]->unlockCriteria);
		xdata->fixRef(xdata->paths[i]->unlockCriteria);

		u32 ucSize = 0;
		u8* unlockCriteria = xdata->paths[i]->unlockCriteria;
		findUnlockCriteriaSize(unlockCriteria, ucSize);

		this->paths[i]->unlockCriteria = new u8[ucSize];
		memcpy(this->paths[i]->unlockCriteria, unlockCriteria, ucSize);
	}
}
