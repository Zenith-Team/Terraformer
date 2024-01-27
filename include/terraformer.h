#pragma once

#include <iostream>
#include <string>

#include "map.h"

class Terraformer {
public:
	Map* map = nullptr;
	std::string message = "";

	int selectedNode = -1;
	int selectedPath = -1;

	void Update();

	void UINodes();
	void UIPaths();
	void UIProperties();

	void LoadFile(const std::string& filePath);
    void NewFile();

	void ShowMessage();
    void Exit();
};
