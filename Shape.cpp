#include "Shape.h"

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <time.h>
#include <climits>  
#include <algorithm>

using namespace std;

// for camera matrix
static Vector3 e = Vector3(75, 4, 0); // origin
static Vector3 d = Vector3(74, 4, 0); // look at
static Vector3 up = Vector3(0, 1, 0); // up

static char texture_building[32] = "building1.ppm";
static char texture_road[32] = "road1.ppm";
static char texture_top[32] = "";
GLuint shadowMapTexture;
GLuint buildingTexture1;
GLuint buildingTexture2;
GLuint buildingTexture3;
GLuint buildingTexture4;
GLuint buildingTexture5;
GLuint roadTexture;
GLuint topTexture;
static bool printedfirst = false;
static char displayString[32] = "COLLISION DETECTED";
static char displayString2[32] = "HEIGHTMAP OFF\n";
static char displayString3[32] = "HEIGHTMAP ON\n";
static bool heightMapEnabled = true;
static bool falling = false;
static float velocity = 2.5;
static bool collisiondetected = false;
static bool heightmaptoggledon = false;
static bool heightmaptoggledoff = false;
static int counter = 0;
static int counter2 = 0;
static int counter3 = 0;
static float curr_height = 0.0;

static bool moved = false;
static bool showShadows = false;
static bool sun_underground = false;
const int shadowMapSizex=1366;
const int shadowMapSizey=768;

Matrix4 lightProjectionMatrix, lightViewMatrix, cameraProjectionMatrix, cameraViewMatrix;
Vector3 lightPos = Vector3(0.0, 1000.0, 0.0);

static bool warp = true;
static bool speed_up = true;
static float sun_speed = 0.001;
static bool shadowMode = false;
static bool godMode = false;
static float city_scale = 0.1;

static float anglex = 0.0;
static float angley = 0.0;
static float anglex_change = 0.0;
static float angley_change = 0.0;
static float anglex_factor = 1.0;
static float angley_factor = 1.0;

static float walk_x_factor = 1.0;
static float walk_y_factor = 1.0;
static float walk_z_factor = 1.0;

/*
-278.163 minx
-0.00
-463.471
740.302 maxx
356.627
322.844
*/
static int x_shift = 278;
static int z_shift = 463;
static vector<vector<int> > heightMap(1019, vector<int>(787, 0));
static bool init_heightMap = false;

static Shape shape;
static int shape_key = 1;

static bool left_clicked = false;
static bool right_clicked = false;
static int x_mouse;
static int y_mouse;

static bool shader_toggle = false;
static bool toggle1 = false;
static bool toggle2 = false;
static bool toggle3 = false;
static bool toggle_freeze = true;
static bool toggle_frus = false;
static bool toggle_tex = false;

int Window::width  = 512;   // set window width in pixels here
int Window::height = 512;   // set window height in pixels here
static bool fullscreen = true;

static int num_fire = 5000;
static int num_snow = 1000;
static int num_rain = 1000;
static int num_leaves = 50;

static vector<Particle> fire (num_fire, Particle());
static vector<Particle> snow (num_snow, Particle());
static vector<Particle> rain (num_rain, Particle());
static vector<Particle> leaves (num_leaves, Particle());

/*
0 = winter
1 = spring
2 = summer
3 = fall
*/
static int season = 0;

//----------------------------------------------------------------------------
// Callback method called when system is idle.
void Window::idleCallback(void) {
	displayCallback();
}

//----------------------------------------------------------------------------
// Callback method called when window is resized.
void Window::reshapeCallback(int w, int h) {
	glPushMatrix();
		glLoadIdentity();
		gluPerspective(90.0, float(Window::width)/float(Window::height), 0.1, 10000);
		glGetFloatv(GL_MODELVIEW_MATRIX, shape.getModelViewMatrix().getGLMatrix());
	glPopMatrix();
}

void Shape::loadTexture(char* the_texture) {
	//GLuint texture;			 // storage for one texture
	int twidth, theight;   // texture width/height [pixels]
	unsigned char* tdata;  // texture pixel data
	// Load image file
	tdata = loadPPM(the_texture, twidth, theight);
	if (tdata==NULL) return;
	int num;
	
	// Create ID for texture
	if (the_texture[0] == 'b') {
		
		num = the_texture[8] - '0';
		if (num == 1) {
			
		glGenTextures(1, &buildingTexture1);   
		
		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, buildingTexture1);
		}
		if (num == 2) {
			
		glGenTextures(1, &buildingTexture2);   
		
		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, buildingTexture2);
		}
		if (num == 3) {
		glGenTextures(1, &buildingTexture3);   
		
		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, buildingTexture3);
		}
		if (num == 4) {
		glGenTextures(1, &buildingTexture4);   
		
		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, buildingTexture4);
		}
		if (num == 5) {
		glGenTextures(1, &buildingTexture5);   
		
		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, buildingTexture5);
		}
	}
	else if (the_texture[0] == 'r') {
		glGenTextures(1, &roadTexture);   

		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, roadTexture);
	}
	else if (the_texture == texture_top) {
		glGenTextures(1, &topTexture);   

		// Set this texture to be the one we are working with
		glBindTexture(GL_TEXTURE_2D, topTexture);
	}
	
	// Generate the texture
	glTexImage2D(GL_TEXTURE_2D, 0, 3, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
	
	// Set bi-linear filtering for both minification and magnification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

unsigned char* Shape::loadPPM(const char* filename, int& width, int& height) {
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ( (fp=fopen(filename, "rb")) == NULL) {
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do {
		retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf=sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width  = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do {
		retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1) {
		std::cerr << "error parsing ppm file, incomplete data" << std::endl;
		delete[] rawData;
		width = 0;
		height = 0;
		return NULL;
	}

	return rawData;
}

Matrix4& Shape::getModelMatrix() {
	return shape.model;
}

Matrix4& Shape::getViewportMatrix() {
	return shape.viewport;
}

Matrix4& Shape::getProjectionMatrix() {
	return shape.projection;
}

void Shape::setProjectionMatrix() {
	getProjectionMatrix().identity();

	float fov = 3.141592654*90.0/180.0;
	
	float aspect;
	if (fullscreen) {
		aspect = 1366.0/768.0;
	}
	else {
		aspect = 512.0/512.0;
	}
	float nearv = 0.1;
	float farv = 10000.0;
	
	projection = 
		Matrix4(
			1.0/(aspect), 0, 0, 0,
			0, 1.0, 0, 0,
			0, 0, (nearv+farv)/(nearv-farv), 2*nearv*farv/(nearv-farv),
			0, 0, -1, 0);
	projection.transpose();
}

void Shape::setViewportMatrix() {
	float x = Window::width;
	float y = Window::height;
	float x0 = 0;
	float y0 = 0;

	getViewportMatrix() = 
		Matrix4((x-x0)/2, 0, 0, (x+x0)/2,
				0, (y-y0)/2, 0, (y+y0)/2,
				0, 0, 0.5, 0.5,
				0, 0, 0, 1);
	viewport.transpose();
}

Matrix4& Shape::getModelViewMatrix() {
	return shape.modelview;
}

void Shape::updateModelViewMatrix() {
	Matrix4 cam_inv = Matrix4(shape.getCameraMatrix());
	cam_inv.inverse();
	shape.getModelViewMatrix() = cam_inv.multiply(shape.getModelMatrix());
}

void Shape::updateCameraMatrix(float dx, float dy, float dz) {
	float new_ex = e.getX() + dx;
	float new_ey = e.getY() + dy;
	float new_ez = e.getZ() + dz;
	e = Vector3(new_ex, new_ey, new_ez);

	float new_dx = d.getX() + dx;
	float new_dy = d.getY() + dy;
	float new_dz = d.getZ() + dz;
	d = Vector3(new_dx, new_dy, new_dz);

	shape.getCameraMatrix() = Matrix4::createCameraMatrix(e, d, up);

	shape.updateModelViewMatrix();
}

//----------------------------------------------------------------------------
// Callback method called when window readraw is necessary or
// when glutPostRedisplay() was called.
void Window::displayCallback(void) {
	// clear color and depth buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(shape.getModelViewMatrix().getGLMatrix());
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(shape.getProjectionMatrix().getGLMatrix());
	glMatrixMode(GL_MODELVIEW);

	if (!showShadows)
		Window::drawCWSkyBox();

	switch (shape_key) {
		case 1: // city scene
			// uncomment below for rotating lights + shadows
			if (moved) {
				moved = false;
			}

			if (showShadows) {
				if (!sun_underground) {
					shape.initializeShadows();
					shape.makeShadows();
				}
				else {
					glDisable(GL_LIGHTING);
					Window::drawCWSkyBox();

					glEnable(GL_LIGHTING);
					glDisable(GL_LIGHT0);
					glDisable(GL_LIGHT1);
					glDisable(GL_LIGHT2);
					shape.drawShadowWorld();
					shape.drawSeason();
				}
			}
			else {
				if (sun_underground)
					glEnable(GL_LIGHTING);
				else
					glDisable(GL_LIGHTING);

				shape.drawShadowWorld();
				shape.drawSeason();
			}
			break;
		case 2: // house scene
			if (moved) {
				moved = false;
			}

			if (showShadows) {
				shape.initializeShadows();
				shape.makeShadows();
			}
			else {
				if (sun_underground)
					glEnable(GL_LIGHTING);
				else
					glDisable(GL_LIGHTING);

				shape.drawHouse();
			}
			break;
	}

	Matrix4 tmp;
	tmp = Matrix4(
		lightPos.getX(), lightPos.getY(), lightPos.getZ(), 1, 
		0, 0, 0, 0, 
		0, 0, 0, 0, 
		0, 0, 0, 0
	);
	tmp.transpose();
	tmp.rotateZ(sun_speed);
	lightPos = Vector3(tmp.get(0,0), tmp.get(0,1), tmp.get(0,2));
		
	bool print_lightpos = false;
	if (toggle_freeze) {
		sun_speed = 0.0;
	}
	else if (speed_up == true) {		
		if (!showShadows)
			sun_speed = 0.01;
		else
			sun_speed = 0.0025;
	}
	else {
		sun_speed = 0.001;
	}
	
	int tmpx;
	int tmpz;
	
	if (e.getX() < 0) {
		tmpx = -1*floor((-1.0*e.getX())+0.5);
	}
	else {
		tmpx = floor(e.getX()+0.5);
	}

	if (e.getZ() < 0) {
		tmpz = -1*floor((-1.0*e.getZ())+0.5);
	}
	else {
		tmpz = floor(e.getZ()+0.5);
	}
	
	if (falling == true) {
		shape.gravity(tmpx, tmpz);
	}
	
	if (anglex_change != 0.0 || angley_change != 0.0) {
		shape.updateLookAtVector();
		anglex_change = 0.0;
		angley_change = 0.0;
	}

	if (heightmaptoggledon) {
		glColor3f(1,1,1);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();

				//Print text
				glRasterPos2f(-1.0f, 0.8f);
				for(unsigned int i=0; i<strlen(displayString3); ++i)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, displayString3[i]);
		
				//reset matrices
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		counter2++;
		if (counter2 == 100) {
			heightmaptoggledon = false;
			counter2 = 0;
		}
	}


	if (heightmaptoggledoff) {
		glColor3f(1,1,1);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();

				//Print text
				glRasterPos2f(-1.0f, 0.7f);
				for(unsigned int i=0; i<strlen(displayString2); ++i)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, displayString2[i]);
		
				//reset matrices
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		counter3++;
		if (counter3 == 100) {
			heightmaptoggledoff = false;
			counter3 = 0;
		}
	}

	if (collisiondetected) {
		glColor3f(1,0,0);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();

				//Print text
				glRasterPos2f(-1.0f, 0.9f);
				for(unsigned int i=0; i<strlen(displayString); ++i)
					glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, displayString[i]);
		
				//reset matrices
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		counter++;
		if (counter == 100) {
			collisiondetected = false;
			counter = 0;
		}
	}

	// draw sun
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
		glTranslated(lightPos.getX(), lightPos.getY(), lightPos.getZ());
		glColor3f(1, (lightPos.getY())/1000.0f+0.25, 0);
		glutSolidSphere(50, 100, 100);
	glPopMatrix();
	
	if (lightPos.getY() < (0.0f + 50)) {
		sun_underground = true;
	}
	else {
		sun_underground = false;
	}

	glFlush();  
	glutSwapBuffers();
	//glutPostRedisplay();
}

