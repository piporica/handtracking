#pragma once

#define _USE_MATH_DEFINES
#include ".\include\GL\freeglut.h"
#include "opencv2/opencv.hpp"  
#include <iostream>  
#include <string> 
#include <cmath>

//사용할 라이브러리를 지정해줍니다.
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "glew32.lib")

using namespace cv;
using namespace std;

 
Point cubecenter(1.0, 0); //opencv로 구한 큐브 중점

VideoCapture* capture;
Mat img_cam;
int screenW;
int screenH;

GLuint texture_background, texture_cube;
float cubeAngle = 0;

struct handPosition
{
	Point center;
	Point vertices[4];
	
	double Llength; //긴거리 짧은거리
	double Slength;

	double rotateXY; //평면상에서 얼마나 돌아갔는지
	double rotateZ; //돌아간 방향으로의 회전
};

bool isFish;

//각도 튀는거 방지
double prevangle;
int anglecount;

//obj 읽기 관련

struct Vertex {
	float X;
	float Y;
	float Z;
	int index_1;
	int index_2;
	int index_3;
};

struct MMesh {
	int V1;
	int V2;
	int V3;
	int V4;
	int T1;
	int T2;
	int T3;
	int T4;
};

//fish
Vertex *Fvertex = new Vertex[100000];
Vertex *Fvertex_color = new Vertex[100000];;
MMesh *Fmymesh = new MMesh[100000];

Vertex *Dvertex = new Vertex[100000];
Vertex *Dvertex_color = new Vertex[100000];
MMesh *Dmymesh = new MMesh[100000];

GLubyte Fmytexels[1024][1024][3];
GLubyte Dmytexels[1024][1024][3];

//바꾸기 타이머 - 2초간 없으면 도형 바꿈 
int changtimer;
bool timechecking; //계속 타이머 돌아가는 것 방지용
bool stillnothing; //2초 후에도 없니