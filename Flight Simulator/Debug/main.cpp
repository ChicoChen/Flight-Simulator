
/************************************************************************

File:           Flight Simulator.cpp

Description:    A program that simulate a aircraft and the environment.

Author:         Xu Chen

************************************************************************/
#include <GL\glew.h>
#include <GL\freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <math.h>
#include <time.h>
#include "TextureLoader.h"

// define PI
#define PI 3.14
using namespace std;

// define ANGLE_TRANSFER to translate angle velocity 
const double ANGLE_TRANSFER = PI * 2 / 360;

// textureID
GLuint texture[3];

// storing vertices
GLdouble cassnaVertices[6800][3];
// vertices size
int vSize = 0;
// vertices' normals
GLdouble cassnaNormals[6800][3];
// normal size
int nSize = 0;
// storing faces for each sub-object
vector <vector<vector<GLint>>> subObj;
vector <vector<GLint>> faces;
vector <GLint> ver;
// size
int gSize = -1;

// window width and height
GLint windowWidth = 1000;
GLint windowHeight = 700;

// light source and material paras
GLfloat lightPosition[] = { 0,500,-500,1 };
GLfloat diskDiffuse[] = { 0.125,0.88,0.93 };
GLfloat emissive0[] = { 0.0,0.0,0.0,1 };
GLfloat emissive1[] = { 0.5,0.5,0.5,1 };
GLfloat emissive2[] = { 0.7,0.7,0.7,1 };
GLfloat materialAmbient[] = { 0.1745,0.01175,0.01175 };
GLfloat materialDiffuse[] = { 0.1,0.1,0.8 };
GLfloat materialSpecular[] = { 0.727811,0.626959,0.626959,1.0 };
GLfloat shininess = 10;
GLfloat highshininess = 50;

// coordinate paras
GLdouble coordinate[][3] = { {0,0.2,0},{5,0.2,0},{0,5.2,0},{0,0.2,5} };
GLdouble coordColor[][3] = { {1,1,1},{1,0,0},{0,1,0},{0,0,1} };

// camera paras
GLdouble cameraPosition[3] = { 0,7,24 };
GLdouble cameraView[3] = { 0,4,0 };
GLdouble cameraR = 25;
GLdouble thetaTurn = 0;
GLdouble turnAlpha = 0;

// xz-plane paras
GLdouble centerPolygonVertices[][3] = { {0,0,0},{5,0,0},{5,0,5},{0,0,5} };
GLdouble XZColor[3] = { 0.3,0.3,0.45 };
GLdouble XZNormal[3] = { 0,1,0 };
bool planeMode = false;

// sea & sky paras
GLdouble seaParas[] = { 0,510,50,50 };
GLdouble skyParas[] = { 500,500,300,100,100 };
GLUquadric *sea, *sky;

// flag show or not
bool ssMode = false;

// mountain paras
GLfloat a[] = { 32,0,0 };
GLfloat b[] = { 32,16,32 };
GLfloat c[] = { 32,0,64 };
GLfloat mountMatrix[65][65][3];
bool mountflag = false;
bool mountTflag = false;

// fog
GLfloat fogColor[] = { 0,1,0,0.3 };
bool fogMode = false;

// jet paras
GLdouble mouseX = 0;
GLdouble mouseY = 0;
GLdouble jetSpeed = 0.4;
GLfloat jetYellow[] = { 1,1,0,1 };
GLfloat jetBlack[] = { 0.0,0.0,0.0,1 };
GLfloat jetPurpule[] = { 0.6,0.3,1, };
GLfloat jetBlue[] = { 0.2,0.2,1,1 };
GLdouble rotateAngle = 0;

// fullscreen
bool fullScreen = false;


