#include<bits/stdc++.h>
#include<cstdlib>
#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

using namespace std;
#define V_INIT 30
int pn = 9;
struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;
    GLenum PrimitiveMode;
    GLenum FillMode;
    GLenum TextureBuffer;
    GLenum TextureID;
    int NumVertices;
};
typedef struct VAO VAO;
struct GLMatrices {
    glm::mat4 projection;
    glm::mat4 model;
    glm::mat4 view;
    GLuint MatrixID;
    GLuint TexMatrixID;
} Matrices;
typedef struct Circle{
    VAO *circle;
    float xcentre, ycentre, radius, v,vx;
    int collflagy,collflagx,collflag[101];
    float projectile;
    int exist, detectability,score;
}Circle;
typedef struct Wall{
    VAO *rectangle;
    int life;
    float xcentre, ycentre,l,b;
    float bounce_factor,friction;
    int score;
}Wall;


struct FTGLFont {
    FTFont* font;
    GLuint fontMatrixID;
    GLuint fontColorID;
} GL3Font;
GLuint programID, fontProgramID,textureProgramID;
/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if(VertexShaderStream.is_open())
    {
	std::string Line = "";
	while(getline(VertexShaderStream, Line))
	    VertexShaderCode += "\n" + Line;
	VertexShaderStream.close();
    }
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
    char const * VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
    glCompileShader(VertexShaderID);
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> VertexShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
    char const * FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
    glCompileShader(FragmentShaderID);
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
    glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
    fprintf(stdout, "Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
    glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
    fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);
    return ProgramID;
}
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

glm::vec3 getRGBfromHue (int hue)
{
    float intp;
    float fracp = modff(hue/60.0, &intp);
    float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

    if (hue < 60)
	return glm::vec3(1,x,0);
    else if (hue < 120)
	return glm::vec3(x,1,0);
    else if (hue < 180)
	return glm::vec3(0,1,x);
    else if (hue < 240)
	return glm::vec3(0,x,1);
    else if (hue < 300)
	return glm::vec3(x,0,1);
    else
	return glm::vec3(1,0,x);
}



/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
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
void draw3DObject (struct VAO* vao)
{
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);
    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices);
}

struct VAO* create3DTexturedObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* texture_buffer_data, GLuint textureID, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;
    vao->TextureID = textureID;
    glGenVertexArrays(1, &(vao->VertexArrayID)); 
    glGenBuffers (1, &(vao->VertexBuffer)); 
    glGenBuffers (1, &(vao->TextureBuffer)); 

    glBindVertexArray (vao->VertexArrayID); 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(
	    0,                  
	    3,                  
	    GL_FLOAT,           
	    GL_FALSE,           	    0,
	    (void*)0            
	    );

    glBindBuffer (GL_ARRAY_BUFFER, vao->TextureBuffer); // Bind the VBO textures
    glBufferData (GL_ARRAY_BUFFER, 2*numVertices*sizeof(GLfloat), texture_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
	    2,                  
	    2,                  
	    GL_FLOAT,               GL_FALSE,    
	    0,                  
	    (void*)0            
	    );

    return vao;
}


