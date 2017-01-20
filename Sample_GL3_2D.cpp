#include <bits/stdc++.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
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
bool triangle_rot_status = true;
bool rectangle_rot_status = true;

const float turretPOSX = -3.75;
float turretPOSY = 0.0;
float turretROT = 0.0;

double last_shot_time = -1;
double new_shot_time = glfwGetTime();
const float BULLET_SPEED = 0.01;
typedef struct bullet {
  bool active; // true => draw; false => reinitialise
  float x, y;
  float rot;
} bullet;
vector<bullet> bullets;

float redBucketPOSX = -1.5;
float grnBucketPOSX = 2.5;
const float bucketPOSY = -3.6;
const float BUCKET_SPEED = 0.1;
const float BUCKET_W = 1, BUCKET_H = 0.6;

const int TOTAL_BRICKS = 20;
float BRICK_SPEED = 0.005;
typedef struct brick {
    int color; // 0 = red, 1 = green, 2 = black
    bool active; // true => draw; false => reinitialise
    float x, y;
} brick;
vector<brick> bricks; //erase(i), push_back()

float mirror1_rot, mirror2_rot, mirror3_rot, mirror4_rot;
const float mirror1X = 0.0, mirror1Y = 0.0, mirror2X = 0.0, mirror2Y = 2.5, mirror3X = 3.0, mirror3Y = -1.5, mirror4X = 3.0, mirror4Y = 1.0;
const float MIRROR_W = 0.7, MIRROR_H = 0.02;

float ZOOM = 1.0;
float PAN = 0.0;

bool turret_hover, turret_drag, redBucket_hover, redBucket_drag, grnBucket_hover, grnBucket_drag, bullet_stream, pan_drag;

void collision_mirror(float xm, float ym, float am, int bullet_ind)
{
    float slope = tan(am*M_PI/180.0f);
    float distance = fabs(slope*bullets[bullet_ind].x + (-1)*bullets[bullet_ind].y + ym-slope*xm)/sqrt(slope*slope + 1);

    if (distance <= 0.05)
    {
        distance = sqrt(pow((xm-bullets[bullet_ind].x), 2) + pow((ym-bullets[bullet_ind].y), 2));
        if (distance < MIRROR_W - 0.2)
            bullets[bullet_ind].rot = 2*am - bullets[bullet_ind].rot;
    }
    // cout << distance << endl;
}

bool collision(float x1, float y1, float h1, float w1, float x2, float y2, float h2, float w2)
{
    return (fabs(x1 - x2) < (w1 + w2)/2.0) && (fabs(y1 - y2) < (h1 + h2)/2.0);
}

void init_bricks()
{
    for (int i = 0; i < TOTAL_BRICKS; i++)
    {
        brick temp;
        temp.color = i % 3;
        temp.active = true;

        if (i % 2 == 0)
            temp.x = (( rand() % 151 ) + 100 ) / 100.0; // 1.00 to 2.50

        else
            temp.x = (( rand() % 251 ) * -1) / 100.0; // 0 to -2.50

        // cerr << temp.color << " " << temp.x << " " << temp.y << endl;

        temp.y = (( rand() % 1000 ) + 400) / 100.0; // 4 to 8

        bricks.push_back(temp);
    }
}

void init_bullet()
{
    new_shot_time = glfwGetTime();
    if(new_shot_time - last_shot_time >= 1)
    {
        last_shot_time = new_shot_time; // Comment this line out to get a continuous stream of bullets    
        bullet temp;
        temp.active = true;
        temp.x = turretPOSX;
        temp.y = turretPOSY;
        temp.rot = turretROT;
        bullets.push_back(temp);
    }
}

void init_mirrors() // sets an angle at random from 45 to 135 deg on the x-axis
{
    mirror1_rot = ( rand() % 90 ) + 45;
    mirror2_rot = ( rand() % 90 ) + 45;
    mirror3_rot = ( rand() % 90 ) + 45;
    mirror4_rot = ( rand() % 90 ) + 45;

    // cout << mirror1_rot << " " << mirror2_rot << " " << mirror3_rot << " " << mirror4_rot << endl;
}

