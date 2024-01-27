#include <string>
#include <fstream>
#include <iostream>
#include "map.h"

static constexpr std::size_t pathSize = sizeof(MapData::Path) - sizeof(u8*) - 0x4; // padding

static void findUnlockCriteriaSize(u8* unlockCriteria, u32& idx) {
	u8 controlByte = unlockCriteria[idx++];
	u8 conditionType = controlByte >> 6;

	if (conditionType == 0) {
		u8 subConditionType = (controlByte & 0x3F);
		if (subConditionType < 4) {
			idx += 2;
		}
	} else if (conditionType == 1) {
		++idx;
	} else if (conditionType == 2 || conditionType == 3) {
		u8 termCount = (controlByte & 0x3F) + 1;
		for (int i = 0; i < termCount; i++) {
			findUnlockCriteriaSize(unlockCriteria, idx);
		}
	}
}

Map::Map()
	: header(), worldInfo(), nodes(), paths()
{
	this->header.magic = MapData::MAGIC;
	this->header.version = MapData::VERSION;
	this->header.mapID = 0;

	static const std::string newWorldName = "New World";

	this->worldInfo.worldID = 0;
	std::memset(this->worldInfo.name, 0, 32);
	std::memcpy(this->worldInfo.name, newWorldName.c_str(), newWorldName.length());
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

	MapData::Header* header = reinterpret_cast<MapData::Header*>(data);
	MapData::WorldInfo* worldInfo = reinterpret_cast<MapData::WorldInfo*>(data + sizeof(MapData::Header));
	
	this->header.magic = SwapEndian(header->magic);
	this->header.version = SwapEndian(header->version);
	this->header.mapID = SwapEndian(header->mapID);

	this->worldInfo.worldID = SwapEndian(worldInfo->worldID);
	std::memcpy(this->worldInfo.name, worldInfo->name, 32);
	this->worldInfo.accentColor = SwapEndian(worldInfo->accentColor);

	u32 nodeCount = SwapEndian(*reinterpret_cast<u32*>(data + sizeof(MapData::Header) + sizeof(MapData::WorldInfo)));
	this->nodes.resize(nodeCount);
	for (u32 i = 0; i < nodeCount; i++) {
		MapData::Node* node = reinterpret_cast<MapData::Node*>(data + sizeof(MapData::Header) + sizeof(MapData::WorldInfo) + sizeof(u32) + (sizeof(MapData::Node) * i));

		this->nodes[i].type = SwapEndian(node->type);
		std::memcpy(this->nodes[i].boneName, node->boneName, 32);

		switch (this->nodes[i].type) {
			case MapData::Node::Type::Normal: {
				break;
			}

			case MapData::Node::Type::Passthrough: {
				break;
			}

			case MapData::Node::Type::Level: {
				this->nodes[i].level.levelID = SwapEndian(node->level.levelID);
				this->nodes[i].level.unlocksMapID = SwapEndian(node->level.unlocksMapID);

				break;
			}
		}
	}

	u32 pathCount = SwapEndian(*reinterpret_cast<u32*>(data + sizeof(MapData::Header) + sizeof(MapData::WorldInfo) + sizeof(u32) + (sizeof(MapData::Node) * this->nodes.size())));
	this->paths.resize(pathCount);
	for (u32 i = 0; i < pathCount; i++) {
		MapData::Path* path = reinterpret_cast<MapData::Path*>(data + sizeof(MapData::Header) + sizeof(MapData::WorldInfo) + sizeof(u32) + (sizeof(MapData::Node) * this->nodes.size()) + sizeof(u32) + (pathSize * i));

		this->paths[i].startingNodeIndex = SwapEndian(path->startingNodeIndex);
		this->paths[i].endingNodeIndex = SwapEndian(path->endingNodeIndex);
		this->paths[i].speed = SwapEndian(path->speed);
		this->paths[i].animation = SwapEndian(path->animation);

		u32 unlockCriteriaOffset = SwapEndian(path->unlockCriteriaOffs);
		u8* unlockCriteria = data + unlockCriteriaOffset;

		u32 ucSize = 0;
		findUnlockCriteriaSize(unlockCriteria, ucSize);

		this->paths[i].unlockCriteriaData = new u8[ucSize];
		std::memcpy(this->paths[i].unlockCriteriaData, unlockCriteria, ucSize);
	}
}

