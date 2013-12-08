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

//TIMER timer;
//float angle=timer.GetTime()/10;
static bool showShadows = true;
FPS_COUNTER fpsCounter;
const int shadowMapSize=512;
GLuint shadowMapTexture;
Matrix4 lightProjectionMatrix, lightViewMatrix, cameraProjectionMatrix, cameraViewMatrix;
Vector3 lightPos = Vector3(0, 0, 0);

static bool warp = false;
static float speed_up = false;
static float sun_speed = 0.1;
static bool shadowMode = false;
static bool godMode = false;
static float city_scale = 0.1;

// for camera matrix
static Vector3 e = Vector3(0, 4, 10); // origin
static Vector3 d = Vector3(0, 4, 9); // look at
static Vector3 up = Vector3(0, 1, 0); // up
static float anglex = 0.0;
static float angley = 0.0;
static float anglex_change = 0.0;
static float angley_change = 0.0;
static float anglex_factor = 1.0;
static float angley_factor = 1.0;
//static float rad_angle_change = -3.14*angle_change/180.0;
static float walk_x_factor = 2.0;
static float walk_y_factor = 2.0;
static float walk_z_factor = 2.0;
static vector<vector<int>> heightMap;


static int selected_point = -1;
static int threshold = 50;
static const int degs = 5;
static float asdf =-3.14*degs/180;
static float controls[7][3] = {
	{0, 2, 0}, {0.5,1.5,0}, {1.5, 1, 0}, {0.75, 0.5, 0.0}, 
	{1.5,0.25,0}, {1,0,0}, {0.5, -0.25, 0.0}};
static const float max_range = 1.0;
static const int num_points = 20;
static float inter[num_points][3];
static float inter2[num_points][3];
static float inter_ymax = FLT_MIN;
static float inter_ymin = FLT_MAX;
static float normal[num_points][3];
static float normal2[num_points][3];
static float rotated_inter[360/degs][num_points][3];
static float rotated_inter2[360/degs][num_points][3];
static float rotated_normal[360/degs][num_points][3];
static float rotated_normal2[360/degs][num_points][3];

static bool TURN_LIGHTS_ON = true;

static bool DEBUGGER = false;
static bool DEBUG_LOAD_OBJS = false;
static bool DEBUG_DRAW_LIGHTS = true;

static Shape shape;
static double spin_angle = 0.000;
static int shape_key = 8;

static bool red = false;

static bool left_clicked = false;
static bool right_clicked = false;
static int x_mouse;
static int y_mouse;

GLfloat no_mat[] = { 0.0, 0.0, 0.0, 1.0 };
GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
GLfloat mat_ambient_color[] = { 0.8, 0.8, 0.2, 1.0 };
GLfloat mat_diffuse[] = { 0.1, 0.5, 0.8, 1.0 };
GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
GLfloat no_shininess[] = { 0.0 };
GLfloat low_shininess[] = { 5.0 };
GLfloat high_shininess[] = { 128 };
GLfloat mat_emission[] = {0.3, 0.2, 0.2, 0.0};

GLfloat d_position[] = {sqrt(100.0), sqrt(100.0), 0, 0};
GLfloat p_position[] = {0, -5, 0, 1};
GLfloat s_position[] = {-10, 0, 0, 1};
GLfloat s_direction[] = {1, 0, 0};
static Matrix4 sun_mv = Matrix4();
static float theta = 0;

static bool shader_toggle = false;
static bool toggle1 = false;
static bool toggle2 = false;
static bool toggle3 = false;
static bool toggle_freeze = false;
static bool toggle_frus = false;
static bool toggle_tex = false;

int Window::width  = 512;   // set window width in pixels here
int Window::height = 512;   // set window height in pixels here
static bool fullscreen = false;

//MatrixTransform army;
static int army_size = 5;
vector<vector<MatrixTransform*>> robot(army_size, vector<MatrixTransform*> (army_size));
static float spin_leftarm = 3.14/2;
static float leftarm_dir = -4;
static float spin_rightarm = 0;
static float rightarm_dir = 4;

static float spin_leftleg = 0;
static float leftleg_dir = -2;
static float spin_rightleg = 0;
static float rightleg_dir = 2;

clock_t Start = clock();
static int noculltimer = 0;
static int culltimer = 0;

//----------------------------------------------------------------------------
// Callback method called when system is idle.
void Window::idleCallback(void)
{
  //shape.spin(spin_angle);
  displayCallback();
}

//----------------------------------------------------------------------------
// Callback method called when window is resized.

void Window::reshapeCallback(int w, int h)
{
	glPushMatrix();
	glLoadIdentity();
    gluPerspective(90.0, Window::width/Window::height, 0.1, 1000);
	glGetFloatv(GL_MODELVIEW_MATRIX, shape.getModelViewMatrix().getGLMatrix());
	glPopMatrix();
/*	
	if (!fullscreen) {
		width = w;
		height = h;
	}
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glFrustum(-10.0, 10.0, -10.0, 10.0, 10, 1000.0); // set perspective projection viewing frustum
  glTranslatef(0, 0, -20);
*/
  
}


