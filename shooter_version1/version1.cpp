// Include standard headers
#include <cstdio>
#include <cstdlib>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "polyhedron/polyhedron.cpp"
#include "sphere/sphere.cpp"
#include "workspace/floor.cpp"
#include "workspace/leftWall.cpp"
#include "workspace/rightWall.cpp"
#include "shooter_version1/workspace/frontWall.cpp"
#include "shooter_version1/workspace/ceiling.cpp"
#include "sphere/controllerSphere.cpp"

using namespace glm;

#include <common/controls.hpp>
#include <common/shader.hpp>

void drawFigure(float num_points, GLuint cubeVertex, GLuint cubeColor, GLuint MatrixID, GLuint programID, glm::mat4 MVP) {
// Use our shader
    glUseProgram(programID);

    // Send our transformation to the currently bound shader,
    // in the "MVP" uniform
    glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // 1rst attribute buffer : vertices
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVertex);
    glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
    );

    // 2nd attribute buffer : colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, cubeColor);
    glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            3,                                // size
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
    );
    glDrawArrays(GL_TRIANGLES, 0, num_points); // 12*3 indices starting at 0 -> 12 triangles

}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		getchar();
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		getchar();
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

    float params1[] = {0.7, -6.0, -1.0, 5.0};
    float params2[] = {0.5, 6.0, -1.0, 10.0};
    float params3[] = {0.5, 0.0, -1.0, 12.0};
    float params4[] = {0.3, 0.0, 0.0, -8.0};
    float direction[] = {0.0, 0.0, 0.035};

    std::vector<Polyhedron> polyhedrons;
    polyhedrons.push_back(Polyhedron(0, params1));
    polyhedrons.push_back(Polyhedron(1, params2));
    polyhedrons.push_back(Polyhedron(2, params3));

    std::vector<ControllerSphere> spheres;
    spheres.push_back(ControllerSphere(params4, direction));

    std::vector<std::pair<GLuint, GLuint> > workspace;
    workspace.push_back(std::make_pair(getFloorVertex(), getFloorColor()));
    workspace.push_back(std::make_pair(getLeftWallVertex(), getLeftWallColor()));
    workspace.push_back(std::make_pair(getRightWallVertex(), getRightWallColor()));
    workspace.push_back(std::make_pair(getDistanceWallVertex(), getDistanceWallColor()));
    workspace.push_back(std::make_pair(getTopWallVertex(), getTopWallColor()));

    do{
       if (polyhedrons.size() == 0) {
           break;
       }
        // Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Compute the MVP matrix from keyboard and mouse input
        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		for (int i = 0; i < polyhedrons.size(); i++) {
            drawFigure(polyhedrons[i].getNumPoints(), polyhedrons[i].getFigureVertex(), polyhedrons[i].getFigureColor(), MatrixID, programID, MVP);
		}

        for (int i = 0; i < workspace.size(); i++) {
            drawFigure(3*4, workspace[i].first, workspace[i].second, MatrixID, programID, MVP);
        }

        if (glfwGetKey( window, GLFW_KEY_ENTER ) == GLFW_PRESS) {
            spheres.push_back(ControllerSphere(params4, direction));
        }

        for (int i = 0; i < spheres.size(); i++) {
            drawFigure(14700, spheres[i].getSphereV(), spheres[i].getSphereC(), MatrixID, programID, MVP);
            spheres[i].changeSphere();
        }

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );


	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

