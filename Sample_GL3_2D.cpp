#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include<time.h>
#include<stdio.h>
//#include <SDL/SDL.h>
//#include <SDL/SDL_mixer.h>

#include <GL/glew.h>
#include <GL/glu.h>
#include <GL/freeglut.h>

//#include <thread>

//#include<FTGL/ftgl.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define PI 3.14159265

using namespace std;

struct VAO {
	GLuint VertexArrayID;
	GLuint VertexBuffer;
	GLuint ColorBuffer;

	GLenum PrimitiveMode;
	GLenum FillMode;
	int NumVertices;
};
typedef struct VAO VAO;

typedef struct Sprite Sprite;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
	struct VAO* vao = new struct VAO;
	vao->PrimitiveMode = primitive_mode;
	vao->NumVertices = numVertices;
	vao->FillMode = fill_mode;

	// Create Vertex Array Object
	glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
	glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
	glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

	glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
	glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
	glVertexAttribPointer(
			0,                  // attribute 0. Vertices
			3,                  // size (x,y,z)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
	glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
	glVertexAttribPointer(
			1,                  // attribute 1. Color
			3,                  // size (r,g,b)
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);

	return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
	GLfloat* color_buffer_data = new GLfloat [3*numVertices];
	for (int i=0; i<numVertices; i++) {
		color_buffer_data [3*i] = red;
		color_buffer_data [3*i + 1] = green;
		color_buffer_data [3*i + 2] = blue;
	}

	return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
	// Change the Fill Mode for this object
	glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

	// Bind the VAO to use
	glBindVertexArray (vao->VertexArrayID);

	// Enable Vertex Attribute 0 - 3d Vertices
	glEnableVertexAttribArray(0);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

	// Enable Vertex Attribute 1 - Color
	glEnableVertexAttribArray(1);
	// Bind the VBO to use
	glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

	// Draw the geometry !
	glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1;
float rectangle_rot_dir = 1;
float cannon_rot_dir = 1;
float basket1_trans_dir ;
float basket2_trans_dir ;
float cannon_trans_dir ;
bool triangle_rot_status = false;
bool rectangle_rot_status = true;
bool basket1_trans_status = false;
bool basket2_trans_status = false;
bool cannon_rot_status = false;
bool laser_rot_status = false;
bool laser2_rot_status = false;
bool cannon_trans_status = false;
//bool laser_trans_status = false;
//bool laser2_trans_status = false;
float x_change = 0;
float y_change = 0;
float laser_trans_status[4] = {0};
time_t old;
time_t t;
float laser_draw[4] = {1 ,0, 0 ,0};
float flag[4]={0};
float flag_shoot = 0;
float Laser_y[4]={0, 0,0,0};
float Laser_x[4] = {0,0,0,0};
float y_pos = 0;
float flag_c[6] = {0};
int c[6];
float t_Lx[4] = {-3.8,-3.8,-3.8,-3.8};
float t_Ly[4] = {0.25,0.25,0.25,0.25};
float M_x1[3] = {1,2,-0.5};
float M_x2[3] = {2,3, 0};
float M_y1[3] = {0,3, -2};
float M_y2[3] = {1, 2, -1};
int j;
int value = 0;
float x2pos = 1;
float x1pos = 0;
int flag_click = 0;
float cannon_rotation = 0;
float laser_rotation[4] = {0,0,0,0};
float zoom_x=1.0;
float zoom_status=0;
float zoom_dir=0;
float pan_x=0.0;
float speed_brick = 0.03;
float H_x[6] = {0,1.4, 1.9, 2.4, 2.9, 3.4};
float H_y[6] = {0,3.7, 3.7, 3.7 ,3.7, 3.7};
float num = 5;
int status = 1;
long int counter = 0;
int width;
int height;

void play_audio()
{
		system("canberra-gtk-play -f ~/Documents/laser.wav");
}

 void
 showMessage(GLfloat x, GLfloat y, GLfloat z, char *message)
 {
		   glPushMatrix();
		   glDisable(GL_LIGHTING);
		   glTranslatef(x, y, z);
		   glScalef(.02, .02, .02);
		   while (*message) {
			    glutStrokeCharacter(GLUT_STROKE_ROMAN, *message);
			     message++;
			   }
		   glEnable(GL_LIGHTING);
		   glPopMatrix();
		 }

/*void mousescroll(double xoffset, double yoffset)
  {
  if(yoffset == -1)
  {
  zoom_camera/= 1.1;
  }
  else if(yoffset == 1)
  {
  zoom_camera *= 1.1;
  }
  if(zoom_camera <= 1)
  {
  zoom_camera = 1;
  }
  if(zoom_camera >= 4)
  {
  zoom_camera=4;
  }
  if(x_change-400.0f/zoom_camera<-400)
  x_change=-400+400.0f/zoom_camera;
  else if(x_change+400.0f/zoom_camera>400)
  x_change=400-400.0f/zoom_camera;
  if(y_change-300.0f/zoom_camera<-300)
  y_change=-300+300.0f/zoom_camera;
  else if(y_change+300.0f/zoom_camera>300)
  y_change=300-300.0f/zoom_camera;
  Matrices.projection = glm::ortho((float)(-400.0f/zoom_camera+x_change), (float)(400.0f/zoom_camera+x_change), (float)(-300.0f/zoom_camera+y_change), (float)(300.0f/zoom_camera+y_change), 0.1f, 500.0f);
  }
  }
 */
/* Executed when a regular key is pressed */
void keyboardDown (unsigned char key, int x, int y)
{
	switch (key) {
		case 'Q':
		case 'q':
		case 27: //ESC
			exit (0);

		case 'r':
			triangle_rot_status = true;
			break;
		case 'a':
			cannon_rot_status = true;
			//	    if(L_x == false)
			//	    	laser_rot_status = true;
			cannon_rot_dir = 1;
			//			laser_rot_dir = 1;
			break;
		case 'd':
			cannon_rot_status = true;
			//	    if(laser_trans_status == false)
			//	    	laser_rot_status = true;
			cannon_rot_dir = -1;
			//			laser_rot_dir = -1;
			break;
		case 's':
			cannon_trans_status = true;
			cannon_trans_dir = 1;
			break;
		case 'f':
			cannon_trans_status = true;
			cannon_trans_dir = -1;
			break;
		case 'n':
			if(speed_brick < 1)
			speed_brick = speed_brick + 0.01;
			break;
		case 'm':
			if(speed_brick > 0.01)
			speed_brick = speed_brick - 0.01;
			break;
		case ' ':
			if(flag_shoot == 0)
			{
				//thread(play_audio).detach();
				value = counter%4;
			//	system("canberra-gtk-play -f ~/Documents/Laser.wav");
				printf("%d\n", value);
				laser_draw[value] = 1;
				laser_trans_status[value] = 1;
				flag[value] = 1;
	//			system("canberra-gtk-play -f ~/Documents/Laser.wav");
				printf("yes\n");
				time(&old);
				flag_shoot = 1;
			}
			break;
		default:
			break;
	}
}

/* Executed when a regular key is released */
void keyboardUp (unsigned char key, int x, int y)
{
	switch (key) {
		case 'c':
		case 'C':
			rectangle_rot_status = !rectangle_rot_status;
			break;
		case 'p':
		case 'P':
			triangle_rot_status = !triangle_rot_status;
			break;
		case 'x':
			// do something
			break;
		case 'r':
			triangle_rot_status = false;
			break;
		case 'a':
			cannon_rot_status = false;
			laser_rot_status = false;
			break;
		case 'd':
			cannon_rot_status = false;
			laser_rot_status = false;
			break;
		case 's':
			cannon_trans_status = false;
			break;
		case 'f':
			cannon_trans_status = false;
			break;
		default:
			break;
	}
}

/* Executed when a special key is pressed */
void keyboardSpecialDown (int key, int x, int y)
{
	int mod;
	switch(key) {
		case GLUT_KEY_LEFT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT)
			{
				basket2_trans_dir = -1;
				basket2_trans_status = true;
			}
			else if (mod == GLUT_ACTIVE_CTRL)
			{
				basket1_trans_dir = -1;
				basket1_trans_status = true;
			}
			pan_x -= 1.0;
			if(-4/zoom_x + pan_x < -4)
				pan_x = -4 + 4/zoom_x;

			break;
		case GLUT_KEY_RIGHT:
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_SHIFT)
			{
				basket2_trans_dir = 1;
				basket2_trans_status = true;
			}
			else if (mod == GLUT_ACTIVE_CTRL)
			{
				basket1_trans_dir = 1;
				basket1_trans_status = true;
			}
			pan_x += 1.0;
			if(4/zoom_x + pan_x > 4)
				pan_x = 4 - 4/zoom_x;
			break;
		case  GLUT_KEY_UP:
//			zoom_status = 1;
//			zoom_dir = 1;
			zoom_x += .05;
	//		printf("%lf\n",rotate_x);
			break;
		case  GLUT_KEY_DOWN:
			if(zoom_x > 1)
	/*		{
				zoom_status = 1;
				zoom_dir=-1;
			}*/
			zoom_x -= .05;
			cout<<zoom_x<<endl;
			if(-4/zoom_x + pan_x < -4)
				pan_x = -4 + 4/zoom_x;
			if(4/zoom_x + pan_x > 4)
				pan_x = 4 - 4/zoom_x;
			break;
		default:
			break;
	}
	glutPostRedisplay();
}

