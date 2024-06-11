#include <iostream>
#include <fstream>
#include <sstream>

#include "./GraphicsEngine/graphicsEngine.h"
#include "./GraphicsEngine/glm/glm/glm.hpp"
#include "./GraphicsEngine/glm/glm/ext/matrix_clip_space.hpp"
#include "./GraphicsEngine/glm/glm/ext/matrix_transform.hpp"

#define SCREENWIDTH 1920
#define SCREENHEIGHT 1080

#define PI 3.1415

class MainWindow : public Window{
	public:

		GLuint shaderId;
		GLuint VAO;


		struct Camera {
			glm::vec3 pos;
			glm::vec3 focusPoint;
			float zoomSpeed = 1;
			float zoomAmount = 3;
			float minZoom = 1;
			float moveSpeed = 10;
			float rotAngleX = 0;
			float rotAngleY = 0;
		} camera;

		double prevMouseX,prevMouseY;

		MainWindow(int w, int h,std::string name) : Window(w,h,name){
			prevMouseX = 0;
			prevMouseY = 0;
		};
		
		
	
		void OnStartup() override {
			glClearColor(0.0f,0.0f,0.0f,1.0f);
			glEnable(GL_DEPTH_TEST);

			GLfloat vertices[] = {
				-0.5f,-0.5f, -0.5f,1.0f,0.0f,0.0f,
			     0.5f,-0.5f, -0.5f,0.0f,1.0f,0.0f,
				 0.5f, 0.5f, -0.5f,0.0f,0.0f,1.0f,
				 -0.5f,0.5f, -0.5f,1.0f,0.0f,1.0f,
				 -0.5f,-0.5f, 0.5f,0.0f,1.0f,1.0f,
			     0.5f,-0.5f, 0.5f,1.0f,1.0f,0.0f,
				 0.5f, 0.5f, 0.5f,1.0f,1.0f,1.0f,
				 -0.5f,0.5f, 0.5f,0.3f,0.3f,0.3f,


			};
			
			GLuint indices[] = {
				0,1,2,
				2,3,0,
				1,5,6,
				6,2,1,
				5,4,7,
				7,6,5,
				4,0,3,
				3,7,4,
				3,2,6,
				6,7,3,
				4,5,1,
				1,0,4
			};
			
			glBindVertexArray(VAO);

			unsigned int VBO,IBO;
			glGenBuffers(1,&VBO);
			glGenBuffers(1,&IBO);

			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
			
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			glVertexAttribPointer(0,3,GL_FLOAT,false,6*sizeof(GLfloat),(void*)0);
			glVertexAttribPointer(1,3,GL_FLOAT,false,6*sizeof(GLfloat),(void*)(3*sizeof(GLfloat)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			//Shaders
			unsigned int vertex, fragment;
			int success;
			char infoLog[512];
				
			//vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			
			std::ifstream vsFile;
			vsFile.open("./src/Shaders/vertexShader");
			std::stringstream vsStream;
			vsStream << vsFile.rdbuf();
			vsFile.close();
			std::string vsString = vsStream.str();
			const char *vCode = vsString.c_str();
			
			glShaderSource(vertex,1,&vCode,NULL);
			glCompileShader(vertex);
			glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
			if (!success){
				glGetShaderInfoLog(vertex, 512, NULL, infoLog);
				std::cout << "Vertex Shader Compilation Failed: " << infoLog << std::endl;
			}

			//fragment shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			std::ifstream fsFile;
			fsFile.open("./src/Shaders/fragmentShader");
			std::stringstream fsStream;
			fsStream << fsFile.rdbuf();
			fsFile.close();
			std::string fsString = fsStream.str();
			const char *fCode = fsString.c_str();

			glShaderSource(fragment,1,&fCode,NULL);
			glCompileShader(fragment);
			glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
			if (!success){
				glGetShaderInfoLog(fragment, 512, NULL, infoLog);
				std::cout << "Fragment Shader Compilation Failed: " << infoLog << std::endl;
			}

			shaderId = glCreateProgram();
		    glAttachShader(shaderId,vertex);
			glAttachShader(shaderId,fragment);
			glLinkProgram(shaderId);
			
			glGetProgramiv(shaderId, GL_LINK_STATUS, &success);
			if (!success){
				glGetProgramInfoLog(shaderId, 512, NULL, infoLog);
				std::cout << "Shader Linking Error: " << infoLog << std::endl;
			}	

			glDeleteShader(vertex);
			glDeleteShader(fragment);
			

			camera.pos = {0.0f,0.0f,camera.zoomAmount};
			camera.focusPoint = {0.0f,0.0f,0.0f};
		}
		void OnUpdate(double dTime) override {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			glUseProgram(shaderId);

			float aspectRatio = (float)width/(float)height; 
			glm::vec3 camDir = camera.pos - camera.focusPoint;
			float orientation = 1.0f;
			if (camera.rotAngleY > (PI/2.0f) && camera.rotAngleY < (3*PI/2.0f))
			   orientation = -1.0f;	
			glm::vec3 camRight = glm::normalize(glm::cross(glm::vec3(0.0f,orientation,0.0f),camDir)); 
			glm::vec3 camUp = glm::cross(camDir,camRight);

			glm::mat4 proj = glm::perspective(glm::radians(45.0f),aspectRatio,0.1f,100.0f);
			glm::mat4 view = glm::lookAt(camera.pos,camera.focusPoint,camUp);
			
			int projUniform = glGetUniformLocation(shaderId,"proj");
			glUniformMatrix4fv(projUniform,1,false,&proj[0][0]);
			
			int viewUniform = glGetUniformLocation(shaderId,"view");
			glUniformMatrix4fv(viewUniform,1,false,&view[0][0]);
			
			glBindVertexArray(VAO);
	 	 	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);	
			
		}
		void OnShutdown() override {
		}
		void OnEvent(Event &ev) override {
			guiMan.HandleEvent(ev);
			switch (ev.GetType()){
				case Event::MouseScroll:
					{
						
						float zoom = static_cast<MouseScrollEvent*>(&ev)->GetScrollOffset() * camera.zoomSpeed * -1;
						glm::vec3 camDir = glm::normalize(camera.pos - camera.focusPoint);
						camera.pos += camDir*glm::vec3(zoom);
						float distX = camera.pos.x-camera.focusPoint.x; 
						float distY = camera.pos.y-camera.focusPoint.y; 
						float distZ = camera.pos.z-camera.focusPoint.z; 
						float camDist = std::sqrt(distX*distX+distY*distY+distZ*distZ);

						if (camDist < camera.minZoom)
							camera.pos = camDir*glm::vec3(camera.minZoom);
						break;
					}
				case Event::MouseCursor:
						{
							int midMouseBtn = GraphicsEngine::input.GetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
							int rightMouseBtn = GraphicsEngine::input.GetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
							double currMouseX = static_cast<MouseMoveEvent*>(&ev)->GetMouseX();
							double currMouseY = static_cast<MouseMoveEvent*>(&ev)->GetMouseY();
							
							double mouseDeltaX = (prevMouseX-currMouseX)/width*camera.moveSpeed;
							double mouseDeltaY = (prevMouseY-currMouseY)/height*camera.moveSpeed;

							prevMouseX = currMouseX;
							prevMouseY = currMouseY;

							if (midMouseBtn != GLFW_RELEASE){
								glm::vec3 camDir = camera.pos - camera.focusPoint;
								float orientation = 1.0f;
								if (camera.rotAngleY > (PI/2.0f) && camera.rotAngleY < (3*PI/2.0f))
								   orientation = -1.0f;	
								glm::vec3 camRight = glm::normalize(glm::cross(glm::vec3(0.0f,orientation,0.0f),camDir)); 
								glm::vec3 camUp = glm::cross(camDir,camRight);

								camera.pos += camRight*glm::vec3(mouseDeltaX) + camUp*glm::vec3(mouseDeltaY); 
							}
							if (rightMouseBtn != GLFW_RELEASE){
								
								camera.rotAngleX -= mouseDeltaX;
								camera.rotAngleY += mouseDeltaY;
								if (camera.rotAngleX > 2*PI)
									camera.rotAngleX = 0.0f;
								if (camera.rotAngleY > 2*PI)
									camera.rotAngleY = 0.0f;
								if (camera.rotAngleX < 0.0f)
									camera.rotAngleX = 2*PI - camera.rotAngleX;
								if (camera.rotAngleY < 0.0f)
									camera.rotAngleY = 2*PI - camera.rotAngleY;
								float distX = camera.pos.x-camera.focusPoint.x; 
								float distY = camera.pos.y-camera.focusPoint.y; 
								float distZ = camera.pos.z-camera.focusPoint.z; 
								float camDist = std::sqrt(distX*distX+distY*distY+distZ*distZ);
								camera.pos = glm::vec3(std::cos(camera.rotAngleX)*std::cos(camera.rotAngleY),std::sin(camera.rotAngleY),std::cos(camera.rotAngleY)*std::sin(camera.rotAngleX)) * glm::vec3(camDist);
								camera.pos += camera.focusPoint;
							}

							break;
						}
			};

		}
};


int main(){
	
	GraphicsEngine::Init();

	GraphicsEngine::AddWindow(new MainWindow(SCREENWIDTH,SCREENHEIGHT,"3D Renderer"));
	GraphicsEngine::Run();

	return 0;
}