void divide(GLfloat x[3], GLfloat y[3], GLfloat z[3], int level)
{
	GLfloat m1[3], m2[3];

	if (level>0)
	{
		for (int i = 0; i < 3; i++)
		{
			m1[i] = (x[i] + y[i]) / 2;
		}
		float temp = fabs(x[1] - y[1]) / 2;
		m1[1] += rand() % ((int)temp * 100 + 1) * 2 / 100.0 - temp;
		for (int i = 0; i < 3; i++)
		{
			m2[i] = (y[i] + z[i]) / 2;
		}
		temp = fabs(y[1] - z[1]) / 2;
		m2[1] += rand() % ((int)temp * 100 + 1) * 2 / 100.0 - temp;
		mountMatrix[(int)x[0]][(int)x[2]][1] = x[1];
		mountMatrix[(int)y[0]][(int)y[2]][1] = y[1];
		mountMatrix[(int)z[0]][(int)z[2]][1] = z[1];
		mountMatrix[(int)m1[0]][(int)m1[2]][1] = m1[1];
		mountMatrix[(int)m2[0]][(int)m2[2]][1] = m2[1];
		divide(x, m1, y, level - 1);
		divide(y, m2, z, level - 1);
	}
	else
	{
		return;
	}
}


/************************************************************************

Function:       drawLight

Description:    set light position.

************************************************************************/
void drawLight()
{
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
}


/************************************************************************

Function:       drawCoordinate

Description:    draw coordinate bars and sphere.

************************************************************************/
void drawCoordinate()
{
	// define a GLUquadic pointer and new a quadric
	GLUquadric *qobj = gluNewQuadric();
	// set sphere's color
	glColor3dv(coordColor[0]);
	// generate a sphere and move it
	glPushMatrix();
	glTranslatef(0, 0.2, 0);
	gluSphere(qobj, 0.2, 20, 20);
	glPopMatrix();
	// set sphere draw style to FILL
	gluQuadricDrawStyle(qobj, GLU_FILL);
	
	glPushMatrix();
	// draw coordinates
	for (int i = 1; i <= 3; i++)
	{
		// set coordinates' colors
		glColor3dv(coordColor[i]);
		glBegin(GL_LINES);
		{
			glVertex3dv(coordinate[0]);
			glVertex3dv(coordinate[i]);
		}
		glEnd();
	}
	glDisable(GL_COLOR_MATERIAL);
	glPopMatrix();
}

/************************************************************************

Function:       drawXZPlane

Description:    draw xz plane.

************************************************************************/
void drawXZPlane()
{
	// enable light settings
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// set xz-plane's material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, materialDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	// set normals to be unit ones
	glEnable(GL_NORMALIZE);
	
	// draw quads individually
	for (int i = -50; i < 50; i++)
	{
		for (int j = -50; j < 50; j++)
		{
			glBegin(GL_QUADS);
			for (int k = 0; k < 4; k++)
			{
				glNormal3dv(XZNormal);
				glVertex3d(centerPolygonVertices[k][0] + i * 5, centerPolygonVertices[k][1], centerPolygonVertices[k][2] + j * 5);
			}
			glEnd();
		}
	}
	// disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

/************************************************************************

Function:       drawSS

Description:    draw sea and sky.

************************************************************************/
void drawSS()
{
	// enable light settings
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// set plane's material properties
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive1);
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diskDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	// set normals to be unit ones
	glEnable(GL_NORMALIZE);
	glNormal3d(0, 1, 0);
	// define 2 GLUquadric pointer
	GLUquadric *sky, *sea;
	// allocate sea and sky
	sea = gluNewQuadric();
	sky = gluNewQuadric();

	glPushMatrix();
	// rotate around x axis
	glRotatef(-90, 1, 0, 0);

	// enable 2d texture
	glEnable(GL_TEXTURE_2D);
	// bind texture
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	// set material to be self emissive
	// set texture wrapper and filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// enable quadric texture
	gluQuadricTexture(sea, 1);
	// draw disk
	gluDisk(sea, seaParas[0], seaParas[1], seaParas[2], seaParas[3]);
	
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive2);
	// bind texture
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	// set texture wrapper and filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// enable quadric texture
	gluQuadricTexture(sky, 1);
	// draw cylinder
	gluCylinder(sky, skyParas[0], skyParas[1], skyParas[2], skyParas[3], skyParas[4]);
	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	// set quadric drawing style
	gluQuadricDrawStyle(sea, GLU_LINE);
	gluQuadricDrawStyle(sky, GLU_LINE);
	glPopMatrix();
	glDisable(GL_LIGHTING);
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive0);
}