Map::~Map() {
	for (auto& path : this->paths) {
		delete[] path.unlockCriteriaData;
	}
}

void Map::AddNode() {
	this->nodes.push_back(MapData::Node());
}

void Map::AddPath() {
	MapData::Path path = { 0 };
	path.unlockCriteriaData = new u8[1];
	path.unlockCriteriaData[0] = 0x0F;

	this->paths.push_back(path);
}

void Map::RemoveNode(u32 index) {
	this->nodes.erase(this->nodes.begin() + index);
}

void Map::RemovePath(u32 index) {
	delete[] this->paths[index].unlockCriteriaData;
	this->paths.erase(this->paths.begin() + index);
}

void Map::Save(const std::string& filePath) {
	std::ofstream file(filePath, std::ios::binary);

	if (!file.is_open()) throw std::runtime_error("Failed to open file");

	MapData::Header headerBE;
	headerBE.magic = SwapEndian(this->header.magic);
	headerBE.version = SwapEndian(this->header.version);
	headerBE.mapID = SwapEndian(this->header.mapID);

	MapData::WorldInfo worldInfoBE;
	worldInfoBE.worldID = SwapEndian(this->worldInfo.worldID);
	std::memcpy(worldInfoBE.name, this->worldInfo.name, 32);
	worldInfoBE.accentColor = SwapEndian(this->worldInfo.accentColor);

	file.write((char*)&headerBE, sizeof(MapData::Header));
	file.write((char*)&worldInfoBE, sizeof(MapData::WorldInfo));
	
	u32 nodeCount = SwapEndian(static_cast<u32>(this->nodes.size()));
	file.write((char*)&nodeCount, sizeof(u32));

	for (auto& node : this->nodes) {
		MapData::Node nodeBE;
		nodeBE.type = SwapEndian(node.type);
		std::memcpy(nodeBE.boneName, node.boneName, 32);
		
		switch (node.type) {
			case MapData::Node::Type::Normal: {
				break;
			}

			case MapData::Node::Type::Passthrough: {
				break;
			}

			case MapData::Node::Type::Level: {
				nodeBE.level.levelID = SwapEndian(node.level.levelID);
				nodeBE.level.unlocksMapID = SwapEndian(node.level.unlocksMapID);

				break;
			}
		}

		file.write((char*)&nodeBE, sizeof(MapData::Node));
	}

	u32 pathCount = SwapEndian(static_cast<u32>(this->paths.size()));
	file.write((char*)&pathCount, sizeof(u32));

	u32 unlockDataOffset = sizeof(MapData::Header) + sizeof(MapData::WorldInfo) + sizeof(u32) + (sizeof(MapData::Node) * this->nodes.size()) + sizeof(u32) + (pathSize * this->paths.size());
	for (auto& path : this->paths) {
		MapData::Path pathBE;
		pathBE.startingNodeIndex = SwapEndian(path.startingNodeIndex);
		pathBE.endingNodeIndex = SwapEndian(path.endingNodeIndex);
		pathBE.speed = SwapEndian(path.speed);
		pathBE.animation = SwapEndian(path.animation);
		pathBE.unlockCriteriaOffs = SwapEndian(unlockDataOffset);

		file.write((char*)&pathBE, pathSize);

		u32 ucSize = 0;
		findUnlockCriteriaSize(path.unlockCriteriaData, ucSize);
		unlockDataOffset += ucSize;
	}

	for (auto& path : this->paths) {
		u32 ucSize = 0;
		findUnlockCriteriaSize(path.unlockCriteriaData, ucSize);
		file.write((char*)path.unlockCriteriaData, ucSize);
	}

	file.close();
}
