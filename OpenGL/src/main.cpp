#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include "GraphicsEngine/graphicsEngine.h"
#include "GraphicsEngine/glm/glm/glm.hpp"
#include "GraphicsEngine/glm/glm/ext/matrix_transform.hpp"
#include "GraphicsEngine/glm/glm/ext/matrix_clip_space.hpp"



class MainWindow : public Window {
	public:
		GLuint shaderId;
		GLuint VAO;

		struct Vertex{
			GLfloat pos[3] = {0,0,0};
			GLfloat normal[3] = {0,0,0};
		};

		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		
		float scaleVal = 1.0f;
		float rotAngle = 0.0f;
		glm::vec3 camPos = {0.0f,0.0f,-10.0f};


		double prevMouseX, prevMouseY;

		GLfloat color[3] = {1.0f,0.0f,0.0f};

		MainWindow(int w, int h, std::string name):Window(w,h,name){
		}

		void OnStartup() override {
			glClearColor(0.01f,0.0f,0.05f,1.0f);
			glEnable(GL_DEPTH_TEST);

			std::string modelName = "";

			std::ifstream modelFile;
			modelFile.open("./Models/teapot.obj");
			while (!modelFile.eof()){
				std::string type;
				modelFile >> type;
				if (!type.compare("o"))
					modelFile >> modelName;
				else if (!type.compare("v")){
					GLfloat vx,vy,vz;
					modelFile >> vx >> vy >> vz;
					Vertex v;
					v.pos[0] = vx;
					v.pos[1] = vy;
					v.pos[2] = vz;
					vertices.push_back(v);
				}
				else if (!type.compare("f")){
					GLuint f1,f2,f3;
					modelFile >> f1 >> f2 >> f3;
					indices.push_back(f1-1);	
					indices.push_back(f2-1);	
					indices.push_back(f3-1);
				}
				else
					modelFile.ignore(1024,'\n');
			}
			modelFile.close();

			for (int i = 0;i<indices.size();i+=3){
				Vertex *v1 = &vertices[indices[i]];
				Vertex *v2 = &vertices[indices[i+1]];
				Vertex *v3 = &vertices[indices[i+2]];

				glm::vec3 vector1 = glm::vec3(v2->pos[0],v2->pos[1],v2->pos[2]) - glm::vec3(v1->pos[0],v1->pos[1],v1->pos[2]);	
				glm::vec3 vector2 = glm::vec3(v3->pos[0],v3->pos[1],v3->pos[2]) - glm::vec3(v1->pos[0],v1->pos[1],v1->pos[2]);	
				glm::vec3 crossProd = glm::cross(vector1,vector2)*glm::vec3(-1.0f);


				v1->normal[0] += crossProd[0];
				v1->normal[1] += crossProd[1];
				v1->normal[2] += crossProd[2];

				v2->normal[0] += crossProd[0];
				v2->normal[1] += crossProd[1];
				v2->normal[2] += crossProd[2];

				v3->normal[0] += crossProd[0];
				v3->normal[1] += crossProd[1];
				v3->normal[2] += crossProd[2];


			}

			for (int i = 0;i<vertices.size();i++){
				float mag = std::sqrt(vertices[i].normal[0]*vertices[i].normal[0]+vertices[i].normal[1]*vertices[i].normal[1]+vertices[i].normal[2]*vertices[i].normal[2]);
				if (mag == 0)
					continue;
				vertices[i].normal[0] /= mag;
				vertices[i].normal[1] /= mag;
				vertices[i].normal[2] /= mag;
			}
			

			glBindVertexArray(VAO);

			unsigned int VBO,IBO;
			glGenBuffers(1,&VBO);
			glGenBuffers(1,&IBO);
			
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex), &vertices[0],GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size()*sizeof(GLuint), &indices[0],GL_STATIC_DRAW);

			glVertexAttribPointer(0,3,GL_FLOAT,false,6*sizeof(GLfloat),(void*)0);
			glVertexAttribPointer(1,3,GL_FLOAT,false,6*sizeof(GLfloat),(void*)(3*sizeof(GLfloat)));
			glEnableVertexAttribArray(0);
			glEnableVertexAttribArray(1);

			unsigned int vertex, fragment;
			int success;
			char infoLog[512];

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
			glGetShaderiv(fragment,GL_COMPILE_STATUS, &success);
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

			
			
			
		}
		
		void OnUpdate(double dTime) override {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(shaderId);

			constexpr float aspectRatio = 1920.0/1080.0;
			glm::mat4 model = glm::rotate(glm::mat4(1.0f),rotAngle,glm::vec3(0.0f,1.0f,0.0f));
			model = glm::scale(model,glm::vec3(scaleVal));
			glm::mat4 proj = glm::perspective(glm::radians(45.0f),aspectRatio,0.1f,100.0f);
			glm::mat4 view = glm::translate(glm::mat4(1.0f),camPos);
			
			int modelUniform = glGetUniformLocation(shaderId,"model");
			int viewUniform = glGetUniformLocation(shaderId,"view");
			int projUniform = glGetUniformLocation(shaderId,"proj");
			glUniformMatrix4fv(modelUniform,1,false,&model[0][0]);
			glUniformMatrix4fv(viewUniform,1,false,&view[0][0]);
			glUniformMatrix4fv(projUniform,1,false,&proj[0][0]);

			glBindVertexArray(VAO);
			glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

		}
		
		void OnShutdown() override {
		}

		void OnEvent(Event &ev) override {
			guiMan.HandleEvent(ev);
			switch (ev.GetType()){
				case Event::MouseScroll:
					{
						float scrollAmt = static_cast<MouseScrollEvent*>(&ev)->GetScrollOffset() * 0.05;
						scaleVal += scrollAmt;
						break;
					}
				case Event::MouseCursor:
					{
						int midMouseBtn = GraphicsEngine::input.GetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
						double currMouseX = static_cast<MouseMoveEvent*>(&ev)->GetMouseX();
						double currMouseY = static_cast<MouseMoveEvent*>(&ev)->GetMouseY();

						double mouseDeltaX = (prevMouseX-currMouseX);
						double mouseDeltaY = (prevMouseY-currMouseY);

						prevMouseX = currMouseX;	
						prevMouseY = currMouseY;	

						if (midMouseBtn != GLFW_RELEASE){
							rotAngle += mouseDeltaX;
						}	

						break;
					}
			}
		}
		
};

int main() {
	GraphicsEngine::Init();

	GraphicsEngine::AddWindow(new MainWindow(1920,1080,"3d Renderer"));
	GraphicsEngine::Run();

	return 0;
}
