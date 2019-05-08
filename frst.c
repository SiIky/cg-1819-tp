#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

struct Point {
	float x;
	float y;
	float z;
};
struct Plane {
	struct Point p;
	struct Point n;
};
struct Point Point(float x, float y, float z)
{
	struct Point ret;
	ret.x = x;
	ret.y = y;
	ret.z = z;
	return ret;
}
struct Plane Plane(struct Point p, struct Point n )
{
	struct Plane ret;
	ret.p = p;
	ret.n = n;
	return ret;
}

//far plane
struct Plane far_plane;
//near plane
struct Plane near_plane;
//top plane
struct Plane top_plane;
//bot plane
struct Plane bot_plane;
//lrft plane
struct Plane left_plane;
//right plane
struct Plane right_plane;

int out = 0;
float fov = 45;
float nearDist = 1;
float farDist = 100; //1000
float Hnear;
float Wnear;
float Hfar;
float Wfar;
float lX = 0, lY = 0, lZ = 0;
float uX = 0, uY = 1, uZ = 0;
int startX, startY, tracking = 0;

int alpha = 0, beta = 0, r = 5;

float camX = +0, camY = 30, camZ = 40;


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if (h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0f / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(fov, ratio, nearDist, farDist);
	Hnear = 2 * tan(fov / 2) * nearDist;
	Wnear = Hnear * ratio;
	Hfar = 2 * tan(fov / 2) * farDist;
	Wfar = Hfar * ratio;
	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}