void Shape::loadTexture()
{
  GLuint texture[1];     // storage for one texture
  int twidth, theight;   // texture width/height [pixels]
  unsigned char* tdata;  // texture pixel data
  // Load image file
  tdata = loadPPM("sky1.ppm", twidth, theight);
  if (tdata==NULL) return;
  
  // Create ID for texture
  glGenTextures(1, &texture[0]);   

  // Set this texture to be the one we are working with
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  
  // Generate the texture
  glTexImage2D(GL_TEXTURE_2D, 0, 3, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
  
  // Set bi-linear filtering for both minification and magnification
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
  //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); 
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

unsigned char* Shape::loadPPM(const char* filename, int& width, int& height)
{
	const int BUFSIZE = 128;
	FILE* fp;
	unsigned int read;
	unsigned char* rawData;
	char buf[3][BUFSIZE];
	char* retval_fgets;
	size_t retval_sscanf;

	if ( (fp=fopen(filename, "rb")) == NULL)
	{
		std::cerr << "error reading ppm file, could not locate " << filename << std::endl;
		width = 0;
		height = 0;
		return NULL;
	}

	// Read magic number:
	retval_fgets = fgets(buf[0], BUFSIZE, fp);

	// Read width and height:
	do
	{
		retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');
	retval_sscanf=sscanf(buf[0], "%s %s", buf[1], buf[2]);
	width  = atoi(buf[1]);
	height = atoi(buf[2]);

	// Read maxval:
	do
	{
	  retval_fgets=fgets(buf[0], BUFSIZE, fp);
	} while (buf[0][0] == '#');

	// Read image data:
	rawData = new unsigned char[width * height * 3];
	read = fread(rawData, width * height * 3, 1, fp);
	fclose(fp);
	if (read != 1)
	{
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

  double fov = 3.141592654*90.0/180.0;
  double aspect = Window::width/Window::height;
  double nearv = 0.1;
  double farv = 1000.0;

  projection = 
	  Matrix4(1.0/(aspect), 0, 0, 0,
	          0, 1.0, 0, 0,
			  0, 0, (nearv+farv)/(nearv-farv), 2*nearv*farv/(nearv-farv),
			  0, 0, -1, 0);
  projection.transpose();
  //projection.print();
  //getProjectionMatrix().translate(0, 0, -20);
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
	//shape.getModelMatrix().print();
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
void Window::displayCallback(void)
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear color and depth buffers
  
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(shape.getModelViewMatrix().getGLMatrix());
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(shape.getProjectionMatrix().getGLMatrix());
	glMatrixMode(GL_MODELVIEW);
	/*
	if (speed_up == true) {
		
	GLfloat temp_mv[16];
					glGetFloatv(GL_PROJECTION_MATRIX, temp_mv);
					sun_mv = Matrix4(
						temp_mv[0], temp_mv[1], temp_mv[2], temp_mv[3],
						temp_mv[4], temp_mv[5], temp_mv[6], temp_mv[7],
						temp_mv[8], temp_mv[9], temp_mv[10], temp_mv[11],
						temp_mv[12], temp_mv[13], temp_mv[14], temp_mv[15]
					);
					sun_mv.transpose();
					sun_mv.print();
		
		//shape.getProjectionMatrix().print();
	}
	*/
	//shape.getModelViewMatrix().print();
	//shape.getProjectionMatrix().print();

	Material cube = Material(GL_FRONT_AND_BACK);
	Material dragon = Material(GL_FRONT);
	Material bunny = Material(GL_FRONT_AND_BACK);
	Material sandal = Material(GL_FRONT_AND_BACK);

	//drawSkyBox();
	
	//shape.viewFrustumCulling();
	switch (shape_key) {
		case 1:
			if (TURN_LIGHTS_ON) {
				cube.setAmbient(no_mat);
				cube.setDiffuse(mat_diffuse);
				cube.setSpecular(mat_specular);
				cube.setShininess(high_shininess);
				cube.setEmission(no_mat);
			}

			break;

		case 2: // dragon
			dragon.setAmbient(no_mat);
			dragon.setDiffuse(mat_diffuse);
			dragon.setSpecular(mat_specular);
			dragon.setShininess(high_shininess);
			dragon.setEmission(no_mat);

			drawShape(dragon_nVerts, dragon_vertices, dragon_normals);
			break;
		case 3: // bunny
			bunny.setAmbient(no_mat);
			bunny.setDiffuse(no_mat);
			bunny.setSpecular(mat_specular);
			bunny.setShininess(high_shininess);
			bunny.setEmission(no_mat);

			drawShape(bunny_nVerts, bunny_vertices, bunny_normals);
			break;
		case 4: // sandle
			sandal.setAmbient(no_mat);
			sandal.setDiffuse(no_mat);
			sandal.setSpecular(mat_specular);
			sandal.setShininess(high_shininess);
			sandal.setEmission(no_mat);

			drawShape(sandal_nVerts, sandal_vertices, sandal_normals);
			break;
		case 8: // house scene1
			// uncomment below for rotating lights + shadows
			shape.initializeShadows();
			shape.makeShadows();
			break;
		case 9: // house scene2
			shape.drawHouse();
			break;
	}

	if (toggle1) {
			glPushMatrix();
				glRotatef(theta, 0, 0, 1);
				if (!toggle_freeze)
					
					shape.directional.setPosition(d_position);
					
					GLfloat temp_mv[16];
					glGetFloatv(GL_MODELVIEW_MATRIX, temp_mv);
					sun_mv = Matrix4(
						temp_mv[0], temp_mv[1], temp_mv[2], temp_mv[3],
						temp_mv[4], temp_mv[5], temp_mv[6], temp_mv[7],
						temp_mv[8], temp_mv[9], temp_mv[10], temp_mv[11],
						temp_mv[12], temp_mv[13], temp_mv[14], temp_mv[15]
					);
					sun_mv.transpose();
					Vector4 d_position2 = Vector4(d_position[0], d_position[1], d_position[2], 1);
					Vector4 lightPos2 = sun_mv.multiply(d_position2);

					lightPos = Vector3(lightPos2[0], lightPos2[1], lightPos[2]);
					lightPos.scale(5);
					//lightPos.print();

				if (DEBUG_DRAW_LIGHTS && warp == true) {
					drawDirectionalLight();
				}
	
			glPopMatrix();
	}

	if (toggle2) {
		glPushMatrix();
			glRotatef(theta, 0, 0, 1);
			if (!toggle_freeze)
				shape.point.setPosition(p_position);
			if (DEBUG_DRAW_LIGHTS) drawPointLight();
		glPopMatrix();
	}

	if (toggle3) {
		glPushMatrix();
			glRotatef(theta, 0, 1, 0);
			if (!toggle_freeze) {
				shape.spot.setPosition(s_position);
				shape.spot.setSpotDirection(s_direction);
			}
			if (DEBUG_DRAW_LIGHTS) drawSpotLight();
		glPopMatrix();
	}
	bool print_lightpos = false;
	if (!toggle_freeze) {
		if (speed_up == true) {
			toggle1 = true;
			theta+=4.5*sun_speed;
		}
		else {
			theta+=0;
			toggle1 = false;
		}
	}
	if (theta > 360 || theta < 0) theta = 0;

	/*
	// TODO: sphere for testing
	glPushMatrix();
		glColor3f(1,1,1);
		Matrix4 i = Matrix4();
		i.identity();
		glLoadMatrixf(i.getGLMatrix());
		glTranslatef(-5,0,0);
		glutSolidSphere(1,10,10);
	glPopMatrix();
	*/
	//angle_change = 0.0;

	if (anglex_change != 0.0 || angley_change != 0.0) {
		shape.updateLookAtVector();
		anglex_change = 0.0;
		angley_change = 0.0;
		
	}
	//if (speed_up == true) {
	shape.drawLookAtPoint();
	//}
	spin_leftarm+=0.00*leftarm_dir;
	spin_rightarm+=0.00*rightarm_dir;
	if (spin_leftarm > 3.14*3/4 || spin_leftarm < 3.14/4) leftarm_dir=-1*leftarm_dir;
	if (spin_rightarm > 3.14/4 || spin_rightarm < -3.14/4) rightarm_dir=-1*rightarm_dir;

	spin_leftleg+=0.00*leftleg_dir;
	spin_rightleg+=0.00*rightleg_dir;
	if (spin_leftleg > 3.14/16 || spin_leftleg < -3.14/16) leftleg_dir=-1*leftleg_dir;
	if (spin_rightleg > 3.14/16 || spin_rightleg < -3.14/16) rightleg_dir=-1*rightleg_dir;

	// axis
	glBegin(GL_LINES);
		glColor3f(1, 0, 1);
				
		glVertex3f(0,-Window::height,0);
		glVertex3f(0,Window::height,0);
	glEnd();

  glFlush();  
  glutSwapBuffers();
}

void Window::drawDirectionalLight() {
		//glBegin(GL_LINES);
			glColor3f(1, 1, 0);
			//glVertex3f(0, 50, 0);
			//glVertex3f(10, 10 - 5*d_position[1], 0 - 5*d_position[2]);
			glTranslated(0, 25, 0);
			glutSolidSphere(2.5, 100, 100);
		//glEnd();
}

void Window::drawPointLight() {
		glColor3f(10, 10, 10);
		glTranslated(p_position[0], p_position[1], p_position[2]);
		glScaled(0.5, 0.5, 0.5);
		glutSolidSphere(1, 10, 10);
}

void Window::drawSpotLight() {		
		glBegin(GL_LINES);
			glColor3f(10, 10, 10);
			glVertex3f(s_position[0], s_position[1], s_position[2]);
			glVertex3f(s_position[0] + 3*s_direction[0], s_position[1] + 3*s_direction[1], s_position[2] + 3*s_direction[2]);
		glEnd();
}

void Shape::initializeShadows() {
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	//Depth states
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_NORMALIZE);
	glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
  	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(	GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0,
					GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
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

	/*
	cout << v[0] << '\n';
	cout << v[1] << '\n';
	cout << v[2] << '\n';
	cout << v[3] << '\n';
	*/

			
	glPushMatrix();
	
	glLoadIdentity();
	gluPerspective(90.0f, (float)Window::width/Window::height, 0.1f, 1000.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, p);
	cameraProjectionMatrix = Matrix4(
		p[0],p[1],p[2],p[3],
		p[4],p[5],p[6],p[7],
		p[8],p[9],p[10],p[11],
		p[12],p[13],p[14],p[15]);
	cameraProjectionMatrix.transpose();
	glLoadIdentity();
	gluLookAt(e.getX(), e.getY(), e.getZ(),
				d.getX(), d.getY(), d.getZ(),
				up.getX(), up.getY(), up.getZ());
	glGetFloatv(GL_MODELVIEW_MATRIX, p);
		cameraViewMatrix = Matrix4(
		p[0],p[1],p[2],p[3],
		p[4],p[5],p[6],p[7],
		p[8],p[9],p[10],p[11],
		p[12],p[13],p[14],p[15]);
	cameraViewMatrix.transpose();
	
	glLoadIdentity();
	gluPerspective(90.0f, 1.0f, 2.0f, 80.0f);
	glGetFloatv(GL_MODELVIEW_MATRIX, p);
	lightProjectionMatrix = Matrix4(
		p[0],p[1],p[2],p[3],
		p[4],p[5],p[6],p[7],
		p[8],p[9],p[10],p[11],
		p[12],p[13],p[14],p[15]);
	lightProjectionMatrix.transpose();
	glLoadIdentity();


	gluLookAt(sqrt(100.0), sqrt(100.0), sqrt(100.0),
	//gluLookAt(	lightPos.getX(), lightPos.getY(), lightPos.getZ(),
				d.getX(), d.getY(), d.getZ(),
				up.getX(), up.getY(), up.getZ());
	glGetFloatv(GL_MODELVIEW_MATRIX, p);
	lightViewMatrix = Matrix4(
		p[0],p[1],p[2],p[3],
		p[4],p[5],p[6],p[7],
		p[8],p[9],p[10],p[11],
		p[12],p[13],p[14],p[15]);
	lightViewMatrix.transpose();
	
	glPopMatrix();
	//timer.Reset();

	
	cameraProjectionMatrix.transpose();
	cameraViewMatrix.transpose();
	lightProjectionMatrix.transpose();
	lightViewMatrix.transpose();
	
}

void Shape::makeShadows() {
	if (showShadows == true) {

		//float angle=timer.GetTime()/10;	
		//cout << "angle: " << angle << '\n';
		//cout << "hihi\n";
	//First pass - from light's point of view
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // causes spasms
	//glClear(GL_DEPTH_BUFFER_BIT);
	//lightProjectionMatrix.print();
	//lightViewMatrix.print();
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(lightProjectionMatrix.getGLMatrix());

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(lightViewMatrix.getGLMatrix());

	//Use viewport the same size as the shadow map
	glViewport(0, 0, shadowMapSize, shadowMapSize);

	//Draw back faces into the shadow map
	glCullFace(GL_FRONT);

	//Disable color writes, and use flat shading for speed
	glShadeModel(GL_FLAT);
	glColorMask(0, 0, 0, 0);
	
	//Draw the scene
	shape.drawHouse();
	//DrawScene(angle);
		
	//Read the depth buffer into the shadow map texture
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, shadowMapSize, shadowMapSize);

	//restore states
	glCullFace(GL_BACK);
	glShadeModel(GL_SMOOTH);
	glColorMask(1, 1, 1, 1);
	

	

	//2nd pass - Draw from camera's point of view
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(cameraProjectionMatrix.getGLMatrix());
	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(cameraViewMatrix.getGLMatrix());

	glViewport(0, 0, Window::width, Window::height);
	//lightPos.print();
	GLfloat tmp[3] = {GLfloat(lightPos.getX()), GLfloat(lightPos.getY()), GLfloat(lightPos.getZ())};
	GLfloat almostwhite[3] = {GLfloat(0.2), GLfloat(0.2), GLfloat(0.2)};
	GLfloat black[3] = {GLfloat(0), GLfloat(0), GLfloat(0)};
	GLfloat white[3] = {GLfloat(1), GLfloat(1), GLfloat(1)};
	//Use dim light to represent shadowed areas
	glLightfv(GL_LIGHT1, GL_POSITION, tmp);
	glLightfv(GL_LIGHT1, GL_AMBIENT, almostwhite);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, almostwhite);
	glLightfv(GL_LIGHT1, GL_SPECULAR, black);
	//glLightfv(GL_LIGHT1, GL_AMBIENT, white*0.2f);
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, white*0.2f);
	//glLightfv(GL_LIGHT1, GL_SPECULAR, black);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHTING);

	shape.drawHouse();

	//3rd pass
	//Draw with bright light
	//glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	//glLightfv(GL_LIGHT1, GL_SPECULAR, white);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);

	//Calculate texture matrix for projection
	//This matrix takes us from eye space to the light's clip space
	//It is postmultiplied by the inverse of the current view matrix when specifying texgen
	Matrix4 biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.5f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.5f, 0.0f,
								0.5f, 0.5f, 0.5f, 1.0f);	//bias from [-1, 1] to [0, 1]
	//biasMatrix.transpose(); // MAYBE NOT TRANSPOSE
	biasMatrix.scale(1.0025, 1.0025, 1.0025);
	Matrix4 textureMatrix = biasMatrix.multiply(lightProjectionMatrix.multiply(lightViewMatrix));
	textureMatrix.transpose(); // MAYBE NOT TRANSPOSE
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

	shape.drawHouse();
	//DrawScene(angle);

	//Disable textures and texgen
	glDisable(GL_TEXTURE_2D);

	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);
	glDisable(GL_TEXTURE_GEN_R);
	glDisable(GL_TEXTURE_GEN_Q);

	//Restore other states
	glDisable(GL_LIGHTING);
	glDisable(GL_ALPHA_TEST);



	//Update frames per second counter
	//fpsCounter.Update();

	//Print fps
	static char fpsString[32];
	//sprintf(fpsString, "%.2f", fpsCounter.GetFps());
	
	//Set matrices for ortho
	
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);

	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glLoadIdentity();
	
	//Print text
	//glRasterPos2f(-1.0f, 0.9f);
	//for(unsigned int i=0; i<strlen(fpsString); ++i)
	//	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, fpsString[i]);

	//reset matrices
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
	glFinish();
	glutSwapBuffers();
	glutPostRedisplay();

	}
	else {
		shape.drawHouse();
	}
}