/* Executed when a special key is released */
void keyboardSpecialUp (int key, int x, int y)
{
	int mod;
	switch(key) {
		case GLUT_KEY_LEFT:
			basket1_trans_status = false;
			basket2_trans_status = false;
			break;
		case GLUT_KEY_RIGHT:
			basket1_trans_status = false;
			basket2_trans_status = false;
			break;
		case  GLUT_KEY_UP:
			zoom_status = 0;
			break;
		case  GLUT_KEY_DOWN:
			zoom_status = 0;
			break;
		default:			
			mod = glutGetModifiers();
			if (mod == GLUT_ACTIVE_ALT)
			{
				basket2_trans_status = false;
			}
			else if (mod == GLUT_ACTIVE_CTRL)
			{
				basket1_trans_status = false;
			}
			break;

	}
}


void scroll(int wheel, int direction, int x, int y)
{
	if(direction == -1)
	{
		//printf("scroll\n");
			zoom_x += .05;
	}
}
/* Executed when a mouse button 'button' is put into state 'state'
   at screen position ('x', 'y')
 */

void reshapeWindow (int width, int height);
void mouseClick (int button, int state, int x, int y)
{
	printf("%d %d\n", x, y);
	float val1;
	float val2;
	float slope;
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
			{
				val1 = (float)x/75 - 4;
				val2 = 4-(float)y/75;
				if(( val1 >= x2pos && val1 <= x2pos + 1) && (val2 <= -3.5 ))
					flag_click = 2;
				else if(( val1 >= x1pos && val1 <= x1pos + 1) && (val2 <= -3.5))
					flag_click = 1;
				else if((val2 >= 0.25 + y_pos && val2 <= 0.25 + y_pos + 0.5) && (val1 <= -3))
					flag_click = 3;
				else
				{
					slope = (val2 - (0.25 + y_pos))/(val1 + 3.8);
					cannon_rotation = (atan(slope)) * 180/PI  ;
					if(flag_shoot == 0)
					{
						value = counter%4;
						laser_rotation[value] = (atan(slope))* 180/PI;	
						//printf("%d\n", value);
						laser_draw[value] = 1;
						laser_trans_status[value] = 1;
						flag[value] = 1;
						//printf("yes\n");
						time(&old);
						flag_shoot = 1;
					}
				}

			}
			if (state == GLUT_UP)
				flag_click = 0;

			break;
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_UP) {
				rectangle_rot_dir *= -1;
			}
			break;
		default:
			break;
	}
}

