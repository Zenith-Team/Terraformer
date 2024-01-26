#include "terraformer.h"

#include <imgui/imgui.h>

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
        
        ImGui::EndMenu();
    }
	ImGui::EndMainMenuBar();
	
	if (this->map == nullptr) {
		return;
	}
	
	if (ImGui::Begin((std::string{"Terraformer | Editing map: "} + std::string{this->map->worldInfo.name}).c_str()), &windowOpen, ImGuiWindowFlags_UnsavedDocument) {
		this->UINodes();
		this->UIPaths();
	} ImGui::End();
}

void Terraformer::UINodes() {

}

void Terraformer::UIPaths() {

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