void Window::drawCube() {
		glBegin(GL_QUADS);
			// Draw front face:
			glColor3f(1, 0, 0);
			glNormal3f(0.0, 0.0, 1.0);   
			glVertex3f(-1.0,  1.0,  1.0);
			glVertex3f( 1.0,  1.0,  1.0);
			glVertex3f( 1.0, -1.0,  1.0);
			glVertex3f(-1.0, -1.0,  1.0);
    
			// Draw left side:
			glColor3f(0, 1, 0);
			glNormal3f(-1.0, 0.0, 0.0);
			glVertex3f(-1.0,  1.0,  1.0);
			glVertex3f(-1.0,  1.0, -1.0);
			glVertex3f(-1.0, -1.0, -1.0);
			glVertex3f(-1.0, -1.0,  1.0);
    
			// Draw right side:
			glColor3f(0, 0, 1);
			glNormal3f(1.0, 0.0, 0.0);
			glVertex3f( 1.0,  1.0,  1.0);
			glVertex3f( 1.0,  1.0, -1.0);
			glVertex3f( 1.0, -1.0, -1.0);
			glVertex3f( 1.0, -1.0,  1.0);
  
			// Draw back face:
			glColor3f(1, 0, 1);
			glNormal3f(0.0, 0.0, -1.0);
			glVertex3f(-1.0,  1.0, -1.0);
			glVertex3f( 1.0,  1.0, -1.0);
			glVertex3f( 1.0, -1.0, -1.0);
			glVertex3f(-1.0, -1.0, -1.0);
  
			// Draw top side:
			glColor3f(1, 1, 0);
			glNormal3f(0.0, 1.0, 0.0);
			glVertex3f(-1.0,  1.0,  1.0);
			glVertex3f( 1.0,  1.0,  1.0);
			glVertex3f( 1.0,  1.0, -1.0);
			glVertex3f(-1.0,  1.0, -1.0);
  
			// Draw bottom side:
			glColor3f(0, 1, 1);
			glNormal3f(0.0, -1.0, 0.0);
			glVertex3f(-1.0, -1.0, -1.0);
			glVertex3f( 1.0, -1.0, -1.0);
			glVertex3f( 1.0, -1.0,  1.0);
			glVertex3f(-1.0, -1.0,  1.0);
		glEnd();
}