void Shape::initializeShadows() {
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity(); // TODO
		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		//Depth states
		glClearDepth(1.0f);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_NORMALIZE);
		glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
		glGenTextures(1, &shadowMapTexture);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glTexImage2D(	GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSizex, shadowMapSizey, 0,
						GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		//glTexSubImage2D(	GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0,
		//				GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	
		GLfloat white[3] = {GLfloat(1), GLfloat(1), GLfloat(1)};

		glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
	
		//White specular material color, shininess 16
		glMaterialfv(GL_FRONT, GL_SPECULAR, white);
		glMaterialf(GL_FRONT, GL_SHININESS, 16.0f);
		GLfloat p[16];

		glPushMatrix();
			glLoadIdentity();
			gluPerspective(90, float(Window::width)/float(Window::height), 0.1, 10000);

			glGetFloatv(GL_MODELVIEW_MATRIX, p);
			cameraProjectionMatrix = Matrix4(
				p[0],p[1],p[2],p[3],
				p[4],p[5],p[6],p[7],
				p[8],p[9],p[10],p[11],
				p[12],p[13],p[14],p[15]);
			glLoadIdentity();
			gluLookAt(
				e.getX(), e.getY(), e.getZ(),
				d.getX(), d.getY(), d.getZ(),
				0, 1, 0
			);

			glGetFloatv(GL_MODELVIEW_MATRIX, p);
			cameraViewMatrix = Matrix4(
				p[0],p[1],p[2],p[3],
				p[4],p[5],p[6],p[7],
				p[8],p[9],p[10],p[11],
				p[12],p[13],p[14],p[15]
			);
			glLoadIdentity();
			gluPerspective(90.0, float(Window::width)/float(Window::height), 0.1, 10000);
		
			glGetFloatv(GL_MODELVIEW_MATRIX, p);
			lightProjectionMatrix = Matrix4(
				p[0],p[1],p[2],p[3],
				p[4],p[5],p[6],p[7],
				p[8],p[9],p[10],p[11],
				p[12],p[13],p[14],p[15]
			);
			glLoadIdentity();
			gluLookAt(
				lightPos.getX(), lightPos.getY(), lightPos.getZ(),
				d.getX(), d.getY(), d.getZ(),
				0, 1, 0
			);
				
			glGetFloatv(GL_MODELVIEW_MATRIX, p);
			lightViewMatrix = Matrix4(
				p[0],p[1],p[2],p[3],
				p[4],p[5],p[6],p[7],
				p[8],p[9],p[10],p[11],
				p[12],p[13],p[14],p[15]
			);
		glPopMatrix();
	
		gluLookAt(
			e.getX(), e.getY(), e.getZ(), 
			d.getX(), d.getY(), d.getZ(),
			0, 1, 0
		);

		glMatrixMode(GL_MODELVIEW);
}

