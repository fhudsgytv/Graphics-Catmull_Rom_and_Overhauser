// *******************************
// ShaderMgrSDM.h - Version 1.0 - March 17, 2018
//
// ShaderMgrSDM.cpp code defines, compiles and manages shaders
//           for the SimpleDrawModern.cpp program
//
// Software is "as-is" and carries no warranty. It may be used without
//  restriction, but if you modify it, please change the filenames to
//  prevent confusion between different versions.
// Bug reports: Sam Buss, sbuss@ucsd.edu
// *******************************


#pragma once

void setup_shaders();
unsigned int setup_shader_vertfrag(const char* vertexShaderSource, const char* fragmentShaderSource);

GLuint check_compilation_shader(GLuint shader);
GLuint check_link_status(GLuint program);



