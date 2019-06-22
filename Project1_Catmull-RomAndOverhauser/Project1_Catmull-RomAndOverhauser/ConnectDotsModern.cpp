/*
 * ConnectDotsModern.cpp - Version 1.0, March 28, 2019.
 *
 * Example program illustrating a simple use
 * of Modern OpenGL to take mouse clicks and
 * connects points with straight lines.
 *
 * Author: Sam Buss
 *
 * Software accompanying POSSIBLE SECOND EDITION TO the book
 *		3D Computer Graphics: A Mathematical Introduction with OpenGL,
 *		by S. Buss, Cambridge University Press, 2003.
 *
 * Software is "as-is" and carries no warranty.  It may be used without
 *   restriction, but if you modify it, please change the filenames to
 *   prevent confusion between different versions.
 * Bug reports: Sam Buss, sbuss@ucsd.edu.
 * Web page: http://math.ucsd.edu/~sbuss/MathCG2
 */

// Use space to toggle what image is shown.
// Use Escape or 'X' or 'x' to exit.

// These libraries are needed to link the program (Visual Studio specific)
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"glew32s.lib")
#pragma comment(lib,"glew32.lib")

// Use the static library (so glew32.dll is not needed):
#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h>

#include "ShaderMgrSDM.h"
bool check_for_opengl_errors();     // Function prototype (should really go in a header file)

// Enable standard input and output via printf(), etc.
// Put this include *after* the includes for glew and GLFW!
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include "LinearR2.h"

#define MeshRes 20 // number of the points on each Bezier curve
#define numOfArrays 4 // number of VBO vetertexes need to be generated 
// ********************
// Animation controls and state infornation
// ********************

 

constexpr int MaxNumDots = 100;     // Maximum allowed number of dots
int NumDots = 0;                    // Currenmbt number of points
float dotArray[MaxNumDots][2];


int windowWidth, windowHeight;

int selectedVert = -1;          // Either currently selected vertex or -1 (none selected)

float controlPoints[3 * (MaxNumDots - 1) + 1][2];

float pointsOnCurve [MeshRes * (MaxNumDots - 1) + 1][2]; // in this arrray, it doesn't count the point of the p0 to pn

int countControlPoins = 0; // countControlPoins counts the number of elements in the controlPoints array
int countPointsOnCurve = 0; // countPointsOnCurve counts the number of elements in the pointsOnCurve array

float initialVelocity_X = 0.0;
float finalVelocity_X = -0.0;

float initialVelocity_Y = 0.0;
float finalVelocity_Y = -0.0;

float alpha = 0.5; // the constant of linear interpolatation

//int countRecursion = 0; 

int mode; // mode1 - Catmull_Rom; mode2 -chord-length; mode3 - centripetal 

int showingControlPoints = 0; 

// ************************
// General data helping with setting up VAO (Vertex Array Objects)
//    and Vertex Buffer Objects.
// ***********************

//unsigned int myVBO;  // Vertex Buffer Object - holds an array of data
//unsigned int myVAO;  // Vertex Array Object - holds info about an array of vertex data;

unsigned int myVBO[numOfArrays];  // a Vertex Buffer Object holds an array of data
unsigned int myVAO[numOfArrays];  // a Vertex Array Object - holds info about an array of vertex data;

// We create one shader program: it consists of a vertex shader and a fragment shader
unsigned int shaderProgram1;
const unsigned int vertPos_loc = 0;   // Corresponds to "location = 0" in the verter shader definition
const unsigned int vertColor_loc = 1; // Corresponds to "location = 1" in the verter shader definition



// function declaration 
//void rerangePointsOnCurveArray_increasing(int startingIndex, int endingIndex);
void renderCurve();
void RemoveFirstPoint();
void renderControlPoints();
void  myRenderScene();

// *************************
// mySetupGeometries defines the scene data, especially vertex  positions and colors.
//    - It also loads all the data into the VAO's (Vertex Array Objects) and
//      into the VBO's (Vertex Buffer Objects).
// This routine is only called once to initialize the data.
// *************************
void mySetupGeometries() {

	// In this simple example, we do not use the Projection or
	//   ModelView matrices. Hence, all x, y, z positions
	//   should be in the range [-1,1].

    // Allocate Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs).
    glGenVertexArrays(numOfArrays, &myVAO[0]);
    glGenBuffers(numOfArrays, &myVBO[0]);

    // Bind (and initialize) the Vertex Array Object and Vertex Buffer Object
    // The glBufferData command allocates space in the VBO for the vertex data, but does
    //    not any data into the VBO.  For this, the third parameter is the null pointer, (void*)0.
    // The VBO will hold only the vertex positions.  The color will be a generic attribute.
    glBindVertexArray(myVAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[0]);
    glBufferData(GL_ARRAY_BUFFER, MaxNumDots*2*sizeof(float), (void*)0, GL_STATIC_DRAW);
    glVertexAttribPointer(vertPos_loc, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);	
    glEnableVertexAttribArray(vertPos_loc);									                

	glBindBuffer(GL_ARRAY_BUFFER, 0);  
	glBindVertexArray(0);


	// Bind VAO, VBO for array controlPoints
	glBindVertexArray(myVAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), (void*)0, GL_STATIC_DRAW);
	glVertexAttribPointer(vertPos_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(vertPos_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);   
	glBindVertexArray(0);


	// Bind VAO, VBO for array pointsOnCurve
	glBindVertexArray(myVAO[2]);
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pointsOnCurve), (void*)0, GL_STATIC_DRAW);
	glVertexAttribPointer(vertPos_loc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); 
	glEnableVertexAttribArray(vertPos_loc);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

    check_for_opengl_errors();  
}