void Shape::makeShadows() {
		//============================================================================
		//First pass - from light's point of view
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glClear(GL_DEPTH_BUFFER_BIT);
		//lightProjectionMatrix.print();
		//lightViewMatrix.print();

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(lightProjectionMatrix.getGLMatrix());

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(lightViewMatrix.getGLMatrix());

		//Use viewport the same size as the shadow map
		glViewport(0, 0, shadowMapSizex, shadowMapSizey);

		//Draw back faces into the shadow map
		glCullFace(GL_FRONT);

		//Disable color writes, and use flat shading for speed
		glShadeModel(GL_FLAT);
		glColorMask(0, 0, 0, 0);
	
		//Draw the scene
		if (shape_key == 1)
			shape.drawShadowWorld();
		else
			shape.drawHouse();
		
		//Read the depth buffer into the shadow map texture
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSizex, shadowMapSizey);

		//restore states
		glCullFace(GL_BACK);
		glShadeModel(GL_SMOOTH);
		glColorMask(1, 1, 1, 1);



		//============================================================================
		//2nd pass - Draw from camera's point of view
		glClear(GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(cameraProjectionMatrix.getGLMatrix());
	
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(cameraViewMatrix.getGLMatrix());

		glViewport(0, 0, float(Window::width), float(Window::height));

		GLfloat tmp[3] = {GLfloat(10), GLfloat(10), GLfloat(0)};
		GLfloat almostwhite[3] = {GLfloat(0.2), GLfloat(0.2), GLfloat(0.2)};
		GLfloat black[3] = {GLfloat(0), GLfloat(0), GLfloat(0)};
		GLfloat white[3] = {GLfloat(1), GLfloat(1), GLfloat(1)};
		//Use dim light to represent shadowed areas
		glLightfv(GL_LIGHT1, GL_POSITION, tmp);
		glLightfv(GL_LIGHT1, GL_AMBIENT, almostwhite);
		glLightfv(GL_LIGHT1, GL_DIFFUSE, almostwhite);
		glLightfv(GL_LIGHT1, GL_SPECULAR, black);

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT1);

		if (shape_key == 1) {
			shape.drawShadowWorld();
		}
		else
			shape.drawHouse();



		//============================================================================
		//3rd pass
		//Draw with bright light
		glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
		glLightfv(GL_LIGHT1, GL_SPECULAR, white);

		//Calculate texture matrix for projection
		//This matrix takes us from eye space to the light's clip space
		//It is postmultiplied by the inverse of the current view matrix when specifying texgen
		Matrix4 biasMatrix(
									0.5f, 0.0f, 0.0f, 0.0f,
									0.0f, 0.5f, 0.0f, 0.0f,
									0.0f, 0.0f, 0.5f, 0.0f,
									0.5f, 0.5f, 0.5f, 1.0f
		); //bias from [-1, 1] to [0, 1]
		biasMatrix.scale(1.0, 1.0, 1.0);

		Matrix4 textureMatrix = biasMatrix.multiply(lightProjectionMatrix.multiply(lightViewMatrix));
		textureMatrix.transpose();
		GLfloat* tmp2;
	
		tmp2 = textureMatrix.getGLMatrix();

		GLfloat tmp3[4] = {tmp2[0], tmp2[1], tmp2[2], tmp2[3]};
		GLfloat tmp4[4] = {tmp2[4], tmp2[5], tmp2[6], tmp2[7]};
		GLfloat tmp5[4] = {tmp2[8], tmp2[9], tmp2[10], tmp2[11]};
		GLfloat tmp6[4] = {tmp2[12], tmp2[13], tmp2[14], tmp2[15]};
	
		//Set up texture coordinate generation.
		glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_S, GL_EYE_PLANE, tmp3);
		glEnable(GL_TEXTURE_GEN_S);

		glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_T, GL_EYE_PLANE, tmp4);
		glEnable(GL_TEXTURE_GEN_T);

		glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_R, GL_EYE_PLANE, tmp5);
		glEnable(GL_TEXTURE_GEN_R);

		glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
		glTexGenfv(GL_Q, GL_EYE_PLANE, tmp6);
		glEnable(GL_TEXTURE_GEN_Q);

		//Bind & enable shadow map texture
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glEnable(GL_TEXTURE_2D);

		//Enable shadow comparison
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);

		//Shadow comparison should be true (ie not in shadow) if r<=texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);

		//Shadow comparison should generate an INTENSITY result
		glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

		//Set alpha test to discard foralse comparisons
		glAlphaFunc(GL_GEQUAL, 0.99f);
		glEnable(GL_ALPHA_TEST);

		if (shape_key == 1) {
			shape.drawShadowWorld();
		}
		else
			shape.drawHouse();

		//Disable textures and texgen
		glDisable(GL_TEXTURE_2D);

		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		glDisable(GL_TEXTURE_GEN_R);
		glDisable(GL_TEXTURE_GEN_Q);

		//Restore other states
		glDisable(GL_LIGHTING);
		glDisable(GL_ALPHA_TEST);
		glDeleteTextures(1, &shadowMapTexture);

		/*
		//Set matrices for ortho
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
				glLoadIdentity();
	
				//reset matrices
				glMatrixMode(GL_PROJECTION);
			glPopMatrix();
			glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		*/

		glMatrixMode(GL_MODELVIEW);
}

void Shape::drawShadowWorld() {
	Window::drawShape(city_nVerts, city_vertices, city_normals, city_texcoords);
}