/* Executed when the mouse moves to position ('x', 'y') */
void mouseMotion (int x, int y)
{ 
	float slope;
	float val1;
	float val2;
	val1 = (float)x/75 - 4;
	val2 = 4-(float)y/75;

	if(flag_click == 2)
	{
		x2pos = ((float)x/75) - 4;
	}
	if(flag_click == 1)
	{
		x1pos = ((float)x/75) - 4;
	}
	if(flag_click == 3)
	{
		if(((3.75 - (float)y/75) >= -3.5) && (3.75 - (float)y/75 <= 3.5))
			y_pos = 3.75 - (float)y/75;
	}
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (int width, int height)
{
	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) width, (GLsizei) height);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
//	gluPerspective(45.0,                  //The camera angle
//			(double)w / (double)h, //The width-to-height ratio
//			1.0,                   //The near z clipping coordinate
//			200.0);       

	// set the projection matrix as perspective/ortho
	// Store the projection matrix in a variable for future use

	// Perspective projection for 3D views
	// Matrices.projection = glm::perspective (fov, (GLfloat) width / (GLfloat) height, 0.1f, 500.0f);

	// Ortho projection for 2D views
	Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *wall2, *wall, *heart, *star, *triangle, *rectangle, *square, *cannon, *laser, *brick1, *brick2, *brick3, *mirror1, *mirror2, *mirror3;