void calculateControlPoints_CatMull_Rom() {
	float x1, x2, x3, y1, y2, y3;
	float x1_p = 0, y1_p = 0, x2_m = 0, y2_m = 0;
	float velocityAtPoint1_X, velocityAtPoint1_Y, 
		velocityAtPoint2_X, velocityAtPoint2_Y;

	for (int i = 0; i <= NumDots - 2; i++) {
		
		// (x1, y1), (x2, y2) are the starting and ending control points for one piece of Bezier curve 
		x1 = dotArray[i][0];
		y1 = dotArray[i][1];

		x2 = dotArray[i + 1][0];
		y2 = dotArray[i + 1][1];

		x3 = dotArray[i + 2][0];
		y3 = dotArray[i + 2][1];

		// drawing the first Bezier curve
		if (i == 0) {

			// calculate the velocity
			velocityAtPoint1_X = initialVelocity_X;
			velocityAtPoint1_Y = initialVelocity_Y;
			velocityAtPoint2_X = (x3 - x1) / 2;
			velocityAtPoint2_Y = (y3 - y1) / 2;
		}
		else if (i == NumDots - 2) {
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];

			// calculate the velocity 
			velocityAtPoint1_X = (x2 - x0) / 2;
			velocityAtPoint1_Y = (y2 - y0) / 2;
			velocityAtPoint2_X = finalVelocity_X;
			velocityAtPoint2_Y = finalVelocity_Y;
		}
		else {
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];
			// calculate the velocity 
			velocityAtPoint1_X = (x2 - x0) / 2;
			velocityAtPoint1_Y = (y2 - y0) / 2;
			velocityAtPoint2_X = (x3 - x1) / 2;
			velocityAtPoint2_Y = (y3 - y1) / 2;
		}
		
		// calculating the x, y coordinates for the pi, pi+, p(i+1), (i+1)- 
		// pi is x1

		// pi+
		x1_p = x1 + 1 / 3.0 * velocityAtPoint1_X;
		y1_p = y1 + 1 / 3.0 * velocityAtPoint1_Y;

		//p(i+1) is x2

		//p(i+1)-
		x2_m = x2 - 1 / 3.0 * velocityAtPoint2_X;
		y2_m = y2 - 1 / 3.0 * velocityAtPoint2_Y;

		
		// ewven though it iterates through all of the points 
		// but it should not all of the points in each iteration 
		if (i == 0) {

			// add vectors to array
			controlPoints[countControlPoins][0] = x1;
			controlPoints[countControlPoins++][1] = y1;

			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}
		else {
			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}

		if (countControlPoins > (MaxNumDots - 1) * 3 + 1) {
			countControlPoins = (MaxNumDots - 1) * 3 + 1;
		}
	}
	
}


