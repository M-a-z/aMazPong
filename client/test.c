#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>

void drawitems_callback(void)
{
    int y=200;
    int x=300;
    glClear(GL_COLOR_BUFFER_BIT);
    printf("Window width is %u\n", glutGet(GLUT_WINDOW_WIDTH));
    printf("Window height is %u\n",glutGet(GLUT_WINDOW_HEIGHT));
    
//    glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();
        //glColor3b((unsigned char)0xd, (unsigned char)0x12, (unsigned char)0x17);
        glColor3b(127,127,127);
//        glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_TRIANGLE_FAN);
        glVertex2i(x,y);
        glVertex2i(x-4,y+4);
        glVertex2i(x-6,y);
        glVertex2i(x-4,y-4);
        glVertex2i(x,y-6);
        glVertex2i(x+4,y-4);
        glVertex2i(x+6,y);
        glVertex2i(x+4,y+4);
        glVertex2i(x,y+6);
        glVertex2i(x-4,y+4);
    glEnd();
    glutSwapBuffers();    
}


int main(int argc, char *argv[])
{
    int height=400;
    int width=600;
    printf("Initializing glut...\n");
	glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); //Init displaymode: RGB colors, Double buffering)
    printf("glutInitWindowSize(width=%d,height=%d)\n",width,height);
    glutInitWindowSize(width,height); //Set window size, width,height (pixels)
    glutInitWindowPosition(0,0);
    /* winhandler  = */ glutCreateWindow("Maz Bot ... errm PONG!");
    printf("viewport as whole window: %d,%d\n",width,height);
	glViewport(0,0,width,height); // map the actual pixels to window pixels.

    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    printf("Set world window? %d,%d\n",width,height);
	gluOrtho2D(0,width,0,height);
    glScalef(1.0,-1.0,1.0);
    glTranslatef(0.0,(double)-height,0.0);
	/* this flips the coordinate system so
				 that y increases going down the
				 screen (see below) */
//    glTranslatef(((double)area.width)/((double)width),((double)-1.0)*((double)area.height)/((double)height),1.0);
//	glTranslatef(0.0,-height,0.0);
//    glTranslatef(0.0,((double)-1.0)*((double)area.height)/((double)height),0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375,0.375,0.0);
    /* so we can draw using integer
				   coordinates (see above) */
    
    
    //glLoadIdentity();
//    glOrtho
/*    DEFINE THESE */
    glutDisplayFunc(&drawitems_callback);
    //glutIdleFunc(&drawitems_callback);
//    glutReshapeFunc(&CBresizeWindow);
//    glutKeyboardFunc(&CBkeyPress);
//    glutIdleFunc(&CBdisplay);
    glClearColor(0,0,0,0); //Set 'clearing color' to be black

    glutMainLoop();
    return 0;
}