void createSquare ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0,0,
		0,0.5,0,
		0.5,0.5,0,

		0.5,0.5,0,
		0,0,0,
		0.5,0,0
	};

	static const GLfloat color_buffer_data [] = {
		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1
	};
	square = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createMirror ()
{
	static const GLfloat vertex_buffer_data1 [] = {
		1,0,0,
		2,1,0,
	};

	static const GLfloat vertex_buffer_data2 [] = {
		3,2,0,
		2,3,0,
	};

	static const GLfloat vertex_buffer_data3 [] = {
		0,-1,0,
		-0.5,-2,0,
	};

	static const GLfloat color_buffer_data [] = {
		0,0,1,
		0,0,1,
	};


	mirror1 = create3DObject(GL_LINES, 2, vertex_buffer_data1, color_buffer_data, GL_LINE);
	mirror2 = create3DObject(GL_LINES, 2, vertex_buffer_data2, color_buffer_data, GL_LINE);
	mirror3 = create3DObject(GL_LINES, 2, vertex_buffer_data3, color_buffer_data, GL_LINE);

}
void createCannon ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.1,0,
		0,-0.1,0,
		1,0.1,0,

		1,0.1,0,
		0,-0.1,0,
		1,-0.1,0
	};

	static const GLfloat color_buffer_data [] = {
		0,0,1,
		0,0,1,
		0,0,1,

		0,0,1,
		0,0,1,
		0,0,1
	};
	cannon = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createWall ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,-4.0,0,
		0.20,-4.0,0,
		0,4.0,0,

		0.20,-4.0,0,
		0.20,4.0,0,
		0,4.0,0
	};

	static const GLfloat color_buffer_data [] = {
		0.65,0.16,0.16,
		0.65,0.16,0.16,
		0.65,0.16,0.16,

		0.65,0.16,0.16,
		0.65,0.16,0.16,
		0.65,0.16,0.16
	};
	wall = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createWall2 ()
{
	static const GLfloat vertex_buffer_data [] = {
		-4,4.0,0,
		-4,3.25,0,
		4,4.0,0,

		-4,3.25,0,
		4,4.0,0,
		4,3.25,0
	};

	static const GLfloat color_buffer_data [] = {
		0.65,0.16,0.16,
		0.65,0.16,0.16,
		0.65,0.16,0.16,

		0.65,0.16,0.16,
		0.65,0.16,0.16,
		0.65,0.16,0.16
	};
	wall2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createBrick ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0,0,
		0.5,0,0,
		0.5,-0.25,0,

		0.5,-0.25,0,
		0,0,0,
		0,-0.25,0
	};

	static const GLfloat color_buffer_data1 [] = {
		1,0,0,
		1,0,0,
		1,0,0,

		1,0,0,
		1,0,0,
		1,0,0
	};
	static const GLfloat color_buffer_data2 [] = {
		0,1,0.5,
		0,1,0.5,
		0,1,0.5,

		0,1,0.5,
		0,1,0.5,
		0,1,0.5
	};
	static const GLfloat color_buffer_data3 [] = {
		0,0,0,
		0,0,0,
		0,0,0,

		0,0,0,
		0,0,0,
		0,0,0
	};
	brick1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data1, GL_FILL);
	brick2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data2, GL_FILL);
	brick3 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data3, GL_FILL);
}


void createLaser ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.0,0,
		0.2,0.05,0,
		0,0.05,0,

		0.2,0.05,0,
		0.2,0.0,0,
		0,0.0,0
	};
	static const GLfloat color_buffer_data [] = {
		1,0,0,
		1,0,0,
		1,0,0,

		1,0,0,
		1,0,0,
		1,0,0
	};
	laser = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createStar ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.0,0,
		0.3,0.0,0,
		0.15,-0.225,0,

		0.0,-0.15,0,
		0.3,-0.15,0,
		0.15,0.075,0
	};

	static const GLfloat color_buffer_data [] = {
		0.85,0.64,0.12,
		0.85,0.64,0.12,
		0.85,0.64,0.12,

		0.85,0.64,0.12,
		0.85,0.64,0.12,
		0.85,0.64,0.12
	};
	star = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createHeart ()
{
	static const GLfloat vertex_buffer_data [] = {
		0,0.0,0,
		0.4,0.0,0,
		0.2,-0.225,0,

		0.0,0.0,0,
		0.2,0.0,0,
		0.1,0.08,0,

		0.2,0.0,0.0,
		0.4,0.0,0.0,
		0.3,0.08,0
	};

	static const GLfloat color_buffer_data [] = {
		1.0,0.3,0.0,
		1.0,0.3,0.0,
		1.0,0.3,0.0,

		1.0,0.3,0.0,
		1.0,0.3,0.0,
		1.0,0.3,0.0,
		
		1.0,0.3,0.0,
		1.0,0.3,0.0,
		1.0,0.3,0.0
	};
	heart = create3DObject(GL_TRIANGLES, 9, vertex_buffer_data, color_buffer_data, GL_FILL);
}
// Creates the triangle object used in this sample code
/*void createTriangle ()
  {
// ONLY vertices between the bounds specified in glm::ortho will be visible on screen 

// Define vertex array as used in glBegin (GL_TRIANGLES) 
static const GLfloat vertex_buffer_data [] = {
0, 1,0, // vertex 0
-1,-1,0, // vertex 1
1,-1,0, // vertex 2
};

static const GLfloat color_buffer_data [] = {
1,0,0, // color 0
0,1,0, // color 1
0,0,1, // color 2
};

// create3DObject creates and returns a handle to a VAO that can be used later
triangle = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_LINE);
}
 */
VAO *basket1, *basket2;
void createBasket ()
{
	static const GLfloat vertex_buffer_data1 [] = {
		0,0,0, //vertex 1
		1,0,0, //vertex 2
		1,0.5,0, //vertex 3

		0,0,0, //vertex 1
		1,0.5,0, //vertex 2
		0,0.5,0 //vertex 3
	};

	static const GLfloat vertex_buffer_data2 [] = {
		0,0,0, //vertex 1
		1,0,0, //vertex 2
		1,0.5,0, //vertex 3

		0,0,0, //vertex 1
		1,0.5,0, //vertex 2
		0,0.5,0 //vertex 3
	};

	static const GLfloat color_buffer_data1 [] = {
		1,0,0,
		1,0,0,
		1,0,0,

		1,0,0,
		1,0,0,
		1,0,0
	};
	static const GLfloat color_buffer_data2 [] = {
		0,1,0.5,
		0,1,0.5,
		0,1,0.5,

		0,1,0.5,
		0,1,0.5,
		0,1,0.5
	};

	basket1 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data1, color_buffer_data1, GL_FILL);
	basket2 = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);
}