bool pan(int direction) // -1: left, 1: right
{
    if (ZOOM >= 1)
    {
        if (4 < fabs(direction*4*ZOOM - PAN))
        {
            PAN += direction*0.1;
            return true;
        }
    }
    return false;
}

void zoom(int direction) // -1: out, 1: in
{
    // cout << PAN << " " << ZOOM <<endl;
    if ((ZOOM + direction*0.1) >= 0.9)
        ZOOM += direction*0.1;
    if (fabs(ZOOM - 1) < 0.1) // to deal with floating point error
    {
        ZOOM = 1;
        PAN = 0;
    }
    if (direction < 0 && ZOOM >= 0.9)
    {
        float prev_PAN = PAN;
        if (PAN > 0)
            while(!pan(1))
            {
                pan(-1);
                if(prev_PAN == PAN)
                    break;
                else
                    prev_PAN = PAN;
            }
        else if(PAN < 0)
            while(!pan(-1))
            {
                pan(1);
                if(prev_PAN == PAN)
                    break;
                else
                    prev_PAN = PAN;
            }
    }
}

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }

    else if (action == GLFW_REPEAT || action == GLFW_PRESS) {
        switch(key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            // Bullet SHOOT
            case GLFW_KEY_SPACE:
                init_bullet();
                break;
            // Turret UP/DOWN
            case GLFW_KEY_W:
                turretPOSY += 0.1;
                break;
            case GLFW_KEY_S:
                turretPOSY -= 0.1;
                break;
            // Turret ROTATION
            case GLFW_KEY_A:
                turretROT += 3;
                break;
            case GLFW_KEY_D:
                turretROT -= 3;
                break;
            // Bucket Controls
            case GLFW_KEY_LEFT:
                if (mods == GLFW_MOD_CONTROL)
                    redBucketPOSX -= BUCKET_SPEED;
                else if (mods == GLFW_MOD_ALT)
                    grnBucketPOSX -= BUCKET_SPEED;
                else // PAN CONTROL
                    pan(-1);
                break;
            case GLFW_KEY_RIGHT:
                if (mods == GLFW_MOD_CONTROL)
                    redBucketPOSX += BUCKET_SPEED;
                else if (mods == GLFW_MOD_ALT)
                    grnBucketPOSX += BUCKET_SPEED;
                else // PAN CONTROL
                    pan(1);
                break;
            // BRICK SPEED
            case GLFW_KEY_N:
                BRICK_SPEED += 0.001;
                break;
            case GLFW_KEY_M:
                BRICK_SPEED -= 0.001;
                break;
            // ZOOM CONTROL
            case GLFW_KEY_UP:
                zoom(1);
                // ZOOM += 0.1;
                break;
            case GLFW_KEY_DOWN:
                zoom(-1);
                // if (ZOOM > 1)
                //     ZOOM -= 0.1;
                // else
                //     PAN = 0;
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

double mouseX, mouseY;
double mousePanX;
bool mouseIn = false;
/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            if (action == GLFW_RELEASE)
            {
                triangle_rot_dir *= -1;
                if (turret_drag)
                    turret_drag = false;
                if (redBucket_drag)
                    redBucket_drag = false;
                if (grnBucket_drag)
                    grnBucket_drag = false;
                if (bullet_stream)
                    bullet_stream = false;
            }
            if (action == GLFW_PRESS)
            {
                if (turret_hover)
                    turret_drag = true;
                if (redBucket_hover)
                {
                    if (!collision(mouseX, mouseY, 0, 0, grnBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W))
                      redBucket_drag = true;
                }
                if (grnBucket_hover)
                    grnBucket_drag = true;
                if (!turret_drag && !redBucket_drag && !grnBucket_drag)
                    bullet_stream = true;
            }
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_RELEASE) {
                rectangle_rot_dir *= -1;
                if (pan_drag)
                pan_drag = false;
            }
            if (action == GLFW_PRESS)
            {
                pan_drag = true;
                mousePanX = mouseX;
            }
            break;
        default:
            break;
    }
}

void mousePos (GLFWwindow* window, double x, double y)
{
    x = (x - 350) * 4 / 350.0;
    x = (x + PAN)/ZOOM;

    y = (y - 350) * -4 / 350.0;
    y = (y + PAN)/ZOOM;

    mouseX = x;
    mouseY = y;
    
    // cout << x << " " << y << endl;
}