void draw3DTexturedObject (struct VAO* vao)
{
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    glBindVertexArray (vao->VertexArrayID);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    glBindTexture(GL_TEXTURE_2D, vao->TextureID);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, vao->TextureBuffer);

    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices);

    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint createTexture (const char* filename)
{
    GLuint TextureID;
    glGenTextures(1, &TextureID);
    glBindTexture(GL_TEXTURE_2D, TextureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int twidth, theight;
    unsigned char* image = SOIL_load_image(filename, &twidth, &theight, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, twidth, theight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D); // Generate MipMaps to use
    SOIL_free_image_data(image); // Free the data read from file after creating opengl texture
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind texture when done, so we won't accidentily mess it up

    return TextureID;
}

/**************************
 * Customizable functions *
 **************************/
int total_score = 0;

float triangle_rot_dir = 1,change=0;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float birdx = 0.0;
float birdy = 0.0;
float adddown,addleft,addright;
float addup = adddown = addleft = addright = 0;
/* Executed when a regular key is pressed/released/held-down */
Circle ball;
float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;
float circle_rotation = 1;
float circle_radius = 13;
void launch();
float trans = -10.0;
float g = -0.423;int allow_acc=1;
float ELEVATION = -90;
int POWER = 2;
int lives=20;
bool RESET = false;
int space_pressed = 0, initialised = 0;
float cangle;
double xclick,yclick;
//mouse events
void returnMouse(GLFWwindow *window, double xpos, double ypos)
{
    xpos -= 900;
    ypos = 480-ypos;
    xclick = xpos;
    yclick = ypos;
    glm::vec2 pointer(xpos,ypos);
    glm::vec2 cannon(-840,-231);
    glm::vec2 di = pointer - cannon;
    glm::vec2 axis(1,0);
    float dota3 = glm::dot(di,axis);
    float cosine = dota3 / glm::length(di);
    cangle = acos(cosine)*180.0f/M_PI;
    if(ypos<-232)
    {
	cangle = -cangle;
    }
    cangle -= 90;

}
int zoomin = 0, zoomout = 0, panleft = 0, panright = 0,scrolldir = 0,panup=0,pandown = 0;
void returnScroll(GLFWwindow * window, double horizontal, double vertical)
{
    scrolldir = vertical;

}
///////////////////////////////////////////
/* Prefered for Keyboard events */
void initialise()
{
    ball.v = 28+ELEVATION/3;
    if(ball.v<0)
	ball.v = -0.1;
    //ball.vx = 90*POWER;
    ball.vx = V_INIT*POWER;
    initialised = 1;
}
float zf = 1.0f,pan = 0.0f,pany = 0.0f;
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
	switch (key) {
	    case GLFW_KEY_DOWN:
		zoomout = 0;
		break;
	    case GLFW_KEY_UP:
		zoomin = 0;
		break;
	    case GLFW_KEY_LEFT:
		panleft = 0;
		break;
	    case GLFW_KEY_RIGHT:
		panright = 0;
		break;
	    case GLFW_KEY_P:
		panup = 0;
		break;
	    case GLFW_KEY_L:
		pandown = 0;
		break;

	    case GLFW_KEY_A:
		change = 0;
		break;
	    case GLFW_KEY_B:
		change = 0;
		break;
	    case GLFW_KEY_F:
		POWER++;
		if(POWER>4)
		    POWER =4;
		break;
	    case GLFW_KEY_S:
		POWER--;
		if(POWER<1)
		    POWER =1;
		break;
	    case GLFW_KEY_SPACE:
		//launchi
		if(lives && !space_pressed)
		{
		    lives--;
		    space_pressed = 1;
		}
		break;
	    default:
		break;
	}
    }
    else if (action == GLFW_PRESS) {
	switch (key) {
	    case GLFW_KEY_A:
		change = 0.5;
		break;
	    case GLFW_KEY_B:
		change = -0.5;
		break;
	    case GLFW_KEY_UP:
		zoomin = 1;
		break;
	    case GLFW_KEY_DOWN:
		zoomout = 1;
		break;
	    case GLFW_KEY_LEFT:
		panleft = 1;
		break;
	    case GLFW_KEY_RIGHT:
		panright = 1;
		break;
	    case GLFW_KEY_P:
		panup = 1;
		break;
	    case GLFW_KEY_L:
		pandown = 1;
		break;

	    case GLFW_KEY_ESCAPE:
		quit(window);
		break;
	    case GLFW_KEY_R:
		RESET = true;
		break;
	    default:
		break;
	}
    }
}
/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
    switch (key) {
	case 'Q':
	case 'q':
	    quit(window);
	    break;
	default:
	    break;
    }
}
int mouse_pressed = 0;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
	case GLFW_MOUSE_BUTTON_LEFT:
	    if (action == GLFW_PRESS)
	    {
		if(yclick < 270/zf)
		    mouse_pressed = 1;
		else if(yclick>(394-pany)/zf && yclick < (406-pany)/zf)
		{
		    if(xclick>(-821-pan)/zf && xclick<(-799-pan)/zf)
		    {
			POWER--;
			if(POWER<1)
			    POWER =1;
		    }
		    else if(xclick>(-710-pan)/zf && xclick<(-680-pan)/zf)
		    {
			POWER++;
			if(POWER>4)
			    POWER =4;
		    }
		}

		else if(yclick>(285-pany)/zf && yclick<(315-pany)/zf && xclick<(-734-pan)/zf && xclick>(-765-pan)/zf)
		{
		    if(lives && !space_pressed)
		    {
			lives--;
			space_pressed = 1;
		    }
		}

	    }
	    if (action == GLFW_RELEASE)
		mouse_pressed = 0;
	    break;
	case GLFW_MOUSE_BUTTON_RIGHT:
	    if (action == GLFW_PRESS)
	    {
		if(yclick>(285-pany)/zf && yclick<(315-pany)/zf && xclick<(-734-pan)/zf && xclick>(-765-pan)/zf)
		    RESET = true;

	    }
	    break;
	default:
	    break;
    }
}


/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);
    GLfloat fov = 90.0f;
    glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);
    Matrices.projection = glm::ortho(zf*((-900.0f)+pan), zf*((900.0f)+pan), zf*(-480.0f+pany), zf*(480.0f+pany), 0.1f, 500.0f);
}
Circle shaft,ammo[20];
float r1=53,r2=6,a1=0,a2=0;
void createAmmo()
{
    GLfloat vertex_buffer_data [1089], color_buffer_data [1089];
    for(int i=0;i<1089;i++)
    {
	if(i!=0)
	{
	    color_buffer_data[i]=0.3;
	    vertex_buffer_data[i++] = r1*cos(a1*M_PI/180.0f) - 750;
	    color_buffer_data[i]=0.06;
	    vertex_buffer_data[i++] = r1*sin(a1*M_PI/180.0f) + 300;
	    color_buffer_data[i]=0.6;
	    vertex_buffer_data[i]=0;
	    a1++;
	}
	else
	{
	    color_buffer_data[i] = 0.3;
	    vertex_buffer_data[i++] = -750;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[	i++] = 300;
	    color_buffer_data[i] = 0.4;
	    vertex_buffer_data[i] = 0;
	}
    }
    shaft.circle = create3DObject(GL_TRIANGLE_FAN, 363, vertex_buffer_data, color_buffer_data, GL_FILL);
    GLfloat vertex_buffer_data2 [20][1089], color_buffer_data2 [1089];
    for(int i=0;i<1089;i++)
	color_buffer_data2[i] = 0;
    a1 = 0;
    for(int j=0;j<20;j++)
    {a2 = 0;
	for(int i=0;i<1089;i++)
	{
	    if(i!=0)
	    {
		vertex_buffer_data2[j][i++]=r2*cos(a2*M_PI/180.0f)-750+42*cos(a1*M_PI/180.0f);
		vertex_buffer_data2[j][i++]=r2*sin(a2*M_PI/180.0f)+300+42*sin(a1*M_PI/180.0f);
		vertex_buffer_data2[j][i]=0;
		a2++;
	    }
	    else
	    {
		vertex_buffer_data2[j][i++] = 42*cos(a1*M_PI/180.0f) - 750;
		vertex_buffer_data2[j][i++] = 42*sin(a1*M_PI/180.0f) + 300;
		vertex_buffer_data2[j][i] = 0;
	    }
	}
	a1 += 18;
	ammo[j].circle= create3DObject( GL_TRIANGLE_FAN, 363, vertex_buffer_data2[j], color_buffer_data2, GL_FILL);
    }
}

