#pragma once

#define _USE_MATH_DEFINES
#include ".\include\GL\freeglut.h"
#include "opencv2/opencv.hpp"  
#include <iostream>  
#include <string> 
#include <cmath>

//����� ���̺귯���� �������ݴϴ�.
#pragma comment(lib, "freeglut.lib")
#pragma comment(lib, "glew32.lib")

using namespace cv;
using namespace std;

 
Point cubecenter(1.0, 0); //opencv�� ���� ť�� ����

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
	
	double Llength; //��Ÿ� ª���Ÿ�
	double Slength;

	double rotateXY; //���󿡼� �󸶳� ���ư�����
	double rotateZ; //���ư� ���������� ȸ��
};

bool isFish;

//���� Ƣ�°� ����
double prevangle;
int anglecount;

//obj �б� ����

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

//�ٲٱ� Ÿ�̸� - 2�ʰ� ������ ���� �ٲ� 
int changtimer;
bool timechecking; //��� Ÿ�̸� ���ư��� �� ������
bool stillnothing; //2�� �Ŀ��� ����