void calculateControlPoints_Chord() {
	float x1, x2, x3, y1, y2, y3;
	float x1_p = 0, y1_p = 0, x2_m = 0, y2_m = 0;

	float velocity1_m_half_X, velocity1_m_half_Y,
		velocity1_p_half_X, velocity1_p_half_Y;

	float velocity2_m_half_X, velocity2_m_half_Y,
		velocity2_p_half_X, velocity2_p_half_Y;

	float velocityAtPoint1_X, velocityAtPoint1_Y,
		velocityAtPoint2_X, velocityAtPoint2_Y;

	float timeInterval2_m = 1, timeInterval2_p = 1, timeInterval2,
		timeInterval1_m = 1, timeInterval1_p = 1, timeInterval1; // the time interval value

	for (int i = 0; i <= NumDots - 2; i++) {

		// (x1, y1), (x2, y2) are the starting and ending control points for one piece of Bezier curve 
		x1 = dotArray[i][0];
		y1 = dotArray[i][1];

		x2 = dotArray[i + 1][0];
		y2 = dotArray[i + 1][1];

		x3 = dotArray[i + 2][0];
		y3 = dotArray[i + 2][1];

		// those are the time interval value regrading p2(x2, y2) as the pi,
		//p1(x1, x1) as p(i-1), and p3(x3, y3) as p(i+1)
		timeInterval2_m = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
		timeInterval2_p = sqrt((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3));
		timeInterval2 = sqrt((x3 - x1) * (x3 - x1) + (y3 - y1) * (y3 - y1));

		// drawing the first Bezier curve
		if (i == 0) {

			// calculate the velocity for the starting point
			velocityAtPoint1_X = initialVelocity_X;
			velocityAtPoint1_Y = initialVelocity_Y;

			// following steps are the process of calculating the velocity at point (x2, y2)
			// calculate velocity2_m_half
			velocity2_m_half_X = (x2 - x1) / timeInterval2_m;
			velocity2_m_half_Y = (y2 - y1) / timeInterval2_m;

			// calculate velocity2_p_half
			velocity2_p_half_X = (x3 - x2) / timeInterval2_p;
			velocity2_p_half_Y = (y3 - y2) / timeInterval2_p;

			// calculate the real velocity at point (x2, y2)
			velocityAtPoint2_X = ( (timeInterval2_m * velocity2_p_half_X) + (timeInterval2_p * velocity2_m_half_X) ) / timeInterval2;
			velocityAtPoint2_Y = ( (timeInterval2_m * velocity2_p_half_Y) + (timeInterval2_p * velocity2_m_half_Y) ) / timeInterval2;

		}
		else if (i == NumDots - 2) {
			// introducing p0 for calculating the velocity at P1(x1, y1)
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];

			// those are the time interval value regrading p1(x1, y1) as the pi,
			//p0(x0, x0) as p(i-1), and p2(x2, y2) as p(i+1)
			timeInterval1_m = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
			timeInterval1_p = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			//timeInterval1 = sqrt((x2 - x0) * (x2 - x0) + (y2 - y0) * (y2 - y0));   wrong!!!!!!
			timeInterval1 = timeInterval1_m + timeInterval1_p;

			// calculate the velocityAtP2  
			velocityAtPoint2_X = finalVelocity_X;
			velocityAtPoint2_Y = finalVelocity_Y;

			// following steps are the process of calculating the velocity at point (x1, y1)
			// calculate velocity1_m_half
			velocity1_m_half_X = (x1 - x0) / timeInterval2_m;
			velocity1_m_half_Y = (y1 - y0) / timeInterval2_m;

			// calculate velocity1_p_half
			velocity1_p_half_X = (x2 - x1) / timeInterval2_p;
			velocity1_p_half_Y = (y2 - y1) / timeInterval2_p;

			// calculate the real velocity at point (x1, y1)
			velocityAtPoint1_X = ((timeInterval1_m * velocity1_p_half_X) + (timeInterval1_p * velocity1_m_half_X)) / timeInterval1;
			velocityAtPoint1_Y = ((timeInterval1_m * velocity1_p_half_Y) + (timeInterval1_p * velocity1_m_half_Y)) / timeInterval1;

		}
		else {
			// introducing p0 for calculating the velocity at P1(x1, y1)
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];

			// those are the time interval value regrading p1(x1, y1) as the pi,
			//p0(x0, x0) as p(i-1), and p2(x2, y2) as p(i+1)
			timeInterval1_m = sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0));
			timeInterval1_p = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			//timeInterval1 = sqrt((x2 - x0) * (x2 - x0) + (y2 - y0) * (y2 - y0));  worng!!!!!!!!
			timeInterval1 = timeInterval1_m + timeInterval1_p;

			// following steps are the process of calculating the velocity at point (x1, y1)
			// calculate velocity1_m_half
			velocity1_m_half_X = (x1 - x0) / timeInterval2_m;
			velocity1_m_half_Y = (y1 - y0) / timeInterval2_m;

			// calculate velocity1_p_half
			velocity1_p_half_X = (x2 - x1) / timeInterval2_p;
			velocity1_p_half_Y = (y2 - y1) / timeInterval2_p;

			// calculate the real velocity at point (x1, y1)
			velocityAtPoint1_X = ((timeInterval1_m * velocity1_p_half_X) + (timeInterval1_p * velocity1_m_half_X)) / timeInterval1;
			velocityAtPoint1_Y = ((timeInterval1_m * velocity1_p_half_Y) + (timeInterval1_p * velocity1_m_half_Y)) / timeInterval1;



			// following steps are the process of calculating the velocity at point (x2, y2)
			// calculate velocity2_m_half
			velocity2_m_half_X = (x2 - x1) / timeInterval2_m;
			velocity2_m_half_Y = (y2 - y1) / timeInterval2_m;

			// calculate velocity2_p_half
			velocity2_p_half_X = (x3 - x2) / timeInterval2_p;
			velocity2_p_half_Y = (y3 - y2) / timeInterval2_p;

			// calculate the real velocity at point (x2, y2)
			velocityAtPoint2_X = ((timeInterval2_m * velocity2_p_half_X) + (timeInterval2_p * velocity2_m_half_X)) / timeInterval2;
			velocityAtPoint2_Y = ((timeInterval2_m * velocity2_p_half_Y) + (timeInterval2_p * velocity2_m_half_Y)) / timeInterval2;
		}

		// calculating the x, y coordinates for the pi, pi+, p(i+1), (i+1)- 
		// pi is x1

		// pi+
		x1_p = x1 + (1 / 3.0) * timeInterval1_p * velocityAtPoint1_X;
		y1_p = y1 + (1 / 3.0) * timeInterval1_p * velocityAtPoint1_Y;

		//p(i+1) is x2

		//p(i+1)-
		x2_m = x2 - (1 / 3.0) * timeInterval2_m * velocityAtPoint2_X;
		y2_m = y2 - (1 / 3.0) * timeInterval2_m * velocityAtPoint2_Y;




		/*Is it ok to use timeInterval1_p = 1 for the first case?*/


		// ewven though it iterates through all of the points 
		// but it should not all of the points in each iteration 
		if (i == 0) {

			controlPoints[countControlPoins][0] = x1;
			controlPoints[countControlPoins++][1] = y1;

			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}
		else {

			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}

		if (countControlPoins > (MaxNumDots - 1) * 3 + 1) {
			countControlPoins = (MaxNumDots - 1) * 3 + 1;
		}

	}

}