void Window::drawCube() {
	glBegin(GL_QUADS); // Begin drawing the color cube with 6 quads
		// Top face (y = 1.0f)
		// Define vertices in counter-clockwise (CCW) order with normal pointing out
		glColor3f(0.0f, 1.0f, 0.0f);     // Green
		glVertex3f( 1.0f, 1.0f, -1.0f);
		glVertex3f(-1.0f, 1.0f, -1.0f);
		glVertex3f(-1.0f, 1.0f,  1.0f);
		glVertex3f( 1.0f, 1.0f,  1.0f);
 
		// Bottom face (y = -1.0f)
		glColor3f(1.0f, 0.5f, 0.0f);     // Orange
		glVertex3f( 1.0f, -1.0f,  1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
 
		// Front face  (z = 1.0f)
		glColor3f(1.0f, 0.0f, 0.0f);     // Red
		glVertex3f( 1.0f,  1.0f, 1.0f);
		glVertex3f(-1.0f,  1.0f, 1.0f);
		glVertex3f(-1.0f, -1.0f, 1.0f);
		glVertex3f( 1.0f, -1.0f, 1.0f);
 
		// Back face (z = -1.0f)
		glColor3f(1.0f, 1.0f, 0.0f);     // Yellow
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f, -1.0f);
 
		// Left face (x = -1.0f)
		glColor3f(0.0f, 0.0f, 1.0f);     // Blue
		glVertex3f(-1.0f,  1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
 
		// Right face (x = 1.0f)
		glColor3f(1.0f, 0.0f, 1.0f);     // Magenta
		glVertex3f(1.0f,  1.0f, -1.0f);
		glVertex3f(1.0f,  1.0f,  1.0f);
		glVertex3f(1.0f, -1.0f,  1.0f);
		glVertex3f(1.0f, -1.0f, -1.0f);
   glEnd(); // End of drawing color-cube
}

void Shape::loadData() {
	ObjReader::readObj("city.obj", city_nVerts, &city_vertices, &city_normals, &city_texcoords, city_nIndices, &city_indices);
}

void Window::drawShape(int nVerts, float *vertices, float *normals, float *texcoords) {
	glColor3f(1, 1, 1);
	float red = 0.0;
	float green = 0.0;
	float blue = 0.0;
	GLfloat m[4] = {GLfloat(0.0), GLfloat(1.0), GLfloat(1.0), GLfloat(0.0)};
	GLfloat n[4] = {GLfloat(0.0), GLfloat(0.0), GLfloat(1.0), GLfloat(0.0)};
	
	glPushMatrix();
	int o = 0;
	// scale city down
	GLfloat curr[16];
	bool text = false;
	glGetFloatv(GL_MODELVIEW_MATRIX, curr);
	Matrix4 curr_mv = Matrix4(
		curr[0], curr[1], curr[2], curr[3],
		curr[4], curr[5], curr[6], curr[7],
		curr[8], curr[9], curr[10], curr[11],
		curr[12], curr[13], curr[14], curr[15]
	);
	//curr_mv.scale(city_scale, city_scale, city_scale);
	if (showShadows == false) {
		glEnable(GL_TEXTURE_2D);
	}
	glLoadMatrixf(curr_mv.getGLMatrix());
	
	//glBindTexture(GL_TEXTURE_2D, buildingTexture1);
	glBegin(GL_TRIANGLES);
	int p = 0;
	int k = 0;
	int l = 0;
	for (int i=0; i<nVerts/3; i++) {
		if (k % 282 == 0) {
			red = city_colors[l][0];
			green = city_colors[l][1];
			blue = city_colors[l][2];
				l++;
		}

		text = false;
		glColor3f(red, green, blue);
		if ((showShadows == true)  && ((abs(vertices[9*i]) < 0.5) || (abs(vertices[9*i+1]) < 0.5) || (abs(vertices[9*i+2]) < 0.5))) {
			glColor3f(0.8, 0.8, 0.8);
		}
		//text = false;
		if (showShadows == false) {
			if ((abs(vertices[9*i]) < 0.5) || (abs(vertices[9*i+1]) < 0.5) || (abs(vertices[9*i+2]) < 0.5)) {
				glColor3f(0.5,0.5,0.5);
				o = k;
				//glActiveTexture(GL_TEXTURE0);
				glEnd();
				if (showShadows == false) {
					glBindTexture(GL_TEXTURE_2D, roadTexture);
				}
				glBegin(GL_TRIANGLES);
				//glEnable(GL_TEXTURE_2D);
				text = true;
			}
			else { 
				//glActiveTexture(GL_TEXTURE1);
				glEnd();


				if (l%5 == 0) {
					glBindTexture(GL_TEXTURE_2D, buildingTexture1);
				}
				else if (l%5 == 1) {
					glBindTexture(GL_TEXTURE_2D, buildingTexture2);
				}
				else if (l%5 == 2) {
					glBindTexture(GL_TEXTURE_2D, buildingTexture3);
				}
				else if (l%5 == 3) {
					glBindTexture(GL_TEXTURE_2D, buildingTexture4);
				}
				else if (l%5 == 4) {
					glBindTexture(GL_TEXTURE_2D, buildingTexture5);
				}
			
				glBegin(GL_TRIANGLES);
				//glEnable(GL_TEXTURE_2D);
			}
		}
		for (int v=0; v<3; v++) {
			glNormal3f(normals[9*i+3*v], normals[(9*i)+(3*v)+1], normals[(9*i)+(3*v)+2]);
			if (showShadows == false) {
				glTexCoord2f(texcoords[2*k]/1.0, texcoords[2*k+1]/1.0);
			}
			//else {
				//glTexCoord2f
			glVertex3f(vertices[9*i+3*v], vertices[(9*i)+(3*v)+1], vertices[(9*i)+(3*v)+2]);
			k++;
		}
	}
	glEnd();

	glPopMatrix();
}

Shape::Shape() {
	shape.getModelMatrix().identity();

	shape.getCameraMatrix().identity();
	shape.getCameraMatrix() = Matrix4::createCameraMatrix(e, d, up);
	
	shape.updateModelViewMatrix();

	cout << "initialized model matrix:\n";
	shape.getModelMatrix().print();

	cout << "initialized camera matrix:\n";
	shape.getCameraMatrix().print();

	cout << "initialized modelview matrix:\n";
	shape.getModelViewMatrix().print();

	shape.setProjectionMatrix();
	shape.setViewportMatrix();
}

Matrix4& Shape::getCameraMatrix() {
	return shape.camera;
}

int main(int argc, char *argv[]) {
	srand(time(NULL)); // set pseudo random seed

	float specular[]  = {1.0, 1.0, 1.0, 1.0};
	float shininess[] = {100.0};

	glutInit(&argc, argv); // initialize GLUT
	// open an OpenGL context with double buffering, RGB colors, and depth buffering
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// initialize window
	if (fullscreen) {
		Window::width = 1366;
		Window::height = 768;
	}
	else {
		Window::width = 512;
		Window::height = 512;
	}
	glutInitWindowSize(Window::width, Window::height); // set initial window size
	glutCreateWindow("CityScape"); // open window and set window title
	if (fullscreen) glutFullScreen();

	if(!GLEE_ARB_depth_texture || !GLEE_ARB_shadow) {
		printf("I require ARB_depth_texture and ARB_shadow extensionsn\n");
	}

	glDisable(GL_LIGHTING);
	
	glEnable(GL_DEPTH_TEST);            	      // enable depth buffering
	glClear(GL_DEPTH_BUFFER_BIT);       	      // clear depth buffer
	glClearColor(0.0, 0.0, 0.0, 0.0);   	      // set clear color to black
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // set polygon drawing mode to fill front and back of each polygon
	glEnable(GL_CULL_FACE);											// disable backface culling to render both sides of polygons
	glShadeModel(GL_SMOOTH);             	      // set shading to smooth

	glMatrixMode(GL_PROJECTION);
	gluPerspective(90, float(Window::width)/float(Window::height), 0.1, 10000);

	// Generate material properties:
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	// Install callback functions:
	glutDisplayFunc(Window::displayCallback);
	glutReshapeFunc(Window::reshapeCallback);
	glutIdleFunc(Window::idleCallback);
	
	// load textures
	shape.loadTexture("building1.ppm");
	shape.loadTexture("building3.ppm");
	shape.loadTexture("building2.ppm");
	shape.loadTexture("building4.ppm");
	shape.loadTexture("building5.ppm");
	shape.loadTexture("road2.ppm");
	
	// Process keyboard input
	glutKeyboardFunc(Window::processNormalKeys);
	glutSpecialFunc(Window::processSpecialKeys);
	//glutMouseFunc(Window::processMouseClick);
	glutPassiveMotionFunc(Window::processMouseMove);
	
	// load obj files
	shape.loadData();

	// hide mouse cursor
	glutSetCursor(GLUT_CURSOR_NONE);

	//shape.findminsmaxs();
	shape.initializeHeightMap();
	
	shape.initializeFire();
	shape.initializeSnow();
	shape.initializeRain();
	shape.initializeLeaves();

	for (int i = 0; i < 45; i++) {
		city_colors[i][0] = 0.01*(rand()%100+1);
		city_colors[i][1] = 0.01*(rand()%100+1);
		city_colors[i][2] = 0.01*(rand()%100+1);
	}

	glutMainLoop();

	return 0;
}

void Shape::drawSeason() {
	glDisable(GL_LIGHTING);

	switch (season) {
		case 0: // toggle off
			break;
		case 1: // winter
			shape.updateSnow();
			shape.drawSnow();
			break;
		case 2: // spring
			shape.updateRain();
			shape.drawRain();
			break;
		case 3: // summer
			shape.updateFire();
			shape.drawFire();
			break;
		case 4: // fall
			shape.updateLeaves();
			shape.drawLeaves();
			break;
	}
}

void Shape::initializeFire() {

	for (int i=0; i<num_fire; i++) {
		fire[i].pos = Vector3(rand()%50+50,0,0);
		fire[i].vel = Vector3(0,0,0);
		fire[i].age = rand()%100+1;
		fire[i].lifetime = rand()%100+1;
	}
}

void Shape::updateFire() {

	for (int i=0; i<num_fire; i++) {
		if (fire[i].age > fire[i].lifetime) { // reset particle when reaching end of life
			fire[i].pos = Vector3(rand()%50+50,0,0);
			fire[i].vel = Vector3(0,0,0);
			fire[i].age = 0;
		}
		else { // else next step in life
			float x = fire[i].pos.getX();
			float y = fire[i].pos.getY();
			float z = fire[i].pos.getZ();
		
			float vx = fire[i].vel.getX();
			float vy = fire[i].vel.getY();
			float vz = fire[i].vel.getZ();

			x += vx;
			y += vy;
			z += vz;

			float pos_vx = 0.01*(rand()%100+1);
			float neg_vx = -0.01*(rand()%100+1);
			bool pick_pos_vx = rand()%2;
			
			float pos_vz = 0.01*(rand()%100+1);
			float neg_vz = -0.01*(rand()%100+1);
			bool pick_pos_vz = rand()%2;

			if (!showShadows) {
				vx = 0.1* ((pick_pos_vx) ? pos_vx : neg_vx);
				vy = 0.5*(0.01*(rand()%100+1));
				vz = 0.1* ((pick_pos_vz) ? pos_vz : neg_vz);
			}
			else {
				vx = 0.1* ((pick_pos_vx) ? pos_vx : neg_vx);
        vy = 0.1*(0.01*(rand()%100+1));
				vz = 0.1* ((pick_pos_vz) ? pos_vz : neg_vz);
			}

			fire[i].pos = Vector3(x,y,z);
			fire[i].vel = Vector3(vx,vy,vz);
			if (!showShadows)
				fire[i].age += 1;
			else
				fire[i].age += 0.2;
		}
	}
}

void Shape::drawFire() {
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10);

	glBegin(GL_POINTS);
		for (int i=0; i<num_fire; i++) {
			glColor3f(1-fire[i].age/fire[i].lifetime,rand()%2*(1-fire[i].age/fire[i].lifetime),0);
			glVertex3f(fire[i].pos.getX(), fire[i].pos.getY(), fire[i].pos.getZ());
		}
	glEnd();
}

void Shape::initializeSnow() {

	for (int i=0; i<num_snow; i++) {
		snow[i].vel = Vector3(0,0,0);
		snow[i].age = rand()%100+1;
		snow[i].lifetime = rand()%100+1;
		snow[i].pos = Vector3(0.1*(rand()%201-100),10-10*(snow[i].age - snow[i].lifetime),0.1*(rand()%201-100));
	}
}