VAO *increase, *decrease;
void createTriangle ()
{
    static const GLfloat vertex_buffer_data [] = {
	20,0,0, // vertex 0
	0,20,0, // vertex 1i
	0,0,0, // vertex 2
    };
    static const GLfloat color_buffer_data [] = {
	0.61,0.164,0.15, // color 0
	0.61,0.164,0.15, // color 0
	0.61,0.164,0.15, // color 0
    };
    increase = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
    decrease = create3DObject(GL_TRIANGLES, 3, vertex_buffer_data, color_buffer_data, GL_FILL);
}
Wall boundary[100];
void setParams(int i, float length, float breadth)
{
    boundary[i].xcentre = 0;
    boundary[i].ycentre = 0;
    boundary[i].l = length;
    boundary[i].b = breadth;
    boundary[i].score = 0;
    boundary[i].friction = 0.5;
    boundary[i].bounce_factor = 0.8;
}
void createRectangle()
{
    static const GLfloat vertex_buffer_data [] = {
	1100, -10,0, // vertex 3
	-1100,10,0, // vertex 1
	-1100,-10,0, // vertex 2

	1100, 10,0, // vertex 3
	1100, -10,0, // vertex 4
	-1100,10,0  // vertex 1
    };
    setParams(0,2200,20);
    setParams(5,2200,20);
    setParams(6,20,2200);
    setParams(7,20,2200);
    static const GLfloat color_buffer_data [] = {
	0.231,0.325,0.137, // color 1
	0.118,0.294,0.0823, // color 2
	0.231,0.235,0.137, // color 3

	0.231,0.325,0.137, // color 3
	0.231,0.325,0.137, // color 4
	0.118,0.294,0.137  // color 1
    };
    boundary[0].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    boundary[5].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    boundary[6].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    boundary[7].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

void createRectangle1()
{
    static const GLfloat vertex_buffer_data [] = {
	-5,-20,0, -5,20,0, 5,20,0,
	5,20,0, 5,-20,0, -5,-20,0
    };
    static const GLfloat color_buffer_data [] = {
	1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1
    };
    int i = 95;
    for(;i<99;i++){
	boundary[i].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
    }
}


void createCircle ()
{
    int i;

    GLfloat vertex_buffer_data [1089], color_buffer_data [1089];
    for(i=0;i<1089;i++)
    {
	if(i!=0)
	{
	    color_buffer_data[i]=1;
	    vertex_buffer_data[i++] = circle_radius*cos(circle_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i++] = circle_radius*sin(circle_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i]=0;
	    circle_rotation  = circle_rotation + 1;
	}
	else
	{
	    color_buffer_data[i] = 1;
	    vertex_buffer_data[i++] = 0;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[	i++] = 0;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[i] = 0;
	}
    }
    ball.radius = circle_radius;
    ball.xcentre = 0;
    ball.ycentre = 0;
    for(int j=0;j<100;j++)
	ball.collflag[j] = 0;
    //ELEVATION iAFFECTS INIT V's
    //
    birdx = 0.0;
    birdy = 0.0;
    ball.circle = create3DObject(GL_TRIANGLE_FAN, 363, vertex_buffer_data, color_buffer_data, GL_FILL);
}
Circle cannon;
Wall barrel;
void createPlatform()
{
    static const GLfloat vertex_buffer_datiaa [] = {
	-50,-100,0, -50,100,0, 50,100,0,
	50,100,0, 50,-100,0, -50,-100,0
    };
    static const GLfloat color_buffer_datiaa [] = {
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0 
    };
    setParams(8,100,200);
    boundary[8].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, color_buffer_datiaa, GL_FILL);
    int i;
    int R = 50;
    GLfloat vertex_buffer_data [1089], color_buffer_data [1089];
    for(i=0;i<1089;i++)
    {
	if(i!=0)
	{
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i++] = R*cos(circle_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i++] = R*sin(circle_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i]=0;
	    circle_rotation  = circle_rotation + 1;
	}
	else
	{
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[i++] = 0;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[	i++] = 0;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[i] = 0;
	}
    }
    cannon.radius = 50;
    cannon.xcentre = 0;
    cannon.ycentre = 0;
    cannon.circle = create3DObject(GL_TRIANGLE_FAN, 363, vertex_buffer_data, color_buffer_data, GL_FILL);


    static const GLfloat vertex_buffer_data2 [] = {
	-14,4,0, 14,75,0, -14,75,0,
	14,75,0, 14,4,0, -14,4,0
    };
    static const GLfloat color_buffer_data2 [] = {
	0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0 
    };
    barrel.rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data2, color_buffer_data2, GL_FILL);


}

void createStone(GLuint tid)
{
    static const GLfloat vertex_buffer_datiaa [] = {
	-20,-50,0, -20,50,0, 20,50,0,
	20,50,0, 20,-50,0, -20,-50,0
    };
    
    static const GLfloat texture_buffer_data [] = {
	0,1, // TexCoord 1 - bot left
	1,1, // TexCoord 2 - bot right
	1,0, // TexCoord 3 - top right

	1,0, // TexCoord 3 - top right
	0,0, // TexCoord 4 - top left
	0,1  // TexCoord 1 - bot left
    };
    for(int i=1;i<=4;i++){
	setParams(i,40,100);
	boundary[i].rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, texture_buffer_data,tid, GL_FILL);
    }
}
void createWood(GLuint tid)
{
    static const GLfloat vertex_buffer_datiaa [] = {
	-18,-45,0, -18,45,0, 18,45,0,
	18,45,0, 18,-45,0, -18,-45,0
    };
    static const GLfloat color_buffer_datiaa [] = {
	0.588,0.435,0.2,
	0.588,0.435,0.2,
	0.588,0.435,0.2,
	0.588,0.435,0.2,
	0.588,0.435,0.2,
	0.588,0.435,0.2,
    };
    static const GLfloat texture_buffer_data [] = {
	0,1, // TexCoord 1 - bot left
	1,1, // TexCoord 2 - bot right
	1,0, // TexCoord 3 - top right

	1,0, // TexCoord 3 - top right
	0,0, // TexCoord 4 - top left
	0,1  // TexCoord 1 - bot left
    };
    for(int i=1;i<=4;i++){
	boundary[i+9].life = 2;
	setParams(i+9,36,90);
	boundary[i].score = 10;
	boundary[i+9].rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, texture_buffer_data,tid, GL_FILL);
	//boundary[i+9].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, color_buffer_datiaa, GL_FILL);
    }
}