void Shape::drawRobot() {
	//unsigned long long Int64 = 0;
    //clock_t Start = clock();
	glMatrixMode(GL_MODELVIEW);
	glColor3f(0,1,0);

	for (int j=0; j<army_size; j++) {
		for (int k=0; k<army_size; k++) {
			
			Matrix4 i;
			i.identity();
			robot[j][k] = &MatrixTransform(i);

			// head
			Matrix4 s = Matrix4(i);
			s.scale(0.5,0.5,0.5);
			s.rotateX(2*spin_leftleg);
			MatrixTransform head = MatrixTransform(s);
			head.addChild(&Sphere());

			// torso
			Matrix4 t = Matrix4(i);
			t.scale(1.5,2,1);
			t.translate(0,-1.5,0);
			t.rotateY(0.5*spin_leftleg);
			MatrixTransform torso = MatrixTransform(t);
			torso.addChild(&Cube());

			// left arm
			Matrix4 la = Matrix4(i);
			la.rotateZ(3.14/4);
			la.translate(1,-0.5,0);
			la.rotateX(spin_leftarm);
			MatrixTransform leftarm = MatrixTransform(la);

			Matrix4 ula = Matrix4(i);
			ula.translate(-1,-0.5,0);
			ula.scale(0.5,1.5,0.5);
			MatrixTransform upper_leftarm = MatrixTransform(ula);
			upper_leftarm.addChild(&Cube());

			Matrix4 lla = Matrix4(i);
			lla.rotateX(3.14/2);
			lla.scale(0.25,0.25,0.25);
			lla.translate(-0.5,-1.5,0);
			MatrixTransform lower_leftarm = MatrixTransform(lla);
			lower_leftarm.addChild(&Cone());
	
			leftarm.addChild(&upper_leftarm);
			leftarm.addChild(&lower_leftarm);

			// right arm
			Matrix4 ra = Matrix4(i);
			ra.rotateZ(-3.14/4);
			ra.translate(-1,-0.5,0);
			ra.rotateX(spin_rightarm);
			MatrixTransform rightarm = MatrixTransform(ra);
	
			Matrix4 ura = Matrix4(i);
			ura.translate(1,-0.5,0);
			ura.scale(0.5,1.5,0.5);
			MatrixTransform upper_rightarm = MatrixTransform(ura);
			upper_rightarm.addChild(&Cube());

			Matrix4 lra = Matrix4(i);
			lra.rotateX(3.14/2);
			lra.scale(0.25,0.25,0.25);
			lra.translate(0.5,-1.5,0);
			MatrixTransform lower_rightarm = MatrixTransform(lra);
			lower_rightarm.addChild(&Cone());
	
			rightarm.addChild(&upper_rightarm);
			rightarm.addChild(&lower_rightarm);

			// left leg
			Matrix4 ll = Matrix4(i);
			ll.scale(0.5,1.5,0.5);
			ll.translate(0,-3,0);
			ll.rotateWindowX(spin_leftleg);
			MatrixTransform leftleg = MatrixTransform(ll);

			Matrix4 ull = Matrix4(i);
			ull.translate(1,0,0);
			MatrixTransform upper_leftleg = MatrixTransform(ull);
			upper_leftleg.addChild(&Cube());

			Matrix4 lll = Matrix4(i);
			lll.scale(1,0.1,2);
			lll.translate(1,-0.5,0.5);
			MatrixTransform lower_leftleg = MatrixTransform(lll);
			lower_leftleg.addChild(&Cube());
	
			leftleg.addChild(&upper_leftleg);
			leftleg.addChild(&lower_leftleg);

			// right leg
			Matrix4 rl = Matrix4(i);
			rl.scale(0.5,1.5,0.5);
			rl.translate(0,-3,0);
			rl.rotateWindowX(spin_rightleg);
			MatrixTransform rightleg = MatrixTransform(rl);

			Matrix4 url = Matrix4(i);
			url.translate(-1,0,0);
			MatrixTransform upper_rightleg = MatrixTransform(url);
			upper_rightleg.addChild(&Cube());

			Matrix4 lrl = Matrix4(i);
			lrl.scale(1,0.1,2);
			lrl.translate(-1,-0.5,0.5);
			MatrixTransform lower_rightleg = MatrixTransform(lrl);
			lower_rightleg.addChild(&Cube());
	
			rightleg.addChild(&upper_rightleg);
			rightleg.addChild(&lower_rightleg);

			// add to robot
			robot[j][k]->addChild(&head);
			robot[j][k]->addChild(&torso);
			robot[j][k]->addChild(&leftarm);
			robot[j][k]->addChild(&rightarm);
			robot[j][k]->addChild(&leftleg);
			robot[j][k]->addChild(&rightleg);

			la.rotateX(-3.14/2);
			leftarm.setTransformationMatrix(la);

			i.translate(-5*j,0,-5*k);
			robot[j][k]->setTransformationMatrix(i);
			robot[j][k]->draw(shape.modelview);
			//robot[j][k]->setBoundingBox(robot[j][k]->x_min, robot[j][k]->x_max, robot[j][k]->y_min, robot[j][k]->y_max, robot[j][k]->z_min, robot[j][k]->z_max);
			//cout << "miliseconds elapsed for rendering(without culling) " <<  clock() - Start << '\n';
		}
	}

}