/*
   void createRectangle ()
   {
// GL3 accepts only Triangles. Quads are not supported static
const GLfloat vertex_buffer_data [] = {
-1.2,-1,0, // vertex 1
1.2,-1,0, // vertex 2
1.2, 1,0, // vertex 3

1.2, 1,0, // vertex 3
-1.2, 1,0, // vertex 4
-1.2,-1,0  // vertex 1
};

static const GLfloat color_buffer_data [] = {
1,0,0, // color 1
0,0,1, // color 2
0,1,0, // color 3

0,1,0, // color 3
0.3,0.3,0.3, // color 4
1,0,0  // color 1
};

// create3DObject creates and returns a handle to a VAO that can be used later
rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
 */


float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float mirror_rotation = 0;
//float cannon_rotation = 0;
//float laser_rotation = 0;
//float laser2_rotation = 0;
//float laser_rotation[4] = {0,0,0,0};
float L_x[4] = {1, 0 ,0 ,0};
float L_y[4] = {0, 0 ,0 , 0};
float x_b[10];
float y_b[10];
int score = 0;
int count = 0;
int flag_star = 0;
float s_x = -6;
float s_y = -6;
float y_star=0;
int n_b = 6;
int level = 1;
//long int counter = 0;

void initBricks()
{
	int i;
	for(i=0; i<n_b; i++)
	{
		x_b[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(5)));
		y_b[i] = 4 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4)));
	}
}

/*void initLaser()
  {
  t_Lx = -3.8;
  t_Ly = 0.25;
  }
 */

void collide_star()
{
	int i,k;
	for(i=0; i<4; i++)
	{
			if(((Laser_x[i] ) >= s_x  ) && ((Laser_x[i] )<= s_x+0.5 ) && (Laser_y[i]  <= s_y) && (Laser_y[i] >= s_y -0.5))
			{
				s_x = -6;
				s_y = -6;
				score = score + 50;
				printf("level = %d, score = %d\n", level, score);
				laser_trans_status[i] = 0;
				t_Lx[i] = 5;
				t_Ly[i] = 0;
				L_x[i] = 0;
			}
	}
}
void collide()
{
	int i,k;
	for(i=0; i<4; i++)
	{
		for(k=0; k<n_b; k++)
		{
			if(((Laser_x[i] ) >= x_b[k]-0.15 ) && ((Laser_x[i] )<=x_b[k] + 0.45) && (Laser_y[i]  <= y_b[k] + 0.3) && (Laser_y[i] >= y_b[k]-0.20 ))
			{
				if(c[k] == 3 /*&& flag_c[i] == 0*/)
				{
					score = score + 10;
					printf("level = %d, score = %d\n", level, score);
				}
				if((c[k] == 1) || (c[k] == 2))
				{
					if(num > 0)
					num--;
				}
				laser_trans_status[i] = 0;
				t_Lx[i] = 5;
				t_Ly[i] = 0;
				L_x[i] = 0;

				x_b[k] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(5)));
				y_b[k] = 4 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4)));
			}
		}
	}
}

void collide_basket()
{
	int i;
	for(i=0; i<n_b; i++)
	{
		if((x_b[i] >= x1pos) && ((x_b[i] + 0.5) <= (x1pos + 1) ) && (y_b[i] <= -3.1))
		{
			if(c[i] == 1)
				score = score + 10;
				printf("level = %d, score = %d\n", level, score);
			if(c[i] == 3)
				status = 0;
			x_b[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(5)));
			y_b[i] = 4 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4)));	
		}

		else if((x_b[i] >= x2pos) && ((x_b[i] + 0.5) <= (x2pos + 1)) && (y_b[i] <= -3.1))
		{
			//			printf("x2pos = %f\n", x2pos);
			if(c[i] == 2)
				score = score + 10;
				printf("level = %d, score = %d\n", level, score);
			if(c[i] == 3)
				status = 0;
			x_b[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(5)));
			y_b[i] = 4 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4)));	
		}
	}
}