void Shape::updateSnow() {

	for (int i=0; i<num_snow; i++) {
		if (snow[i].age > snow[i].lifetime) {
			snow[i].pos = Vector3(0.1*(rand()%201-100),10,0.1*(rand()%201-100));
			snow[i].vel = Vector3(0,0,0);
			snow[i].age = 0;
		}
		else {
			float x = snow[i].pos.getX();
			float y = snow[i].pos.getY();
			float z = snow[i].pos.getZ();
		
			float vx = snow[i].vel.getX();
			float vy = snow[i].vel.getY();
			float vz = snow[i].vel.getZ();

			x += vx;
			y += vy;
			z += vz;

			float pos_vx = 0.01*(rand()%50+1);
			float neg_vx = -0.01*(rand()%50+1);
			bool pick_pos_vx = rand()%2;
			
			float pos_vz = 0.01*(rand()%50+1);
			float neg_vz = -0.01*(rand()%50+1);
			bool pick_pos_vz = rand()%2;

			if (!showShadows) {
				vx = 0.25*((pick_pos_vx) ? pos_vx : neg_vx);
				vy = -0.5*(0.01*(rand()%50+1));
				vz = 0.25*((pick_pos_vz) ? pos_vz : neg_vz);
			}
			else {
				vx = 0.1*((pick_pos_vx) ? pos_vx : neg_vx);
        vy = -0.1*(0.01*(rand()%50+1));
        vz = 0.1*((pick_pos_vz) ? pos_vz : neg_vz);
			}
			snow[i].pos = Vector3(x,y,z);
			snow[i].vel = Vector3(vx,vy,vz);
			if (!showShadows)
				snow[i].age += 0.1;
			else
				snow[i].age += 0.01;
		}
	}
}

void Shape::drawSnow() {
	glPushMatrix();
	
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5);
	
	// translate to center around player
	GLfloat curr[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, curr);
	Matrix4 mv = Matrix4(
		curr[0], curr[1], curr[2], curr[3],
		curr[4], curr[5], curr[6], curr[7],
		curr[8], curr[9], curr[10], curr[11],
		curr[12], curr[13], curr[14], curr[15]
	);
	
	Matrix4 cam =  shape.getCameraMatrix();
	Matrix4 follow_cam = mv.multiply(cam);
	
	mv.set(0,3,follow_cam.get(0,3));
	mv.set(1,3,follow_cam.get(1,3));
	mv.set(2,3,follow_cam.get(2,3));
	
	glLoadMatrixf(mv.getGLMatrix());
	
	
	glBegin(GL_POINTS);
		for (int i=0; i<num_snow; i++) {
			glColor3f(1,1,1);
			glVertex3f(snow[i].pos.getX(), snow[i].pos.getY(), snow[i].pos.getZ());
		}
	glEnd();


	glPopMatrix();
}

void Shape::initializeRain() {

	for (int i=0; i<num_rain; i++) {
		if (!showShadows)
			rain[i].vel = Vector3(0,-0.5,0);
		else
			rain[i].vel = Vector3(0,-0.05,0);
		rain[i].age = rand()%100+1;
		rain[i].lifetime = rand()%100+1;
		rain[i].pos = Vector3(0.1*(rand()%201-100),10-10*(rain[i].age/rain[i].lifetime),0.1*(rand()%201-100));
	}
}

void Shape::updateRain() {

	for (int i=0; i<num_rain; i++) {
		if (rain[i].age > rain[i].lifetime) {
			rain[i].pos = Vector3(0.1*(rand()%201-100),10,0.1*(rand()%201-100));
			if (!showShadows)
				rain[i].vel = Vector3(0,-0.5,0);
			else
				rain[i].vel = Vector3(0,-0.05,0);
			rain[i].age = 0;
		}
		else {
			float x = rain[i].pos.getX();
			float y = rain[i].pos.getY();
			float z = rain[i].pos.getZ();
		
			float vy = rain[i].vel.getY();

			y += vy;

			rain[i].pos = Vector3(x,y,z);
			rain[i].age += 0.1;
		}
	}
}

void Shape::drawRain() {
	glPushMatrix();
	
	//glEnable(GL_POINT_SMOOTH);
	//glPointSize(5);
	glLineWidth(2);
	
	// translate to center around player
	GLfloat curr[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, curr);
	Matrix4 mv = Matrix4(
		curr[0], curr[1], curr[2], curr[3],
		curr[4], curr[5], curr[6], curr[7],
		curr[8], curr[9], curr[10], curr[11],
		curr[12], curr[13], curr[14], curr[15]
	);
	
	Matrix4 cam =  shape.getCameraMatrix();
	Matrix4 follow_cam = mv.multiply(cam);
	
	mv.set(0,3,follow_cam.get(0,3));
	mv.set(1,3,follow_cam.get(1,3));
	mv.set(2,3,follow_cam.get(2,3));
	
	glLoadMatrixf(mv.getGLMatrix());
	
	
	glBegin(GL_LINES);
		for (int i=0; i<num_rain; i++) {
			glColor3f(0,0,1);
			glVertex3f(rain[i].pos.getX(), rain[i].pos.getY()-0.2, rain[i].pos.getZ());
			glVertex3f(rain[i].pos.getX(), rain[i].pos.getY()+0.2, rain[i].pos.getZ());
		}
	glEnd();


	glPopMatrix();
}

void Shape::initializeLeaves() {

	for (int i=0; i<num_leaves; i++) {
		leaves[i].pos = Vector3(-10,0.01*(rand()%10000),0.1*(rand()%201-100));
		leaves[i].vel = Vector3(0,0,0);
		leaves[i].age = rand()%100+1;
		leaves[i].lifetime = rand()%100+1;
		leaves[i].color = Vector4(0.01*(rand()%100+1),0.01*(rand()%100),0,1);
	}
}

void Shape::updateLeaves() {

	for (int i=0; i<num_leaves; i++) {
		if (leaves[i].age > leaves[i].lifetime) {
			leaves[i].pos = Vector3(-10,-0.1*(rand()%201-100),0.1*(rand()%201-100));
			leaves[i].vel = Vector3(0,0,0);
			leaves[i].age = 0;
			leaves[i].color = Vector4(0.01*(rand()%100+1),0.01*(rand()%100),0,1);
		}
		else {
			float x = leaves[i].pos.getX();
			float y = leaves[i].pos.getY();
			float z = leaves[i].pos.getZ();
		
			float vx = leaves[i].vel.getX();
			float vy = leaves[i].vel.getY();
			float vz = leaves[i].vel.getZ();

			x += vx;
			y += vy;
			z += vz;

			float pos_vy = 0.01*(rand()%50+1);
			float neg_vy = -0.01*(rand()%50+1);
			bool pick_pos_vy = rand()%2;
			
			float pos_vz = 0.01*(rand()%50+1);
			float neg_vz = -0.01*(rand()%50+1);
			bool pick_pos_vz = rand()%2;

			if (!showShadows) {
				vx = 1*(0.01*(rand()%50+1));
				vy = 0.5*((pick_pos_vy) ? pos_vy : neg_vy);
				vz = 0.5*((pick_pos_vz) ? pos_vz : neg_vz);
			}
			else {
				vx = 0.1*(0.01*(rand()%50+1));
        vy = 0.1*((pick_pos_vy) ? pos_vy : neg_vy);
        vz = 0.1*((pick_pos_vz) ? pos_vz : neg_vz);
			}

			leaves[i].pos = Vector3(x,y,z);
			leaves[i].vel = Vector3(vx,vy,vz);
			if (!showShadows)
				leaves[i].age += 0.5;
			else
				leaves[i].age += 0.05;
		}
	}
}

void Shape::drawLeaves() {
	glPushMatrix();
	
	glEnable(GL_POINT_SMOOTH);
	glPointSize(15);
	
	// translate to center around player
	GLfloat curr[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, curr);
	Matrix4 mv = Matrix4(
		curr[0], curr[1], curr[2], curr[3],
		curr[4], curr[5], curr[6], curr[7],
		curr[8], curr[9], curr[10], curr[11],
		curr[12], curr[13], curr[14], curr[15]
	);
	
	Matrix4 cam =  shape.getCameraMatrix();
	Matrix4 follow_cam = mv.multiply(cam);
	
	mv.set(0,3,follow_cam.get(0,3));
	mv.set(1,3,follow_cam.get(1,3));
	mv.set(2,3,follow_cam.get(2,3));
	
	glLoadMatrixf(mv.getGLMatrix());
	
	
	glBegin(GL_POINTS);
		for (int i=0; i<num_leaves; i++) {
			glColor3f(leaves[i].color.getX(),leaves[i].color.getY(),leaves[i].color.getZ());
			glVertex3f(leaves[i].pos.getX(), leaves[i].pos.getY(), leaves[i].pos.getZ());
		}
	glEnd();


	glPopMatrix();
}

