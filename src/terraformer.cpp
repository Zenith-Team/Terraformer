#include "terraformer.h"

#include <imgui/imgui.h>
#include <GLFW/glfw3.h>

void Terraformer::Update() {
	if (!this->message.empty()) {
		this->ShowMessage();
	}
	
	ImGui::BeginMainMenuBar();
	if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("New"))
            this->NewFile();
		if (ImGui::MenuItem("Open"))
            this->LoadFile("CS_W1.a2ls");
        if (ImGui::MenuItem("Save"))
            this->map->Save("CS_W1.a2ls");
        if (ImGui::MenuItem("Exit"))
            this->Exit();
        
        ImGui::EndMenu();
    }
	ImGui::EndMainMenuBar();
	
	if (this->map == nullptr) {
		return;
	}

	if (ImGui::Begin((std::string{"Terraformer | Editing map: "} + std::string{this->map->worldInfo.name}).c_str())) {
		this->UINodes();
        ImGui::SameLine();
		this->UIPaths();
	} ImGui::End();
}

void Terraformer::UINodes() {
    ImGui::BeginChild("Nodes", ImVec2(500, 0), true);
    for (u32 i = 0; i < this->map->nodes.size(); i++) {
        MapData::Node& node = this->map->nodes[i];
        
        ImGui::PushID(i);

        ImGui::Text("Node %d", i);
        ImGui::InputText("Bone Name", node.boneName, sizeof(node.boneName));
        ImGui::Combo("Type", (int*)&node.type, "Normal\0Passthrough\0Level\0");

        switch (node.type) {
            case MapData::Node::Type::Normal: {
                break;
            }

            case MapData::Node::Type::Passthrough: {
                break;
            }

            case MapData::Node::Type::Level: {
                ImGui::InputScalar("Level", ImGuiDataType_S32, &node.level.levelID);
                ImGui::InputScalar("Unlocks Map", ImGuiDataType_S32, &node.level.unlocksMapID);
                break;
            }
        }

        ImGui::PopID();

        if (i != this->map->nodes.size() - 1) {
            ImGui::Separator();
        }
    }

    if (ImGui::Button("Add Node")) {
        this->map->AddNode();
    }
    ImGui::EndChild();
}

void Terraformer::UIPaths() {
    ImGui::BeginChild("Paths", ImVec2(500, 0), true);

    static const char* const animationString =
		"Walk" "\0"
		"WalkSand" "\0"
		"WalkSnow" "\0"
		"WalkWet" "\0"
		"Jump" "\0"
		"JumpSand" "\0"
		"JumpSnow" "\0"
		"JumpWet" "\0"
		"Climb" "\0"
		"ClimbLeft" "\0"
		"ClimbRight" "\0"
		"Fall" "\0"
		"Swim" "\0"
		"Run" "\0"
		"Pipe" "\0"
        "Door" "\0"
    ;

    std::string nodeNames = "";
    for (u32 i = 0; i < this->map->nodes.size(); i++) {
        nodeNames += this->map->nodes[i].boneName;
        nodeNames += '\0';
    }

    for (u32 i = 0; i < this->map->paths.size(); i++) {
        MapData::Path& path = this->map->paths[i];
        
        ImGui::PushID(i);
        ImGui::Text("Path %d", i);
        ImGui::Combo("Starting Node", (int*)&path.startingNodeIndex, nodeNames.c_str());
        ImGui::Combo("Ending Node", (int*)&path.endingNodeIndex, nodeNames.c_str());
        ImGui::InputFloat("Speed", &path.speed);
        ImGui::Combo("Animation", (int*)&path.animation, animationString);
        ImGui::PopID();

        if (i != this->map->paths.size() - 1) {
            ImGui::Separator();
        }
    }

    if (ImGui::Button("Add Path")) {
        this->map->AddPath();
    }
    ImGui::EndChild();
}

void Terraformer::LoadFile(const std::string& filePath) {
	std::cout << "Loading file: " << filePath << std::endl;

	if (this->map != nullptr) {
		delete this->map;
	}

	try {
		this->map = new Map(filePath);
	} catch (const std::runtime_error& error) {
		this->message = error.what();
	}
}

void Terraformer::NewFile() {
    if (this->map != nullptr) {
        delete this->map;
    }

    this->map = new Map();    
}

void Terraformer::ShowMessage() {
	ImGui::OpenPopup("Message");

	if (ImGui::BeginPopupModal("Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove)) {
		ImGui::Text(this->message.c_str());

		if (ImGui::Button("OK")) {
			ImGui::CloseCurrentPopup();
			std::cout << this->message.c_str() << std::endl;
			this->message = "";
		}

		ImGui::EndPopup();
	}
}

void Terraformer::Exit() {
    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
}