void Shape::drawRobot2() {

	//unsigned long long Int64 = 0;
    //clock_t Start = clock();
	glMatrixMode(GL_MODELVIEW);
	glColor3f(0,1,0);

	for (int j=0; j<army_size; j++) {
		for (int k=0; k<army_size; k++) {
			
			Matrix4 i;
			i.identity();
			robot[j][k] = &MatrixTransform(i);

			// head
			Matrix4 s = Matrix4(i);
			s.scale(0.5,0.5,0.5);
			s.rotateX(2*spin_leftleg);
			MatrixTransform head = MatrixTransform(s);
			head.addChild(&Sphere());

			// torso
			Matrix4 t = Matrix4(i);
			t.scale(1.5,2,1);
			t.translate(0,-1.5,0);
			t.rotateY(0.5*spin_leftleg);
			MatrixTransform torso = MatrixTransform(t);
			torso.addChild(&Cube());

			// left arm
			Matrix4 la = Matrix4(i);
			la.rotateZ(3.14/4);
			la.translate(1,-0.5,0);
			la.rotateX(spin_leftarm);
			MatrixTransform leftarm = MatrixTransform(la);

			Matrix4 ula = Matrix4(i);
			ula.translate(-1,-0.5,0);
			ula.scale(0.5,1.5,0.5);
			MatrixTransform upper_leftarm = MatrixTransform(ula);
			upper_leftarm.addChild(&Cube());

			Matrix4 lla = Matrix4(i);
			lla.rotateX(3.14/2);
			lla.scale(0.25,0.25,0.25);
			lla.translate(-0.5,-1.5,0);
			MatrixTransform lower_leftarm = MatrixTransform(lla);
			lower_leftarm.addChild(&Cone());
	
			leftarm.addChild(&upper_leftarm);
			leftarm.addChild(&lower_leftarm);

			// right arm
			Matrix4 ra = Matrix4(i);
			ra.rotateZ(-3.14/4);
			ra.translate(-1,-0.5,0);
			ra.rotateX(spin_rightarm);
			MatrixTransform rightarm = MatrixTransform(ra);
	
			Matrix4 ura = Matrix4(i);
			ura.translate(1,-0.5,0);
			ura.scale(0.5,1.5,0.5);
			MatrixTransform upper_rightarm = MatrixTransform(ura);
			upper_rightarm.addChild(&Cube());

			Matrix4 lra = Matrix4(i);
			lra.rotateX(3.14/2);
			lra.scale(0.25,0.25,0.25);
			lra.translate(0.5,-1.5,0);
			MatrixTransform lower_rightarm = MatrixTransform(lra);
			lower_rightarm.addChild(&Cone());
	
			rightarm.addChild(&upper_rightarm);
			rightarm.addChild(&lower_rightarm);

			// left leg
			Matrix4 ll = Matrix4(i);
			ll.scale(0.5,1.5,0.5);
			ll.translate(0,-3,0);
			ll.rotateWindowX(spin_leftleg);
			MatrixTransform leftleg = MatrixTransform(ll);

			Matrix4 ull = Matrix4(i);
			ull.translate(1,0,0);
			MatrixTransform upper_leftleg = MatrixTransform(ull);
			upper_leftleg.addChild(&Cube());

			Matrix4 lll = Matrix4(i);
			lll.scale(1,0.1,2);
			lll.translate(1,-0.5,0.5);
			MatrixTransform lower_leftleg = MatrixTransform(lll);
			lower_leftleg.addChild(&Cube());
	
			leftleg.addChild(&upper_leftleg);
			leftleg.addChild(&lower_leftleg);

			// right leg
			Matrix4 rl = Matrix4(i);
			rl.scale(0.5,1.5,0.5);
			rl.translate(0,-3,0);
			rl.rotateWindowX(spin_rightleg);
			MatrixTransform rightleg = MatrixTransform(rl);

			Matrix4 url = Matrix4(i);
			url.translate(-1,0,0);
			MatrixTransform upper_rightleg = MatrixTransform(url);
			upper_rightleg.addChild(&Cube());

			Matrix4 lrl = Matrix4(i);
			lrl.scale(1,0.1,2);
			lrl.translate(-1,-0.5,0.5);
			MatrixTransform lower_rightleg = MatrixTransform(lrl);
			lower_rightleg.addChild(&Cube());
	
			rightleg.addChild(&upper_rightleg);
			rightleg.addChild(&lower_rightleg);

			// add to robot
			robot[j][k]->addChild(&head);
			robot[j][k]->addChild(&torso);
			robot[j][k]->addChild(&leftarm);
			robot[j][k]->addChild(&rightarm);
			robot[j][k]->addChild(&leftleg);
			robot[j][k]->addChild(&rightleg);

			la.rotateX(-3.14/2);
			leftarm.setTransformationMatrix(la);

			i.translate(-5*j,0,-5*k);
			robot[j][k]->setTransformationMatrix(i);
			robot[j][k]->draw2(shape.modelview);
			//robot[j][k]->setBoundingBox(robot[j][k]->x_min, robot[j][k]->x_max, robot[j][k]->y_min, robot[j][k]->y_max, robot[j][k]->z_min, robot[j][k]->z_max);
			//cout << "miliseconds elapsed for rendering(with culling) " <<  clock() - Start << '\n';
		}

	}
}

void Shape::nearestNeighbor(Vector4 vec1, Vector4* arr1)
{
	Vector4 *arr2 = arr1;
	Vector4 tmp1;
	Vector4 tmp2;
	Vector4 tmp3;
	float min_dist = threshold;
	int min_index = -1;
	for (int k = 0; k < 7; k++)
	{
		arr1[k];
		arr1[k].setY(Window::height-arr1[k].getY()); 
		//cout << "index: " << k << ": \n";
		//arr1[k].print();
		
	}
	for (int i = 0; i < 7; i++)
	{

		tmp1 = vec1;
		tmp2 = arr1[i];
		tmp1.subtract(tmp2);
		if (tmp1.magnitude() < min_dist)
		{
			min_dist = tmp1.magnitude();
			min_index = i;
		}
		
	}
	//cout << "min_dist: " << min_dist << '\n';
	selected_point = min_index;

}

void Shape::loadData()
{
	/*
  	// put code to load data model here
	ObjReader::readObj("dragon_smooth.obj", dragon_nVerts, &dragon_vertices, &dragon_normals, &dragon_texcoords, dragon_nIndices, &dragon_indices);
	ObjReader::readObj("bunny_n.obj", bunny_nVerts, &bunny_vertices, &bunny_normals, &bunny_texcoords, bunny_nIndices, &bunny_indices);
	ObjReader::readObj("sandal.obj", sandal_nVerts, &sandal_vertices, &sandal_normals, &sandal_texcoords, sandal_nIndices, &sandal_indices);
	*/

	ObjReader::readObj("city.obj", city_nVerts, &city_vertices, &city_normals, &city_texcoords, city_nIndices, &city_indices);
}

void Shape::calculateStuff(int nVerts, float *vertices) {
	float max_arr[3] = {-1000, -1000, -1000};
	float min_arr[3] = {1000, 1000, 1000};

	for (int i = 0; i < nVerts/3; i++) {
		for (int v = 0; v < 3; v++) {
			if (vertices[9*i+3*v] < min_arr[0]) {
				min_arr[0] = vertices[9*i+3*v];
			}
			if (vertices[9*i+3*v] > max_arr[0]) {
				max_arr[0] = vertices[9*i+3*v];
			}
			if (vertices[(9*i)+(3*v)+1] < min_arr[1]) {
				min_arr[1] = vertices[(9*i)+(3*v)+1];
			}
			if (vertices[(9*i)+(3*v)+1] > max_arr[1]) {
				max_arr[1] = vertices[(9*i)+(3*v)+1];
			}
			if (vertices[(9*i)+(3*v)+2] < min_arr[2]) {
				min_arr[2] = vertices[(9*i)+(3*v)+2];
			}
			if (vertices[(9*i)+(3*v)+2] > max_arr[2]) {
				max_arr[2] = vertices[(9*i)+(3*v)+2];
			}

		}
	}

	shape.x = 0.5*(max_arr[0] + min_arr[0]);
	shape.y = 0.5*(max_arr[1] + min_arr[1]);
	shape.z = 0.5*(max_arr[2] + min_arr[2]);

	shape.translation.identity();
	shape.translation.m[0][3] = -shape.x;
	shape.translation.m[1][3] = -shape.y;
	shape.translation.m[2][3] = -shape.z;

	cout << "minimum values: " << min_arr[0] << ", " << min_arr[1] << ", " << min_arr[2] << "\n";
  cout << "maximum values: " << max_arr[0] << ", " << max_arr[1] << ", " << max_arr[2] << "\n\n";

	cout << "center: (" << shape.x << ", " << shape.y << ", " << shape.z << ")\n\n";
	
	float x_diff = max_arr[0]-min_arr[0];
	float y_diff = max_arr[1]-min_arr[1];
	float z_diff = max_arr[2]-min_arr[2];

	float max1 = max(x_diff, y_diff);
	max1 = max(max1, z_diff);

	scaling_x = 28/max1;
	scaling_y = 28/max1;
	scaling_z = 28/max1;

	shape.scale.identity();
	shape.scale.m[0][0] = scaling_x;
	shape.scale.m[1][1] = scaling_y;
	shape.scale.m[2][2] = scaling_z;

	cout << "scaling factor : " << scaling_x << ", " << scaling_y <<  ", " << scaling_z <<"\n\n";

}