void createIce(GLuint tid)
{
    static const GLfloat vertex_buffer_datiaa [] = {
	-18,-45,0, -18,45,0, 18,45,0,
	18,45,0, 18,-45,0, -18,-45,0
    };
static const GLfloat texture_buffer_data [] = {
	0,1, // TexCoord 1 - bot left
	1,1, // TexCoord 2 - bot right
	1,0, // TexCoord 3 - top right

	1,0, // TexCoord 3 - top right
	0,0, // TexCoord 4 - top left
	0,1  // TexCoord 1 - bot left
    };
    
    for(int i=14;i<=20;i++){
	boundary[i].life = 1;
	setParams(i,36,90);
	boundary[i].score = 20;
	boundary[i].rectangle = create3DTexturedObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, texture_buffer_data,tid, GL_FILL);
	//boundary[i].rectangle = create3DObject(GL_TRIANGLES, 6, vertex_buffer_datiaa, color_buffer_datiaa, GL_FILL);
    }
}
VAO *tree[10];
void createTrees()
{
    static const GLfloat vb[] = {
	0,0,0, 50,0,0, 25,43.33,0,
	0,20,0, 50,20,0, 25,63.33,0,
	0,40,0, 50,40,0, 25,83.33,0,

	20,0,0, 30,0,0, 20,-10,0,
	20,-10,0, 30,-10,0, 30,0,0
    };
    static const GLfloat cb[] = {
	0.088,0.199,0.029, 0.088,0.199,0.029, 0.088,0.199,0.029,
	0.088,0.229,0.029, 0.088,0.229,0.029, 0.088,0.229,0.029,
	0.088,0.289,0.029, 0.088,0.289,0.029, 0.088,0.289,0.029,
	0.1545,0.1,0.04, 0.1545,0.1,0.04, 0.1545,0.1,0.04, 
	0.1545,0.1,0.04, 0.1545,0.1,0.04, 0.1545,0.1,0.04, 
    };
    for(int i = 0; i<10;i++)
	tree[i]= create3DObject(GL_TRIANGLES, 15, vb, cb, GL_FILL);

}
VAO* BP[2];
Circle pig[9];
void createBadaPig()
{
    static const GLfloat vb [] = {
	0,0,0, 30,0,0, 15,25.98,0,
	15,25.98,0,-15,25.98,0, 0,0,0,
	0,0,0,-30,0,0,-15,25.98,0,
	0,0,0,30,0,0,15,-25.98,0,
	15,-25.98,0,-15,-25.98,0,0,0,0,
	0,0,0,-15,-25.98,0,-30,0,0,
    };
    static const GLfloat cb[] = {
	0,1,0, 0,1,0, 0,1,0,
	0,1,0, 0,1,0, 0,1,0,
	0,1,0, 0,1,0, 0,1,0,
	0,1,0, 0,1,0, 0,1,0,
	0,1,0, 0,1,0, 0,1,0,
	0,1,0, 0,1,0, 0,1,0,
    };
    for(int i=0;i<2;i++)
	BP[i] = create3DObject(GL_TRIANGLES, 18, vb, cb, GL_FILL);

    pig[7].detectability = pig[8].detectability = 1;
    pig[7].score = pig[8].score = 100;
    pig[7].xcentre = pig[7].ycentre = pig[8].xcentre = pig[8].ycentre = 0;
    pig[7].radius = pig[8].radius = 30;

}