void calculateControlPoints_Centrpetal() {
	float x1, x2, x3, y1, y2, y3;
	float x1_p = 0, y1_p = 0, x2_m = 0, y2_m = 0;

	float velocity1_m_half_X, velocity1_m_half_Y,
		velocity1_p_half_X, velocity1_p_half_Y;

	float velocity2_m_half_X, velocity2_m_half_Y,
		velocity2_p_half_X, velocity2_p_half_Y;

	float velocityAtPoint1_X, velocityAtPoint1_Y,
		velocityAtPoint2_X, velocityAtPoint2_Y;

	float timeInterval2_m = 1, timeInterval2_p = 1, timeInterval2,
		timeInterval1_m = 1, timeInterval1_p = 1, timeInterval1; // the time interval value

	for (int i = 0; i <= NumDots - 2; i++) {

		// (x1, y1), (x2, y2) are the starting and ending control points for one piece of Bezier curve 
		x1 = dotArray[i][0];
		y1 = dotArray[i][1];

		x2 = dotArray[i + 1][0];
		y2 = dotArray[i + 1][1];

		x3 = dotArray[i + 2][0];
		y3 = dotArray[i + 2][1];

		// those are the time interval value regrading p2(x2, y2) as the pi,
		//p1(x1, x1) as p(i-1), and p3(x3, y3) as p(i+1)
		timeInterval2_m = sqrt( sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) );
		timeInterval2_p = sqrt( sqrt((x2 - x3) * (x2 - x3) + (y2 - y3) * (y2 - y3)) );
		//timeInterval2 = sqrt( sqrt((x3 - x1) * (x3 - x1) + (y3 - y1) * (y3 - y1)) );   worng!!!!!!!
		timeInterval2 = timeInterval2_m + timeInterval2_p;

		// drawing the first Bezier curve
		if (i == 0) {

			// calculate the velocity for the starting point
			velocityAtPoint1_X = initialVelocity_X;
			velocityAtPoint1_Y = initialVelocity_Y;

			// following steps are the process of calculating the velocity at point (x2, y2)
			// calculate velocity2_m_half
			velocity2_m_half_X = (x2 - x1) / timeInterval2_m;
			velocity2_m_half_Y = (y2 - y1) / timeInterval2_m;

			// calculate velocity2_p_half
			velocity2_p_half_X = (x3 - x2) / timeInterval2_p;
			velocity2_p_half_Y = (y3 - y2) / timeInterval2_p;

			// calculate the real velocity at point (x2, y2)
			velocityAtPoint2_X = ((timeInterval2_m * velocity2_p_half_X) + (timeInterval2_p * velocity2_m_half_X)) / timeInterval2;
			velocityAtPoint2_Y = ((timeInterval2_m * velocity2_p_half_Y) + (timeInterval2_p * velocity2_m_half_Y)) / timeInterval2;

		}
		else if (i == NumDots - 2) {
			// introducing p0 for calculating the velocity at P1(x1, y1)
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];

			// those are the time interval value regrading p1(x1, y1) as the pi,
			//p0(x0, x0) as p(i-1), and p2(x2, y2) as p(i+1)
			timeInterval1_m = sqrt( sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) );
			timeInterval1_p = sqrt( sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) );
			//timeInterval1 = sqrt( sqrt((x2 - x0) * (x2 - x0) + (y2 - y0) * (y2 - y0)) );
			timeInterval1 = timeInterval1_m + timeInterval1_p;

			// calculate the velocityAtP2  
			velocityAtPoint2_X = finalVelocity_X;
			velocityAtPoint2_Y = finalVelocity_Y;

			// following steps are the process of calculating the velocity at point (x1, y1)
			// calculate velocity1_m_half
			velocity1_m_half_X = (x1 - x0) / timeInterval2_m;
			velocity1_m_half_Y = (y1 - y0) / timeInterval2_m;

			// calculate velocity1_p_half
			velocity1_p_half_X = (x2 - x1) / timeInterval2_p;
			velocity1_p_half_Y = (y2 - y1) / timeInterval2_p;

			// calculate the real velocity at point (x1, y1)
			velocityAtPoint1_X = ((timeInterval1_m * velocity1_p_half_X) + (timeInterval1_p * velocity1_m_half_X)) / timeInterval1;
			velocityAtPoint1_Y = ((timeInterval1_m * velocity1_p_half_Y) + (timeInterval1_p * velocity1_m_half_Y)) / timeInterval1;

		}
		else {
			// introducing p0 for calculating the velocity at P1(x1, y1)
			float x0 = dotArray[i - 1][0];
			float y0 = dotArray[i - 1][1];

			// those are the time interval value regrading p1(x1, y1) as the pi,
			//p0(x0, x0) as p(i-1), and p2(x2, y2) as p(i+1)
			timeInterval1_m = sqrt( sqrt((x1 - x0) * (x1 - x0) + (y1 - y0) * (y1 - y0)) );
			timeInterval1_p = sqrt( sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) );
			//timeInterval1 = sqrt( sqrt((x2 - x0) * (x2 - x0) + (y2 - y0) * (y2 - y0)) );
			timeInterval1 = timeInterval1_m + timeInterval1_p;

			// following steps are the process of calculating the velocity at point (x1, y1)
			// calculate velocity1_m_half
			velocity1_m_half_X = (x1 - x0) / timeInterval2_m;
			velocity1_m_half_Y = (y1 - y0) / timeInterval2_m;

			// calculate velocity1_p_half
			velocity1_p_half_X = (x2 - x1) / timeInterval2_p;
			velocity1_p_half_Y = (y2 - y1) / timeInterval2_p;

			// calculate the real velocity at point (x1, y1)
			velocityAtPoint1_X = ((timeInterval1_m * velocity1_p_half_X) + (timeInterval1_p * velocity1_m_half_X)) / timeInterval1;
			velocityAtPoint1_Y = ((timeInterval1_m * velocity1_p_half_Y) + (timeInterval1_p * velocity1_m_half_Y)) / timeInterval1;



			// following steps are the process of calculating the velocity at point (x2, y2)
			// calculate velocity2_m_half
			velocity2_m_half_X = (x2 - x1) / timeInterval2_m;
			velocity2_m_half_Y = (y2 - y1) / timeInterval2_m;

			// calculate velocity2_p_half
			velocity2_p_half_X = (x3 - x2) / timeInterval2_p;
			velocity2_p_half_Y = (y3 - y2) / timeInterval2_p;

			// calculate the real velocity at point (x2, y2)
			velocityAtPoint2_X = ((timeInterval2_m * velocity2_p_half_X) + (timeInterval2_p * velocity2_m_half_X)) / timeInterval2;
			velocityAtPoint2_Y = ((timeInterval2_m * velocity2_p_half_Y) + (timeInterval2_p * velocity2_m_half_Y)) / timeInterval2;
		}

		// calculating the x, y coordinates for the pi, pi+, p(i+1), (i+1)- 
		// pi is x1

		// pi+
		x1_p = x1 + (1 / 3.0) * timeInterval1_p * velocityAtPoint1_X;
		y1_p = y1 + (1 / 3.0) * timeInterval1_p * velocityAtPoint1_Y;

		//p(i+1) is x2

		//p(i+1)-
		x2_m = x2 - (1 / 3.0) * timeInterval2_m * velocityAtPoint2_X;
		y2_m = y2 - (1 / 3.0) * timeInterval2_m * velocityAtPoint2_Y;


		// ewven though it iterates through all of the points 
		// but it should not all of the points in each iteration 
		if (i == 0) {

			controlPoints[countControlPoins][0] = x1;
			controlPoints[countControlPoins++][1] = y1;

			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}
		else {

			controlPoints[countControlPoins][0] = x1_p;
			controlPoints[countControlPoins++][1] = y1_p;

			controlPoints[countControlPoins][0] = x2_m;
			controlPoints[countControlPoins++][1] = y2_m;

			controlPoints[countControlPoins][0] = x2;
			controlPoints[countControlPoins++][1] = y2;
		}

		if (countControlPoins > (MaxNumDots - 1) * 3 + 1) {
			countControlPoins = (MaxNumDots - 1) * 3 + 1;
		}

	}

}