void changeSize2(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if (h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0f / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(fov, ratio, nearDist, 100001);
	Hnear = 2 * tan(fov / 2) * nearDist;
	Wnear = Hnear * ratio;
	Hfar = 2 * tan(fov / 2) * farDist;
	Wfar = Hfar * ratio;
	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

static inline struct Point operator* (float s, struct Point A)
{
	return Point(A.x * s, A.y * s, A.z * s);
}
static inline struct Point operator/ (float s, struct Point A)
{
	return Point(A.x / s, A.y / s, A.z / s);
}

/**
 * @brief Performs the scalar operation on a point.
 * @param s - Scalar value.
 * @param A - Point.
 * @returns Returns the point with the X, Y and Z axis values scaled by s.
 */
static inline struct Point operator* (struct Point A, float s)
{
	return s * A;
}
static inline struct Point operator/ (struct Point A, float s)
{
	return s / A;
}
/**
 * @brief Performs the addition of two points.
 * @param A - First point.
 * @param B - Second point.
 * @returns Returns the result of adding both input points .
 */
static inline struct Point operator+ (struct Point A, struct Point B)
{
	return Point(A.x + B.x, A.y + B.y, A.z + B.z);
}

/**
 * @brief Performs the subtraction of two points.
 * @param A - First point.
 * @param B - Second point.
 * @returns Returns the result of sub both input points .
 */
static inline struct Point operator- (struct Point A, struct Point B)
{
	return Point(A.x - B.x, A.y - B.y, A.z - B.z);
}
/**
 * @brief Calculates the norm of a point.
 * @param v - Given point.
 * @returns Returns the result of the operation.
 */
static inline float norm(struct Point v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}



/**
 * @brief Calculates the distance between two points.
 * @param A - First point.
 * @param B - Second point.
 * @returns Returns the distance between the two given points.
 */
static inline float dist(struct Point A, struct Point B)
{
	return norm((-1 * A) + B);
}

/**
 * @brief Operation for normalizing a point.
 * @param A - Input point.
 * @returns Returns the result of normalizing the input point.
 *
 */
static inline struct Point normalize(struct Point A)
{
	return 1 / norm(A) * A;
}
static inline struct Point crossProduct(struct Point A, struct Point B)
{
	struct Point P = Point((A.y * B.z) - (A.z * B.y),( A.z * B.x) - (A.x * B.z), (A.x * B.y) - (A.y * B.x));
	return P;
}

static inline float distpp(struct Point n, struct Point p, struct Point c)
{
	
	float d = -(n.x*p.x + n.y*p.y + n.z*p.z);
	float di = n.x*c.x + n.y*c.y + n.z*c.z + d;
	return di;
}

float distancia(struct Point c, float r, struct Plane plane)
{	
	float d = distpp(plane.n, plane.p, c);
	if ( d + r >= 0.0 )
	{

			return 1;
	}
	return -1;
}

void draw()
{
	out = -1;
	

	struct Point c = Point(0, 0, 0);
	
	float r = 3;



	//check if box is on negative side of any plane
	if (distancia(c,r, far_plane) > 0 && distancia(c,r, near_plane) > 0 && distancia(c, r, top_plane) > 0 && distancia(c, r, bot_plane) > 0 && distancia(c, r, left_plane) > 0 && distancia(c, r, right_plane) > 0)
	{
		out = 1;
		glColor3f(1, 0, 0);
		GLUquadric *quad;
		quad = gluNewQuadric();
		gluSphere(quad, 3, 10, 10);
	}

}
void renderScene() {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(camX, camY, camZ, lX, lY, lZ, uX, uY, uZ);

	struct Point p = Point(camX, camY, camZ);
	struct Point l = Point(lX, lY, lZ);
	struct Point u = Point(uX, uY, uZ);
	struct Point d = normalize(l - p);
	struct Point right = normalize(crossProduct(d, u));

	//far points
	struct Point fc = p + (d * farDist);
	struct Point ftl = fc + (u * Hfar / 2) - (right * Wfar / 2);
	struct Point ftr = fc + (u * Hfar / 2) + (right * Wfar / 2);
	struct Point fbl = fc - (u * Hfar / 2) - (right * Wfar / 2);
	struct Point fbr = fc - (u * Hfar / 2) + (right * Wfar / 2);

	//near points
	struct Point nc = p + (d * nearDist);
	struct Point ntl = nc + (u * (Hnear / 2)) - (right * (Wnear / 2));
	struct Point ntr = nc + (u * (Hnear / 2)) + (right * (Wnear / 2));
	struct Point nbl = nc - (u * (Hnear / 2)) - (right * (Wnear / 2));
	struct Point nbr = nc - (u * (Hnear / 2)) + (right * (Wnear / 2));

	//farplane
	far_plane = Plane(fc, normalize(crossProduct(ftr - ftl, ftl - fbl)));

	//near plane
	near_plane = Plane(nc, normalize(crossProduct(ntl - ntr, ntl - nbl)));

	//top plane
	top_plane = Plane(ftl, normalize(crossProduct(ntl - ntr, ftr - ntr)));

	//bot plane
	bot_plane = Plane(nbl, normalize(crossProduct(nbr - nbl, fbl - nbl)));

	//left plane
	left_plane = Plane(ftl, normalize(crossProduct(ntl - ftl, fbl - ftl)));

	//right plane
	right_plane = Plane(ftr, normalize(crossProduct(nbr - fbr, ftr - fbr)));


	// put drawing instructions here
	draw();
	glutPostRedisplay();
	// End of frame
	glutSwapBuffers();
}
void renderScene2() {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	
	
	gluLookAt(camX, 1000, camZ, lX, lY, lZ, -1, 0, 0);	
	glPushMatrix();
	glTranslatef(camX, camY, camZ);

		
	glColor3f(0, 0, 1);
	GLUquadric *quad;
	quad = gluNewQuadric();
	gluSphere(quad, 3, 20, 20);
	glPopMatrix();
	if (out == 1)
		glColor3f(1, 0, 0);
	else
		glColor3f(0, 1, 0);

	quad = gluNewQuadric();
	gluSphere(quad, 3, 20, 20);
	


	
	glutPostRedisplay();
	// End of frame
	glutSwapBuffers();
}

void processMouseButtons(int button, int state, int xx, int yy)
{
	if (state == GLUT_DOWN)
	{
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON) { tracking = 1; }
		else if (button == GLUT_RIGHT_BUTTON) { tracking = 2; }
		else { tracking = 0; }
	}

	else if (state == GLUT_UP)
	{
		if (tracking == 1)
		{
			alpha += (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2)
		{
			r -= yy - startY;
			if (r < 3) { r = 3.0; }
		}
		tracking = 0;
	}
}


void processMouseMotion(int xx, int yy)
{
	int deltaX, deltaY;
	int alphaAux, betaAux;
	int rAux;

	if (!tracking) { return; }

	deltaX = xx - startX;
	deltaY = yy - startY;

	if (tracking == 1)
	{
		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;
		alpha = alphaAux;
		beta = betaAux;
		if (betaAux > 85.0) { betaAux = 85.0; }
		else if (betaAux < -85.0) { betaAux = -85.0; }
		rAux = r;
	}
	else if (tracking == 2)
	{
		alphaAux = alpha;
		betaAux = beta;
		rAux = r - deltaY;
		if (rAux < 3) { rAux = 3; }
	}
	camX = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camZ = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camY = rAux * sin(betaAux * 3.14 / 180.0);
}

int main(int argc, char **argv) {

	
	// put GLUT init here
	glutInit(&argc, argv);
	int WindowID1, WindowID2;
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	

	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	WindowID1 = glutCreateWindow("CG@DI");
	// put callback registration here
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	//glutIdleFunc(renderScene);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);
	
	
	
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 800);
	WindowID2 = glutCreateWindow("Window 2");
	glutDisplayFunc(renderScene2);
	glutReshapeFunc(changeSize2);
	//glutIdleFunc(renderScene2);
	

	camX = r * sin(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);
	camZ = r * cos(alpha * 3.14 / 180.0) * cos(beta * 3.14 / 180.0);
	camY = r * sin(beta * 3.14 / 180.0);




	
	




	// OpenGL settings 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);


	


	
	// enter GLUT's main loop
	glutMainLoop();

	return 1;
}