/************************************************************************

Function:       drawFog

Description:    draw fog.

************************************************************************/
void drawFog()
{
	// turn on an turn off the fog
	if (fogMode)
		glEnable(GL_FOG);
	else
		glDisable(GL_FOG);
	// set fog mode to be exp^2
	glFogi(GL_FOG_MODE, GL_EXP);
	// set fog color
	glFogfv(GL_FOG_COLOR, fogColor);
	// set fog density
	glFogf(GL_FOG_DENSITY, 0.0005);
}
/************************************************************************

Function:       drawMount

Description:    draw mountain grid.

************************************************************************/
void drawMount()
{
	// enable light settings
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// set plane's material properties
	glMaterialfv(GL_FRONT, GL_AMBIENT, materialAmbient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, diskDiffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, materialSpecular);
	glMaterialf(GL_FRONT, GL_SHININESS, shininess);
	// set normals to be unit ones
	glEnable(GL_NORMALIZE);
	if (mountTflag)
	{
		// enable 2d texture
		glEnable(GL_TEXTURE_2D);
	}
	
	// bind texture
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	// set material to be self emissive
	// set texture wrapper and filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// set polygon MODE to be line mode front and back
	if (planeMode)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	for (int i = 0; i < 64; i++)
	{
		for (int j = 0; j < 64; j++)
		{
			// draw one folded polygon
			glBegin(GL_POLYGON);
			if (i == 0 && j ==0)
			{
				glTexCoord2f(0.0, 1.0);
			}
			glColor3f(mountMatrix[i][j][1] / 16.0, 1, mountMatrix[i][j][1] / 16.0);
			glVertex3fv(mountMatrix[i][j]);
			if (i+1 == 64 && j == 0)
			{
				glTexCoord2f(1.0, 1.0);
			}
			glColor3f(mountMatrix[i + 1][j][1] / 16.0, 1, mountMatrix[i + 1][j][1] / 16.0);
			glVertex3fv(mountMatrix[i + 1][j]);
			if (i+1 == 64 && j+1 == 64)
			{
				glTexCoord2f(1.0, 0.0);
			}
			glColor3f(mountMatrix[i + 1][j + 1][1] / 16.0, 1, mountMatrix[i + 1][j + 1][1] / 16.0);
			glVertex3fv(mountMatrix[i + 1][j + 1]);
			if (i == 0 && j+1 == 64)
			{
				glTexCoord2f(0.0, 0.0);
			}
			glColor3f(mountMatrix[i][j + 1][1] / 16.0, 1, mountMatrix[i][j + 1][1] / 16.0);
			glVertex3fv(mountMatrix[i][j + 1]);
			glEnd();	
		}
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glMaterialfv(GL_FRONT, GL_EMISSION, emissive0);
}

/************************************************************************

Function:       drawMesh

Description:    draw aircraft mesh.

************************************************************************/
void drawMesh(vector<GLint> face,bool f)
{
	// disable depth_test
	glDisable(GL_DEPTH_TEST);
	// draw polygons
	glBegin(GL_POLYGON);
	if (f)
	{
		/*glPushMatrix();
		for (int k = 0; k < face.size(); k++)
		{
			glVertex3dv(cassnaVertices[face[k]]);
		}
		for (int k = 0; k < face.size(); k++)
		{
			glRotatef(180, 1, 0, 0);
			glVertex3dv(cassnaVertices[face[k]]);
		}
		glPopMatrix();*/
	}
	else
	{
		// turn on/off the wire
		if (planeMode)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		// draw mesh
		for (int k = 0; k < face.size(); k++)
		{
			glNormal3dv(cassnaNormals[face[k]]);
			glVertex3dv(cassnaVertices[face[k]]);
		}
			
	}
	glEnd();
	// enable depth test
	glEnable(GL_DEPTH_TEST);
}

/************************************************************************

Function:       drawSubObject

Description:    draw aircraft sub objects.

************************************************************************/
void drawSubObject(int i)
{
	// set craft material properties according to its number
	if (i >= 33 && i <= 34)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jetYellow);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jetYellow);
	}
	if ((i >= 0 && i <= 3) || (i >= 8 && i <= 13) || (i >= 26 && i <= 32))
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jetYellow);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jetYellow);
	}
	if (i == 4 || i == 5)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jetBlack);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jetBlack);
	}
	if (i == 6)
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jetPurpule);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jetPurpule);
	}
	if (i == 7 || (i >= 14 && i <= 25))
	{
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, jetBlue);
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, jetBlue);
	}
	// set unchange properties
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, materialSpecular);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	// draw mesh
	for (int j = 0; j < subObj[i].size(); j++)
		drawMesh(subObj[i][j], false);
}