void Window::drawShape(int nVerts, float *vertices, float *normals) {
	glPushMatrix();
	
	// scale city down
	GLfloat curr[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, curr);
	Matrix4 curr_mv = Matrix4(
		curr[0], curr[1], curr[2], curr[3],
		curr[4], curr[5], curr[6], curr[7],
		curr[8], curr[9], curr[10], curr[11],
		curr[12], curr[13], curr[14], curr[15]
	);
	curr_mv.scale(city_scale, city_scale, city_scale);
	glLoadMatrixf(curr_mv.getGLMatrix());
	
	glBegin(GL_TRIANGLES);
	for (int i=0; i<nVerts/3; i++) {
		if (red == true) {
			glColor3f(1,0,0);
			red = false;
		}
		else {
			glColor3f(1,1,1);
			red = true;
		}

		for (int v=0; v<3; v++) {
			glNormal3f(normals[9*i+3*v], normals[(9*i)+(3*v)+1], normals[(9*i)+(3*v)+2]);
			glVertex3f(vertices[9*i+3*v], vertices[(9*i)+(3*v)+1], vertices[(9*i)+(3*v)+2]);
		}
	}
	glEnd();

	glPopMatrix();
}

Shape::Shape() {
	shape.getModelMatrix().identity();

	//shape.getModelMatrix() = shape.getModelMatrix().multiply(shape.setScaleMatrix(8.0));

	shape.getCameraMatrix().identity();
	shape.getCameraMatrix() = Matrix4::createCameraMatrix(e, d, up);
	//shape.getCameraMatrix() = Matrix4::createCameraMatrix(Vector3(0, 10, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	//shape.getCameraMatrix() = Matrix4::createCameraMatrix(Vector3(0, 0, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
	
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

int main(int argc, char *argv[])
{

	

  float specular[]  = {1.0, 1.0, 1.0, 1.0};
  float shininess[] = {100.0};

  glutInit(&argc, argv);      	      	      // initialize GLUT
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);   // open an OpenGL context with double buffering, RGB colors, and depth buffering
  glutInitWindowSize(Window::width, Window::height);      // set initial window size
  glutCreateWindow("OpenGL Cube for CSE167");    	      // open window and set window title
 // glGetString(GL_EXTENSIONS);
	if(!GLEE_ARB_depth_texture || !GLEE_ARB_shadow)
	{
		printf("I require ARB_depth_texture and ARB_shadow extensionsn\n");
		//return false;
	}

  glDisable(GL_LIGHTING);
  
  glEnable(GL_DEPTH_TEST);            	      // enable depth buffering
  glClear(GL_DEPTH_BUFFER_BIT);       	      // clear depth buffer
  glClearColor(0.0, 0.0, 0.0, 0.0);   	      // set clear color to black
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // set polygon drawing mode to fill front and back of each polygon
  glEnable(GL_CULL_FACE);     // disable backface culling to render both sides of polygons
  glDisable(GL_LIGHTING);
  glShadeModel(GL_SMOOTH);             	      // set shading to smooth

  glMatrixMode(GL_PROJECTION);
  gluPerspective(90, float(Window::width)/float(Window::height), 0.1, 1000);

  // Generate material properties:
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

	if (TURN_LIGHTS_ON) {
		//Generate light source:
		glEnable(GL_LIGHTING);
  
		GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };

		GLfloat light_diffuse_d[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat light_specular_d[] = { 1.0, 1.0, 1.0, 1.0 };

		GLfloat light_diffuse_p[] = { 0.0, 1.0, 0.0, 1.0 };
		GLfloat light_specular_p[] = { 0.0, 1.0, 0.0, 1.0 };

		GLfloat light_diffuse_s[] = { 0.0, 0.0, 1.0, 1.0 };
		GLfloat light_specular_s[] = { 0.0, 0.0, 1.0, 1.0 };
		
		shape.directional = Light(0);
		shape.directional.setPosition(d_position);
		shape.directional.setAmbient(light_ambient);
		shape.directional.setDiffuse(light_diffuse_d);
		shape.directional.setSpecular(light_specular_d);

		shape.point = Light(1);
		shape.point.setPosition(p_position);
		shape.point.setAmbient(light_ambient);
		shape.point.setDiffuse(light_diffuse_p);
		shape.point.setSpecular(light_specular_p);

		shape.spot = Light(2);
		shape.spot.setPosition(s_position);
		shape.spot.setAmbient(light_ambient);
		shape.spot.setDiffuse(light_diffuse_s);
		shape.spot.setSpecular(light_specular_s);
		shape.spot.setSpotCutoff(5);
		shape.spot.setSpotDirection(s_direction);

		if (!toggle1)
			shape.directional.disable();
		if (!toggle2)
			shape.point.disable();
		if (!toggle3)
			shape.spot.disable();
	}

	
	
	

  // Install callback functions:
  glutDisplayFunc(Window::displayCallback);
  glutReshapeFunc(Window::reshapeCallback);
  glutIdleFunc(Window::idleCallback);
  shape.loadTexture();

	// to avoid cube turning white on scaling down
  //glEnable(GL_TEXTURE_2D);   // enable texture mapping
  
	// Process keyboard input
  glutKeyboardFunc(Window::processNormalKeys);
  glutSpecialFunc(Window::processSpecialKeys);
	//glutMouseFunc(Window::processMouseClick);
	glutPassiveMotionFunc(Window::processMouseMove);
	
	// load obj files
	if (DEBUG_LOAD_OBJS)
		shape.loadData();

	glutSetCursor(GLUT_CURSOR_NONE);
	shape.initializeHeightMap();
	
	shape.initializeShadows();
	shape.initializeHeightMap();

	/*
	cout << "camproj: \n";
	cameraProjectionMatrix.print();
	cout << "camview: \n";
	cameraViewMatrix.print();
	cout << "lightproj: \n";
	lightProjectionMatrix.print();
	cout << "lightview: \n";
	lightViewMatrix.print();
	*/

	glutMainLoop();

  return 0;
}

