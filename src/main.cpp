#include <iostream>
#include "GraphicsEngine/graphicsEngine.h"

class MainWindow : public Window{
	MainWindow(int w, int h,std::string name) : Window(w,h,name){
	}
	
	void OnStartup() override {
	}
	void OnUpdate(double dTime) override {
	}
	void OnShutdown() override {
	}
	void OnEvent(Event &ev) override {
	}
};


int main(){
	
	GraphicsEngine::Init();

	GraphicsEngine::AddWindow(new MainWindow(720,480,"3D Renderer"));
	GraphicsEngine::Run();

	return 0;
}