/************************************************************************

Function:       drawJet

Description:    draw aircraft.

************************************************************************/
void drawJet()
{
	// enable light settings
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	// set normals to be unit ones
	glEnable(GL_NORMALIZE);

	// transform the aircraft
	glPushMatrix();
	glTranslatef(cameraPosition[0], cameraPosition[1], cameraPosition[2]);
	glRotatef(-thetaTurn, 0, 1, 0);
	glTranslatef(0, -1, -2);
	glRotatef(-rotateAngle, 0, 0, 1);
	glRotatef(-90, 0, 1, 0);
	glScalef(0.8, 0.8, 0.8);
	
	// wing adj
	drawSubObject(18);
	// wing adj
	drawSubObject(14);
	// door
	drawSubObject(8);
	// door
	drawSubObject(9);
	// door
	drawSubObject(10);
	// door
	drawSubObject(11);
	// door
	drawSubObject(12);
	// door
	drawSubObject(13);
	// wings
	drawSubObject(6);
	// nose
	drawSubObject(4);
	// lower body
	drawSubObject(2);
	// chin
	drawSubObject(1);
	// up chin
	drawSubObject(3);
	// upper body
	drawSubObject(0);
	// window glass
	drawSubObject(5);
	// window
	drawSubObject(7);
	
	//glPushMatrix();
	//glTranslatef(-subObj[33][])
	//// propeller
	//drawSubObject(33);
	//// propeller
	//drawSubObject(34);
	//glPopMatrix();

	// eng
	drawSubObject(28);
	// eng
	drawSubObject(29);
	// eng
	drawSubObject(30);
	// eng
	drawSubObject(31);
	// eng
	drawSubObject(32);
	// eng
	drawSubObject(23);
	// eng
	drawSubObject(24);
	// eng
	drawSubObject(25);
	// eng
	drawSubObject(26);
	// eng
	drawSubObject(27);
	// tail1
	drawSubObject(15);
	// tail1
	drawSubObject(16);
	// tail1
	drawSubObject(17);
	// tail1
	drawSubObject(19);
	// tail1
	drawSubObject(20);
	// tail1
	drawSubObject(21);
	// tail1
	drawSubObject(22);
	
	glPopMatrix();
	// disable lighting
	glDisable(GL_LIGHTING);
}

