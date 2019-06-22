

// *******************************
// ShaderMgrSDM.cpp - Version 1.0 - March 17, 2018
//
// ShaderMgrSDM.cpp code defines, compiles and manages shaders
//           for the SimpleDrawModern program
//           and the BasicDrawModes program.
//
// Author: Sam Buss
//
// Software accompanying POSSIBLE SECOND EDITION TO the book
// 3D Computer Graphics : A Mathematical Introduction with OpenGL,
// by S.Buss, Cambridge University Press, 2003.
//
// Software is "as-is" and carries no warranty. It may be used without
//  restriction, but if you modify it, please change the filenames to
//  prevent confusion between different versions.
// Bug reports : Sam Buss, sbuss@ucsd.edu.
// Web page : http://math.ucsd.edu/~sbuss/MathCG
// *******************************

// These libraries are needed to link the program.
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")

#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include <stdio.h>

#include "ShaderMgrSDM.h"

/*
 * The shader programs are compiled and linked with the code below. 
 * They are declared as global variables SimpleDrawModern.cpp
 *   and used there in the myRenderScene routine.
 */
extern unsigned int shaderProgram1;

// ***********************************
// The vertex shader and fragment shader allow each
//  vertex to have its own color. The colors are specified
// in the same array that specifies the vertex positions.
// Colors can alternately be specified with glVertexAttrib3f() to be
//    a fixed value, instead of varying per vertex.
// ***********************************

// Sets the position and color of a vertex.
//   This implementations just copies the position with no transformations.
//   It copies the color to "theColor" so that the fragment shader can access it.
const char *vertexShader_PosColorOnly =
"#version 330 core\n"
"layout (location = 0) in vec3 vertPos;	   // Position in attribute location 0\n"
"layout (location = 1) in vec3 vertColor;  // Color in attribute location 1\n"
"out vec3 theColor;					       // Output a color to the fragment shader\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(vertPos.x, vertPos.y, vertPos.z, 1.0);\n"
"   theColor = vertColor;\n"
"}\0";

// Set a general color using a fragment shader. (A "fragment" is a "pixel".)
//    The color value is passed in, obtained from the color(s) on the vertice(s).
//    Color values range from 0.0 to 1.0.
//    First three values are Red/Green/Blue (RGB).
//    Fourth color value (alpha) is 1.0, meaning there is no transparency.
const char *fragmentShader_ColorOnly =
"#version 330 core\n"
"in vec3 theColor;		// Color value came from the vertex shader (smoothed) \n"
"out vec4 FragColor;	// Color that will be used for the fragment\n"
"void main()\n"
"{\n"
"   FragColor = vec4(theColor, 1.0f);   // Add alpha value of 1.0.\n"
"}\n\0";

/*
 * Build and compile our shader programs
 */ 
void setup_shaders() {
	// A very simple shader program: has no transformations and no Phong lighting.
	//      Has a position and color for each vertex. 
	shaderProgram1 = setup_shader_vertfrag(vertexShader_PosColorOnly, fragmentShader_ColorOnly);
}

/*
 * Compile a vertex shader and a fragment shader,
 * and combine them into a shader program.
 * Check for compilation and linkage errors (Highly recommended!)
 * Returns the "shaderProgram"
 */
unsigned int setup_shader_vertfrag( const char* vertexShaderSource, const char* fragmentShaderSource ) {
	// vertex shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	check_compilation_shader(vertexShader);

	// fragment shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	check_compilation_shader(fragmentShader);

	// link shaders
	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	check_link_status(shaderProgram);

	// Deallocate shaders since we do not need to use these for other shader programs.
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;		// Return the compiled shaders as a single shader program.

}


/*
 * Check for compile errors for a shader.
 * Parameters:
 *    - shader. The shader identifier (an unsigned integer)
 * Returns:
 *    shader (the same value as was passed in) if compile succeeded.  Or,
 *    0 if an error occured in compilation or if not a valid shader.
 */
GLuint check_compilation_shader(GLuint shader) {
	if (!glIsShader(shader)) {
		printf("ERROR: Not a shader! Possibly an allocation error.\n");
		return 0;
	}

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (success) {
		return shader;	// Compilation was successful
	}

	// Compilation failed
	int infoLogLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	char* infoLog = new char[infoLogLength];
	glGetShaderInfoLog(shader, 512, NULL, infoLog);
	printf("ERROR::Shader compilation failed!\n%s\n", infoLog);
	delete infoLog;
	return 0;
}

/*
* Check for link errors for a program.
* Parameters:
*    - program. The program identifier (an unsigned integer)
*          A "program" is a combination of one or more shaders.
* Returns:
*    program (the same value as was passed in) if there are no link errors.  Or,
*    0 if there is a link error or if not a valid program identifier.
*/
GLuint check_link_status(GLuint program) {
	if (!glIsProgram(program)) {
		printf("ERROR: Not a shader program! Possibly an allocation error.\n");
		return 0;
	}

	int success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (success) {
		return program;		// Linkage was successful
	}

	int infoLogLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
	char* infoLog = new char[infoLogLength];
	glGetProgramInfoLog(program, 512, NULL, infoLog);
	printf("ERROR::Shader program link failed!\n%s\n", infoLog);
	return 0;
}
