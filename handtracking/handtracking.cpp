#include "getHandPosition.h"



//OpenCV Mat을 OpenGL Texture로 변환 
GLuint MatToTexture(Mat image)
{
	if (image.empty())  return -1;

	//OpenGL 텍스처 생성
	GLuint textureID;
	glGenTextures(1, &textureID);

	//텍스처 ID를 바인딩 -  사용할 텍스처 차원을 지정해준다.
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows,
		0, GL_RGB, GL_UNSIGNED_BYTE, image.ptr());

	return textureID;
}


void draw_background()
{
	int x = screenW / 80.0;
	int y = screenH / 80.0;

	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0); glVertex3f(-x, -y, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex3f(x, -y, 0.0);
	glTexCoord2f(1.0, 0.0); glVertex3f(x, y, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex3f(-x, y, 0.0);
	glEnd();
}


//큐브의 한 면, 화면 안쪽 방향인 -Z축방향으로 0.5이동하여 정사각형을 그린다.
static void cubebase(double size)
{

	glBegin(GL_QUADS);
	glVertex3d(-size/2, -size / 2, -size / 2);
	glVertex3d(-size / 2, size / 2, -size / 2);
	glVertex3d(size / 2, size / 2, -size / 2);
	glVertex3d(size / 2, -size / 2, -size / 2);
	glEnd();
}

//cubebase함수에서 그린 사각형을 회전 및 이동시켜
//큐브를 완성시킨다.
void draw_cube(double size)
{
	glPushMatrix();

	glColor3f(0.0f, 1.0f, 0.0f);     // Green, -Z축 방향
	cubebase(size);

	glPushMatrix();

	glTranslated(size, 0.0, 0.0);
	glRotated(90.0, 0.0, 1.0, 0.0);
	glColor3f(0.0f, 0.0f, 1.0f);     // Blue, +X축 방향
	cubebase(size);

	glPopMatrix();

	glPushMatrix();
	glTranslated(-size, 0.0, 0.0);
	glRotated(-90.0, 0.0, 1.0, 0.0);
	glColor3f(1.0f, 0.5f, 0.0f);     // Orange, -X축 방향
	cubebase(size);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, size, 0.0);
	glRotated(-90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 0.0f, 0.0f);     // Red, +Y축 방향
	cubebase(size);
	glPopMatrix();

	glPushMatrix();
	glTranslated(0.0, -size, 0.0);
	glRotated(90.0, 1.0, 0.0, 0.0);
	glColor3f(1.0f, 1.0f, 0.0f);     // Yellow, -Y축 방향
	cubebase(size);
	glPopMatrix();

	glBegin(GL_QUADS);
	glColor3f(1.0f, 0.0f, 1.0f);     // Magenta, +Z축 방향
	glVertex3d(-size/2, -size / 2, size / 2);
	glVertex3d(size / 2, -size / 2, size / 2);
	glVertex3d(size / 2, size / 2, size / 2);
	glVertex3d(-size / 2, size / 2, size / 2);
	glEnd();

	glPopMatrix();

	glFlush();
}


// 카메라 초기화
void cameraInit()
{

	capture = new VideoCapture(0);

	if (!capture) {
		printf("Could not capture a camera\n\7");
		return;
	}

	Mat img_frame;

	capture->read(img_frame);

	screenW = img_frame.cols;
	screenH = img_frame.rows;

	cout << screenW << " " << screenH << endl;
}