/************************************************************************

Function:       initial

Description:    initial opengl context.

************************************************************************/
void initial()
{
	// set srand time seed
	srand(time(NULL));

	// initial mountMatrix
	for (int i = 0; i < 65; i++)
		for (int j = 0; j < 65; j++)
			for (int k = 0; k < 3; k++)
				if (k != 1)
					mountMatrix[i][j][k] = k % 3 ? j : i;
	// divide a piece of mountain
	divide(a, b, c, 5);
	GLfloat u[3], v[3], q[3];
	// divide each piece
	for (int i = 0; i < 65; i++)
	{
		u[0] = 64;
		u[1] = 0;
		u[2] = i;

		v[0] = 32;
		v[1] = mountMatrix[32][i][1];
		v[2] = i;

		q[0] = 0;
		q[1] = 0;
		q[2] = i;
		if (i != 32)
		{
			divide(u, v, q, 5);
		}
	}

	// clear screen color to black
	glClearColor(0.0, 0.0, 0.0, 0.0);
	// shade model to be flat
	glShadeModel(GL_FLAT);
	// enable depth test
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);

	unsigned int width, height;
	unsigned char *data1 = loadBMPRaw("sea02.bmp", width, height);
	glGenTextures(1, &texture[0]);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data1);
	delete[] data1;

	unsigned char *data2 = loadBMPRaw("sky08.bmp",width, height);
	glGenTextures(1, &texture[1]);
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data2);
	delete[] data2;

	unsigned char *data3 = loadBMPRaw("mount03.bmp", width, height);
	glGenTextures(1, &texture[2]);
	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data3);
	delete[] data3;

}
/************************************************************************

Function:       reshape

Description:    window reshape function.

************************************************************************/
void reshape(int newWidth, int newHeight)
{
	// set window width and height to be current ones
	windowWidth = newWidth;
	windowHeight = newHeight;
	// set viewport
	glViewport(0, 0, windowWidth, windowHeight);
	// change to projection mode
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	// set perspective 
	gluPerspective(60, (double)windowWidth / (double)windowHeight, 0.1, 2000);
	// change back
	glMatrixMode(GL_MODELVIEW);
}

/************************************************************************

Function:       specialKeyboard

Description:    special keyboard specifications.

************************************************************************/
void specialKeyboard(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		// add speed if key_up pressed
		if (jetSpeed <= 0.8)
		{
			jetSpeed += 0.05;
		}
		// redraw the view
		glutPostRedisplay();
		break;
	case GLUT_KEY_DOWN:
		// subtract speed if key_down pressed
		if (jetSpeed >= 0.1)
		{
			jetSpeed -= 0.05;
		}
		// redraw the view
		glutPostRedisplay();
		break;
	case GLUT_KEY_PAGE_UP:
		// move camera and jet up
		cameraPosition[1] += 0.1;
		cameraView[1] += 0.1;
		glutPostRedisplay();
		break;
	case GLUT_KEY_PAGE_DOWN:
		// move camera and jet down
		cameraPosition[1] -= 0.1;
		cameraView[1] -= 0.1;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

/************************************************************************

Function:       keyboard

Description:    keyboard specifications.

************************************************************************/
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'm':
		// turn on mountain
		mountflag = !mountflag;
		glutPostRedisplay();
		break;
	case 't':
		// turn on mountain texture flag
		mountTflag = !mountTflag;
		glutPostRedisplay();
		break;
	case 'q':
		// exit program
		exit(0);
		break;
	case 'w':
		// turn on/off the wire
		planeMode = !planeMode;
		glutPostRedisplay();
		break;
	case 'f':
		// turn on/off the full screen
		if (!fullScreen)
		{
			glutFullScreen();
			fullScreen = !fullScreen;
		}
		else
		{
			glutReshapeWindow(700, 700);
			fullScreen = !fullScreen;
		}
		glutPostRedisplay();
		break;
	case 's':
		// turn on/off the sea and sky
		ssMode = !ssMode;
		glutPostRedisplay();
		break;
	case 'g':
		// turn on/off the fog
		fogMode = !fogMode;
		glutPostRedisplay();
		break;
	default:
		break;
	}
}

