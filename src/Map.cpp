#include <string>
#include <fstream>
#include <iostream>
#include "map.h"

Map::Map()
	: header(), worldInfo(), nodes(), paths()
{
	this->header.magic = MapData::MAGIC;
	this->header.version = MapData::VERSION;
	this->header.mapID = 0;

	this->worldInfo.worldID = 0;
	memcpy(this->worldInfo.name, "New World", 32);
	this->worldInfo.accentColor = 0;
}

Map::Map(const std::string& filePath) {
	std::string buffer;

	std::ifstream file(filePath, std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Failed to open file");

	file.seekg(0, std::ios::end);
	buffer.resize(file.tellg());

	file.seekg(0);
	file.read(buffer.data(), buffer.size());
	file.close();

	u8* data = (u8*)buffer.data();

	MapData dataStruct(data);

	this->header = dataStruct.header;
	this->worldInfo = dataStruct.worldInfo;

	for (u32 i = 0; i < dataStruct.nodeCount; i++) {
		this->nodes.push_back(dataStruct.nodesPtr[i]);
	}

	for (u32 i = 0; i < dataStruct.pathCount; i++) {
		this->paths.push_back(dataStruct.pathsPtr[i]);
	}
}

Map::~Map() {
	for (auto& path : this->paths) {
		delete[] path.unlockCriteriaData;
	}
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
	this->nodesPtr = nullptr;
	this->pathCount = 0;
	this->pathsPtr = nullptr;

	MapData* xdata = reinterpret_cast<MapData*>(data);

	// header

	this->header.magic = SwapEndian(xdata->header.magic);
	this->header.version = SwapEndian(xdata->header.version);
	this->header.mapID = SwapEndian(xdata->header.mapID);

	if (this->header.magic != MapData::MAGIC) throw std::runtime_error("Invalid magic");
	if (this->header.version != MapData::VERSION) throw std::runtime_error("Invalid version");

	// worldinfo

	this->worldInfo.worldID = SwapEndian(xdata->worldInfo.worldID);
	memcpy(this->worldInfo.name, xdata->worldInfo.name, 32);
	this->worldInfo.accentColor = SwapEndian(xdata->worldInfo.accentColor);

	// nodes

	this->nodeCount = SwapEndian(xdata->nodeCount);
	this->nodes = SwapEndian(xdata->nodes);

	const u32* nodeOffsets = (u32*)AddOffset(xdata, this->nodes);
	Node** nodes = new Node*[this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		nodes[i] = (Node*)AddOffset(xdata, SwapEndian(nodeOffsets[i]));
	}

	this->nodesPtr = new Node[this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		this->nodesPtr[i] = *nodes[i];

		Node& node = this->nodesPtr[i];
		node.type = SwapEndian(node.type);

		switch (node.type) {
			case Node::Type::Level: {
				node.level.levelID = SwapEndian(node.level.levelID);
				node.level.unlocksMapID = SwapEndian(node.level.unlocksMapID);
				break;
			}
		}
	}

	// paths

	this->pathCount = SwapEndian(xdata->pathCount);
	this->paths = SwapEndian(xdata->paths);

	const u32* pathOffsets = (u32*)AddOffset(xdata, this->paths);
	Path** paths = new Path*[this->pathCount];
	for (u32 i = 0; i < this->pathCount; i++) {
		paths[i] = (Path*)AddOffset(xdata, SwapEndian(pathOffsets[i]));
	}

	this->pathsPtr = new Path[this->pathCount];
	for (u32 i = 0; i < this->pathCount; i++) {
		this->pathsPtr[i] = *paths[i];

		Path& path = this->pathsPtr[i];
		path.startingNodeIndex = SwapEndian(path.startingNodeIndex);
		path.endingNodeIndex = SwapEndian(path.endingNodeIndex);
		path.animation = SwapEndian(path.animation);

		u32 unlockCriteriaSize = 0;
		u8* unlockCriteria = (u8*)AddOffset(xdata, SwapEndian(path.unlockCriteriaOffs));
		findUnlockCriteriaSize(unlockCriteria, unlockCriteriaSize);
		path.unlockCriteriaData = new u8[unlockCriteriaSize];
		memcpy(path.unlockCriteriaData, unlockCriteria, unlockCriteriaSize);
	}

	delete[] nodes;
	delete[] paths;
}

MapData::~MapData() {
	delete[] this->nodesPtr;
	delete[] this->pathsPtr;
}