void collide_mirror()
{
	int i,k;
	float val;
	for(k=0; k<4; k++)
	{
		for(i=0; i<3; i++)
		{
			if((Laser_y[k] >= ((M_y2[i] - M_y1[i])/(M_x2[i] - M_x1[i]))*(Laser_x[k] - M_x2[i]) + M_y2[i] - 0.05) && (Laser_y[k] <=((M_y2[i] - M_y1[i])/(M_x2[i] - M_x1[i]))*( Laser_x[k] - M_x2[i]) +M_y2[i] + 0.05) && (Laser_x[k] <= M_x2[i]) && (Laser_x[k] >= M_x1[i]))
			{
				//				printf("collision\n");
				t_Lx[k] = Laser_x[k];
				t_Ly[k] = Laser_y[k];
				L_x[k] = 0;
				L_y[k] = 0;
				val = 2*(atan((M_y2[i] - M_y1[i])/(M_x2[i] - M_x1[i])) *180/PI) - laser_rotation[k];
				laser_rotation[k] = val;
			}
		}
	}
}

	char message[] = {"prinnttt"};
void print()
{
	int i;
	Matrices.model = glm::mat4(1.0f);
	//char message[] = {"prinnttt"};
//	glPushMatrix();
//	glColor3f(0.0f, 0.0f, 0.0f);
	glRasterPos3f(2.0f, 2.0f, 50.0f);
	glScalef(1.0f, 1.0f, 1.0f);


	for(i=0; message[i] != '\0'; i++)
	{
		//		printf("i=%d\n", i);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, message[i]);
	}
//	glPopMatrix();
	glutPostRedisplay();

}

void zoompan()
{
	zoom_x = zoom_x + 0.05*zoom_status*zoom_dir;
	Matrices.projection = glm::ortho((-4.0f/zoom_x)+pan_x, (4.0f/zoom_x)+pan_x, (-4.0f/(zoom_x)), (4.0f/(zoom_x)),0.1f, 500.0f);
}
void levels()
{
	int i;
	if((level == 1) && (score>100))
	{
		system("canberra-gtk-play -f ~/Documents/happy.wav");
		value = 0;
		level = 2;
		n_b = 8;
		counter = 0;
		score = 0;
		initBricks();
	}
	
}


/* Render the scene with openGL */
/* Edit this function according to your assignment */