/************************************************************************

Function:       mouseControl

Description:    passive motion mouseControl .

************************************************************************/
void mouseControl(int x, int y)
{
	// calculate mouse's new coordinates
	mouseX = ((GLdouble)x *   2.0 / (GLdouble)windowWidth - 1.0) / 2;
	mouseY = (GLdouble)y * (-1.0) / (GLdouble)windowHeight + 1.0;
	// set turnAlpha related to mouseX's value
	turnAlpha = 2.5*fabs(mouseX);
	// dont rotate if mouse is out of window
	if (fabs(mouseY - 0) < 0.0001)
	{
		rotateAngle = 0;
	}
	else
	{
		// calculate angle
		double temp = atan(mouseX / mouseY) * 180 / PI;
		// if mouseX's absolute value is smaller than 0.025, don't rotate
		if (mouseX <= 0.025 && mouseX >= -0.025)
		{
			rotateAngle = 0;
		}
		// up limit of rotate angle is 45
		else if (temp - 45 > 0)
		{
			rotateAngle = 45;
		}
		else if (temp + 45 < 0)
		{
			rotateAngle = -45;
		}
		// else rotate angle = temp
		else
		{
			rotateAngle = temp;
		}
	}
}

/************************************************************************

Function:       readFile

Description:    read file from the given directory .

************************************************************************/
void readFile(char *filename)
{
	// set flag to avoid reading same data
	bool flag = true;
	if (strcmp(filename,"propeller.txt"))
	{
		flag = false;
	}

	// Open file in read only mode
	FILE *file = fopen(filename, "r");
	// Declare a buffer char array
	char buf[750];
	while (fgets(buf,750,file))
	{
		// Set strlen(buf)th position to \0
		buf[strlen(buf)] = '\0';
		// String length
		int slen = strlen(buf);
		// Allocate the space exactly for the size of input
		char *str = (char *)malloc((slen + 1) * sizeof(char));
		// Copy the buffer to the str
		strcpy(str, buf);

		// if str starts with g
		if (str[0] == 'g')
		{
			// if it is not the first g
			if (gSize != -1)
			{
				// push back the previous faces
				subObj.push_back(faces);
			}
			// add gSize with one
			gSize++;
			// clear faces
			faces.clear();
			continue;
		}

		// Declare a pointer to split string into sub strings
		char *p;
		// remove token 'vngf \n\0'
		p = strtok(buf, "vngf \0\n");
		

		// read the data
		if ((str[0] == 'v' || str[0] == 'n') && flag)
		{
			int counter = 0;
			while (p != NULL && strcmp(p, "\n"))
			{
				// read cassnaVertices
				if (str[0] == 'v')
					cassnaVertices[vSize][counter++] = atof(p);
				// read cassnaNormals
				else if (str[0] == 'n')
					cassnaNormals[nSize][counter++] = atof(p);
				// avoid fall into infinite loop
				p = strtok(NULL, " \n");
			}
			// add vSize with one
			if (str[0] == 'v')
				vSize++;
			// add nSize with one
			else
				nSize++;
		}
		// if str starts with 'f'
		if (str[0] == 'f')
		{
			// clear ver
			ver.clear();
			// push back data
			while (p != NULL && strcmp(p,"\n"))
			{
				ver.push_back(atoi(p)-1);
				p = strtok(NULL, " \n");
			}
			// push back ver into faces
			faces.push_back(ver);
		}
	}
	// push back faces
	if (gSize != 0)
	{
		subObj.push_back(faces);
	}
}

