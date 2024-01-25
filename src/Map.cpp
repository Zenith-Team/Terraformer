#include <string>
#include <fstream>
#include <iostream>
#include "map.h"

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

	MapData dataStruct{data};

	this->header = dataStruct.header;
	this->worldInfo = dataStruct.worldInfo;

	for (u32 i = 0; i < dataStruct.nodeCount; i++) {
		this->nodes.push_back(*dataStruct.nodesPtr[i]);
	}

	for (u32 i = 0; i < dataStruct.pathCount; i++) {
		this->paths.push_back(*dataStruct.pathsPtr[i]);
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

	this->header.magic = DataBE::swapEndian(xdata->header.magic);
	this->header.version = DataBE::swapEndian(xdata->header.version);
	this->header.mapID = DataBE::swapEndian(xdata->header.mapID);

	if (this->header.magic != MapData::MAGIC) throw std::runtime_error("Invalid magic");
	if (this->header.version != MapData::VERSION) throw std::runtime_error("Invalid version");

	this->worldInfo.worldID = DataBE::swapEndian(xdata->worldInfo.worldID);
	memcpy(this->worldInfo.name, xdata->worldInfo.name, 32);
	this->worldInfo.accentColor = DataBE::swapEndian(xdata->worldInfo.accentColor);

	this->nodeCount = DataBE::swapEndian(xdata->nodeCount);
	this->pathCount = DataBE::swapEndian(xdata->pathCount);

	Node** xdataNodesPtr = xdata->fixRefPtrConv<Node**>(DataBE::swapEndian(xdata->nodes));
	Path** xdataPathsPtr = xdata->fixRefPtrConv<Path**>(DataBE::swapEndian(xdata->paths));

	this->nodesPtr = new Node* [this->nodeCount];
	for (u32 i = 0; i < this->nodeCount; i++) {
		DataBE::swapEndian(xdataNodesPtr[i]);
		xdata->fixRef<MapData::Node>(xdataNodesPtr[i]);

		this->nodesPtr[i] = new Node();

		Node* n = this->nodesPtr[i];
		Node* o = xdataNodesPtr[i];
		n->type = DataBE::swapEndian(o->type);

		memcpy(&this->nodesPtr[i]->boneName, &xdataNodesPtr[i]->boneName, 32);

		if (this->nodesPtr[i]->type == MapData::Node::Type::Level) {
			this->nodesPtr[i]->level.levelID = DataBE::swapEndian(xdataNodesPtr[i]->level.levelID);
			this->nodesPtr[i]->level.unlocksMapID = DataBE::swapEndian(xdataNodesPtr[i]->level.unlocksMapID);
		}

		std::cout << "Node " << i << ":" << std::endl;
		std::cout << "    Type: " << this->nodesPtr[i]->type << std::endl;
		std::cout << "    BoneName: " << this->nodesPtr[i]->boneName << std::endl << std::endl;
	}

	this->pathsPtr = new Path* [this->pathCount];
	for (u32 i = 0; i < this->pathCount; i++) {
		auto a = DataBE::swapEndian(xdataPathsPtr[i]);
		xdata->fixRef(a);
		xdataPathsPtr[i] = a;

		this->pathsPtr[i] = new Path();
		xdata->fixRef(xdataPathsPtr[i]->startingNode);
		xdata->fixRef(xdataPathsPtr[i]->endingNode);

		this->pathsPtr[i]->startingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdataPathsPtr[i]->startingNode == xdataNodesPtr[j]) {
				this->pathsPtr[i]->startingNode = DataBE::swapEndian(this->nodesPtr[j]);
			}
		}

		this->pathsPtr[i]->endingNode = nullptr;
		for (u32 j = 0; j < this->nodeCount; j++) {
			if (xdataPathsPtr[i]->endingNode == xdataNodesPtr[j]) {
				this->pathsPtr[i]->endingNode = DataBE::swapEndian(this->nodesPtr[j]);
			}
		}

		this->pathsPtr[i]->speed = DataBE::swapEndian(xdataPathsPtr[i]->speed);
		this->pathsPtr[i]->animation = DataBE::swapEndian(xdataPathsPtr[i]->animation);

		auto b = DataBE::swapEndian(xdataPathsPtr[i]->unlockCriteria);
		xdata->fixRef(b);
		xdataPathsPtr[i]->unlockCriteria = b;

		u32 ucSize = 0;
		u8* unlockCriteria = xdataPathsPtr[i]->unlockCriteria;
		findUnlockCriteriaSize(unlockCriteria, ucSize);

		this->pathsPtr[i]->unlockCriteria = new u8[ucSize];
		memcpy(this->pathsPtr[i]->unlockCriteria, unlockCriteria, ucSize);
	}
}