void enterCallback(GLFWwindow* window, int entered)
{
    mouseIn = entered;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom(yoffset);
}

/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *rectangle;
VAO *turret, *hlTurret, *bullet_vao;
VAO *redBucket, *grnBucket, *hlBucket;
VAO *redBrick, *grnBrick, *blkBrick;
VAO *mirror;

// Creates the triangle object used in this sample code
void createTriangle ()
{
  /* ONLY vertices between the bounds specified in glm::ortho will be visible on screen */

  /* Define vertex array as used in glBegin (GL_TRIANGLES) */
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

// Creates the rectangle object used in this sample code
void createRectangle ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
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

// Creates the turret object
const float TURRET_W = 0.4, TURRET_H = 0.4;
void createTurret ()
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.2,-0.2,0, // vertex 1
    0.2,-0.2,0, // vertex 2
    0.2, 0.2,0, // vertex 3

    0.2, 0.2,0, // vertex 3
    -0.2, 0.2,0, // vertex 4
    -0.2,-0.2,0,  // vertex 1

    0.2, -0.05, 0,
    0.4, -0.05, 0,
    0.4, 0.05, 0,

    0.4, 0.05, 0,
    0.2, 0.05, 0,
    0.2, -0.05, 0,
  };

  static const GLfloat color_buffer_data [] = {
    1, 1, 1, // color 1
    1, 1, 1, // color 2
    1, 1, 1, // color 3

    1, 1, 1, // color 3
    1, 1, 1, // color 4
    1, 1, 1, // color 1

    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0  // color 1
  };

  static const GLfloat color_buffer_data_hl [] = { // highlight
    0,0,0, // color 1
    0,0,0, // color 2
    0,0,0, // color 3

    0,0,0, // color 3
    0,0,0, // color 4
    0,0,0, // color 1

    1, 1, 1, // color 1
    1, 1, 1, // color 2
    1, 1, 1, // color 3

    1, 1, 1, // color 3
    1, 1, 1, // color 4
    1, 1, 1 // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  turret = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data, GL_FILL);
  hlTurret = create3DObject(GL_TRIANGLES, 12, vertex_buffer_data, color_buffer_data_hl, GL_LINE);
}

// Creates buckets
void createBuckets () // W: 1 | H: 0.6
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.5, -0.3, 0, // vertex 1
    0.5, -0.3, 0, // vertex 2
    0.5, 0.3, 0, // vertex 3

    0.5, 0.3, 0, // vertex 3
    -0.5, 0.3, 0, // vertex 4
    -0.5, -0.3, 0 // vertex 1
  };

  static const GLfloat color_buffer_data_red [] = {
    1, 0, 0, // color 1
    1, 0, 0, // color 2
    1, 0, 0, // color 3

    1, 0, 0, // color 3
    1, 0, 0, // color 4
    1, 0, 0, // color 1
  };

  static const GLfloat color_buffer_data_grn [] = {
    0, 1, 0, // color 1
    0, 1, 0, // color 2
    0, 1, 0, // color 3

    0, 1, 0, // color 3
    0, 1, 0, // color 4
    0, 1, 0, // color 1
  };

  static const GLfloat color_buffer_data_hl [] = { // highlight
    1, 1, 1, // color 1
    1, 1, 1, // color 2
    1, 1, 1, // color 3

    1, 1, 1, // color 3
    1, 1, 1, // color 4
    1, 1, 1 // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  redBucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_red, GL_FILL);
  grnBucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_grn, GL_FILL);
  hlBucket = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_hl, GL_LINE);
}