void storePoints_OneBezierCurve(VectorR2 p0, VectorR2 p1, VectorR2 p2, VectorR2 p3) {

	for ( int i = 0; i < MeshRes; i++) { // because here doesn't have equal sign, so the last point of the curve won't be add to the array
		float alpha = (float) i / 20.0;
		VectorR2 r0 = (1 - alpha) * p0 + alpha * p1;
		VectorR2 r1 = (1 - alpha) * p1 + alpha * p2;
		VectorR2 r2 = (1 - alpha) * p2 + alpha * p3;

		VectorR2 t0 = (1 - alpha) * r0 + alpha * r1;
		VectorR2 t1 = (1 - alpha) * r1 + alpha * r2;

		VectorR2 s0 = (1 - alpha) * t0 + alpha * t1;

		pointsOnCurve[countPointsOnCurve][0] = s0.x;
		pointsOnCurve[countPointsOnCurve++][1] = s0.y;
	}


}


void storePoints_AllBezierCurves() {
	VectorR2 p0, p1, p2, p3;
	int numberOfCurves = NumDots - 1;
	countPointsOnCurve = 0;

	for (int i = 0; i < numberOfCurves; i++) {
		if (i == 0) {
			p0 = VectorR2(controlPoints[0][0], controlPoints[0][1]);
			p1 = VectorR2(controlPoints[1][0], controlPoints[1][1]);
			p2 = VectorR2(controlPoints[2][0], controlPoints[2][1]);
			p3 = VectorR2(controlPoints[3][0], controlPoints[3][1]);
		}
		else {
			p0 = VectorR2(controlPoints[i * 4 - i][0], controlPoints[i * 4 - i][1]);
			p1 = VectorR2(controlPoints[i * 4 - i + 1][0], controlPoints[i * 4 - i + 1][1]);
			p2 = VectorR2(controlPoints[i * 4 - i + 2][0], controlPoints[i * 4 - i + 2][1]);
			p3 = VectorR2(controlPoints[i * 4 - i + 3][0], controlPoints[i * 4 - i + 3][1]);
		}

		storePoints_OneBezierCurve(p0, p1, p2, p3);
	}

	// add the last point of the whole curve
	float p_last_x = dotArray[NumDots - 1][0];
	float p_last_y = dotArray[NumDots - 1][1];

	pointsOnCurve[countPointsOnCurve][0] = p_last_x;
	pointsOnCurve[countPointsOnCurve++][1] = p_last_y;
}