/************************************************************************

Function:       myIdle

Description:    my idle function. Change variables' values when idle.

************************************************************************/
void myIdle()
{
	// move camera position
	cameraPosition[0] += jetSpeed * sin(thetaTurn*ANGLE_TRANSFER);
	cameraPosition[2] -= jetSpeed * cos(thetaTurn*ANGLE_TRANSFER);
	// move lookat center 
	cameraView[0] += jetSpeed * sin(thetaTurn*ANGLE_TRANSFER);
	cameraView[2] -= jetSpeed * cos(thetaTurn*ANGLE_TRANSFER);
	// rotate to right if mouseX >= 0.025
	if (mouseX >= 0.025)
	{
		if (thetaTurn < 0)
			thetaTurn += 360;
		else if (thetaTurn >= 360)
			thetaTurn -= 360;
		thetaTurn += 0.7*turnAlpha;
		glutPostRedisplay();
	}
	// rotate to left if mouseX <= -0.025
	else if (mouseX <= -0.025)
	{
		if (thetaTurn < 0)
			thetaTurn += 360;
		else if (thetaTurn >= 360)
			thetaTurn -= 360;
		thetaTurn -= 0.7*turnAlpha;
		glutPostRedisplay();
	}
	glutPostRedisplay();
}
/************************************************************************

Function:       cameraControl

Description:    set gluLookAt in this function.

************************************************************************/
void cameraControl()
{
	// the code is apparent
	gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], cameraView[0] + cameraR*sin(thetaTurn * ANGLE_TRANSFER), cameraView[1], cameraView[2] + cameraR*(1 - cos(thetaTurn*ANGLE_TRANSFER)), 0, 1, 0);
}

/************************************************************************

Function:       drawObjects

Description:    draw different objects.

************************************************************************/
void drawObjects()
{
	// set shade mode to be smooth
	glShadeModel(GL_SMOOTH);
	// turn on/off the wire
	if (!planeMode)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// clear color and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// set up gluLookAt
	cameraControl();
	// set light postion
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	// draw fog
	drawFog();
	
	// draw coordinate
	drawCoordinate();
	// draw xz-plane or sea and sky 
	if (!ssMode)
		drawXZPlane();
	else
		drawSS();
	if (mountflag)
	{
		glPushMatrix();
		glTranslatef(32, -1, 32);
		glScalef(0.5, 0.5, 0.5);
		glTranslatef(-32, 0, -32);
		drawMount();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-64, -1, 32);
		glTranslatef(-32, 0, -32);
		drawMount();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(64, -1, -32);
		glScalef(0.9, 0.3, 0.5);
		glTranslatef(-32, 0, -32);
		drawMount();
		glPopMatrix();
	}
	
	// draw aircraft
	drawJet();

	// swap buffers
	glutSwapBuffers();
}
void control()
{
	printf("\n\n");

	printf("Scene Control\n");
	printf("---------------------------\n");
	printf("f: toggle fullscreen\n");
	printf("g: toggle fog\n");
	printf("s: toggle sea and sky\n");
	printf("w: toggle wire\n");
	printf("q: toggle quit\n");
	printf("t: toggle texture\n");
	printf("m: toggle mountain\n");


	printf("\n\n");

	printf("Camera Control\n");
	printf("---------------------------\n");
	printf("Page Up   :\tfaster\n");
	printf("Page Down :\tslower\n");
	printf("Up Arrow  :\tmove up\n");
	printf("Down Arrow:\tmove down\n");
	printf("Mouse Right:\tturn right\n");
	printf("Mouse Right:\tturn left\n");
}
/************************************************************************

Function:       main

Description:    specify the opengl context and do the main loop.

************************************************************************/
int main(int argc, char** argv)
{
	
	// read files
	readFile("cessna.txt");
	readFile("propeller.txt");
	// initialize parameter
	glutInit(&argc, argv);
	// set up display mode
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	// set up window size
	glutInitWindowSize(windowWidth, windowHeight);
	// create window
	glutCreateWindow("Flight Simulator");
	
	// callback funcs
	glutIdleFunc(myIdle);
	glutDisplayFunc(drawObjects);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(specialKeyboard);
	glutPassiveMotionFunc(mouseControl);
	
	

	// initial opengl context
	initial();
	/*for (int i = 0; i < 65; i++)
	{
		printf("%lf,%lf,%lf\n", mountMatrix[16][i][0], mountMatrix[16][i][1], mountMatrix[16][i][2]);
	}*/
	control();
	// dive into main loop
	glutMainLoop();
	return 0;
}