bool ReadingOBJ(char* bmpfilename, char* objfilename)
{
	vertex = new Vertex[100000];
	vertex_color = new Vertex[100000];
	mymesh = new MMesh[100000];

	//비트맵 정보 읽기

	int i, j, k = 0;
	FILE* f;
	fopen_s(&f, bmpfilename, "rb");
	unsigned char info[54];
	fread(info, sizeof(unsigned char), 54, f); // read the 54-byte header
											   // extract image height and width from header
	int width = *(int*)& info[18];
	int height = *(int*)& info[22];

	int size = 3 * width * height;
	unsigned char* data = new unsigned char[size]; // allocate 3 bytes per pixel
	fread(data, sizeof(unsigned char), size, f); // read the rest of the data at once
	fclose(f);

	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			mytexels[i][j][0] = data[k * 3 + 2];
			mytexels[i][j][1] = data[k * 3 + 1];
			mytexels[i][j][2] = data[k * 3];
			k++;
		}
	}
	
	//obj 정보 읽기
	FILE* file;
	fopen_s(&file,objfilename, "r");
	if (file == NULL) {
		printf("Impossible to open the file !\n");
		return false;
	}

	// 파일 읽기 시작
	float x, y, z;
	int x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4;

	int vcount = 0;
	int vtcount = 0;
	int fcount = 0;
	while (1) {
		char lineHeader[128];
		// read the first word of the line
		int res = fscanf_s(file, "%s", lineHeader, 128);
		if (res == EOF)
			break; // EOF = End Of File. Quit the loop.
				   // else : parse lineHeader
		else
		{
			//버텍스
			if (strcmp(lineHeader, "v") == 0)
			{
				fscanf_s(file, "%f %f %f\n", &x, &y, &z);
				vertex[vcount].X = x;
				vertex[vcount].Y = z;
				vertex[vcount].Z = y;

				vcount++;
			}

			//버텍스 텍스쳐
			else if (strcmp(lineHeader, "vt") == 0)
			{
				fscanf_s(file, "%f %f %f\n", &x, &y, &z);
				vertex_color[vtcount].X = x;
				vertex_color[vtcount].Y = y;
				vertex_color[vtcount].Z = z;

				vtcount++;
			}

			//face
			else if (strcmp(lineHeader, "f") == 0)
			{
				fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n", &x1, &y1, &z1, &x2, &y2, &z2, &x3, &y3, &z3, &x4, &y4, &z4);

				mymesh[fcount].V1 = x1;
				mymesh[fcount].V2 = x2;
				mymesh[fcount].V3 = x3;
				mymesh[fcount].V4 = x4;

				mymesh[fcount].T1 = y1;
				mymesh[fcount].T2 = y2;
				mymesh[fcount].T3 = y3;
				mymesh[fcount].T4 = y4;

				fcount++;
			}

		}
	}
	
	fclose(file);
	
	return true;
}

void DrawOBJ(double scale)
{
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat diffuse0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat ambient0[4] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat specular0[4] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light0_pos[4] = { 2.0, 2.0, 2.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_POSITION, light0_pos);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular0);


	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.2);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05);

	//빨간색 플라스틱과 유사한 재질을 다음과 같이 정의
	GLfloat mat_ambient[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat mat_diffuse[4] = { 0.6f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_specular[4] = { 0.8f, 0.6f, 0.6f, 1.0f };
	GLfloat mat_shininess = 32.0;

	// 폴리곤의 앞면의 재질을 설정 
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	

	glTexImage2D(GL_TEXTURE_2D, 0, 3, 1024, 1024, 0, GL_RGB, GL_UNSIGNED_BYTE, mytexels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	glEnable(GL_TEXTURE_2D);

	glBegin(GL_QUADS);
	for (int jj = 0; jj < 24960; jj++)
	{
		glTexCoord2d(vertex_color[mymesh[jj].T1 - 1].X, vertex_color[mymesh[jj].T1 - 1].Y);
		glVertex3f(vertex[mymesh[jj].V1 - 1].X * scale, vertex[mymesh[jj].V1 - 1].Y* scale, vertex[mymesh[jj].V1 - 1].Z* scale);
		glTexCoord2d(vertex_color[mymesh[jj].T2 - 1].X, vertex_color[mymesh[jj].T2 - 1].Y);
		glVertex3f(vertex[mymesh[jj].V2 - 1].X* scale, vertex[mymesh[jj].V2 - 1].Y* scale, vertex[mymesh[jj].V2 - 1].Z* scale);
		glTexCoord2d(vertex_color[mymesh[jj].T3 - 1].X, vertex_color[mymesh[jj].T3 - 1].Y);
		glVertex3f(vertex[mymesh[jj].V3 - 1].X* scale, vertex[mymesh[jj].V3 - 1].Y* scale, vertex[mymesh[jj].V3 - 1].Z* scale);
		glTexCoord2d(vertex_color[mymesh[jj].T4 - 1].X, vertex_color[mymesh[jj].T4 - 1].Y);
		glVertex3f(vertex[mymesh[jj].V4 - 1].X* scale, vertex[mymesh[jj].V4 - 1].Y* scale, vertex[mymesh[jj].V4 - 1].Z* scale);
	}

	glEnd();
}

void display()
{
	//화면을 지운다. (컬러버퍼와 깊이버퍼)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//이후 연산은 ModelView Matirx에 영향을 준다.
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	texture_background = MatToTexture(img_cam);
	if (texture_background < 0) return;


	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);

	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0f, 1.0f, 1.0f); //큐브나 좌표축 그릴 때 사용한 색의 영향을 안받을려면 필요
	glBindTexture(GL_TEXTURE_2D, texture_background);
	glPushMatrix();
	glTranslatef(0.0, 0.0, -14.28);
	draw_background(); //배경그림
	glPopMatrix();


	handPosition hp;
	if (Handpos.getHandPosition(img_cam, hp))
	{
		cout << "ok";
		glDisable(GL_TEXTURE_2D);
		glPushMatrix();
		glTranslatef((hp.center.x-320)*0.01, (hp.center.y-240)*-0.01, -5.712);
		
		if (anglecount == -1) {
			prevangle = hp.rotateXY; //최초
			anglecount++;
		}
		if ((abs(prevangle - hp.rotateXY) > 20)||(abs(prevangle - hp.rotateXY))>340)
		{
			if (anglecount > 10)
			{
				//10프레임 이상 지속 - 잠깐 튄게 아니라 제대로 바뀐 값인 것으로 추정
				anglecount = 0;
				prevangle = hp.rotateXY;
			}
			else 
			{
				cout << "너무급하게돌아감!";
				//완충하기
				hp.rotateXY = prevangle;
				anglecount++;
			}
		}
		else
		{
			prevangle = hp.rotateXY;
			anglecount = 0;
		}

		glRotatef(hp.rotateXY-90, 0, 0, -1.0);
		//glRotatef(cubeAngle, 0.0, 1.0, 0.0);
		//draw_cube(hp.Slength*0.005); //큐브
		DrawOBJ(0.0008*hp.Slength); //물고기
		glPopMatrix();
	}
	//cout << hp.center;

	glutSwapBuffers();
}