void LoadPointsIntoVBO() 
{
    // Using glBufferSubData (with "Sub") does not resize the VBO.  
    // The VBO was sized earlier with glBufferData
    glBindBuffer(GL_ARRAY_BUFFER, myVBO[0]);
    glBufferSubData(GL_ARRAY_BUFFER, 0, NumDots * 2 * sizeof(float), dotArray);
    check_for_opengl_errors();

	// controlPoints Array
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[1]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (3 * (NumDots - 1) + 1) * 2 * sizeof(float), controlPoints);
	check_for_opengl_errors();

	// pointsOnCurve Array
	glBindBuffer(GL_ARRAY_BUFFER, myVBO[2]);
	glBufferSubData(GL_ARRAY_BUFFER, 0, (20 * (NumDots - 1)) * 2 * sizeof(float), pointsOnCurve);
	check_for_opengl_errors();
}





void AddPoint(float x, float y)
{

	// check for the reapted points 
	if (x == dotArray[NumDots - 1][0] && y == dotArray[NumDots - 1][1]) {
		return;
	}

    if (NumDots < MaxNumDots) {
        dotArray[NumDots][0] = x;
        dotArray[NumDots][1] = y;
        NumDots++;
        LoadPointsIntoVBO();
    }
	else { // if the current number of points are
		//more than than the maximum points 
		RemoveFirstPoint();
		dotArray[NumDots][0] = x;
		dotArray[NumDots][1] = y;
	}


	// every time add new points recalculate the controlPoints array and 
	// draw the Bezier curves 
	countControlPoins = 0;

	switch (mode) {
	case 1:
		calculateControlPoints_CatMull_Rom();
		break;
	case 2:
		calculateControlPoints_Chord();
		break;
	case 3: 
		calculateControlPoints_Centrpetal();
		break;
	}

	// recalculate the points in Bezier curve
	storePoints_AllBezierCurves();

	// load the points in VBO
	LoadPointsIntoVBO();
	
	// redraw the curve
	renderCurve();
}


void ChangePoint(int i, float x, float y)
{
	if (mode == 0) {
		assert(i >= 0 && i < NumDots);
		dotArray[i][0] = x;
		dotArray[i][1] = y;
		LoadPointsIntoVBO();
	}
	else {
		assert(i >= 0 && i < MeshRes * (MaxNumDots - 1) + 1);
		dotArray[i][0] = x;
		dotArray[i][1] = y;

		countControlPoins = 0;
		if (mode == 1) {
			calculateControlPoints_CatMull_Rom();
		}
		else if (mode == 2) {
			calculateControlPoints_Chord();
		}
		else if (mode == 3) {
			calculateControlPoints_Centrpetal();
		}

		storePoints_AllBezierCurves();
		LoadPointsIntoVBO();

	}
}


void RemoveFirstPoint()
{
    if (NumDots == 0) {
        return;
    }
    for (int i = 0; i < NumDots - 1; i++) {
        dotArray[i][0] = dotArray[i + 1][0];
        dotArray[i][1] = dotArray[i + 1][1];
    }
    NumDots--;
    if (NumDots > 0) {
        LoadPointsIntoVBO();
    }
}

void RemoveLastPoint()
{
    NumDots = (NumDots > 0) ? NumDots - 1 : 0;
    // No need to load points into the VBO again: they are already loaded.
}


// *************************************
// Main routine for rendering the scene
// myRenderScene() is called every time the scene needs to be redrawn.
// mySetupGeometries() has already created the vertex and buffer objects, etc.
// setup_shaders() has already created the shader programs.
// *************************************
void myRenderScene() {
	// Clear the rendering window
    static const float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, white);
    const float clearDepth = 1.0f;
    glClearBufferfv(GL_DEPTH, 0, &clearDepth);	// Must pass in a pointer to the depth value!

	// render the control points  
	if (NumDots > 0) {
		if (showingControlPoints == 1 && mode != 0) {
			renderControlPoints();
			check_for_opengl_errors();
		}
	}

	// render the curve
	if (NumDots > 0) {
		renderCurve();
		check_for_opengl_errors();
	}



	// render the straight line mode
    if (NumDots == 0) {
        return;
    }

	glUseProgram(shaderProgram1);
    glBindVertexArray(myVAO[0]);

    // Draw the line segments
    if (NumDots > 0 && mode == 0) {
        glVertexAttrib3f(vertColor_loc, 1.0f, 0.7f, 0.9f);		
        glDrawArrays(GL_LINE_STRIP, 0, NumDots);
    }

    // Draw the dots
	glVertexAttrib3f(vertColor_loc, 1.0f, 0.7f, 0.9f);		
	glDrawArrays(GL_POINTS, 0, NumDots);

	glBindVertexArray(0);
	check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!


}