// Creates bricks
const float BRICK_W = 0.2, BRICK_H = 0.3;
void createBricks () // W: 0.2 | H: 0.3
{
  // GL3 accepts only Triangles. Quads are not supported
  static const GLfloat vertex_buffer_data [] = {
    -0.1,-0.15,0, // vertex 1
    0.1,-0.15,0, // vertex 2
    0.1, 0.15,0, // vertex 3

    0.1, 0.15,0, // vertex 3
    -0.1, 0.15,0, // vertex 4
    -0.1,-0.15,0  // vertex 1
  };

  static const GLfloat color_buffer_data_red [] = {
    1, 0, 0, // color 1
    1, 0, 0, // color 2
    1, 0, 0, // color 3

    1, 0, 0, // color 3
    1, 0, 0, // color 4
    1, 0, 0, // color 1
  };

  static const GLfloat color_buffer_data_grn [] = {
    0, 1, 0, // color 1
    0, 1, 0, // color 2
    0, 1, 0, // color 3

    0, 1, 0, // color 3
    0, 1, 0, // color 4
    0, 1, 0, // color 1
  };

  static const GLfloat color_buffer_data_blk [] = {
    0, 0, 0, // color 1
    0, 0, 0, // color 2
    0, 0, 0, // color 3

    0, 0, 0, // color 3
    0, 0, 0, // color 4
    0, 0, 0, // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  redBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_red, GL_FILL);
  grnBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_grn, GL_FILL);
  blkBrick = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data_blk, GL_FILL);   
}

// Creates Bullet
const float BULLET_W = 0.1, BULLET_H = 0.1;
void createBullet() // W: 0.1 | H: 0.1
{
    // GL3 accepts only Triangles. Quads are not supported
    static const GLfloat vertex_buffer_data [] = {
      -0.05,-0.05,0, // vertex 1
      0.05,-0.05,0, // vertex 2
      0.05, 0.05,0, // vertex 3

      0.05, 0.05,0, // vertex 3
      -0.05, 0.05,0, // vertex 4
      -0.05,-0.05,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
      1, 0, 1, // color 1
      1, 1, 0, // color 2
      0, 1, 1, // color 3

      1, 1, 0, // color 3
      0, 1, 1, // color 4
      1, 0, 1, // color 1
  };

  // create3DObject creates and returns a handle to a VAO that can be used later
  bullet_vao = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);    
}

