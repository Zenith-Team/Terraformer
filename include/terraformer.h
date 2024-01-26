#pragma once

#include <iostream>
#include <string>

#include "map.h"

class Terraformer {
public:
	Map* map = nullptr;
	std::string message = "";
	bool windowOpen = true;


	void Update();

	void UINodes();
	void UIPaths();

	void LoadFile(const std::string& filePath);
    void NewFile();

	void ShowMessage();
};