void renderControlPoints() {

	if (NumDots <= 1) {
		return;
	}
	glUseProgram(shaderProgram1);
	glBindVertexArray(myVAO[1]);

	// Draw the dots
	glVertexAttrib3f(vertColor_loc, 0.0f, 0.5f, 0.8f);		// dark blue
	glDrawArrays(GL_POINTS, 0, 3 * (NumDots - 1) + 1);

	glBindVertexArray(0);
	check_for_opengl_errors();

}


void renderCurve() {

	if (NumDots <= 1) {
		return;
	}

	glUseProgram(shaderProgram1);
	glBindVertexArray(myVAO[2]);


	// Draw the line segments
	if (NumDots > 0 && (mode == 1 || mode == 2 || mode == 3)) {
		if (mode == 1) {
			glVertexAttrib3f(vertColor_loc, 0.7f, 0.5f, 0.8f);  //purple
			glDrawArrays(GL_LINE_STRIP, 0, MeshRes * (NumDots - 1));
		}
		else if (mode == 2) {
			glVertexAttrib3f(vertColor_loc, 1.0f, 1.0f, 0.0f);  // yellow
			glDrawArrays(GL_LINE_STRIP, 0, MeshRes * (NumDots - 1));
		}
		else {
			glVertexAttrib3f(vertColor_loc, 0.5f, 0.8f, 0.5f);  // green
			glDrawArrays(GL_LINE_STRIP, 0, MeshRes * (NumDots - 1));
		}
	}


	if (mode == 1 || mode == 2 || mode == 3) {
		if (mode == 1) {
			glVertexAttrib3f(vertColor_loc, 0.8f, 0.8f, 0.8f);
			glDrawArrays(GL_POINTS, 0, MeshRes * (NumDots - 1));
		}
		else if (mode == 2) {
			glVertexAttrib3f(vertColor_loc, 0.8f, 0.8f, 0.8f);
			glDrawArrays(GL_POINTS, 0, MeshRes * (NumDots - 1));
		}
		else {
			glVertexAttrib3f(vertColor_loc, 0.8f, 0.8f, 0.8f);
			glDrawArrays(GL_POINTS, 0, MeshRes * (NumDots - 1));
		}
	}
	glBindVertexArray(0);
	check_for_opengl_errors();

}

void my_setup_SceneData() {
	mySetupGeometries();

	setup_shaders();

    check_for_opengl_errors();   // Really a great idea to check for errors -- esp. good for debugging!
}

// *******************************************************
// Process all key press events.
// This routine is called each time a key is pressed or released.
// *******************************************************
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_RELEASE) {
		return;			// Ignore key up (key release) events
	}
	if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_X) {
		glfwSetWindowShouldClose(window, true);
	}
    else if (key == GLFW_KEY_F) {
        if (selectedVert != 0) {   // Don't allow removing "selected" vertex
            RemoveFirstPoint();
            selectedVert = (selectedVert<0) ? selectedVert : selectedVert - 1;
        }
    }
    else if (key == GLFW_KEY_L) {
        if (selectedVert < NumDots - 1) {   // Don't allow removing "selected" vertex
            RemoveLastPoint();
        }
    }
	else if (key == '0') {
		mode = 0; 

	}
	else if (key == '1') {
		mode = 1;

		// recalculate the controlPoints array every time press 1
		countControlPoins = 0;
		calculateControlPoints_CatMull_Rom();

		storePoints_AllBezierCurves();
		
		LoadPointsIntoVBO();
	
	}
	else if (key == '2') {
		mode = 2;

		// recalculate the controlPoints array every time press 2
		countControlPoins = 0;
		calculateControlPoints_Chord();

		storePoints_AllBezierCurves();

		LoadPointsIntoVBO();

	}
	else if (key == '3') {
		mode = 3;

		// recalculate the controlPoints array every time press 3
		countControlPoins = 0;
		calculateControlPoints_Centrpetal();

		storePoints_AllBezierCurves();

		LoadPointsIntoVBO();

	}
	else if (key == 'C' || key == 'c') {
		if (showingControlPoints) {
			showingControlPoints = 0;
		}
		else {
			showingControlPoints = 1; 
		}
	}
}

// *******************************************************
// Process all mouse button events.
// This routine is called each time a mouse button is pressed or released.
// *******************************************************
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // Scale x and y values into the range [-1,1]. 
        // Note that y values are negated since mouse measures y position from top of the window
        float dotX = (2.0f*(float)xpos / (float)(windowWidth-1)) - 1.0f;
        float dotY = 1.0f - (2.0f*(float)ypos / (float)(windowHeight-1));

        AddPoint(dotX, dotY);
    }

    else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            assert(selectedVert == -1);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            float dotX = (2.0f*(float)xpos / (float)(windowWidth - 1)) - 1.0f;
            float dotY = 1.0f - (2.0f*(float)ypos / (float)(windowHeight - 1));
            // Find closest extant point, if any. (distances minDist and thisDist are measured in pixels)
            float minDist = 10000.0f;
            int minI;
            for (int i = 0; i < NumDots; i++) {
                float thisDistX = 0.5f*(dotX - dotArray[i][0])*(float)windowWidth;
                float thisDistY = 0.5f*(dotY - dotArray[i][1])*(float)windowHeight;
                float thisDist = sqrtf(thisDistX*thisDistX + thisDistY * thisDistY);
                if (thisDist < minDist) {
                    minDist = thisDist;
                    minI = i;
                }
            }
            if (minDist <= 4.0) {      // If clicked within 4 pixels of the vertex
                selectedVert = minI;
				
				ChangePoint(selectedVert, dotX, dotY);
            }
        }
        else if (action == GLFW_RELEASE) {
            selectedVert = -1;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double x, double y) {
    if (selectedVert == -1) {
        return;
    }
    float dotX = (2.0f*(float)x / (float)(windowWidth - 1)) - 1.0f;
    float dotY = 1.0f - (2.0f*(float)y / (float)(windowHeight - 1));

	ChangePoint(selectedVert, dotX, dotY);
	
}



