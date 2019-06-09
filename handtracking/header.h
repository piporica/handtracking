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