float pig_rotation = 1;
void createPig ()
{
    int i;
    GLfloat vertex_buffer_data [1089], color_buffer_data [1089], vbd1 [1089],cbd1[1089], vbd2[1089], cbd2[1089];
    for(i=0;i<1089;i++)
    {
	if(i!=0)
	{
	    color_buffer_data[i]=1;
	    vertex_buffer_data[i++] = 20*cos(pig_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0.7;
	    vertex_buffer_data[i++] = 20*sin(pig_rotation*M_PI/180.0f);
	    color_buffer_data[i]=0;
	    vertex_buffer_data[i]=0.2;
	    pig_rotation++;
	}
	else
	{
	    color_buffer_data[i] = 1;
	    vertex_buffer_data[i++] = 0;
	    color_buffer_data[i] = 0.7;
	    vertex_buffer_data[i++] = 0;
	    color_buffer_data[i] = 0;
	    vertex_buffer_data[i] = 0.2;
	}
    }
    for(i=0;i<7;i++){
	pig[i].radius = 20;
	pig[i].score = 50;
	pig[i].detectability = 1;
	pig[i].v = 0; pig[i].vx = 0;pig[i].xcentre = 0;pig[i].ycentre = 0;
	pig[i].circle = create3DObject(GL_TRIANGLE_FAN, 363, vertex_buffer_data, color_buffer_data, GL_FILL);
    }
}
float t1[10],t2[10];
bool allowed = false;
typedef struct point{
    float x,y;
    VAO* vertex;
}PP;
PP points[150];
void traceroute()
{

    GLfloat vb[] = {0,0,0};
    GLfloat cb[]={1,1,0.2};

    for(int i=0;i<200;i++)
    {
	points[i].vertex = create3DObject(GL_POINTS, 1, vb, cb, GL_FILL);
    }
}
int num=0;int num2=0;float yyc = 2.5,yyd=0;
float tempx[150],tempy[150];
int old;
void draw ()
{
    //printf("%d\n",total_score);
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram (programID);
    glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
    glm::vec3 target (0, 0, 0);
    glm::vec3 up (0, 0, 0);
    Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane
    glm::mat4 VP = Matrices.projection * Matrices.view;
    glm::mat4 MVP;  // MVP = Projection * View * Model

    if(zoomin)
	if(zf>0.25)
	    zf -= 0.01;
    if(zoomout)
	if(zf<1)
	    zf += 0.01;
    if(scrolldir == 1)
	if(zf>0.25)
	    zf -= 0.05;
    if(scrolldir == -1)
	if(zf<1)
	    zf += 0.05;
    scrolldir = 0;
    if(panleft)
	pan -= 10;
    if(panright)
	pan += 10;
    if(panup)
	pany += 10;
    if(pandown)
	pany -= 10;
    // Load identity to model matrix
    Matrices.model = glm::mat4(1.0f);
    //trans -= 0.2;
    //glm::mat4 translateTriangle = glm::translate (glm::vec3(200+trans, 200.0f, 0.0f));
    glm::vec3 myRotationAxis(0,0,1);
    glm::mat4 rotateTriangle =glm::rotate((float)(-45*M_PI/180.0f), myRotationAxis);  
    glm::mat4 translateTriangle = glm::translate (glm::vec3(-810, 400.0f, 0.0f));
    glm::mat4 triangleTransform = translateTriangle*rotateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(increase);
    /////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    //trans -= 0.2;
    //glm::mat4 translateTriangle = glm::translate (glm::vec3(200+trans, 200.0f, 0.0f));
    rotateTriangle =glm::rotate((float)(135*M_PI/180.0f), myRotationAxis);  
    translateTriangle = glm::translate (glm::vec3(-690, 400.0f, 0.0f));
    triangleTransform = translateTriangle*rotateTriangle;
    Matrices.model *= triangleTransform; 
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(decrease);
    /////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateRectangle = glm::translate (glm::vec3(0, -470, 0));
    boundary[0].ycentre = -470;
    glm::mat4 rotateRectangle=glm::rotate((float)(2*rectangle_rotation*M_PI/180.0f),glm::vec3(0,0,1));
    Matrices.model *= translateRectangle;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(boundary[0].rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(0, 480, 0));
    boundary[5].ycentre = 480;
    Matrices.model *= translateRectangle;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(boundary[5].rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    float xp[] = {0, 100, 200, -56, -430, -600, 800,450,-333,699};
    for(int tr=0;tr<10;tr++)
    {
	Matrices.model = glm::mat4(1.0f);
	translateRectangle = glm::translate (glm::vec3(xp[tr], -450, 0));
	Matrices.model *= translateRectangle;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(tree[tr]);
    }
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(shaft.circle);
    for(int i =0;i<lives;i++)
    {
	Matrices.model = glm::mat4(1.0f);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ammo[i].circle);    
    }
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(-900, 0, 0));
    rotateRectangle = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,0,1));
    boundary[6].ycentre = 0;
    boundary[6].xcentre = -900;
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(boundary[6].rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(900, 0, 0));
    rotateRectangle = glm::rotate((float)(90*M_PI/180.0f),glm::vec3(0,0,1));
    boundary[7].ycentre = 0;
    boundary[7].xcentre = 900;
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(boundary[7].rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(-840, -360, 0));
    boundary[8].ycentre = -360;
    boundary[8].xcentre = -840;
    Matrices.model *= (translateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(boundary[8].rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    glUseProgram(textureProgramID);
    Matrices.model = glm::mat4(1.0f);
    rotateRectangle=glm::rotate((float)(rectangle_rotation*M_PI/180.0f),glm::vec3(0,0,1));
    Matrices.model *= (rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
    draw3DTexturedObject(boundary[1].rectangle);	   
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(700, 80, 0));
    boundary[2].xcentre = 700;
    boundary[2].ycentre = 80;
    rotateRectangle=glm::rotate((float)(-2*rectangle_rotation*M_PI/180.0f),glm::vec3(0,0,1));
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
    draw3DTexturedObject(boundary[2].rectangle);	   
   
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(700, -80, 0));
    boundary[3].xcentre = 700;
    boundary[3].ycentre = -80;
    rotateRectangle=glm::rotate((float)(2*rectangle_rotation*M_PI/180.0f),glm::vec3(0,0,1));
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
     glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
    draw3DTexturedObject(boundary[3].rectangle);	   
   
    ////////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateRectangle = glm::translate (glm::vec3(-200, 400+yyd, 0));
    yyd -= yyc;
    boundary[4].xcentre = -200;
    boundary[4].ycentre = 400+yyd;
    if(boundary[4].ycentre==400 || boundary[4].ycentre == -400)
	yyc *= (-1);
    Matrices.model *= (translateRectangle);
    MVP = VP * Matrices.model;
     glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
    draw3DTexturedObject(boundary[4].rectangle);	   
   
    ////////////////////////////////////////////////////////////////////////////////////

    glUseProgram(programID);

    if(pig[7].detectability){
	Matrices.model = glm::mat4(1.0f);
	translateRectangle = glm::translate (glm::vec3(150,0,0));
	pig[7].xcentre = 150;
	Matrices.model *= (translateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(BP[0]);}
    if(pig[8].detectability){
	Matrices.model = glm::mat4(1.0f);
	translateRectangle = glm::translate (glm::vec3(788,0,0));
	pig[8].xcentre = 790;
	Matrices.model *= (translateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(BP[1]);}
    ////////////////////////////////////////////////////////////////////////////////////
    for(int i =10;i<14;i++)
    {
	if(boundary[i].life>0)
	{
	  glUseProgram(textureProgramID);
	    Matrices.model = glm::mat4(1.0f);
	    switch(i){
		case 10:
		    translateRectangle = glm::translate (glm::vec3(0, 200, 0));
		    boundary[i].xcentre = 0;
		    boundary[i].ycentre = 200;
		    break;
		case 11:
		    translateRectangle = glm::translate (glm::vec3(0, -200, 0));
		    boundary[i].xcentre = 0;
		    boundary[i].ycentre = -200;
		    break;
		case 12:
		    translateRectangle = glm::translate (glm::vec3(50, 350, 0));
		    boundary[i].xcentre = 50;
		    boundary[i].ycentre = 350;
		    break;
		case 13:
		    translateRectangle = glm::translate (glm::vec3(50, -350, 0));
		    boundary[i].xcentre = 50;
		    boundary[i].ycentre = -350;
		    break;
	    }
	    Matrices.model *= translateRectangle;
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
	    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
	    draw3DTexturedObject(boundary[i].rectangle);
	    //aw3DObject(boundary[i].rectangle);

	}
    }
    glUseProgram(programID);
    ////////////////////////////////////////////////////////////////////////////////////
    for(int i =14;i<=20;i++)
    {
	if(boundary[i].life>0)
	{ glUseProgram(textureProgramID);
	    Matrices.model = glm::mat4(1.0f);
	    switch(i){
		case 14:
		    translateRectangle = glm::translate (glm::vec3(-150, 100, 0));
		    boundary[i].xcentre = -150;
		    boundary[i].ycentre = 100;
		    break;
		case 15:
		    translateRectangle = glm::translate (glm::vec3(-100, 300, 0));
		    boundary[i].xcentre = -100;
		    boundary[i].ycentre = 300;
		    break;
		case 16:
		    translateRectangle = glm::translate (glm::vec3(-150, -100, 0));
		    boundary[i].xcentre = -150;
		    boundary[i].ycentre = -100;
		    break;
		case 17:
		    translateRectangle = glm::translate (glm::vec3(-100, -300, 0));
		    boundary[i].xcentre = -100;
		    boundary[i].ycentre = -300;
		    break;
		case 18:
		    translateRectangle = glm::translate (glm::vec3(400, 310, 0));
		    boundary[i].xcentre = 400;
		    boundary[i].ycentre = 310;
		    break;
		case 19:
		    translateRectangle = glm::translate (glm::vec3(400, -310, 0));
		    boundary[i].xcentre = 400;
		    boundary[i].ycentre = -310;
		    break;
		case 20:
		    translateRectangle = glm::translate (glm::vec3(520, 0, 0));
		    boundary[i].xcentre = 520;
		    boundary[i].ycentre = 1;
		    break;
	    }
	    Matrices.model *= translateRectangle;
	    MVP = VP * Matrices.model;
	    glUniformMatrix4fv(Matrices.TexMatrixID, 1, GL_FALSE, &MVP[0][0]);
	    glUniform1i(glGetUniformLocation(textureProgramID, "texSampler"), 0);
	    draw3DTexturedObject(boundary[i].rectangle);
        }
    } glUseProgram(programID);
    ////////////////////////////////////////////////////////////////////////////////////
    for(int iter = 1;iter<=POWER;iter++)
    {
	Matrices.model = glm::mat4(1.0f);
	translateRectangle = glm::translate (glm::vec3((-800+(iter)*20), 400, 0));
	Matrices.model *= (translateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(boundary[95+POWER-1].rectangle);
    }
    //
    Matrices.model = glm::mat4(1.0f);
    if(mouse_pressed)
    {
	ELEVATION = cangle;
    }
    else
	ELEVATION += change;
    if(ELEVATION > -10)
	ELEVATION = -10;
    if(ELEVATION < -100)
	ELEVATION = -100;
    translateRectangle = glm::translate (glm::vec3(-840, -230, 0));
    rotateRectangle = glm::rotate((float)(ELEVATION*M_PI/180.0f),glm::vec3(0,0,1));
    Matrices.model *= (translateRectangle*rotateRectangle);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(barrel.rectangle);
    ////////////////////////////////////////////////////////////////////////////////////
    glm::mat4 translateCircle, rotateCircle, circleTransform;
    if(old == 0 && space_pressed == 1)
    {
	num = 0;
    }
    //plot(ball.xcentre,ball.ycentre);

    tempx[num] = ball.xcentre;
    tempy[num] = ball.ycentre;
    if(num<150)
	num++;
    for(int i=0;i<num;i++)
    {
	Matrices.model = glm::mat4(1.0f);
	translateRectangle = glm::translate (glm::vec3(tempx[i],tempy[i],0));
	Matrices.model *= (translateRectangle);
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(points[i].vertex);
    }
    old = space_pressed;
    // Load identity to model matrix
    if(space_pressed){
	if(!initialised)
	    initialise();
	Matrices.model = glm::mat4(1.0f);
	if(allow_acc)
	{
	    ball.v += g;
	}
	allow_acc = 1;
	for(int i=1;i<21;i++)
	{
	    if(i!=5)
	    {
		if(ball.collflag[i] == 1)
		{
		    allow_acc = 0;
		    ball.v *= (-1*0.9);
		    birdy += circle_radius;
		}
		else if(ball.collflag[i] == 2 && i!=8)
		{
		    ball.vx *= (-1*0.58);
		}
		else if(ball.collflag[i] == -1)
		{
		    ball.vx *= (-1*0.58);
		    birdy -= circle_radius;
		}
	    }

	}
	if(ball.ycentre < -260)
	    if(ball.xcentre < -760)
		ball.vx *= (-1*0.8);
	if(ball.collflag[0]){
	    birdy = -447.9+250;
	    ball.v *= (-1*0.6);
	    if(ball.v<2)
		ball.v = 0;
	    if(ball.vx > 0)
	    {
		ball.vx -= (boundary[0].friction/40);
		if(ball.vx<0.08)
		    ball.vx = 0;
	    }
	    else if(ball.vx < 0)
	    {
		ball.vx += (boundary[0].friction/40);
		if(ball.vx> (-0.08))
		    ball.vx = 0;
	    }
	}
	else
	{
	    if(ball.vx>1)
		ball.vx -= 0.04; //air friction
	    else if(ball.vx<-1)
		ball.vx += 0.04;
	}
	birdy = birdy + ball.v;
	birdx += ball.vx;
	if(ball.collflag[5] != 0)
	{
	    birdy -= circle_radius;
	    ball.v *= (-0.8);
	}
	if(ball.collflag[6] != 0)
	    birdx += circle_radius;
	if(ball.collflag[7] != 0)
	    birdx -= circle_radius;
	translateCircle = glm::translate (glm::vec3(birdx-840,birdy-250, 0.0f));
	ball.ycentre = birdy-250;
	ball.xcentre = birdx-840;
	//cout<<ball.xcentre<<", "<<ball.ycentre<<endl;
	rotateCircle = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1));  
	circleTransform = translateCircle;
	Matrices.model *= circleTransform;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(ball.circle);
	if((!ball.v && !ball.vx) || RESET)
	{
	    RESET = false;
	    space_pressed = 0;
	    initialised = 0;
	    createCircle();
	}
    }

    ////////////////////////////////////////////////////////////////////////////////////
    for(int i=0;i<7;i++)
    {
	Matrices.model = glm::mat4(1.0f);
	float v1 = pig[i].vx;
	float v2 = pig[i].v;
	if(!pig[i].detectability)
	    pig[i].v += g;
	t1[i] += pig[i].vx;
	t2[i] += pig[i].v;

	switch(i){
	    case 0:
		translateCircle = glm::translate (glm::vec3(200.0f+t1[i], 200.0f+t2[i], 0.0f));
		pig[i].xcentre = 200+t1[i];
		pig[i].ycentre = 200+t2[i];
		break;
	    case 1:
		translateCircle = glm::translate (glm::vec3(500.0f+t1[i], 300.0f+t2[i], 0.0f));
		pig[i].xcentre = 500+t1[i];
		pig[i].ycentre = 300+t2[i];
		break;
	    case 2:
		translateCircle = glm::translate (glm::vec3(600.0f+t1[i], 400.0f+t2[i], 0.0f));
		pig[i].xcentre = 600+t1[i];
		pig[i].ycentre = 400+t2[i];
		break;
	    case 3:
		translateCircle = glm::translate (glm::vec3(500.0f+t1[i], -300.0f+t2[i], 0.0f));
		pig[i].xcentre = 500+t1[i];
		pig[i].ycentre =-300+t2[i];
		break;
	    case 4:
		translateCircle = glm::translate (glm::vec3(600.0f+t1[i], -400.0f+t2[i], 0.0f));
		pig[i].xcentre = 600+t1[i];
		pig[i].ycentre =-400+t2[i];
		break;
	    case 5:
		translateCircle = glm::translate (glm::vec3(200.0f+t1[i], -200.0f+t2[i], 0.0f));
		pig[i].xcentre =     200+t1[i];
		pig[i].ycentre =     -200+t2[i];
		break;
	    case 6:
		translateCircle = glm::translate (glm::vec3(350.0f+t1[i], 0.0f+t2[i], 0.0f));
		pig[i].xcentre =   350 + t1[i];
		pig[i].ycentre = t2[i];
		break;
	}
	circleTransform = translateCircle;
	Matrices.model *= circleTransform;
	MVP = VP * Matrices.model;
	glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
	draw3DObject(pig[i].circle);
    }
    ////////////////////////////////////////////////////////////////////////////////
    Matrices.model = glm::mat4(1.0f);
    translateCircle = glm::translate (glm::vec3(-840.0f, -260.0f, 0.0f));
    circleTransform = translateCircle;
    Matrices.model *= circleTransform;
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
    draw3DObject(cannon.circle);
    ////////////////////////////////////////////////////////////////////////////////
    /*Matrices.model = glm::mat4(1.0f);
      translateRectangle = glm::translate (glm::vec3(400,-400, 0));   
      boundary[1].ycentre = -400;
      boundary[1].xcentre = 400;
      rotateRectangle=glm::rotate((float)(rectangle_rotation*M_PI/180.0f),glm::vec3(0,0,1));
      Matrices.model = Matrices.model * (translateRectangle *rotateRectangle);
      MVP = VP * Matrices.model;
      glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
      draw3DObject(boundary[1].rectangle);*/
    /////////////////////////////////////////////////////////////////////////////
    for(int i1=0;i1<1;i1++)
	for(int i2=0;i2<21;i2++)
	{
	    if(i2>9 && i2<21 && boundary[i2].life<=0)
	    {
		ball.collflag[i2] = 0;
		continue;
	    }
	    glm::vec2 c1(ball.xcentre,ball.ycentre); //xi + yj
	    glm::vec2 c2(boundary[i2].xcentre,boundary[i2].ycentre);
	    glm::vec2 half(boundary[i2].l/2,boundary[i2].b/2);

	    glm::vec2 diff = c1 - c2;
	    glm::vec2 clamp = glm::clamp(diff,-half,half);

	    glm::vec2 contact = c2 + clamp;
	    diff = c1 - contact;
	    glm::vec2 y(0,1);
	    glm::vec2 xx(1,0);
	    float dota = glm::dot(y,diff);
	    float dota2 = glm::dot(xx,diff);
	    if(glm::length(diff) <= ball.radius)
	    {
		if(i2!=9 && i2!=1 && i2!=2 && i2!=3 && i2!=4)
		    total_score += boundary[i2].score;
		if(dota > 0)
		{
		    ball.collflag[i2] = 1; // invert y from -ve to +ve
		}
		else if(dota < 0)
		{
		    ball.collflag[i2] = -1; // invert y from +ve to -ve
		}
		else
		{
		    ball.collflag[i2] = 2;
		}
		if(i2>9 && i2<21)
		    boundary[i2].life -= 1;
	    }
	    else
		ball.collflag[i2]=0;
	}

    //////////////////////////////////////////////////////////////////////////////
    if(space_pressed)
    {
	for(int i=0;i<9;i++)
	{
	    glm::vec2 c1(ball.xcentre,ball.ycentre);
	    glm::vec2 c2(pig[i].xcentre,pig[i].ycentre);
	    glm::vec2 diff = c1 - c2;
	    if(circle_radius + pig[i].radius >= glm::length(diff) && pig[i].detectability)
	    {
	      total_score += pig[i].score;
	      pn--;
		pig[i].detectability = 0;
		if(i<7){
		    pig[i].vx = ball.vx * 0.5;
		    pig[i].v = ball.v * 0.2;
		}
		if(ball.vx>0)
		    birdx -= 13;
		else if(ball.vx<0)
		    birdx += 13;
		ball.vx = ball.vx * (-0.5);
		if(ball.v>0)
		    birdy -= 13;
		else if(ball.v<0)
		    birdx += 13;
		ball.v = ball.v * (-0.5);
	    }
	}
    }


    glUseProgram(fontProgramID);
    // Increment angles
    float increments = 1;
    int fontScale = 40;
    int fontScaleValue=80;
    glm::vec3 fontColor = getRGBfromHue (1);

    /* Transform the text*/
    Matrices.model = glm::mat4(1.0f);
    glm::mat4 translateText = glm::translate(glm::vec3(-830,200,0));
    glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue,fontScaleValue,fontScaleValue));
    Matrices.model *= (translateText * scaleText) ;
    MVP = Matrices.projection * Matrices.view * Matrices.model;
    /* send font's MVP and font color to fond shaders*/
    glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
    glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);
    //total_score
    if(pn && lives)
      {
    std::ostringstream s;
    s << "Score: " << total_score;
    std::string A(s.str());
    char tab2[1024];
    strncpy(tab2, A.c_str(), sizeof(tab2));
    tab2[sizeof(tab2) - 1] = 0;
    GL3Font.font->Render(tab2);
      }
    else if(pn<1)
      {
	 std::ostringstream s;
    s << "Congrats! Score: " << total_score;
    std::string A(s.str());
    char tab2[1024];
    strncpy(tab2, A.c_str(), sizeof(tab2));
    tab2[sizeof(tab2) - 1] = 0;
    GL3Font.font->Render(tab2);
      }
    else if(lives<1)
      {

	 std::ostringstream s;
    s << "Sorry! Out of ammo, score:" << total_score;
    std::string A(s.str());
    char tab2[1024];
    strncpy(tab2, A.c_str(), sizeof(tab2));
    tab2[sizeof(tab2) - 1] = 0;
    GL3Font.font->Render(tab2);	
      }
    //fontScale = (fontScale + 1) % 360;


    //camera_rotation_angle++; // Simulating camera rotation
    triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
    rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_rot_status;

}
/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
	exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
	glfwTerminate();
	exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
       is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetCursorPosCallback(window, returnMouse);
    glfwSetScrollCallback(window, returnScroll);


    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    glActiveTexture(GL_TEXTURE0);
    GLuint tid1 = createTexture("wood.png");
    GLuint tid2 = createTexture("ice.png");
    GLuint tid3 = createTexture("stone.png");
    
    textureProgramID = LoadShaders( "TextureRender.vert", "TextureRender.frag" );
    Matrices.TexMatrixID = glGetUniformLocation(textureProgramID, "MVP");
    

    /* Objects should be created before any other gl function and shaders */
    // Create the models
    createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
    createPlatform ();
    createRectangle ();
    createTrees();
    createBadaPig();
    createRectangle1 ();
    createCircle ();
    createPig ();
    createAmmo();
    createStone(tid3);
    createWood(tid1);
    createIce(tid2);

    traceroute();

    // Create and compile our GLSL program from the shaders
    programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
    // Get a handle for our "MVP" uniform
    Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

    reshapeWindow (window, width, height);

    // Background color of the scene
    glClearColor (0.2f, 0.2f, 0.27f, 0.0f); // R, G, B, A
    glClearDepth (1.0f);

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LEQUAL);
    //glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    /* Initialise FTGL stuff*/
    const char* fontfile = "arial.ttf";
    GL3Font.font = new FTExtrudeFont(fontfile);

    if(GL3Font.font->Error())
    {
	cout << "Error: Could not load font `" << fontfile << "'" << endl;
	glfwTerminate();
	exit(EXIT_FAILURE);
    }

    /*Create and compile our GLSL program from the font shaders*/
    fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
    GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
    fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
    fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
    fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
    GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
    GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

    GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
    GL3Font.font->FaceSize(1);
    GL3Font.font->Depth(0);
    GL3Font.font->Outset(0, 0);
    GL3Font.font->CharMap(ft_encoding_unicode);

}
int main (int argc, char** argv)
{
    int width = 1800;
    int height = 960;
    GLFWwindow* window = initGLFW(width, height);
    initGL (window, width, height);
    double last_update_time = glfwGetTime(), current_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
	draw();
	reshapeWindow (window, width, height);
	glfwSwapBuffers(window);
	glfwPollEvents();
	current_time = glfwGetTime(); // Time in seconds
	if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
	    // do something every 0.5 seconds ..
	    last_update_time = current_time;
	}
    }
    glfwTerminate();
    exit(EXIT_SUCCESS);
    return 0;
}
