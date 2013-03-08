#include "display.h"
#include "opengl.h"
#include "font.h"

static int disp_height = 1;
static Font * fnt_reg = 0;
static const char * const fnt_reg_name = "./font/Lekton-Regular.ttf";

void
Disp_Init(int font_size)
{
	fnt_reg = Font_Init(fnt_reg_name, font_size);

	glShadeModel(GL_SMOOTH);
    glClearColor(1.0f, 1.0f, 0.97f, 0.0f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void
Disp_Destroy()
{
	Font_Destroy(fnt_reg);
}

//Frame is passed in, since input needs to deal with the Frame
// and Display does not handle input, but only displaying
void
Disp_Render(Frame * frm)
{
 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.0f);

	glColor3f(0.2f, 0.2f, 0.2f);

	glPushMatrix();
		glLoadIdentity();
		//have to shift over since origin is in the center by default
		glTranslatef(-230.0f, 0, 0);
		Font_Print(fnt_reg, frm, disp_height - 20);
	glPopMatrix();
}

void
Disp_Resize(int w, int h)
{
	// height and width ratio
    GLfloat ratio;
 
    // protect against a divide by zero
	if(h == 0) {
		h = 1;
	}

	disp_height = h;

    ratio = (GLfloat)w / (GLfloat)h;

    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ratio, 0.1f, 100.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}