void reshape(GLsizei width, GLsizei height)
{
	glViewport(0, 0, (GLsizei)width, (GLsizei)height); //윈도우 크기로 뷰포인트 설정 

	glMatrixMode(GL_PROJECTION); //이후 연산은 Projection Matrix에 영향을 준다.
	glLoadIdentity();

	//Field of view angle(단위 degrees), 윈도우의 aspect ratio, Near와 Far Plane설정
	gluPerspective(45, (GLfloat)width / (GLfloat)height, 1.0, 100.0);
	gluLookAt(0, 0, 0, 0, 0, -1, 0, 1, 0);
	glMatrixMode(GL_MODELVIEW); //이후 연산은 ModelView Matirx에 영향을 준다. 
}


void timer(int value) {
	//웹캠으로부터 이미지 캡처
	capture->read(img_cam);
	cvtColor(img_cam, img_cam, COLOR_BGR2RGB);

	cubeAngle += 1.0f;
	if (cubeAngle > 360) {
		cubeAngle -= 360;
	}

	glutPostRedisplay();      //윈도우를 다시 그리도록 요청
	glutTimerFunc(1, timer, 0); //다음 타이머 이벤트는 1밀리세컨트 후  호출됨.
}



void init()
{
	glGenTextures(1, &texture_background);

	//화면 지울때 사용할 색 지정
	glClearColor(0.0, 0.0, 0.0, 0.0);

	//깊이 버퍼 지울 때 사용할 값 지정
	glClearDepth(1.0);

	//깊이 버퍼 활성화
	glEnable(GL_DEPTH_TEST);

}

void keyboard(unsigned char key, int x, int y)
{
	//ESC 키가 눌러졌다면 프로그램 종료
	if (key == 27)
	{
		capture->release();
		exit(0);
	}
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);  //GLUT 초기화

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH); //더블 버퍼와 깊이 버퍼를 사용하도록 설정, GLUT_RGB=0x00임

	cameraInit();
	glutInitWindowSize(screenW, screenH);
	glutInitWindowPosition(100, 100); //윈도우의 위치 (x,y)
	glutCreateWindow("OpenGL Example"); //윈도우 생성

	char* bmpf = (char*)("./obj-fish/13001_Ryukin_Goldfish_diff.bmp");
	char* objf = (char*)("./obj-fish/13001_Ryukin_Goldfish_v1_L3.obj");
	ReadingOBJ(bmpf, objf);//obj 읽기

	init();

	//디스플레이 콜백 함수 등록, display함수는 윈도우 처음 생성할 때와 화면 다시 그릴 필요 있을때 호출된다. 
	glutDisplayFunc(display);

	//reshape 콜백 함수 등록, reshape함수는 윈도우 처음 생성할 때와 윈도우 크기 변경시 호출된다.
	glutReshapeFunc(reshape);
	//타이머 콜백 함수 등록, 처음에는 바로 호출됨.
	glutTimerFunc(0, timer, 0);
	//키보드 콜백 함수 등록, 키보드가 눌러지면 호출된다. 
	glutKeyboardFunc(keyboard);

	//GLUT event processing loop에 진입한다.
	//이 함수는 리턴되지 않기 때문에 다음줄에 있는 코드가 실행되지 않는다. 
	glutMainLoop();


	delete[] vertex;
	delete[] mymesh;
	delete[] vertex_color;

	return 0;
}