//long int counter = 0;
void draw ()
{
	int i;

	// clear the color and depth in the frame buffer
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	
//	Matrices.projection = glm::ortho(-4.0f*rotate_x, 4.0f*rotate_x, -4.0f*rotate_x, 4.0f*rotate_x, 0.1f*rotate_x, 500.0f*rotate_x);
	// use the loaded shader program
	// Don't change unless you know what you are doing
	glUseProgram (programID);

	// Eye - Location of camera. Don't change unless you are sure!!
	glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
	// Target - Where is the camera looking at.  Don't change unless you are sure!!
	glm::vec3 target (0, 0, 0);
	// Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
	glm::vec3 up (0, 1, 0);

	// Compute Camera matrix (view)
	// Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
	//  Don't change unless you are sure!!
	Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

	// Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
	//  Don't change unless you are sure!!
	glm::mat4 VP = Matrices.projection * Matrices.view;

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// For each model you render, since the MVP will be different (at least the M part)
	//  Don't change unless you are sure!!
	glm::mat4 MVP;	// MVP = Projection * View * Model

	// Load identity to model matrix
	/*  Matrices.model = glm::mat4(1.0f);

	// Render your scene 

	glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
	glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
	Matrices.model *= triangleTransform; 
	MVP = VP * Matrices.model; // MVP = p * V * M

	//  Don't change unless you are sure!!
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(triangle);
	 */

//	Matrices.model = glm::mat4(1.0f);
//	glScalef( rotate_x,rotate_x,1.0f );
	
	
	//basket1
	Matrices.model = glm::mat4(1.0f);
//	glScalef( rotate_x,rotate_x,1.0f );

//	glm::mat4 scalebasket = glm::scale (glm::vec3( rotate_x,rotate_x,1.0f));
	glm::mat4 translateBasket1 = glm::translate (glm::vec3(x1pos, -3.8, 0));        // glTranslatef	
//	glm::mat4 basketTransform = scalebasket * translateBasket1;
	Matrices.model *= (translateBasket1);
	MVP = VP * Matrices.model;

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(basket1);


	for(i=0; i<n_b; i++)
	{
		Matrices.model = glm::mat4(1.0f);

		glm::mat4 translateBrick = glm::translate (glm::vec3(x_b[i], y_b[i], 0));        // glTranslatef
		Matrices.model *= (translateBrick);
		MVP = VP * Matrices.model;

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

		if(flag_c[i] == 0)
		{
			if(i%3 == 0)
			{
				c[i]=1;
				draw3DObject(brick1);
			}
			else if(i%3 == 1)
			{
				c[i]=2;
				draw3DObject(brick2);
			}
			else if(i%3 == 2)
			{
				c[i]=3;
				draw3DObject(brick3);
			}
		}
	}

	//cannon
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateCannon = glm::translate (glm::vec3(-3.8, 0.25+ y_pos, 0.0)); // glTranslatef
	glm::mat4 rotateCannon = glm::rotate((float)(cannon_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 cannonTransform = translateCannon * rotateCannon;
	Matrices.model *= cannonTransform; 
	MVP = VP * Matrices.model;

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(cannon);

	//laser
	for(i=0; i<4; i++)
	{
		Matrices.model = glm::mat4(1.0f);
		glm::mat4 translateLaser = glm::translate (glm::vec3(t_Lx[i] , t_Ly[i]+ L_y[i], 0.0)); // glTranslatef
		glm::mat4 rotateLaser = glm::rotate((float)(laser_rotation[i]*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
		glm::mat4 translate2_Laser = glm::translate (glm::vec3(L_x[i] , 0.0, 0.0)); // glTranslatef
		glm::mat4 laserTransform = translateLaser *  rotateLaser * translate2_Laser;
		Matrices.model *= laserTransform; 
		MVP = VP * Matrices.model;

		glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
		if(laser_draw[i] == 1)
			draw3DObject(laser);
	}

	//basket2  
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateBasket2 = glm::translate (glm::vec3(x2pos, -3.8, 0));        // glTranslatef
	Matrices.model *= (translateBasket2);
	MVP = VP * Matrices.model;

	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(basket2);

	//square
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateSquare = glm::translate (glm::vec3(-4, 0.0 + y_pos, 0.0)); // glTranslatef
	Matrices.model *= (translateSquare);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(square);

//	topwall
	Matrices.model = glm::mat4(1.0f);

	MVP = VP;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(wall2);
	
//heart
	for(i=1; i<=num; i++)
	{
	Matrices.model = glm::mat4(1.0f);
	glm::mat4 translateHeart = glm::translate (glm::vec3(H_x[i], H_y[i], 0.0)); // glTranslatef

	Matrices.model *= (translateHeart);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(heart);
	}
	
	//mirror1  
	Matrices.model = glm::mat4(1.0f);

	MVP = VP;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(mirror1);

	//mirror2
	Matrices.model = glm::mat4(1.0f);

	MVP = VP;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(mirror2);

	//mirror3
	Matrices.model = glm::mat4(1.0f);

	MVP = VP;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(mirror3);


//	wall
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateWall = glm::translate (glm::vec3(-4.0, 0.0, 0.0)); // glTranslatef
	Matrices.model *= (translateWall);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(wall);

//	wall2
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateWall2 = glm::translate (glm::vec3(3.8, 0.0, 0.0)); // glTranslatef
	Matrices.model *= (translateWall2);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(wall);

//	wall3
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 rotateWall = glm::rotate((float)(90.f*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
	glm::mat4 translateWall3 = glm::translate (glm::vec3(-4.0, 0.0, 0.0)); // glTranslatef
	glm::mat4 WallTransform = rotateWall * translateWall3;
	Matrices.model *= (WallTransform);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(wall);

//	star
	Matrices.model = glm::mat4(1.0f);

	glm::mat4 translateStar = glm::translate (glm::vec3(s_x, s_y + y_star, 0.0)); // glTranslatef
	Matrices.model *= (translateStar);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(star);
	//rectangle  
	/*  Matrices.model = glm::mat4(1.0f);

	    glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
	    glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
	    Matrices.model *= (translateRectangle * rotateRectangle);
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

	// draw3DObject draws the VAO given to it using current MVP matrix
	draw3DObject(rectangle);
	 */
	// Swap the frame buffers
//	glLoadIdentity();
	
	glColor3f(0.0f, 0.0f, 0.0f);
	glPushMatrix();
	Matrices.model = glm::mat4(1.0f);
	print();
//	 showMessage(0, 0, 4.1, "Spin me.");
	glPopMatrix();
	glutSwapBuffers ();

	// Increment angles
	float increments = 1;

	//camera_rotation_angle++; // Simulating camera rotation
	cannon_rotation = cannon_rotation + increments*cannon_rot_dir*cannon_rot_status;
	mirror_rotation = mirror_rotation + increments;

	for(i=0; i<4; i++)
	{
		if(flag[i] == 0)
		{
			L_y[i] = y_pos;
			laser_rotation[i] = cannon_rotation;
		}
	}
	/*	L_x[0] = L_x[0] + 0.05*laser_trans_status;
		L_x[1] = L_x[1] + 0.05*laser2_trans_status;
	 */
	for(i=0; i<4; i++)
	{
		if(laser_draw[i] == 1)
			L_x[i] = L_x[i] + 0.08*laser_trans_status[i];
	}
	x1pos = x1pos + 0.05*basket1_trans_status*basket1_trans_dir;
	x2pos = x2pos + 0.05*basket2_trans_status*basket2_trans_dir;
	y_pos = y_pos + 0.05*cannon_trans_status*cannon_trans_dir;


	for(i=0; i<n_b; i++)
	{
		y_b[i] = y_b[i] - speed_brick;
	}

	for(i=0; i<4; i++)
	{
		Laser_x[i] = (L_x[i])*cos(laser_rotation[i]*(M_PI/180)) + t_Lx[i];
		Laser_y[i] = (L_x[i])*sin(laser_rotation[i]*(M_PI/180)) + L_y[i] + t_Ly[i];
	}

	for(i=0; i<4; i++)
	{
		if((Laser_x[i] <= -3.8) || (Laser_x[i] >= 3.8) || (Laser_y[i] >= 4) || (Laser_y[i] <= -3.8))
		{
			laser_draw[i] = 0;
			laser_trans_status[i] = 0;
		}
	}

	//int value;
	if(flag_shoot == 1)
	{
		time(&t);
		double diff;
		diff = difftime(old, t);
		//		printf("%lf\n", diff);
		if(diff <= -1)
		{
			counter++;
			value = counter%4;
			laser_draw[value] = 1;
			laser_trans_status[value] = 0;
			t_Lx[value] = -3.8;
			t_Ly[value] = 0.25;
			L_x[value] = 1;
			flag[value] = 0;
			flag_shoot = 0;
		}
	}

	for(i=0; i<n_b; i++)
	{
		if((int)y_b[i] == -4.0)
		{
			x_b[i] = -2 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(5)));
			y_b[i] = 4 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(4)));
		}
	}
	count++;
	if(count==800)
	{
//		printf("OK\n");
		//printf("%f\n", y_star);
		flag_star=1;
		count = 0;
		s_x = -3 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(6)));
		s_y = 3 + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(-5)));
	}
	if(flag_star == 1)
	{
//		y_star = y_star - 0.02;
		if(count == 200)
		{
			flag_star=0;
			y_star = 0;
			s_x = -6;
			s_y = -6;
		}
	}
	if(num == 0)
	{
		status = 0;
	}
	if(status == 0)
	{
		printf("GAME OVER\n");
		system("canberra-gtk-play -f ~/Documents/sad.wav");
		exit(0);
	}

	collide();
	collide_mirror();
	collide_basket();
	collide_star();
	zoompan();
	levels();
	glutPostRedisplay();
	//	print();