// Creates Mirror
void createMirror() // W: 0.02 | H: 0.7
{
    static const GLfloat vertex_buffer_data [] = {
      -0.35,-0.01,0, // vertex 1
      0.35,-0.01,0, // vertex 2
      0.35, 0.01,0, // vertex 3

      0.35, 0.01,0, // vertex 3
      -0.35, 0.01,0, // vertex 4
      -0.35,-0.01,0  // vertex 1
    };

    static const GLfloat color_buffer_data [] = {
      0, 0, 1, // color 1
      0, 0, 1, // color 2
      0, 0, 1, // color 3

      0, 0, 1, // color 3
      0, 0, 1, // color 4
      0, 0, 1, // color 1
    };

    mirror = create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);    
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
void draw ()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
  Matrices.projection = glm::ortho((-4.0f + PAN)/ZOOM, (4.0f + PAN)/ZOOM, -4.0f/ZOOM, 4.0f/ZOOM, 0.1f, 500.0f);
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  /* Render your scene */

  // PANNING
  if (pan_drag)
  {
      if(mouseX > mousePanX)
          pan(1);
      else if (mouseX < mousePanX)
          pan(-1);
      mousePanX = mouseX;
  }

  // MIRROR
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateMirror = glm::translate (glm::vec3(mirror1X, mirror1Y, 0.0f));
  glm::mat4 rotateMirror = glm::rotate((float)(mirror1_rot*M_PI/180.0f), glm::vec3(0,0,1));
  glm::mat4 mirrorTransform = translateMirror * rotateMirror;
  Matrices.model *= mirrorTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);

  Matrices.model = glm::mat4(1.0f);
  translateMirror = glm::translate (glm::vec3(mirror2X, mirror2Y, 0.0f));
  rotateMirror = glm::rotate((float)(mirror2_rot*M_PI/180.0f), glm::vec3(0,0,1));
  mirrorTransform = translateMirror * rotateMirror;
  Matrices.model *= mirrorTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);

  Matrices.model = glm::mat4(1.0f);
  translateMirror = glm::translate (glm::vec3(mirror3X, mirror3Y, 0.0f));
  rotateMirror = glm::rotate((float)(mirror3_rot*M_PI/180.0f), glm::vec3(0,0,1));
  mirrorTransform = translateMirror * rotateMirror;
  Matrices.model *= mirrorTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);

  Matrices.model = glm::mat4(1.0f);
  translateMirror = glm::translate (glm::vec3(mirror4X, mirror4Y, 0.0f));
  rotateMirror = glm::rotate((float)(mirror4_rot*M_PI/180.0f), glm::vec3(0,0,1));
  mirrorTransform = translateMirror * rotateMirror;
  Matrices.model *= mirrorTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(mirror);
  
  // TURRET
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateTurret = glm::translate (glm::vec3(turretPOSX, turretPOSY, 0.0f)); // Translates to side of screen and up/down
  glm::mat4 rotateTurret = glm::rotate((float)(turretROT*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 turretTransform = translateTurret * rotateTurret;
  Matrices.model *= turretTransform;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(turret);
  if (collision(mouseX, mouseY, 0, 0, turretPOSX, turretPOSY, TURRET_H, TURRET_W))
  {
    // cout << mouseX << " " << mouseY << endl;
    turret_hover = true;
    draw3DObject(hlTurret);
  }
  else
    turret_hover = false;
  if (turret_drag)
    turretPOSY = mouseY;
  if (mouseIn)
  {
    float slope = atan((turretPOSY - mouseY) / (turretPOSX - mouseX));
    turretROT = slope * 180.0f / M_PI;
  }
  
  // BULLET
  if (bullet_stream)
      init_bullet();
  for (int i = 0; i < bullets.size(); i++)
  {
      if ((bullets[i]).active)
      {
          Matrices.model = glm::mat4(1.0f);
          glm::mat4 translateBullet = glm::translate (glm::vec3(bullets[i].x, bullets[i].y, 0.0f)); // glTranslatef
          glm::mat4 rotateBullet = glm::rotate((float)(bullets[i].rot*M_PI/180.0f), glm::vec3(0,0,1));
          glm::mat4 bulletTransform = translateBullet * rotateBullet;
          Matrices.model *= bulletTransform;
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
          draw3DObject(bullet_vao);

          (bullets[i]).x += BULLET_SPEED * cos(bullets[i].rot*M_PI/180.0f);
          (bullets[i]).y += BULLET_SPEED * sin(bullets[i].rot*M_PI/180.0f);

          if(bullets[i].x >= 4 || bullets[i].x <= -4 || bullets[i].y >= 4 || bullets[i].y <= -4)
            bullets[i].active = false;
          else
          {
              for (int j = 0; j < bricks.size(); j++)
              {
                  // BULLET-BRICK COLLISION
                  if (collision(bullets[i].x, bullets[i].y, BULLET_H, BULLET_W, bricks[j].x, bricks[j].y, BRICK_H, BRICK_W))
                  {
                    bullets[i].active = bricks[j].active = false;
                  }
              }

              // BULLET-MIRROR COLLISION
              collision_mirror(mirror1X, mirror1Y, mirror1_rot, i);
              collision_mirror(mirror2X, mirror2Y, mirror2_rot, i);
              collision_mirror(mirror3X, mirror3Y, mirror3_rot, i);
              collision_mirror(mirror4X, mirror4Y, mirror4_rot, i);
          }
      }
      else
      {
          bullets.erase(bullets.begin() + i);
      }
  }

  // BUCKETS
  glm::mat4 translateBucketRed = glm::translate (glm::vec3(redBucketPOSX, bucketPOSY, 0.0f)); // Translates to bottom of screen and left/right
  glm::mat4 translateBucketGrn = glm::translate (glm::vec3(grnBucketPOSX, bucketPOSY, 0.0f)); // Translates to bottom of screen and left/right
  
  Matrices.model = glm::mat4(1.0f);
  Matrices.model *= translateBucketRed;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(redBucket);
  if (collision(mouseX, mouseY, 0, 0, redBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W) && (redBucket_drag || !collision(mouseX, mouseY, 0, 0, grnBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W)))
  {
    // cout << mouseX << " " << mouseY << endl;
    redBucket_hover = true;
    draw3DObject(hlBucket);
  }
  else
    redBucket_hover = false;
  if (redBucket_drag)
    redBucketPOSX = mouseX;

  Matrices.model = glm::mat4(1.0f);
  Matrices.model *= translateBucketGrn;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  draw3DObject(grnBucket);
  if (collision(mouseX, mouseY, 0, 0, grnBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W) && !redBucket_drag)
  {
    // cout << mouseX << " " << mouseY << endl;
    grnBucket_hover = true;
    draw3DObject(hlBucket);
  }
  else
    grnBucket_hover = false;
  if (grnBucket_drag)
    grnBucketPOSX = mouseX;

  // BRICK
  for (int i = 0; i < bricks.size(); i++)
  {
      if ((bricks[i]).active)
      {
          Matrices.model = glm::mat4(1.0f);

          glm::mat4 translateBrick = glm::translate (glm::vec3( (bricks[i]).x, (bricks[i]).y, 0.0f)); // Translates to side of screen and up/down
          Matrices.model *= translateBrick;
          MVP = VP * Matrices.model;
          glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

          switch ((bricks[i]).color) {
              case 0:
                  draw3DObject(redBrick);
                  break;
              case 1:
                  draw3DObject(grnBrick);
                  break;
              case 2:
                  draw3DObject(blkBrick);
                  break;
              default:
                  break;
          }

          (bricks[i]).y -= BRICK_SPEED;
          if ((bricks[i]).y <= -4.5) // Brick escapes lower boundary
              (bricks[i]).active = false;
          else  if (bricks[i].color == 0 && collision(redBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W, bricks[i].x, bricks[i].y, BRICK_H, BRICK_W)) // red brick collides with red bucket
                (bricks[i]).active = false;
          else if (bricks[i].color == 1 && collision(grnBucketPOSX, bucketPOSY, BUCKET_H, BUCKET_W, bricks[i].x, bricks[i].y, BRICK_H, BRICK_W)) // green brick collides with green bucket
                (bricks[i]).active = false;
      }
      else
      {
          (bricks[i]).color = i % 3;
          (bricks[i]).active = true;

          if (i % 2 == 0)
              (bricks[i]).x = (( rand() % 151 ) + 100) / 100.0; // 100 to 250
          else
              (bricks[i]).x = (( rand() % 251 ) * -1) / 100.0; // 0 to -250

          (bricks[i]).y = (( rand() % 1000 ) + 400) / 100.0; // 4 to 8
      }
  }

  // TRIANGLE
  // Load identity to model matrix
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateTriangle = glm::translate (glm::vec3(-2.0f, 0.0f, 0.0f)); // glTranslatef
  glm::mat4 rotateTriangle = glm::rotate((float)(triangle_rotation*M_PI/180.0f), glm::vec3(0,0,1));  // rotate about vector (1,0,0)
  glm::mat4 triangleTransform = translateTriangle * rotateTriangle;
  Matrices.model *= triangleTransform; 
  MVP = VP * Matrices.model; // MVP = p * V * M

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  // draw3DObject(triangle);

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3(2, 0, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translateRectangle * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  // draw3DObject(rectangle);

  // Increment angles
  float increments = 1;

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
        // exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 8);

    window = glfwCreateWindow(width, height, "Sample OpenGL 3.3 Application", NULL, NULL);

    if (!window) {
        glfwTerminate();
        // exit(EXIT_FAILURE);
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
    glfwSetCursorPosCallback(window, mousePos);
    glfwSetCursorEnterCallback(window, enterCallback);
    glfwSetScrollCallback(window, scrollCallback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
	createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
	createRectangle ();
    createTurret ();
    createBuckets();
    createBricks();
    createBullet();
    createMirror();
	
	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");

	
	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (0.3f, 0.3f, 0.3f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 700;
	int height = 700;

    GLFWwindow* window = initGLFW(width, height);

	initGL (window, width, height);

    double last_update_time = glfwGetTime(), current_time;

    srand(time(NULL));
    init_bricks(); // Initialises vector with the bricks
    init_mirrors(); // Inititalises mirrors at random angles

    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {

        // OpenGL Draw commands
        draw();

        // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

        // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
        current_time = glfwGetTime(); // Time in seconds
        if ((current_time - last_update_time) >= 0.5) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
        }
    }

    glfwTerminate();
    // exit(EXIT_SUCCESS);
}