void Window::drawSkyBox() {
		glPushMatrix();
		
		/*
		// code to make skybox move with camera
		GLfloat curr[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, curr);
		Matrix4 follow_cam = Matrix4(
			curr[0], curr[1], curr[2], curr[3],
			curr[4], curr[5], curr[6], curr[7],
			curr[8], curr[9], curr[10], curr[11],
			curr[12], curr[13], curr[14], curr[15]
		);
		Matrix4 cam_inv = Matrix4::createCameraMatrix(e, d, up);
		cam_inv.inverse();
		follow_cam = follow_cam.multiply(shape.getCameraMatrix());
		glLoadMatrixf(follow_cam.getGLMatrix());
		*/
		
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_LIGHTING);

		glBegin(GL_QUADS);
			// Draw front face:
			glColor3f(0, 0.5, 0.5);
			glNormal3f(0.0, 0.0, 5000.0);   
			glVertex3f(-5000.0,  5000.0,  5000.0);
			glVertex3f( 5000.0,  5000.0,  5000.0);
			glVertex3f( 5000.0, -5000.0,  5000.0);
			glVertex3f(-5000.0, -5000.0,  5000.0);
    
			// Draw left side:
			glNormal3f(-5000.0, 0.0, 0.0);
			glVertex3f(-5000.0,  5000.0,  5000.0);
			glVertex3f(-5000.0,  5000.0, -5000.0);
			glVertex3f(-5000.0, -5000.0, -5000.0);
			glVertex3f(-5000.0, -5000.0,  5000.0);
    
			// Draw right side:
			glNormal3f(5000.0, 0.0, 0.0);
			glVertex3f( 5000.0,  5000.0,  5000.0);
			glVertex3f( 5000.0,  5000.0, -5000.0);
			glVertex3f( 5000.0, -5000.0, -5000.0);
			glVertex3f( 5000.0, -5000.0,  5000.0);
  
			// Draw back face:
			glNormal3f(0.0, 0.0, -5000.0);
			glVertex3f(-5000.0,  5000.0, -5000.0);
			glVertex3f( 5000.0,  5000.0, -5000.0);
			glVertex3f( 5000.0, -5000.0, -5000.0);
			glVertex3f(-5000.0, -5000.0, -5000.0);
  
			// Draw top side:
			glNormal3f(0.0, 5000.0, 0.0);
			glVertex3f(-5000.0,  5000.0,  5000.0);
			glVertex3f( 5000.0,  5000.0,  5000.0);
			glVertex3f( 5000.0,  5000.0, -5000.0);
			glVertex3f(-5000.0,  5000.0, -5000.0);
  
			// Draw bottom side:
			glColor3f(0, 0, 0);
			glNormal3f(0.0, -5000.0, 0.0);
			glVertex3f(-5000.0, -5000.0, -5000.0);
			glVertex3f( 5000.0, -5000.0, -5000.0);
			glVertex3f( 5000.0, -5000.0,  5000.0);
			glVertex3f(-5000.0, -5000.0,  5000.0);
		glEnd();

		glEnable(GL_LIGHTING);
		glEnable(GL_DEPTH_TEST);

		glPopMatrix();
}

void Shape::initializeHeightMap() {
	heightMap.resize(41, vector<int>(41, INT_MIN));

	for (int i = 0; i < 84; i+=3) { // else 84 or  126
		if (heightMap[house_vertices[i]+20][house_vertices[i+2]+20] < house_vertices[i+1]) {
			heightMap[house_vertices[i]+20][house_vertices[i+2]+20] = house_vertices[i+1];
		}
	}
	int finder;
	vector <int> tmp;
	vector <int> minx;
	vector <int> maxx;
	vector <int> miny;
	vector <int> maxy;
	//vector <int> indexes; // minx, maxx, miny, maxy
	for (int i = 0; i < 41; i++)
	{
	  for (int j = 0; j < 41; j++)
	  {
		  if (heightMap[i][j] != INT_MIN)
		  {
			  if(find(tmp.begin(), tmp.end(), heightMap[i][j]) == tmp.end()) {
				  tmp.push_back(heightMap[i][j]);
				  minx.push_back(i);
				  maxx.push_back(i);
				  miny.push_back(j);
				  maxy.push_back(j);
			  }
			  else {
				finder = find(tmp.begin(), tmp.end(), heightMap[i][j]) - tmp.begin();
				if (i < minx[finder]){
					minx[finder] = i;
				}
				if (i > maxx[finder]){
					maxx[finder] = i;
				}
				if (j < miny[finder]){
					miny[finder] = j;
				}
				if (j > maxy[finder]){
					maxy[finder] = j;
				}
			  }
			 //cout << "i: " << i << ", j: " << j << ", val: " << heightMap[i][j] << "\n";
		  }
	  }

	  //cout << '\n';
  }
	vector<int> tmp2 = tmp;

	sort(tmp.begin(), tmp.end()); // tmp = sorted, tmp2 = unsorted 
	//cout << tmp.size() << '\n';
	//cout << "HI\n";
	for (int i = 0; i < tmp.size(); i++)
	{
	  finder = find(tmp2.begin(), tmp2.end(), tmp[i]) - tmp2.begin(); 
	  for (int j = minx[finder]; j < maxx[finder]+1; j++)
	  {
		  for (int k = miny[finder]; k < maxy[finder]+1; k++)
		  {
			  heightMap[j][k] = tmp[i];
		  }
	  }
	}
	for (int i = 0; i < 41; i++)
	{
	  for (int j = 0; j < 41; j++)
	  {
		  if (heightMap[i][j] == INT_MIN)
		  {
			  cout << "ERROR\n";
		  }
		  heightMap[i][j]+=4; // 4 feet tall
		  //else if (heightMap[i][j] == -4)
		  //{
			//  heightMap[i][j] = 0; // 4 feet tall
		  //}
	  }
	}
}


void Shape::drawLookAtPoint() {
	//shape.transformer.print();
	glEnable(GL_POINT_SMOOTH);
	glPointSize(5);

	//control points
	glBegin(GL_POINTS);
		//glVertex3f(1.0, 1.0, 9.5);
		//glVertex3f(d.getX(), d.getY(), d.getZ());
		//glColor3f(1, 0, 1);
		//glVertex3f(e.getX(), e.getY(), e.getZ());
		glColor3f(1, 1, 1);
		//lightPos.print();
		glVertex3f(lightPos.getX(), lightPos.getY(), lightPos.getZ());
	glEnd();
	//updateCameraMatrix(0, 0, 0);
}

void Shape::updateLookAtVector() {
	Matrix4 tmp;
	
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
	//cout << "angley_change: " << angley_change << '\n';
	//cout << "angley: " << angley << '\n';
	/*
	cout << "after: \n";
	cout << "d: ";
	d.print();
	cout << "e: ";
	e.print();
	*/

}

double Shape::getAngle() {
	return angle;
}

void Shape::setAngle(double a) {
	angle = a;
}

Matrix4 Shape::setScaleMatrix(float factor) {
	Matrix4 scale = Matrix4();
	for (int i = 0; i < 3; i++)
	{
		scale.set(i, i, factor);
	}
	return scale;
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

	glBegin(GL_TRIANGLES);
	for (int i=7; i<8; i++) {
		glColor3f(house_colors[12*i], house_colors[12*i+1], house_colors[12*i+2]);
		glNormal3f(house_indices[i*6], house_indices[i*6+1], house_indices[i*6+2]);
		for (int v=0; v<3; v++) {
			glVertex3f(house_vertices[12*i+3*v], house_vertices[(12*i)+(3*v)+1], house_vertices[(12*i)+(3*v)+2]);
		}
		tris++;
	}
	glEnd();

	glBegin(GL_QUADS);
	for (int i=8; i<10; i++) {
		glColor3f(house_colors[12*i-3*tris], house_colors[12*i+1-3*tris], house_colors[12*i+2-3*tris]);
		glNormal3f(house_indices[i*6-3*tris], house_indices[i*6+1-3*tris], house_indices[i*6+2-3*tris]);
		for (int v=0; v<4; v++) {
			glVertex3f(house_vertices[12*i+3*v-3*tris], house_vertices[(12*i)+(3*v)+1-3*tris], house_vertices[(12*i)+(3*v)+2-3*tris]);
		}
		glNormal3f(house_indices[i*6+3-3*tris], house_indices[i*6+4-3*tris], house_indices[i*6+5-3*tris]);
	}
	glEnd();
	
	glBegin(GL_TRIANGLES);
	for (int i=10; i<11; i++) {
		glColor3f(house_colors[12*i-3*tris], house_colors[12*i+1-3*tris], house_colors[12*i+2-3*tris]);
		glNormal3f(house_indices[i*6]-3*tris, house_indices[i*6+1]-3*tris, house_indices[i*6+2]-3*tris);
		for (int v=0; v<3; v++) {
			glVertex3f(house_vertices[12*i+3*v-3*tris], house_vertices[(12*i)+(3*v)+1-3*tris], house_vertices[(12*i)+(3*v)+2-3*tris]);
		}
		tris++;
	}
	glEnd();
}