void Window::drawCWSkyBox() {
	glPushMatrix();
		/*
		// code to make skybox move with camera
		GLfloat curr[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, curr);
		Matrix4 mv = Matrix4(
			curr[0], curr[1], curr[2], curr[3],
			curr[4], curr[5], curr[6], curr[7],
			curr[8], curr[9], curr[10], curr[11],
			curr[12], curr[13], curr[14], curr[15]
		);
		mv.identity();

		Matrix4 cam =  shape.getCameraMatrix();
		Matrix4 follow_cam = mv.multiply(cam);

		mv.set(0,3,follow_cam.get(0,3));
		mv.set(1,3,follow_cam.get(1,3));
		mv.set(2,3,follow_cam.get(2,3));

		follow_cam.print();
		glLoadMatrixf(follow_cam.getGLMatrix());
		*/
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		glDisable(GL_LIGHTING);

		glBegin(GL_QUADS);
			// IMPORTANT =============================================================
			// vertices must be in CW order for each face
			// or they will be considered backfacing b/c of glEnable(GL_CULL_FACE)
			// =======================================================================

			glColor3f(0.5*(lightPos.getY()/1000.0f)+0.1, (lightPos.getY()/1000.0f), (lightPos.getY()/1000.0f));

			// Top face (y = 5000.0f)
			glVertex3f( 5000.0f, 5000.0f, -5000.0f);
			glVertex3f( 5000.0f, 5000.0f,  5000.0f);
			glVertex3f(-5000.0f, 5000.0f,  5000.0f);
			glVertex3f(-5000.0f, 5000.0f, -5000.0f);
 
			// Bottom face (y = -5000.0f)
			glVertex3f( 5000.0f, -5000.0f,  5000.0f);
			glVertex3f( 5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f,  5000.0f);
 
			// Front face  (z = 5000.0f)
			glVertex3f( 5000.0f,  5000.0f, 5000.0f);
			glVertex3f( 5000.0f, -5000.0f, 5000.0f);
			glVertex3f(-5000.0f, -5000.0f, 5000.0f);
			glVertex3f(-5000.0f,  5000.0f, 5000.0f);

			// Back face (z = -5000.0f)
			glVertex3f( 5000.0f, -5000.0f, -5000.0f);
			glVertex3f( 5000.0f,  5000.0f, -5000.0f);
			glVertex3f(-5000.0f,  5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
 
			// Left face (x = -5000.0f)
			glVertex3f(-5000.0f,  5000.0f,  5000.0f);
			glVertex3f(-5000.0f, -5000.0f,  5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f,  5000.0f, -5000.0f);
 
			// Right face (x = 5000.0f)
			glVertex3f(5000.0f,  5000.0f, -5000.0f);
			glVertex3f(5000.0f, -5000.0f, -5000.0f);
			glVertex3f(5000.0f, -5000.0f,  5000.0f);
			glVertex3f(5000.0f,  5000.0f,  5000.0f);
		 glEnd();

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

void Window::drawCCWSkyBox() {
	glPushMatrix();
		/*
		// code to make skybox move with camera
		GLfloat curr[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, curr);
		Matrix4 mv = Matrix4(
			curr[0], curr[1], curr[2], curr[3],
			curr[4], curr[5], curr[6], curr[7],
			curr[8], curr[9], curr[10], curr[11],
			curr[12], curr[13], curr[14], curr[15]
		);
		mv.identity();

		Matrix4 cam =  shape.getCameraMatrix();
		Matrix4 follow_cam = mv.multiply(cam);

		mv.set(0,3,follow_cam.get(0,3));
		mv.set(1,3,follow_cam.get(1,3));
		mv.set(2,3,follow_cam.get(2,3));

		follow_cam.print();
		glLoadMatrixf(follow_cam.getGLMatrix());
		*/
		
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		
		glDisable(GL_LIGHTING);

		glBegin(GL_QUADS);
			// IMPORTANT =============================================================
			// these vertices are in CCW order
			// they will be considered backfacing b/c of glEnable(GL_CULL_FACE)
			// =======================================================================

			// Top face (y = 5000.0f)
			glColor3f(0, 1, 0);     // Green
			glVertex3f( 5000.0f, 5000.0f, -5000.0f);
			glVertex3f(-5000.0f, 5000.0f, -5000.0f);
			glVertex3f(-5000.0f, 5000.0f,  5000.0f);
			glVertex3f( 5000.0f, 5000.0f,  5000.0f);
 
			// Bottom face (y = -5000.0f)
			glColor3f(1, 0.5, 0);     // Orange
			glVertex3f( 5000.0f, -5000.0f,  5000.0f);
			glVertex3f(-5000.0f, -5000.0f,  5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
			glVertex3f( 5000.0f, -5000.0f, -5000.0f);
 
			// Front face  (z = 5000.0f)
			glColor3f(1, 0, 0);     // Red
			glVertex3f( 5000.0f,  5000.0f, 5000.0f);
			glVertex3f(-5000.0f,  5000.0f, 5000.0f);
			glVertex3f(-5000.0f, -5000.0f, 5000.0f);
			glVertex3f( 5000.0f, -5000.0f, 5000.0f);
 
			// Back face (z = -5000.0f)
			glColor3f(1, 1, 0);     // Yellow
			glVertex3f( 5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f,  5000.0f, -5000.0f);
			glVertex3f( 5000.0f,  5000.0f, -5000.0f);
 
			// Left face (x = -5000.0f)
			glColor3f(0, 0, 1);     // Blue
			glVertex3f(-5000.0f,  5000.0f,  5000.0f);
			glVertex3f(-5000.0f,  5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f, -5000.0f);
			glVertex3f(-5000.0f, -5000.0f,  5000.0f);
 
			// Right face (x = 5000.0f)
			glColor3f(1, 0, 1);     // Magenta
			glVertex3f(5000.0f,  5000.0f, -5000.0f);
			glVertex3f(5000.0f,  5000.0f,  5000.0f);
			glVertex3f(5000.0f, -5000.0f,  5000.0f);
			glVertex3f(5000.0f, -5000.0f, -5000.0f);
		 glEnd();

		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}


void Shape::findminsmaxs() {
	float minx = FLT_MAX;
	float miny = FLT_MAX;
	float minz = FLT_MAX;
	float maxx = FLT_MIN;
	float maxy = FLT_MIN;
	float maxz = FLT_MIN;

	for (int i = 0; i < city_nVerts*3; i+=3) {
		if (city_vertices[i] < minx) {
			minx = city_vertices[i];
		}
		if (city_vertices[i+1] < miny) {
			miny = city_vertices[i+1];
		}
		if (city_vertices[i+2] < minz) {
			minz = city_vertices[i+2];
		}
		if (city_vertices[i] > maxx) {
			maxx = city_vertices[i];
		}
		if (city_vertices[i+1] > maxy) {
			maxy = city_vertices[i+1];
		}
		if (city_vertices[i+2] > maxz) {
			maxz = city_vertices[i+2];
		}
	}
	cout << minx << '\n';
	cout << miny << '\n';
	cout << minz << '\n';
	cout << maxx << '\n';
	cout << maxy << '\n';
	cout << maxz << '\n';
}

void Shape::gravity(int x, int y) {
	int needed_height = heightMap[x+x_shift][y+z_shift];
	/*
	//uncomment if you want to fall to ground automatically
	if (heightMapEnabled == false) {
		updateCameraMatrix(0, -1*e.getY(), 0);
		curr_height = 0;
		falling = false;
	}
	*/
	if (heightMapEnabled == true) {
		if (curr_height - velocity <= needed_height) {
			updateCameraMatrix(0, needed_height-curr_height, 0);
			falling = false;
			curr_height = needed_height;
		}
		else {
			updateCameraMatrix(0, -1.0*velocity, 0);
			curr_height-=velocity;
		}
	}
}

void Shape::initializeHeightMap() {
	float minx;
	float maxx;
	float minz;
	float maxz;
	float miny;
	float maxy;
	float tmpminx;
	float tmpminy;
	float tmpminz;
	float tmpmaxx;
	float tmpmaxy;
	float tmpmaxz;

	//int arr[1][2] = {{0, 1}};
	int arr[4][2] = {{0, 1}, {0, 3}, {1, 3}, {2, 3}};  
	//int arr[5][2] = {{0, 1}, {0, 2}, {0, 3}, {1, 3}, {2, 3}};   
	if (!init_heightMap) {
		for (int i = 0; i < city_nVerts*3; i++) {
			city_vertices[i]/=10.0;
		}
	}

	// TODO: fix min/max (only takes 2 args)
	for (int i = 0; i < city_nVerts*3; i+=12) { // else 84 or  126
		tmpminx = min(city_vertices[i], city_vertices[i+3], city_vertices[i+6], city_vertices[i+9]); 
		tmpmaxx = max(city_vertices[i], city_vertices[i+3], city_vertices[i+6], city_vertices[i+9]); 
		tmpminy = min(city_vertices[i+1], city_vertices[i+4], city_vertices[i+7], city_vertices[i+10]); 
		tmpmaxy = max(city_vertices[i+1], city_vertices[i+4], city_vertices[i+7], city_vertices[i+10]); 
		tmpminz = min(city_vertices[i+2], city_vertices[i+5], city_vertices[i+8], city_vertices[i+11]); 
		tmpmaxz = max(city_vertices[i+2], city_vertices[i+5], city_vertices[i+8], city_vertices[i+11]);

		if (tmpminx < 0) {
			minx = -1*floor((-1.0*tmpminx)+0.5);
		}
		else {
			minx = floor(tmpminx+0.5);
		}

		if (tmpminz < 0) {
			minz = -1*floor((-1.0*tmpminz)+0.5);
		}
		else {
			minz = floor(tmpminz+0.5);
		}

		if (tmpmaxx < 0) {
			maxx = -1*floor((-1.0*tmpmaxx)+0.5);
		}
		else {
			maxx = floor(tmpmaxx+0.5);
		}

		if (tmpmaxz < 0) {
			maxz = -1*floor((-1.0*tmpmaxz)+0.5);
		}
		else {
			maxz = floor(tmpmaxz+0.5);
		}

		if (tmpminy < 0) {
			miny = -1*floor((-1.0*tmpminy)+0.5);
		}
		else {
			miny = floor(tmpminy+0.5);
		}

		if (tmpmaxy < 0) {
			maxy = -1*floor((-1.0*tmpmaxy)+0.5);
		}
		else {
			maxy = floor(tmpmaxy+0.5);
		}

		for (int j = minx; j < maxx; j++) {
			for (int k = minz; k < maxz; k++) {
				if (maxy > heightMap[j+x_shift][k+z_shift]) {
					heightMap[j+x_shift][k+z_shift] = maxy;
				}
			}
		}
	}

	
	// player is 4 feet "tall"
	for (int i = 0; i < 1019; i++) {
		for (int j = 0; j < 787; j++) {
				heightMap[i][j]+=4;
		}
	}

	init_heightMap = true;
}

void Shape::updateLookAtVector() {
	Matrix4 tmp;

	tmp = Matrix4(d.getX()-e.getX(), d.getY()-e.getY(), d.getZ()-e.getZ(), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tmp.transpose();
	float rad_anglex_change = 3.14*(anglex_change/10)/180.0;
	tmp.rotateY(rad_anglex_change);
	d = Vector3(tmp.get(0, 0)+e.getX(), tmp.get(0, 1)+e.getY(), tmp.get(0, 2)+e.getZ());

	tmp = Matrix4(d.getX()-e.getX(), d.getY()-e.getY(), d.getZ()-e.getZ(), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tmp.transpose();
	float rad_angley_change = 3.14*(angley_change/10)/180.0;
	tmp.rotateX(rad_angley_change);
	d = Vector3(tmp.get(0, 0)+e.getX(), tmp.get(0, 1)+e.getY(), tmp.get(0, 2)+e.getZ());

	tmp = Matrix4(up.getX(), up.getY(), up.getZ(), 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	tmp.transpose();
	tmp.rotateWindowX(rad_angley_change);
	up = Vector3(tmp.get(0, 0), tmp.get(0, 1), tmp.get(0, 2));
	
	updateCameraMatrix(0, 0, 0);
}

void Shape::drawHouse() {
	int tris = 0;
	glBegin(GL_QUADS);
	for (int i=0; i<7; i++) {
		glColor3f(house_colors[12*i], house_colors[12*i+1], house_colors[12*i+2]);
		glNormal3f(house_indices[i*6], house_indices[i*6+1], house_indices[i*6+2]);
		for (int v=0; v<4; v++) {
			glVertex3f(house_vertices[12*i+3*v], house_vertices[(12*i)+(3*v)+1], house_vertices[(12*i)+(3*v)+2]);
		}
		glNormal3f(house_indices[i*6+3], house_indices[i*6+4], house_indices[i*6+5]);
	}
	glEnd();
}

void Window::processNormalKeys(unsigned char key, int x, int y) {
	Vector3 tmp_vec = Vector3(d.getX()-e.getX(), d.getY()-e.getY(), d.getZ()-e.getZ()); // second arg could be 0 i think
	tmp_vec.normalize();
	Matrix4 tmpa = Matrix4(tmp_vec);
	Matrix4 tmpd = Matrix4(tmpa);
	tmpa.transpose();
	tmpd.transpose();
	float rad_anglea_change = -3.14*0.5;
	float rad_angled_change = 3.14*0.5;
	tmpa.rotateY(rad_anglea_change);
	tmpd.rotateY(rad_angled_change);

	Vector3 a_vec = Vector3(tmpa.get(0, 0), tmpa.get(0, 1), tmpa.get(0, 2));
	Vector3 d_vec = Vector3(tmpd.get(0, 0), tmpd.get(0, 1), tmpd.get(0, 2));
	int tmpx;
	int tmpz;

	moved = false;
	switch (key)  {
		case 27: // Escape key
			glutDestroyWindow(glutGetWindow());
			exit(0);
			break;
		case 'e': // speed up movement
			walk_x_factor*=(3.0/2.0);
			walk_y_factor*=(3.0/2.0);
			walk_z_factor*=(3.0/2.0);
			break;
		case 'q': // slow down movement
			walk_x_factor*=(2.0/3.0);
			walk_y_factor*=(2.0/3.0);
			walk_z_factor*=(2.0/3.0);
			break;
		case 'y': // toggle sun rotation
			toggle_freeze = !toggle_freeze;
			break;
		case 't': // toggle speeding up sun
			speed_up = !speed_up;
			break;
		case 'h': // toggle height map
			heightMapEnabled = !heightMapEnabled;
			if (heightMapEnabled) {
			heightmaptoggledon = true;
			}
			else {
				heightmaptoggledoff = true;
			}
			break;
		case 'c': // toggle always centering mouse pointer
			warp = !warp;
			break;
		case 'o': // toggle shadows
			showShadows = !showShadows;
			break;
		case 'w': // walk forward
			if (!falling || !heightMapEnabled) {
				moved = true;
				if (!heightMapEnabled) {
						if (e.getX()+(tmp_vec.getX()*walk_x_factor) < 0) {
							tmpx = -1*floor((-1.0*(e.getX()+(tmp_vec.getX()*walk_x_factor)))+0.5);
						}
						else {
							tmpx = floor(e.getX()+(tmp_vec.getX()*walk_x_factor)+0.5);
						}

						if (e.getZ()+(tmp_vec.getZ()*walk_z_factor) < 0) {
							tmpz = -1*floor((-1.0*(e.getZ()+(tmp_vec.getZ()*walk_z_factor)))+0.5);
						}
						else {
							tmpz = floor(e.getZ()+(tmp_vec.getZ()*walk_z_factor)+0.5);
						}
						
						if (!(heightMap[tmpx+x_shift][tmpz+z_shift] > curr_height)) {

							shape.updateCameraMatrix(tmp_vec.getX()*walk_x_factor,0,tmp_vec.getZ()*walk_z_factor);
						}
						else if (printedfirst) {
							collisiondetected = true;
						}
						else {
							printedfirst = true;
						}
				}
				else {
					shape.updateCameraMatrix(tmp_vec.getX()*walk_x_factor,0,tmp_vec.getZ()*walk_z_factor);
				}
			}
			break;
		case 's': // walk backwards
			if (!falling || !heightMapEnabled) {
				moved = true;
				if (!heightMapEnabled) {
					if (e.getX()-(tmp_vec.getX()*walk_x_factor) < 0) {
						tmpx = -1*floor((-1.0*(e.getX()-(tmp_vec.getX()*walk_x_factor)))+0.5);
					}
					else {
						tmpx = floor(e.getX()-(tmp_vec.getX()*walk_x_factor)+0.5);
					}

					if (e.getZ()-(tmp_vec.getZ()*walk_z_factor) < 0) {
						tmpz = -1*floor((-1.0*(e.getZ()-(tmp_vec.getZ()*walk_z_factor)))+0.5);
					}
					else {
						tmpz = floor(e.getZ()-(tmp_vec.getZ()*walk_z_factor)+0.5);
					}

					if (!(heightMap[tmpx+x_shift][tmpz+z_shift] > curr_height)) {

						shape.updateCameraMatrix(-1.0*tmp_vec.getX()*walk_x_factor,0,-1.0*tmp_vec.getZ()*walk_z_factor);
					}
					else if (printedfirst) {
						collisiondetected = true;
					}
					else {
							printedfirst = true;
						}
				}
				else {
					shape.updateCameraMatrix(-1.0*tmp_vec.getX()*walk_x_factor,0,-1.0*tmp_vec.getZ()*walk_z_factor);
				}
			}
			break;
		case 'a': // strafe left
			if (!falling || !heightMapEnabled) {
				moved = true;
				//shape.updateCameraMatrix(a_vec.getX()*walk_x_factor,0,a_vec.getZ()*walk_z_factor);
				if (!heightMapEnabled) {
						if (e.getX()+(a_vec.getX()*walk_x_factor) < 0) {
							tmpx = -1*floor((-1.0*(e.getX()+(a_vec.getX()*walk_x_factor)))+0.5);
						}
						else {
							tmpx = floor(e.getX()+(a_vec.getX()*walk_x_factor)+0.5);
						}

						if (e.getZ()+(a_vec.getZ()*walk_z_factor) < 0) {
							tmpz = -1*floor((-1.0*(e.getZ()+(a_vec.getZ()*walk_z_factor)))+0.5);
						}
						else {
							tmpz = floor(e.getZ()+(a_vec.getZ()*walk_z_factor)+0.5);
						}
						
						if (!(heightMap[tmpx+x_shift][tmpz+z_shift] > curr_height)) {

							shape.updateCameraMatrix(a_vec.getX()*walk_x_factor,0,a_vec.getZ()*walk_z_factor);
						}
						else if (printedfirst) {
							collisiondetected = true;
						}
						else {
							printedfirst = true;
						}
				}
				else {
					shape.updateCameraMatrix(a_vec.getX()*walk_x_factor,0,a_vec.getZ()*walk_z_factor);
				}
			}
			break;
		case 'd': // strafe right
			if (!falling || !heightMapEnabled) {
				moved = true;
				//shape.updateCameraMatrix(d_vec.getX()*walk_x_factor,0,d_vec.getZ()*walk_z_factor);
				if (!heightMapEnabled) {
						if (e.getX()+(d_vec.getX()*walk_x_factor) < 0) {
							tmpx = -1*floor((-1.0*(e.getX()+(d_vec.getX()*walk_x_factor)))+0.5);
						}
						else {
							tmpx = floor(e.getX()+(d_vec.getX()*walk_x_factor)+0.5);
						}

						if (e.getZ()+(d_vec.getZ()*walk_z_factor) < 0) {
							tmpz = -1*floor((-1.0*(e.getZ()+(d_vec.getZ()*walk_z_factor)))+0.5);
						}
						else {
							tmpz = floor(e.getZ()+(d_vec.getZ()*walk_z_factor)+0.5);
						}

						if (!(heightMap[tmpx+x_shift][tmpz+z_shift] > curr_height)) {
							shape.updateCameraMatrix(d_vec.getX()*walk_x_factor,0,d_vec.getZ()*walk_z_factor);
						}
						else if (printedfirst) {
							collisiondetected = true;
						}
						else {
							printedfirst = true;
						}
				}
				else {
					shape.updateCameraMatrix(d_vec.getX()*walk_x_factor,0,d_vec.getZ()*walk_z_factor);
				}
			}
			break;
		case 'r': // fly up
			if (!falling || !heightMapEnabled) {
				moved = true;
				shape.updateCameraMatrix(0,walk_y_factor,0);
				curr_height+=walk_y_factor;
			}
			break;
		case 'f': // fly down
			if (!falling || !heightMapEnabled) {
				moved = true;
				shape.updateCameraMatrix(0,-1.0*walk_y_factor,0);
				curr_height-=walk_y_factor;
			}
			break;
		case 'g': // toggle god mode
			moved = true;
			godMode = !godMode;
			if (!godMode) {
				heightMapEnabled = true;
				
				e = Vector3(75, 4, 0); // origin
				d = Vector3(74, 4, 0); // look at
				up = Vector3(0, 1, 0); // up
				shape.updateCameraMatrix(0,0,0);
				
				curr_height = 4;
				walk_x_factor = 1.0;
				walk_y_factor = 1.0;
				walk_z_factor = 1.0;
			}
			else {
				heightMapEnabled = false;
				
				e = Vector3(200,1250,0);
				d = Vector3(200,0,0);
				up = Vector3(0,0,-1);
				shape.updateCameraMatrix(0,0,0);
				
				curr_height = 1250;
				walk_x_factor = 100.0;
				walk_y_factor = 100.0;
				walk_z_factor = 100.0;
			}
			break;
		case 'n': // toggle seasons
			if (season == 4)
				season = 0;
			else
				season++;
	}
	shape.updateModelViewMatrix();

	if (e.getX() < 0) {
		tmpx = -1*floor((-1.0*e.getX())+0.5);
	}
	else {
		tmpx = floor(e.getX()+0.5);
	}			
	if (e.getZ() < 0) {
		tmpz = -1*floor((-1.0*e.getZ())+0.5);
	}
	else {
		tmpz = floor(e.getZ()+0.5);
	}

	if (falling == false) {
		if (heightMap[tmpx+x_shift][tmpz+z_shift] != curr_height) {
			if (heightMap[tmpx+x_shift][tmpz+z_shift] > curr_height) {
				shape.updateCameraMatrix(0, heightMap[tmpx+x_shift][tmpz+z_shift] - e.getY(), 0);
				curr_height = heightMap[tmpx+x_shift][tmpz+z_shift];
			}
			else if (heightMap[tmpx+x_shift][tmpz+z_shift] < curr_height) {
					falling = true;
					shape.gravity(tmpx, tmpz);
			}
		}
	}
	
}

void Window::processSpecialKeys(int key, int x, int y) {
	shape.getModelMatrix().identity();
	shape.getCameraMatrix().identity();

	switch (key) {
		case GLUT_KEY_F1: // city scene
			shape_key = 1;
			godMode = false;
			
			// reinitialize heightmap and toggle on
			shape.initializeHeightMap();
			heightMapEnabled = true;
			heightmaptoggledon = true;

			e = Vector3(75, 4, 0); // origin
			d = Vector3(74, 4, 0); // look at
			up = Vector3(0, 1, 0); // up
			curr_height = 4;
			shape.updateCameraMatrix(0,0,0);

			break;
		case GLUT_KEY_F2: // house scene
			shape_key = 2;
			godMode = false;
			
			// reset heightmap and toggle off
			heightMapEnabled = false;
			heightmaptoggledoff = true;
			for (int i=0; i<heightMap.size(); i++) {
				for (int j=0; j<heightMap[0].size(); j++) {
					heightMap[i][j] = 4;
				}
			}

			e = Vector3(0, 4, 10);
			d = Vector3(0, 4, 9);
			up = Vector3(0, 1, 0);
			curr_height = 4;
			shape.updateCameraMatrix(0,0,0);
			
			break;
	}

	shape.updateModelViewMatrix();
}

void Window::processMouseMove(int x, int y) {
	if (x_mouse != x) {
		anglex_change = float(x_mouse-x)/anglex_factor;
	}
	if (y_mouse != y) {
		angley_change = float(y_mouse-y)/angley_factor;
		float tmp = angley;
		angley+=angley_change;
		
		if (angley > 90.0) {
			angley = 90.0;
			angley_change = 0.0;
		}
		if (angley < -90.0) {
			angley = -90.0;
			angley_change = 0.0;
		}
	}
	
	if (warp == true) {
		if (x != Window::width/2 || y != Window::height/2) {
			glutWarpPointer(Window::width/2, Window::height/2);
		}
	}

	x_mouse = x;
	y_mouse = y;
}
		