// *************************************************
// This function is called with the graphics window is first created,
//    and again whenever it is resized.
// The Projection View Matrix is typically set here.
//    But this program does not use any transformations or matrices.
// *************************************************
void window_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);		// Draw into entire window
    windowWidth = width;
    windowHeight = height;
}

void my_setup_OpenGL() {
	
	glEnable(GL_DEPTH_TEST);	// Enable depth buffering
	glDepthFunc(GL_LEQUAL);		// Useful for multipass shaders

// TRY IT OUT: How do the results look different if you disable the next block
// of code. (By changing "#if 1" to "#if 0"
#if 1
	// The following commands should induce OpenGL to create round points and 
	//	antialias points and lines.  (This is implementation dependent unfortunately.)
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);	// Make round points, not square points
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);		// Antialias the lines
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#endif

	// Specify the diameter of points, and the width of lines. Measured in pixels.
    // Results can be implementation dependent.
	// TRY IT OUT: Experiment with increasing and decreasing these values.
	glPointSize(8);
	glLineWidth(5);
	
}

void error_callback(int error, const char* description)
{
	// Print error
	fputs(description, stderr);
}

void setup_callbacks(GLFWwindow* window) {
	// Set callback function for resizing the window
	glfwSetFramebufferSizeCallback(window, window_size_callback);

	// Set callback for key up/down/repeat events and for mouse button events
	glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Set callbacks for mouse movement (cursor position).
    glfwSetCursorPosCallback(window, cursor_pos_callback);
}

// **********************
// Here is the main program
// **********************

int main() {
	glfwSetErrorCallback(error_callback);	// Supposed to be called in event of errors. (doesn't work?)
	glfwInit();
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	//glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	const int initWidth = 800;
	const int initHeight = 600;
	GLFWwindow* window = glfwCreateWindow(initWidth, initHeight, "ConnectDotsModern", NULL, NULL);
	if (window == NULL) {
		fprintf(stdout, "Failed to create GLFW window!\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewInit();

	// Print info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	printf("Supported GLSL version is %s.\n", (char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#endif
    printf("Using GLEW version %s.\n", glewGetString(GLEW_VERSION));

	printf("------------------------------\n");
	printf("Left-click with mouse to add points.\n");
    printf("Right-click and hold and move mouse to select and move vertices.\n");
    printf("Press 'f' or 'l' to remove the first point or the last point.\n");
    printf("Maximum of %d points permitted.\n", MaxNumDots);
    printf("Press ESCAPE or 'X' or 'x' to exit.\n");
	
    setup_callbacks(window);
    window_size_callback(window, initWidth, initHeight);

	// Initialize OpenGL, the scene and the shaders
    my_setup_OpenGL();
	my_setup_SceneData();
 
    // Loop while program is not terminated.
	while (!glfwWindowShouldClose(window)) {
	
		myRenderScene();				// Render into the current buffer
		glfwSwapBuffers(window);		// Displays what was just rendered (using double buffering).

		// Poll events (key presses, mouse events)
		glfwWaitEvents();					// Use this if no animation.
		//glfwWaitEventsTimeout(1.0/60.0);	// Use this to animate at 60 frames/sec (timing is NOT reliable)
		// glfwPollEvents();				// Use this version when animating as fast as possible
	}

	glfwTerminate();
	return 0;
}

// If an error is found, it could have been caused by any command since the
//   previous call to check_for_opengl_errors()
// To find what generated the error, you can try adding more calls to
//   check_for_opengl_errors().
char errNames[8][36] = {
    "Unknown OpenGL error",
    "GL_INVALID_ENUM", "GL_INVALID_VALUE", "GL_INVALID_OPERATION",
    "GL_INVALID_FRAMEBUFFER_OPERATION", "GL_OUT_OF_MEMORY",
    "GL_STACK_UNDERFLOW", "GL_STACK_OVERFLOW" };
bool check_for_opengl_errors() {
    int numErrors = 0;
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        numErrors++;
        int errNum = 0;
        switch (err) {
        case GL_INVALID_ENUM:
            errNum = 1;
            break;
        case GL_INVALID_VALUE:
            errNum = 2;
            break;
        case GL_INVALID_OPERATION:
            errNum = 3;
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errNum = 4;
            break;
        case GL_OUT_OF_MEMORY:
            errNum = 5;
            break;
        case GL_STACK_UNDERFLOW:
            errNum = 6;
            break;
        case GL_STACK_OVERFLOW:
            errNum = 7;
            break;
        }
        printf("OpenGL ERROR: %s.\n", errNames[errNum]);
    }
    return (numErrors != 0);
}