void Shape::spin(double deg)
{
  if (shape.angle > 360.0 || shape.angle < -360.0) shape.angle = 0.0;
	shape.getModelViewMatrix().rotateWindowY(deg);
}


void Window::processNormalKeys(unsigned char key, int x, int y)
{
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

	switch (key) 
	{
		
		case 't':
			speed_up = !speed_up;
			break;
		case '1':
				toggle1 = !toggle1;
				toggle1 ? shape.directional.enable() : shape.directional.disable();
				toggle1 ? cout << "directional ON\n" : cout << "directional OFF\n";
			break;
		case '2':
				toggle2 = !toggle2;
				toggle2 ? shape.point.enable() : shape.point.disable();
				toggle2 ? cout << "point ON\n" : cout << "point OFF\n";
			break;
		case '3':
				toggle3 = !toggle3;
				toggle3 ? shape.spot.enable() : shape.spot.disable();
				toggle3 ? cout << "spot ON\n" : cout << "spot OFF\n";
			break;
		case '5':
			shader_toggle = !shader_toggle;
			if (shader_toggle)
				Shader shader = Shader("diffuse_shading.vert", "diffuse_shading.frag", true);
			else
				Shader shader = Shader("diffuse_shading.vert", "diffuse_shading.frag", false);
			break;
		case 'c':
			warp = !warp;
			break;
		case 'o':
			showShadows = !showShadows;
			break;
		case 'w':
			shape.updateCameraMatrix(tmp_vec.getX()/walk_x_factor,0,tmp_vec.getZ()/walk_z_factor);
			//cout << "walk forward\n";
			break;
		case 's':
			shape.updateCameraMatrix(-1.0*tmp_vec.getX()/walk_x_factor,0,-1.0*tmp_vec.getZ()/walk_z_factor);
			//cout << "walk back\n";
			break;
		case 'a':
			shape.updateCameraMatrix(a_vec.getX()/walk_x_factor,0,a_vec.getZ()/walk_z_factor);
			//cout << "strafe left\n";
			break;
		case 'd':
			shape.updateCameraMatrix(d_vec.getX()/walk_x_factor,0,d_vec.getZ()/walk_z_factor);
			//cout << "strafe right\n";
			break;
		case 'r':
			shape.updateCameraMatrix(0,1,0);
			//cout << "fly up\n";
			break;
		case 'f':
			shape.updateCameraMatrix(0,-1,0);
			//cout << "fly down\n";
			break;
		case 'g':
			godMode = !godMode;
			int i = 1;
			if (godMode == false) {
				i = -1;
			}
			d = Vector3(0,0,0);
			up = Vector3(0,0,-1);
			shape.updateCameraMatrix(0,i*500, 0);
	}
	/*
	cout << "--------------------\nbefore: \n";
	cout << "d: ";
	d.print();
	cout << "e: ";
	e.print();
	*/
	int tmpx = floor(e.getX() + 0.5);
	int tmpz = floor(e.getZ() + 0.5);
	//cout << "tmpx: "<< tmpx << ", tmpz: " << tmpz << '\n';
	//cout << e.getX() << e.getZ() << '\n';
	//if (heightMap[e.getX()][e.getZ()] != 0) {
	//shape.updateCameraMatrix(0, heightMap[tmpx+20][tmpz+20] - e.getY(), 0);
	//cout << "jumped up a building\n";
		//e.getY() = heightMap[e.getX()][e.getZ()];
	//}
	/*
	cout << "after: \n";
	cout << "d: ";
	d.print();
	cout << "e: ";
	e.print();
	*/
	
}

void Window::processSpecialKeys(int key, int x, int y)
{
	shape.getModelMatrix().identity();
	shape.getCameraMatrix().identity();
	switch (key) 
	{	
		case GLUT_KEY_F1:
			// cube
			shape_key = 1;
			shape.getModelMatrix().identity();
			shape.getModelMatrix() = shape.getModelMatrix().multiply(shape.setScaleMatrix(8.0));
			shape.getCameraMatrix().identity();
			break;
		case GLUT_KEY_F2:
			// dragon
			shape_key = 2;
			shape.getModelMatrix().identity();
			shape.getModelMatrix().translate(0, -5, 0);
			shape.getModelMatrix() = shape.getModelMatrix().multiply(shape.setScaleMatrix(15.0));
			shape.getCameraMatrix().identity();
			break;
		case GLUT_KEY_F3:
			// bunny
			shape_key = 3;
			shape.getModelMatrix().identity();
			shape.getModelMatrix().translate(0, 4, 0);
			shape.getModelMatrix() = shape.getModelMatrix().multiply(shape.setScaleMatrix(12.0));
			shape.getCameraMatrix().identity();
			break;
		case GLUT_KEY_F4:
			// sandal
			shape_key = 4;
			shape.getModelMatrix().identity();
			shape.getModelMatrix().translate(1, 0, 0);
			shape.getModelMatrix() = shape.getModelMatrix().multiply(shape.setScaleMatrix(4.0));
			shape.getCameraMatrix().identity();
			break;
		case GLUT_KEY_F8:
			// house view1
			shape_key = 8;
			shape.getCameraMatrix() = Matrix4::createCameraMatrix(Vector3(0, 10, 10), Vector3(0, 0, 0), Vector3(0, 1, 0));
			break;
		case GLUT_KEY_F9:
			// show house view2
			shape_key = 9;
			shape.getCameraMatrix() = Matrix4::createCameraMatrix(Vector3(-15, 5, 10), Vector3(-5, 0, 0), Vector3(0, 1, 0.5));
			break;
	}
	shape.updateModelViewMatrix();
}

void Window::processMouseMove(int x, int y) {

	/*
	if (left_clicked) { // rotate modelview
		if (x != x_mouse || y != y_mouse) {
			if (!toggle_freeze) {
				shape.getModelViewMatrix().trackballRotation(Window::width, Window::height, x_mouse, y_mouse, x, y);
				}
			x_mouse = x;
			y_mouse = y;
		}
	}
	else if (right_clicked) { // zoom modelview
		if (y < y_mouse) {
			shape.getModelViewMatrix().scale(1.05,1.05,1.05);
			y_mouse = y;
		}
		else if (y > y_mouse) {
			shape.getModelViewMatrix().scale(0.95, 0.95, 0.95);
			y_mouse = y;
		}
	}
	*/
	if (x_mouse != x) {
		anglex_change = float(x_mouse-x)/anglex_factor;
		//cout << "anglex change: " << anglex_change << '\n';
	}
	if (y_mouse != y) {
		angley_change = float(y-y_mouse)/angley_factor;
		float tmp = angley;
		angley+=angley_change;
		//cout << "HIHI: " << angley << " != " << tmp << '\n';
		if (angley > 90.0) {
			angley = 90.0;
			angley_change = 0.0;
		}
		if (angley < -90.0) {
			angley = -90.0;
			angley_change = 0.0;
		}
		//cout << "anglex change: " << angley_change << '\n';
	}
	
	x_mouse = x;
	y_mouse = y;
	if (warp == true) {
		if (x != Window::width/2 || y != Window::height/2) {
		//glutWarpPointer(Window::width/2, Window::height/2);
		}
	}

	//cout << "(" << x << "," << y << ")\n";
}
		