//		printf("score = %d\n", score);
	//printf("%f\n", flag_shoot);
}

/* Executed when the program is idle (no I/O activity) */
void idle () {
	// OpenGL should never stop drawing
	// can draw the same scene or a modified scene
	draw (); // drawing same scene
}


/* Initialise glut window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
void initGLUT (int& argc, char** argv, int width, int height)
{
	// Init glut
	glutInit (&argc, argv);

	// Init glut window
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitContextVersion (3, 3); // Init GL 3.3
	glutInitContextFlags (GLUT_CORE_PROFILE); // Use Core profile - older functions are deprecated
	glutInitWindowSize (width, height);
	glutCreateWindow ("Sample OpenGL3.3 Application");

	// Initialize GLEW, Needed in Core profile
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		cout << "Error: Failed to initialise GLEW : "<< glewGetErrorString(err) << endl;
		exit (1);
	}

	// register glut callbacks
	glutKeyboardFunc (keyboardDown);
	glutKeyboardUpFunc (keyboardUp);

	glutSpecialFunc (keyboardSpecialDown);
	glutSpecialUpFunc (keyboardSpecialUp);

	glutMouseFunc (mouseClick);
	glutMotionFunc (mouseMotion);
	//	glutScrollFunc (mousescroll);

	glutMouseWheelFunc(scroll);
	glutReshapeFunc (reshapeWindow);

	glutDisplayFunc (draw); // function to draw when active
	glutIdleFunc (idle); // function to draw when idle (no I/O activity)

	glutIgnoreKeyRepeat (true); // Ignore keys held down
}

/* Process menu option 'op' */
void menu(int op)
{
	switch(op)
	{
		case 'Q':
		case 'q':
			exit(0);
	}
}

void addGLUTMenus ()
{
	// create sub menus
	int subMenu = glutCreateMenu (menu);
	glutAddMenuEntry ("Do Nothing", 0);
	glutAddMenuEntry ("Really Quit", 'q');

	// create main "middle click" menu
	glutCreateMenu (menu);
	glutAddSubMenu ("Sub Menu", subMenu);
	glutAddMenuEntry ("Quit", 'q');
	glutAttachMenu (GLUT_MIDDLE_BUTTON);
}


/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (int width, int height)
{
	// Create the models
	//	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (width, height);

	// Background color of the scene
	glClearColor (0.87f, 0.87f, 0.87f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

	//	createRectangle ();
	createBasket ();
	createSquare ();
	createCannon ();
	createMirror ();
	createLaser ();
	createBrick ();
	createStar();
	createWall2();
	createHeart();
	createWall();

	cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
	cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
	cout << "VERSION: " << glGetString(GL_VERSION) << endl;
	cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	width = 600;
	height = 600;
	initBricks();
	//	initLaser();
	initGLUT (argc, argv, width, height);

	print();
	addGLUTMenus ();

	initGL (width, height);

	glutMainLoop ();

	return 0;
}
