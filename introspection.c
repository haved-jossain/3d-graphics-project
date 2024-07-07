/*
* Keyboard controls 
*
* left arrow: turn left
* right arrow: turn right
* up arrow: move up
* down arrow: move down
* w: move forward
* a: strafe left
* s: move backward
* d: strafe right
* spacebar: show/hide keyboard controls
* 'l' toggles lighting
* z/x increase/decrease ambient light
* c/v decrease/increase diffuse light
* p pause/unpause animation
* [] lower/rise light
* +/- change field of view
*/

#include "SDL2/SDL.h"
#include "SDL2/SDL_mixer.h"
#include "CSCIx229.h"

// Misc. variable
int move=1;  // Move light
int fov=180;  // Field of view (for perspective)
double asp=1; // Aspect ratio
double dim=100.0;  // Size of world
float time=0;  // Elapsed time
int displayControls=0;  // Display keyboard controls
int first_time=1;

// Light values
int light=1;  // Lighting on/off
int distance=100; // Light distance
int ambient=0; // Ambient intensity (%)
int diffuse=100; // Diffuse intensity (%)
int zh=90; // Light azimuth
float ylight=25.0; // Elevation of light
unsigned int texture[3]; // Texture names
unsigned int stars[3000][2];
int obj;  // Object display list

// Camera variables
float directionX = 0;
float directionY = 0;
float directionZ = 0;
float angleX = 0.0;
float angleY = 0.0;
float xPos = 0;
float yPos = 0;
float zPos = 242;

// Navigation speed variable
float fraction = 0.5f;

// Start with a Little Bang
int bang = 1;
float expansion = 10;

// Compute the normal given three vertices
void computeNormal(double x1, double y1, double z1, double x2, double y2, double z2, double x4, double y4, double z4, double *x, double *y, double *z){
    double x5 = x4-x1;
    double y5 = y4-y1;
    double z5 = z4-z1;
    double x6 = x2-x1;
    double y6 = y2-y1;
    double z6 = z2-z1;
    
    double x7,y7,z7;
    
    x7 = y5*z6 - y6*z5;
    y7 = x5*z6 - x6*z5;
    z7 = x5*y6 - x6*y5;
    
    double d = (-1) * sqrt(x7*x7 + y7*y7 + z7*z7);
    *x = x7/d;
    *y = y7/d;
    *z = z7/d;
}

// Compute the normal of a star
void starNormal(double sx, double sy, double sz, double *x, double *y, double *z){  
    double d = (-1) * sqrt(sx*sx + sy*sy + sz*sz);
    *x = sx/d;
    *y = sy/d;
    *z = sz/d;
}

/*
* Draw vertex in polar coordinates with normal
* Source: Example code from class
*/
static void Vertex(double th,double ph, double rd){
    double x = Sin(th)*Cos(ph)*rd;
    double y = Cos(th)*Cos(ph)*rd;
    double z = Sin(ph)*rd;
    glNormal3d(x,y,z);
    glVertex3d(x,y,z);
}

/*
* Draw a ball at (x,y,z) that has radius (rd)
* Source: Example code from class
*/
static void ball(double x,double y,double z,double rd,double r,double g,double b){
    int th,ph;

    // Save transformation
    glPushMatrix();
    // Offset, scale and rotate
    glTranslated(x,y,z);
    glScaled(rd,rd,rd);
    // White ball
    glColor3f(r,g,b);
    // Bands of latitude
    for (ph=-90;ph<90;ph+=10)
    {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=2*10)
        {
            Vertex(th,ph,10);
            Vertex(th,ph+10,10);
        }
        glEnd();
    }
    // Undo transofrmations
    glPopMatrix();
}

// Draws a drifter at (x,y,z) 
// radius (r), core emission (emi), 
// scale (sc), tentacle rotation angle (rh)
void drawDrifter(double x, double y, double z, double r, double emi, double sc, double rh){

    int th,ph;
    float Emission1[] = {Sin(emi),Sin(emi),Sin(emi),1};
    float Emission2[] = {Cos(emi),Cos(emi),Cos(emi),1};
    float Emission3[] = {0,0,0,1};

    // Draw core
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission1);
    glPushMatrix();
    glTranslated(x,y+Sin(rh),z);
    glScaled(sc,2*sc,sc);

    glColor3f(0,0,0);

    for (ph=-90;ph<90;ph+=5)
    {
        glBegin(GL_QUAD_STRIP);
        for (th=0;th<=360;th+=2*5)
        {
            Vertex(th,ph,(r-1.5));
            Vertex(th,ph+5,(r-1.5));
        }
        glEnd();
    }
    glPopMatrix();

    // Draw tentacle 1
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission2);
    glPushMatrix();
    glTranslated(x,y+Sin(rh),z);
    glScaled(sc,sc,sc);
    glColor3f(1,0,0);
    glRotated(-fmod(90*time,360.0),0,1,0);

    double ty = 0, s = r-1;
    double tx,tz;
    for (ph=0;ph<3000*sc;ph+=10, ty-=0.05, s-=0.005)
    {
        glBegin(GL_POINTS);
        tx = Sin(ph)*s;
        tz = Cos(ph)*s;
        glVertex3d(tx,ty,tz);
        glEnd();
    }
    glPopMatrix();

    // Draw tentacle 2
    glPushMatrix();
    glTranslated(x,y+Sin(rh),z);
    glScaled(sc,sc,sc);
    glColor3f(0,1,0);
    glRotated(fmod(90*time,360.0),0,1,0);

    ty = 0, s = r-1.5;
    for (ph=0;ph<3000*sc;ph+=10, ty-=0.05, s-=0.003)
    {
        glBegin(GL_POINTS);
        tx = Sin(ph)*s;
        tz = Cos(ph)*s;
        glVertex3d(tx,ty,tz);
        glEnd();
    }
    glPopMatrix();

    // Draw tentacle 3
    glPushMatrix();
    glTranslated(x,y+Sin(rh),z);
    glScaled(sc,sc,sc);
    glColor3f(1,1,0);
    glRotated(fmod(90*time,360.0),0,1,0);

    ty = 0, s = r-2;
    for (ph=0;ph<3000*sc;ph+=10, ty-=0.05, s-=0.002)
    {
        glBegin(GL_POINTS);
        tx = Sin(ph)*s;
        tz = Cos(ph)*s;
        glVertex3d(tx,ty,tz);
        glEnd();
    }
    glPopMatrix();

    // Draw outer shell
    glPushMatrix();
    glTranslated(x,y+Sin(rh),z);
    glScaled(sc,sc,sc);
    glRotated(90,1,0,0);
    glColor3f(0,0.3,0);
    glMaterialfv(GL_FRONT,GL_EMISSION,Emission3);    
    for (ph=-90;ph<50;ph+=5)
    {
        glBegin(GL_POINTS);
        glColor3f(1,1,1);
        for (th=0;th<=360;th+=2*5)
        {
            double lx = Sin(th)*Cos(ph)*r;
            double ly = Cos(th)*Cos(ph)*r;
            double lz = Sin(ph)*r;
            glVertex3d(lx,ly,lz);
        }
        glEnd();
    }
    glPopMatrix();
}

// Draw floating base 
// radius (r), height (h), rotation angle (a)
void drawFloatingBase(double r, double h, double a){  
    glPushMatrix();
    glRotated(a,0,1,0);
    glColor3f(1,1,1);
    int i;
    double ran = 0;
    double x,y,z;
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    glBegin(GL_TRIANGLES);

    for(i = 0; i <= 360; i += 10){
        if(i!=0){
            glNormal3d(0,1,0);
            glColor3f(0,0,0);
            glTexCoord2f(0,0);
            glVertex3f(0,0,0);      
            ran = r + Sin(stars[i][0])*(r/5);
            glColor3f(.5,.5,.5);
            glTexCoord2f(0.5,1);
            glVertex3f(Cos(i)*ran,0,Sin(i)*ran);
            glTexCoord2f(1,1);
            glVertex3f(Cos(i+10)*ran,0,Sin(i+10)*ran);
            
        }
        computeNormal(0,-h,0,Cos(i)*ran,0,Sin(i)*ran,Cos(i+10)*ran,0,Sin(i+10)*ran,&x,&y,&z);
        glNormal3d(x,y,z);
        glColor3f(0,0,0);
        glTexCoord2f(0,0);
        glVertex3f(0,-h,0);
        glColor3f(.5,.5,.5);
        glTexCoord2f(0.5,1);
        glVertex3f(Cos(i)*ran,0,Sin(i)*ran);
        glTexCoord2f(1,1);
        glVertex3f(Cos(i+10)*ran,0,Sin(i+10)*ran);

        computeNormal(0,-h,0,Cos(i+10)*ran,0,Sin(i+10)*ran,Cos(i+10)*(r + Sin(stars[i+10][0])*(r/5)),0,Sin(i+10)*(r + Sin(stars[i+10][0])*(r/5)),&x,&y,&z);
        glNormal3d(x,y,z);
        glColor3f(0,0,0);
        glTexCoord2f(0,0);
        glVertex3f(0,-h,0);      
        glColor3f(.5,.5,.5);
        glTexCoord2f(0.5,1);
        glVertex3f(Cos(i+10)*ran,0,Sin(i+10)*ran);
        glTexCoord2f(1,1);
        ran = r + Sin(stars[i+10][0])*(r/5);
        glVertex3f(Cos(i+10)*ran,0,Sin(i+10)*ran);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// Draw the star sphere
void drawStars(){ 
   glPushMatrix();
   glRotated(90,1,0,0);
   glColor3f(1,1,1);
   int i;
   double brightness;
   double nx, ny, nz;
   double x, y, z;
   glBegin(GL_POINTS);
    for(i = 0; i < 3000; i++){
        x = Sin(stars[i][0])*Cos(stars[i][1])*dim;
        y = Cos(stars[i][0])*Cos(stars[i][1])*dim;
        z = Sin(stars[i][1])*dim;
        brightness = (rand() / (double)RAND_MAX);        
        starNormal(x,y,z,&nx,&ny,&nz);
        glNormal3d(nx,ny,nz);
        glColor3f(1*brightness,0.9*brightness,0.9*brightness);
        glVertex3f(x,y,z);
    }
    glEnd();
    glPopMatrix(); 
}

// Draw sand building 
// height (h), radius (r), number of floors (l)
void drawBuilding(double h, double r, int l){
    
    int i=0;
    double x,y,z;
    
    // Enable textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    
    glBegin(GL_QUADS);
    glColor3f(0.75,0.75,0.75);
    for(i=0; i<l; i++){
        computeNormal(0.0*r,0,1.0*r,  0.1*r,0,0.95*r,  0.0*r,(i+1)*h,1.0*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.0*r,i*h,1.0*r);
        glTexCoord2f(0,0); glVertex3f(0.1*r,i*h,0.95*r);
        glTexCoord2f(0,1); glVertex3f(0.1*r,(i+1)*h,0.95*r);
        glTexCoord2f(1,1); glVertex3f(0.0*r,(i+1)*h,1.0*r);
        
        computeNormal(0.1*r,0,0.95*r,  0.2*r,0,0.8*r,  0.1*r,(i+1)*h,0.95*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.1*r,i*h,0.95*r);
        glTexCoord2f(0,0); glVertex3f(0.2*r,i*h,0.8*r);
        glTexCoord2f(0,1); glVertex3f(0.2*r,(i+1)*h,0.8*r);
        glTexCoord2f(1,1); glVertex3f(0.1*r,(i+1)*h,0.95*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(0.2*r,0,0.8*r,  0.3*r,0,0.5*r,  0.2*r,(i+1)*h,0.8*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.2*r,i*h,0.8*r);
        glTexCoord2f(0,0); glVertex3f(0.3*r,i*h,0.5*r);
        glTexCoord2f(0,1); glVertex3f(0.3*r,(i+1)*h,0.5*r);
        glTexCoord2f(1,1); glVertex3f(0.2*r,(i+1)*h,0.8*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(0.3*r,0,0.5*r,  0.35*r,0,0.4*r,  0.3*r,(i+1)*h,0.5*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.3*r,i*h,0.5*r);
        glTexCoord2f(0,0); glVertex3f(0.35*r,i*h,0.4*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+1)*h,0.4*r);
        glTexCoord2f(1,1); glVertex3f(0.3*r,(i+1)*h,0.5*r);
        
        computeNormal(0.35*r,0,0.4*r,  0.4*r,0,0.35*r,  0.35*r,(i+1)*h,0.4*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.35*r,i*h,0.4*r);
        glTexCoord2f(0,0); glVertex3f(0.4*r,i*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.4*r,(i+1)*h,0.35*r);
        glTexCoord2f(1,1); glVertex3f(0.35*r,(i+1)*h,0.4*r);
        
        computeNormal(0.4*r,0,0.35*r,  0.5*r,0,0.3*r,  0.4*r,(i+1)*h,0.35*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.4*r,i*h,0.35*r);
        glTexCoord2f(0,0); glVertex3f(0.5*r,i*h,0.3*r);
        glTexCoord2f(0,1); glVertex3f(0.5*r,(i+1)*h,0.3*r);
        glTexCoord2f(1,1); glVertex3f(0.4*r,(i+1)*h,0.35*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(0.5*r,0,0.3*r,  0.8*r,0,0.2*r,  0.5*r,(i+1)*h,0.3*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.5*r,i*h,0.3*r);
        glTexCoord2f(0,0); glVertex3f(0.8*r,i*h,0.2*r);
        glTexCoord2f(0,1); glVertex3f(0.8*r,(i+1)*h,0.2*r);
        glTexCoord2f(1,1); glVertex3f(0.5*r,(i+1)*h,0.3*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(0.8*r,0,0.2*r,  0.95*r,0,0.1*r,  0.8*r,(i+1)*h,0.2*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.8*r,i*h,0.2*r);
        glTexCoord2f(0,0); glVertex3f(0.95*r,i*h,0.1*r);
        glTexCoord2f(0,1); glVertex3f(0.95*r,(i+1)*h,0.1*r);
        glTexCoord2f(1,1); glVertex3f(0.8*r,(i+1)*h,0.2*r);
        
        computeNormal(0.95*r,0,0.1*r,  1.0*r,0,0.0*r,  0.95*r,(i+1)*h,0.1*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.95*r,i*h,0.1*r);
        glTexCoord2f(0,0); glVertex3f(1.0*r,i*h,0.0*r);
        glTexCoord2f(0,1); glVertex3f(1.0*r,(i+1)*h,0.0*r);
        glTexCoord2f(1,1); glVertex3f(0.95*r,(i+1)*h,0.1*r);
        
        computeNormal(1.0*r,0,-0.0*r,  0.95*r,0,-0.1*r,  1.0*r,(i+1)*h,-0.0*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(1.0*r,i*h,-0.0*r);
        glTexCoord2f(0,0); glVertex3f(0.95*r,i*h,-0.1*r);
        glTexCoord2f(0,1); glVertex3f(0.95*r,(i+1)*h,-0.1*r);
        glTexCoord2f(1,1); glVertex3f(1.0*r,(i+1)*h,-0.0*r);
        
        computeNormal(0.95*r,0,-0.1*r,  0.8*r,0,-0.2*r,  0.95*r,(i+1)*h,-0.1*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.95*r,i*h,-0.1*r);
        glTexCoord2f(0,0); glVertex3f(0.8*r,i*h,-0.2*r);
        glTexCoord2f(0,1); glVertex3f(0.8*r,(i+1)*h,-0.2*r);
        glTexCoord2f(1,1); glVertex3f(0.95*r,(i+1)*h,-0.1*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(0.8*r,0,-0.2*r,  0.5*r,0,-0.3*r,  0.8*r,(i+1)*h,-0.2*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.8*r,i*h,-0.2*r);
        glTexCoord2f(0,0); glVertex3f(0.5*r,i*h,-0.3*r);
        glTexCoord2f(0,1); glVertex3f(0.5*r,(i+1)*h,-0.3*r);
        glTexCoord2f(1,1); glVertex3f(0.8*r,(i+1)*h,-0.2*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(0.5*r,0,-0.3*r,  0.4*r,0,-0.35*r,  0.5*r,(i+1)*h,-0.3*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.5*r,i*h,-0.3*r);
        glTexCoord2f(0,0); glVertex3f(0.4*r,i*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.4*r,(i+1)*h,-0.35*r);
        glTexCoord2f(1,1); glVertex3f(0.5*r,(i+1)*h,-0.3*r);
        
        computeNormal(0.4*r,0,-0.35*r,  0.35*r,0,-0.4*r,  0.4*r,(i+1)*h,-0.35*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.4*r,i*h,-0.35*r);
        glTexCoord2f(0,0); glVertex3f(0.35*r,i*h,-0.4*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+1)*h,-0.4*r);
        glTexCoord2f(1,1); glVertex3f(0.4*r,(i+1)*h,-0.35*r);
        
        computeNormal(0.35*r,0,-0.4*r,  0.3*r,0,-0.5*r,  0.35*r,(i+1)*h,-0.4*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.35*r,i*h,-0.4*r);
        glTexCoord2f(0,0); glVertex3f(0.3*r,i*h,-0.5*r);
        glTexCoord2f(0,1); glVertex3f(0.3*r,(i+1)*h,-0.5*r);
        glTexCoord2f(1,1); glVertex3f(0.35*r,(i+1)*h,-0.4*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(0.3*r,0,-0.5*r,  0.2*r,0,-0.8*r,  0.3*r,(i+1)*h,-0.5*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.3*r,i*h,-0.5*r);
        glTexCoord2f(0,0); glVertex3f(0.2*r,i*h,-0.8*r);
        glTexCoord2f(0,1); glVertex3f(0.2*r,(i+1)*h,-0.8*r);
        glTexCoord2f(1,1); glVertex3f(0.3*r,(i+1)*h,-0.5*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(0.2*r,0,-0.8*r,  0.1*r,0,-0.95*r,  0.2*r,(i+1)*h,-0.8*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.2*r,i*h,-0.8*r);
        glTexCoord2f(0,0); glVertex3f(0.1*r,i*h,-0.95*r);
        glTexCoord2f(0,1); glVertex3f(0.1*r,(i+1)*h,-0.95*r);
        glTexCoord2f(1,1); glVertex3f(0.2*r,(i+1)*h,-0.8*r);
        
        computeNormal(0.1*r,0,-0.95*r,  0.0*r,0,-1.0*r,  0.1*r,(i+1)*h,-0.95*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(0.1*r,i*h,-0.95*r);
        glTexCoord2f(0,0); glVertex3f(0.0*r,i*h,-1.0*r);
        glTexCoord2f(0,1); glVertex3f(0.0*r,(i+1)*h,-1.0*r);
        glTexCoord2f(1,1); glVertex3f(0.1*r,(i+1)*h,-0.95*r);
        
        computeNormal(-0.0*r,0,-1.0*r,  -0.1*r,0,-0.95*r,  -0.0*r,(i+1)*h,-1.0*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.0*r,i*h,-1.0*r);
        glTexCoord2f(0,0); glVertex3f(-0.1*r,i*h,-0.95*r);
        glTexCoord2f(0,1); glVertex3f(-0.1*r,(i+1)*h,-0.95*r);
        glTexCoord2f(1,1); glVertex3f(-0.0*r,(i+1)*h,-1.0*r);
        
        computeNormal(-0.1*r,0,-0.95*r,  -0.2*r,0,-0.8*r,  -0.1*r,(i+1)*h,-0.95*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.1*r,i*h,-0.95*r);
        glTexCoord2f(0,0); glVertex3f(-0.2*r,i*h,-0.8*r);
        glTexCoord2f(0,1); glVertex3f(-0.2*r,(i+1)*h,-0.8*r);
        glTexCoord2f(1,1); glVertex3f(-0.1*r,(i+1)*h,-0.95*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(-0.2*r,0,-0.8*r,  -0.3*r,0,-0.5*r,  -0.2*r,(i+1)*h,-0.8*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.2*r,i*h,-0.8*r);
        glTexCoord2f(0,0); glVertex3f(-0.3*r,i*h,-0.5*r);
        glTexCoord2f(0,1); glVertex3f(-0.3*r,(i+1)*h,-0.5*r);
        glTexCoord2f(1,1); glVertex3f(-0.2*r,(i+1)*h,-0.8*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(-0.3*r,0,-0.5*r,  -0.35*r,0,-0.4*r,  -0.3*r,(i+1)*h,-0.5*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.3*r,i*h,-0.5*r);
        glTexCoord2f(0,0); glVertex3f(-0.35*r,i*h,-0.4*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+1)*h,-0.4*r);
        glTexCoord2f(1,1); glVertex3f(-0.3*r,(i+1)*h,-0.5*r);
        
        computeNormal(-0.35*r,0,-0.4*r,  -0.4*r,0,-0.35*r,  -0.35*r,(i+1)*h,-0.4*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.35*r,i*h,-0.4*r);
        glTexCoord2f(0,0); glVertex3f(-0.4*r,i*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.4*r,(i+1)*h,-0.35*r);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,(i+1)*h,-0.4*r);
        
        computeNormal(-0.4*r,0,-0.35*r,  -0.5*r,0,-0.3*r,  -0.4*r,(i+1)*h,-0.35*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.4*r,i*h,-0.35*r);
        glTexCoord2f(0,0); glVertex3f(-0.5*r,i*h,-0.3*r);
        glTexCoord2f(0,1); glVertex3f(-0.5*r,(i+1)*h,-0.3*r);
        glTexCoord2f(1,1); glVertex3f(-0.4*r,(i+1)*h,-0.35*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(-0.5*r,0,-0.3*r,  -0.8*r,0,-0.2*r,  -0.5*r,(i+1)*h,-0.3*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.5*r,i*h,-0.3*r);
        glTexCoord2f(0,0); glVertex3f(-0.8*r,i*h,-0.2*r);
        glTexCoord2f(0,1); glVertex3f(-0.8*r,(i+1)*h,-0.2*r);
        glTexCoord2f(1,1); glVertex3f(-0.5*r,(i+1)*h,-0.3*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(-0.8*r,0,-0.2*r,  -0.95*r,0,-0.1*r,  -0.8*r,(i+1)*h,-0.2*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.8*r,i*h,-0.2*r);
        glTexCoord2f(0,0); glVertex3f(-0.95*r,i*h,-0.1*r);
        glTexCoord2f(0,1); glVertex3f(-0.95*r,(i+1)*h,-0.1*r);
        glTexCoord2f(1,1); glVertex3f(-0.8*r,(i+1)*h,-0.2*r);
        
        computeNormal(-0.95*r,0,-0.1*r,  -1.0*r,0,-0.0*r,  -0.95*r,(i+1)*h,-0.1*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.95*r,i*h,-0.1*r);
        glTexCoord2f(0,0); glVertex3f(-1.0*r,i*h,-0.0*r);
        glTexCoord2f(0,1); glVertex3f(-1.0*r,(i+1)*h,-0.0*r);
        glTexCoord2f(1,1); glVertex3f(-0.95*r,(i+1)*h,-0.1*r);
        
        computeNormal(-1.0*r,0,0.0*r,  -0.95*r,0,0.1*r,  -1.0*r,(i+1)*h,0.0*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-1.0*r,i*h,0.0*r);
        glTexCoord2f(0,0); glVertex3f(-0.95*r,i*h,0.1*r);
        glTexCoord2f(0,1); glVertex3f(-0.95*r,(i+1)*h,0.1*r);
        glTexCoord2f(1,1); glVertex3f(-1.0*r,(i+1)*h,0.0*r);
        
        computeNormal(-0.95*r,0,0.1*r,  -0.8*r,0,0.2*r,  -0.95*r,(i+1)*h,0.1*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.95*r,i*h,0.1*r);
        glTexCoord2f(0,0); glVertex3f(-0.8*r,i*h,0.2*r);
        glTexCoord2f(0,1); glVertex3f(-0.8*r,(i+1)*h,0.2*r);
        glTexCoord2f(1,1); glVertex3f(-0.95*r,(i+1)*h,0.1*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(-0.8*r,0,0.2*r,  -0.5*r,0,0.3*r,  -0.8*r,(i+1)*h,0.2*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.8*r,i*h,0.2*r);
        glTexCoord2f(0,0); glVertex3f(-0.5*r,i*h,0.3*r);
        glTexCoord2f(0,1); glVertex3f(-0.5*r,(i+1)*h,0.3*r);
        glTexCoord2f(1,1); glVertex3f(-0.8*r,(i+1)*h,0.2*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(-0.5*r,0,0.3*r,  -0.4*r,0,0.35*r,  -0.5*r,(i+1)*h,0.3*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.5*r,i*h,0.3*r);
        glTexCoord2f(0,0); glVertex3f(-0.4*r,i*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.4*r,(i+1)*h,0.35*r);
        glTexCoord2f(1,1); glVertex3f(-0.5*r,(i+1)*h,0.3*r);
        
        computeNormal(-0.4*r,0,0.35*r,  -0.35*r,0,0.4*r,  -0.4*r,(i+1)*h,0.35*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.4*r,i*h,0.35*r);
        glTexCoord2f(0,0); glVertex3f(-0.35*r,i*h,0.4*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+1)*h,0.4*r);
        glTexCoord2f(1,1); glVertex3f(-0.4*r,(i+1)*h,0.35*r);
        
        computeNormal(-0.35*r,0,0.4*r,  -0.3*r,0,0.5*r,  -0.35*r,(i+1)*h,0.4*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.35*r,i*h,0.4*r);
        glTexCoord2f(0,0); glVertex3f(-0.3*r,i*h,0.5*r);
        glTexCoord2f(0,1); glVertex3f(-0.3*r,(i+1)*h,0.5*r);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,(i+1)*h,0.4*r);
        
        if(i%2==1){glColor3f(0.1,0.1,0.1);}
        computeNormal(-0.3*r,0,0.5*r,  -0.2*r,0,0.8*r,  -0.3*r,(i+1)*h,0.5*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.3*r,i*h,0.5*r);
        glTexCoord2f(0,0); glVertex3f(-0.2*r,i*h,0.8*r);
        glTexCoord2f(0,1); glVertex3f(-0.2*r,(i+1)*h,0.8*r);
        glTexCoord2f(1,1); glVertex3f(-0.3*r,(i+1)*h,0.5*r);
        if(i%2==1){glColor3f(0.75,0.75,0.75);}
        
        computeNormal(-0.2*r,0,0.8*r,  -0.1*r,0,0.95*r,  -0.2*r,(i+1)*h,0.8*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.2*r,i*h,0.8*r);
        glTexCoord2f(0,0); glVertex3f(-0.1*r,i*h,0.95*r);
        glTexCoord2f(0,1); glVertex3f(-0.1*r,(i+1)*h,0.95*r);
        glTexCoord2f(1,1); glVertex3f(-0.2*r,(i+1)*h,0.8*r);
        
        computeNormal(-0.1*r,0,0.95*r,  -0.0*r,0,1.0*r,  -0.1*r,(i+1)*h,0.95*r,  &x,&y,&z);
        glNormal3f(x,y,z);
        glTexCoord2f(1,0); glVertex3f(-0.1*r,i*h,0.95*r);
        glTexCoord2f(0,0); glVertex3f(-0.0*r,i*h,1.0*r);
        glTexCoord2f(0,1); glVertex3f(-0.0*r,(i+1)*h,1.0*r);
        glTexCoord2f(1,1); glVertex3f(-0.1*r,(i+1)*h,0.95*r);
        
    }
    glEnd();
    glBegin(GL_QUADS);
    glColor3f(0.75,0.75,0.75);
    computeNormal(0.0*r,i*h,1.0*r,  0.1*r,i*h,0.95*r,  0.0*(r*0.75),(i+1)*h,1.0*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.0*r,i*h,1.0*r);
    glTexCoord2f(0,0); glVertex3f(0.1*r,i*h,0.95*r);
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.0*(r*0.75),(i+1)*h,1.0*(r*0.75));
    
    computeNormal(0.1*r,i*h,0.95*r,  0.2*r,i*h,0.8*r,  0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*r,i*h,0.95*r);
    glTexCoord2f(0,0); glVertex3f(0.2*r,i*h,0.8*r);
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    
    computeNormal(0.2*r,i*h,0.8*r,  0.3*r,i*h,0.5*r,  0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*r,i*h,0.8*r);
    glTexCoord2f(0,0); glVertex3f(0.3*r,i*h,0.5*r);
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    
    computeNormal(0.3*r,i*h,0.5*r,  0.35*r,i*h,0.4*r,  0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*r,i*h,0.5*r);
    glTexCoord2f(0,0); glVertex3f(0.35*r,i*h,0.4*r);
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    
    computeNormal(0.35*r,i*h,0.4*r,  0.4*r,i*h,0.35*r,  0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*r,i*h,0.4*r);
    glTexCoord2f(0,0); glVertex3f(0.4*r,i*h,0.35*r);
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    
    computeNormal(0.4*r,i*h,0.35*r,  0.5*r,i*h,0.3*r,  0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*r,i*h,0.35*r);
    glTexCoord2f(0,0); glVertex3f(0.5*r,i*h,0.3*r);
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    
    computeNormal(0.5*r,i*h,0.3*r,  0.8*r,i*h,0.2*r,  0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*r,i*h,0.3*r);
    glTexCoord2f(0,0); glVertex3f(0.8*r,i*h,0.2*r);
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    
    computeNormal(0.8*r,i*h,0.2*r,  0.95*r,i*h,0.1*r,  0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*r,i*h,0.2*r);
    glTexCoord2f(0,0); glVertex3f(0.95*r,i*h,0.1*r);
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    
    computeNormal(0.95*r,i*h,0.1*r,  1.0*r,i*h,0.0*r,  0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*r,i*h,0.1*r);
    glTexCoord2f(0,0); glVertex3f(1.0*r,i*h,0.0*r);
    glTexCoord2f(0,1); glVertex3f(1.0*(r*0.75),(i+1)*h,0.0*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    
    computeNormal(1.0*r,i*h,-0.0*r,  0.95*r,i*h,-0.1*r,  1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(1.0*r,i*h,-0.0*r);
    glTexCoord2f(0,0); glVertex3f(0.95*r,i*h,-0.1*r);
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75));
    
    computeNormal(0.95*r,i*h,-0.1*r,  0.8*r,i*h,-0.2*r,  0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*r,i*h,-0.1*r);
    glTexCoord2f(0,0); glVertex3f(0.8*r,i*h,-0.2*r);
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    
    computeNormal(0.8*r,i*h,-0.2*r,  0.5*r,i*h,-0.3*r,  0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*r,i*h,-0.2*r);
    glTexCoord2f(0,0); glVertex3f(0.5*r,i*h,-0.3*r);
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    
    computeNormal(0.5*r,i*h,-0.3*r,  0.4*r,i*h,-0.35*r,  0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*r,i*h,-0.3*r);
    glTexCoord2f(0,0); glVertex3f(0.4*r,i*h,-0.35*r);
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    
    computeNormal(0.4*r,i*h,-0.35*r,  0.35*r,i*h,-0.4*r,  0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*r,i*h,-0.35*r);
    glTexCoord2f(0,0); glVertex3f(0.35*r,i*h,-0.4*r);
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    
    computeNormal(0.35*r,i*h,-0.4*r,  0.3*r,i*h,-0.5*r,  0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*r,i*h,-0.4*r);
    glTexCoord2f(0,0); glVertex3f(0.3*r,i*h,-0.5*r);
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    
    computeNormal(0.3*r,i*h,-0.5*r,  0.2*r,i*h,-0.8*r,  0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*r,i*h,-0.5*r);
    glTexCoord2f(0,0); glVertex3f(0.2*r,i*h,-0.8*r);
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    
    computeNormal(0.2*r,i*h,-0.8*r,  0.1*r,i*h,-0.95*r,  0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*r,i*h,-0.8*r);
    glTexCoord2f(0,0); glVertex3f(0.1*r,i*h,-0.95*r);
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    
    computeNormal(0.1*r,i*h,-0.95*r,  0.0*r,i*h,-1.0*r,  0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*r,i*h,-0.95*r);
    glTexCoord2f(0,0); glVertex3f(0.0*r,i*h,-1.0*r);
    glTexCoord2f(0,1); glVertex3f(0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    
    computeNormal(-0.0*r,i*h,-1.0*r,  -0.1*r,i*h,-0.95*r,  -0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.0*r,i*h,-1.0*r);
    glTexCoord2f(0,0); glVertex3f(-0.1*r,i*h,-0.95*r);
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75));
    
    computeNormal(-0.1*r,i*h,-0.95*r,  -0.2*r,i*h,-0.8*r,  -0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*r,i*h,-0.95*r);
    glTexCoord2f(0,0); glVertex3f(-0.2*r,i*h,-0.8*r);
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    
    computeNormal(-0.2*r,i*h,-0.8*r,  -0.3*r,i*h,-0.5*r,  -0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*r,i*h,-0.8*r);
    glTexCoord2f(0,0); glVertex3f(-0.3*r,i*h,-0.5*r);
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    
    computeNormal(-0.3*r,i*h,-0.5*r,  -0.35*r,i*h,-0.4*r,  -0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*r,i*h,-0.5*r);
    glTexCoord2f(0,0); glVertex3f(-0.35*r,i*h,-0.4*r);
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    
    computeNormal(-0.35*r,i*h,-0.4*r,  -0.4*r,i*h,-0.35*r,  -0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*r,i*h,-0.4*r);
    glTexCoord2f(0,0); glVertex3f(-0.4*r,i*h,-0.35*r);
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    
    computeNormal(-0.4*r,i*h,-0.35*r,  -0.5*r,i*h,-0.3*r,  -0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*r,i*h,-0.35*r);
    glTexCoord2f(0,0); glVertex3f(-0.5*r,i*h,-0.3*r);
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    
    computeNormal(-0.5*r,i*h,-0.3*r,  -0.8*r,i*h,-0.2*r,  -0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*r,i*h,-0.3*r);
    glTexCoord2f(0,0); glVertex3f(-0.8*r,i*h,-0.2*r);
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    
    computeNormal(-0.8*r,i*h,-0.2*r,  -0.95*r,i*h,-0.1*r,  -0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*r,i*h,-0.2*r);
    glTexCoord2f(0,0); glVertex3f(-0.95*r,i*h,-0.1*r);
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    
    computeNormal(-0.95*r,i*h,-0.1*r,  -1.0*r,i*h,-0.0*r,  -0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*r,i*h,-0.1*r);
    glTexCoord2f(0,0); glVertex3f(-1.0*r,i*h,-0.0*r);
    glTexCoord2f(0,1); glVertex3f(-1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    
    computeNormal(-1.0*r,i*h,0.0*r,  -0.95*r,i*h,0.1*r,  -1.0*(r*0.75),(i+1)*h,0.0*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-1.0*r,i*h,0.0*r);
    glTexCoord2f(0,0); glVertex3f(-0.95*r,i*h,0.1*r);
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-1.0*(r*0.75),(i+1)*h,0.0*(r*0.75));
    
    computeNormal(-0.95*r,i*h,0.1*r,  -0.8*r,i*h,0.2*r,  -0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*r,i*h,0.1*r);
    glTexCoord2f(0,0); glVertex3f(-0.8*r,i*h,0.2*r);
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    
    computeNormal(-0.8*r,i*h,0.2*r,  -0.5*r,i*h,0.3*r,  -0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*r,i*h,0.2*r);
    glTexCoord2f(0,0); glVertex3f(-0.5*r,i*h,0.3*r);
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    
    computeNormal(-0.5*r,i*h,0.3*r,  -0.4*r,i*h,0.35*r,  -0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*r,i*h,0.3*r);
    glTexCoord2f(0,0); glVertex3f(-0.4*r,i*h,0.35*r);
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    
    computeNormal(-0.4*r,i*h,0.35*r,  -0.35*r,i*h,0.4*r,  -0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*r,i*h,0.35*r);
    glTexCoord2f(0,0); glVertex3f(-0.35*r,i*h,0.4*r);
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    
    computeNormal(-0.35*r,i*h,0.4*r,  -0.3*r,i*h,0.5*r,  -0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*r,i*h,0.4*r);
    glTexCoord2f(0,0); glVertex3f(-0.3*r,i*h,0.5*r);
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    
    computeNormal(-0.3*r,i*h,0.5*r,  -0.2*r,i*h,0.8*r,  -0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*r,i*h,0.5*r);
    glTexCoord2f(0,0); glVertex3f(-0.2*r,i*h,0.8*r);
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    
    computeNormal(-0.2*r,i*h,0.8*r,  -0.1*r,i*h,0.95*r,  -0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*r,i*h,0.8*r);
    glTexCoord2f(0,0); glVertex3f(-0.1*r,i*h,0.95*r);
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    
    computeNormal(-0.1*r,i*h,0.95*r,  -0.0*r,i*h,1.0*r,  -0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*r,i*h,0.95*r);
    glTexCoord2f(0,0); glVertex3f(-0.0*r,i*h,1.0*r);
    glTexCoord2f(0,1); glVertex3f(-0.0*(r*0.75),(i+1)*h,1.0*(r*0.75));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    
    glEnd();
    glBegin(GL_QUADS);
    glColor3f(0.75,0.75,0.75);
    computeNormal(0.0*(r*0.75),(i+1)*h,1.0*(r*0.75),  0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.0*(r*0.75),(i+1)*h,1.0*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5));
    
    computeNormal(0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    
    computeNormal(0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    
    computeNormal(0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    
    computeNormal(0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    
    computeNormal(0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    
    computeNormal(0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    
    computeNormal(0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    
    computeNormal(0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  1.0*(r*0.75),(i+1)*h,0.0*(r*0.75),  0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(1.0*(r*0.75),(i+1)*h,0.0*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    
    computeNormal(1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75),  0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5));
    
    computeNormal(0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    
    computeNormal(0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    
    computeNormal(0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    
    computeNormal(0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    
    computeNormal(0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    
    computeNormal(0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    
    computeNormal(0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    
    computeNormal(0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75),  0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    
    computeNormal(-0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75),  -0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  -0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.0*(r*0.75),(i+1)*h,-1.0*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5));
    
    computeNormal(-0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75),  -0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  -0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.75),(i+1)*h,-0.95*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    
    computeNormal(-0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75),  -0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  -0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.75),(i+1)*h,-0.8*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    
    computeNormal(-0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75),  -0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  -0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.75),(i+1)*h,-0.5*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    
    computeNormal(-0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75),  -0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  -0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.75),(i+1)*h,-0.4*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    
    computeNormal(-0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75),  -0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  -0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.75),(i+1)*h,-0.35*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    
    computeNormal(-0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75),  -0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  -0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.75),(i+1)*h,-0.3*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    
    computeNormal(-0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75),  -0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  -0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.75),(i+1)*h,-0.2*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    
    computeNormal(-0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75),  -1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75),  -0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.75),(i+1)*h,-0.1*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-1.0*(r*0.75),(i+1)*h,-0.0*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    
    computeNormal(-1.0*(r*0.75),(i+1)*h,0.0*(r*0.75),  -0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  -1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-1.0*(r*0.75),(i+1)*h,0.0*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5));
    
    computeNormal(-0.95*(r*0.75),(i+1)*h,0.1*(r*0.75),  -0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  -0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.75),(i+1)*h,0.1*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    
    computeNormal(-0.8*(r*0.75),(i+1)*h,0.2*(r*0.75),  -0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  -0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.75),(i+1)*h,0.2*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    
    computeNormal(-0.5*(r*0.75),(i+1)*h,0.3*(r*0.75),  -0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  -0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.75),(i+1)*h,0.3*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    
    computeNormal(-0.4*(r*0.75),(i+1)*h,0.35*(r*0.75),  -0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  -0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.75),(i+1)*h,0.35*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    
    computeNormal(-0.35*(r*0.75),(i+1)*h,0.4*(r*0.75),  -0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  -0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.75),(i+1)*h,0.4*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    
    computeNormal(-0.3*(r*0.75),(i+1)*h,0.5*(r*0.75),  -0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  -0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.75),(i+1)*h,0.5*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    
    computeNormal(-0.2*(r*0.75),(i+1)*h,0.8*(r*0.75),  -0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  -0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.75),(i+1)*h,0.8*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    
    computeNormal(-0.1*(r*0.75),(i+1)*h,0.95*(r*0.75),  -0.0*(r*0.75),(i+1)*h,1.0*(r*0.75),  -0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.75),(i+1)*h,0.95*(r*0.75));
    glTexCoord2f(0,0); glVertex3f(-0.0*(r*0.75),(i+1)*h,1.0*(r*0.75));
    glTexCoord2f(0,1); glVertex3f(-0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    
    glEnd();
    glBegin(GL_QUADS);
    glColor3f(0.75,0.75,0.75);
    computeNormal(0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5),  0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  0.0*(r*0.25),(i+2)*h,1.0*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.0*(r*0.25),(i+2)*h,1.0*(r*0.25));
    
    computeNormal(0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    
    computeNormal(0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    
    computeNormal(0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    
    computeNormal(0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    
    computeNormal(0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    
    computeNormal(0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    
    computeNormal(0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    
    computeNormal(0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5),  0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(1.0*(r*0.25),(i+2)*h,0.0*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    
    computeNormal(1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5),  0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25));
    
    computeNormal(0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    
    computeNormal(0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    
    computeNormal(0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    
    computeNormal(0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    
    computeNormal(0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    
    computeNormal(0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    
    computeNormal(0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    
    computeNormal(0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5),  0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    
    computeNormal(-0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5),  -0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  -0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.0*(r*0.5),(i+1.5)*h,-1.0*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25));
    
    computeNormal(-0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5),  -0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  -0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,-0.95*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    
    computeNormal(-0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5),  -0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  -0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,-0.8*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    
    computeNormal(-0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5),  -0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  -0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,-0.5*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    
    computeNormal(-0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5),  -0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  -0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,-0.4*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    
    computeNormal(-0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5),  -0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  -0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,-0.35*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    
    computeNormal(-0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5),  -0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  -0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,-0.3*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    
    computeNormal(-0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5),  -0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  -0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,-0.2*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    
    computeNormal(-0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5),  -1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5),  -0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,-0.1*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-1.0*(r*0.5),(i+1.5)*h,-0.0*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    
    computeNormal(-1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5),  -0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  -1.0*(r*0.25),(i+2)*h,0.0*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-1.0*(r*0.5),(i+1.5)*h,0.0*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-1.0*(r*0.25),(i+2)*h,0.0*(r*0.25));
    
    computeNormal(-0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5),  -0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  -0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.5),(i+1.5)*h,0.1*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    
    computeNormal(-0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5),  -0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  -0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.5),(i+1.5)*h,0.2*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    
    computeNormal(-0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5),  -0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  -0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.5),(i+1.5)*h,0.3*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    
    computeNormal(-0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5),  -0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  -0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.5),(i+1.5)*h,0.35*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    
    computeNormal(-0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5),  -0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  -0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.5),(i+1.5)*h,0.4*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    
    computeNormal(-0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5),  -0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  -0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.5),(i+1.5)*h,0.5*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    
    computeNormal(-0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5),  -0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  -0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.5),(i+1.5)*h,0.8*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    
    computeNormal(-0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5),  -0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5),  -0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.5),(i+1.5)*h,0.95*(r*0.5));
    glTexCoord2f(0,0); glVertex3f(-0.0*(r*0.5),(i+1.5)*h,1.0*(r*0.5));
    glTexCoord2f(0,1); glVertex3f(-0.0*(r*0.25),(i+2)*h,1.0*(r*0.25));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    
    glEnd();
    glBegin(GL_QUADS);
    glColor3f(0.75,0.75,0.75);
    computeNormal(0.0*(r*0.25),(i+2)*h,1.0*(r*0.25),  0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  0.0*(r*0),(i+2.25)*h,1.0*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.0*(r*0.25),(i+2)*h,1.0*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0),(i+2.25)*h,0.95*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.0*(r*0),(i+2.25)*h,1.0*(r*0));
    
    computeNormal(0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  0.1*(r*0),(i+2.25)*h,0.95*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0),(i+2.25)*h,0.8*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0),(i+2.25)*h,0.95*(r*0));
    
    computeNormal(0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  0.2*(r*0),(i+2.25)*h,0.8*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0),(i+2.25)*h,0.5*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0),(i+2.25)*h,0.8*(r*0));
    
    computeNormal(0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  0.3*(r*0),(i+2.25)*h,0.5*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0),(i+2.25)*h,0.4*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0),(i+2.25)*h,0.5*(r*0));
    
    computeNormal(0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  0.35*(r*0),(i+2.25)*h,0.4*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0),(i+2.25)*h,0.35*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0),(i+2.25)*h,0.4*(r*0));
    
    computeNormal(0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  0.4*(r*0),(i+2.25)*h,0.35*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0),(i+2.25)*h,0.3*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0),(i+2.25)*h,0.35*(r*0));
    
    computeNormal(0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  0.5*(r*0),(i+2.25)*h,0.3*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0),(i+2.25)*h,0.2*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0),(i+2.25)*h,0.3*(r*0));
    
    computeNormal(0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  0.8*(r*0),(i+2.25)*h,0.2*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0),(i+2.25)*h,0.1*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0),(i+2.25)*h,0.2*(r*0));
    
    computeNormal(0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  1.0*(r*0.25),(i+2)*h,0.0*(r*0.25),  0.95*(r*0),(i+2.25)*h,0.1*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(1.0*(r*0.25),(i+2)*h,0.0*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(1.0*(r*0),(i+2.25)*h,0.0*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0),(i+2.25)*h,0.1*(r*0));
    
    computeNormal(1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25),  0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  1.0*(r*0),(i+2.25)*h,-0.0*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.95*(r*0),(i+2.25)*h,-0.1*(r*0));
    glTexCoord2f(1,1); glVertex3f(1.0*(r*0),(i+2.25)*h,-0.0*(r*0));
    
    computeNormal(0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  0.95*(r*0),(i+2.25)*h,-0.1*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.8*(r*0),(i+2.25)*h,-0.2*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.95*(r*0),(i+2.25)*h,-0.1*(r*0));
    
    computeNormal(0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  0.8*(r*0),(i+2.25)*h,-0.2*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.5*(r*0),(i+2.25)*h,-0.3*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.8*(r*0),(i+2.25)*h,-0.2*(r*0));
    
    computeNormal(0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  0.5*(r*0),(i+2.25)*h,-0.3*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.4*(r*0),(i+2.25)*h,-0.35*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.5*(r*0),(i+2.25)*h,-0.3*(r*0));
    
    computeNormal(0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  0.4*(r*0),(i+2.25)*h,-0.35*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.35*(r*0),(i+2.25)*h,-0.4*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.4*(r*0),(i+2.25)*h,-0.35*(r*0));
    
    computeNormal(0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  0.35*(r*0),(i+2.25)*h,-0.4*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.3*(r*0),(i+2.25)*h,-0.5*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.35*(r*0),(i+2.25)*h,-0.4*(r*0));
    
    computeNormal(0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  0.3*(r*0),(i+2.25)*h,-0.5*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.2*(r*0),(i+2.25)*h,-0.8*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.3*(r*0),(i+2.25)*h,-0.5*(r*0));
    
    computeNormal(0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  0.2*(r*0),(i+2.25)*h,-0.8*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.1*(r*0),(i+2.25)*h,-0.95*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.2*(r*0),(i+2.25)*h,-0.8*(r*0));
    
    computeNormal(0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25),  0.1*(r*0),(i+2.25)*h,-0.95*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(0.0*(r*0),(i+2.25)*h,-1.0*(r*0));
    glTexCoord2f(1,1); glVertex3f(0.1*(r*0),(i+2.25)*h,-0.95*(r*0));
    
    computeNormal(-0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25),  -0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  -0.0*(r*0),(i+2.25)*h,-1.0*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.0*(r*0.25),(i+2)*h,-1.0*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0),(i+2.25)*h,-0.95*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.0*(r*0),(i+2.25)*h,-1.0*(r*0));
    
    computeNormal(-0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25),  -0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  -0.1*(r*0),(i+2.25)*h,-0.95*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.25),(i+2)*h,-0.95*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0),(i+2.25)*h,-0.8*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0),(i+2.25)*h,-0.95*(r*0));
    
    computeNormal(-0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25),  -0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  -0.2*(r*0),(i+2.25)*h,-0.8*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.25),(i+2)*h,-0.8*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0),(i+2.25)*h,-0.5*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0),(i+2.25)*h,-0.8*(r*0));
    
    computeNormal(-0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25),  -0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  -0.3*(r*0),(i+2.25)*h,-0.5*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.25),(i+2)*h,-0.5*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0),(i+2.25)*h,-0.4*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0),(i+2.25)*h,-0.5*(r*0));
    
    computeNormal(-0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25),  -0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  -0.35*(r*0),(i+2.25)*h,-0.4*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.25),(i+2)*h,-0.4*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0),(i+2.25)*h,-0.35*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0),(i+2.25)*h,-0.4*(r*0));
    
    computeNormal(-0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25),  -0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  -0.4*(r*0),(i+2.25)*h,-0.35*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.25),(i+2)*h,-0.35*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0),(i+2.25)*h,-0.3*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0),(i+2.25)*h,-0.35*(r*0));
    
    computeNormal(-0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25),  -0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  -0.5*(r*0),(i+2.25)*h,-0.3*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.25),(i+2)*h,-0.3*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0),(i+2.25)*h,-0.2*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0),(i+2.25)*h,-0.3*(r*0));
    
    computeNormal(-0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25),  -0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  -0.8*(r*0),(i+2.25)*h,-0.2*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.25),(i+2)*h,-0.2*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0),(i+2.25)*h,-0.1*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0),(i+2.25)*h,-0.2*(r*0));
    
    computeNormal(-0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25),  -1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25),  -0.95*(r*0),(i+2.25)*h,-0.1*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.25),(i+2)*h,-0.1*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-1.0*(r*0.25),(i+2)*h,-0.0*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-1.0*(r*0),(i+2.25)*h,-0.0*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0),(i+2.25)*h,-0.1*(r*0));
    
    computeNormal(-1.0*(r*0.25),(i+2)*h,0.0*(r*0.25),  -0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  -1.0*(r*0),(i+2.25)*h,0.0*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-1.0*(r*0.25),(i+2)*h,0.0*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.95*(r*0),(i+2.25)*h,0.1*(r*0));
    glTexCoord2f(1,1); glVertex3f(-1.0*(r*0),(i+2.25)*h,0.0*(r*0));
    
    computeNormal(-0.95*(r*0.25),(i+2)*h,0.1*(r*0.25),  -0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  -0.95*(r*0),(i+2.25)*h,0.1*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.95*(r*0.25),(i+2)*h,0.1*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.8*(r*0),(i+2.25)*h,0.2*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.95*(r*0),(i+2.25)*h,0.1*(r*0));
    
    computeNormal(-0.8*(r*0.25),(i+2)*h,0.2*(r*0.25),  -0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  -0.8*(r*0),(i+2.25)*h,0.2*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.8*(r*0.25),(i+2)*h,0.2*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.5*(r*0),(i+2.25)*h,0.3*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.8*(r*0),(i+2.25)*h,0.2*(r*0));
    
    computeNormal(-0.5*(r*0.25),(i+2)*h,0.3*(r*0.25),  -0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  -0.5*(r*0),(i+2.25)*h,0.3*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.5*(r*0.25),(i+2)*h,0.3*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.4*(r*0),(i+2.25)*h,0.35*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.5*(r*0),(i+2.25)*h,0.3*(r*0));
    
    computeNormal(-0.4*(r*0.25),(i+2)*h,0.35*(r*0.25),  -0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  -0.4*(r*0),(i+2.25)*h,0.35*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.4*(r*0.25),(i+2)*h,0.35*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.35*(r*0),(i+2.25)*h,0.4*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.4*(r*0),(i+2.25)*h,0.35*(r*0));
    
    computeNormal(-0.35*(r*0.25),(i+2)*h,0.4*(r*0.25),  -0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  -0.35*(r*0),(i+2.25)*h,0.4*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.35*(r*0.25),(i+2)*h,0.4*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.3*(r*0),(i+2.25)*h,0.5*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.35*(r*0),(i+2.25)*h,0.4*(r*0));
    
    computeNormal(-0.3*(r*0.25),(i+2)*h,0.5*(r*0.25),  -0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  -0.3*(r*0),(i+2.25)*h,0.5*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.3*(r*0.25),(i+2)*h,0.5*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.2*(r*0),(i+2.25)*h,0.8*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.3*(r*0),(i+2.25)*h,0.5*(r*0));
    
    computeNormal(-0.2*(r*0.25),(i+2)*h,0.8*(r*0.25),  -0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  -0.2*(r*0),(i+2.25)*h,0.8*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.2*(r*0.25),(i+2)*h,0.8*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.1*(r*0),(i+2.25)*h,0.95*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.2*(r*0),(i+2.25)*h,0.8*(r*0));
    
    computeNormal(-0.1*(r*0.25),(i+2)*h,0.95*(r*0.25),  -0.0*(r*0.25),(i+2)*h,1.0*(r*0.25),  -0.1*(r*0),(i+2.25)*h,0.95*(r*0),  &x,&y,&z);
    glNormal3f(x,-y,z);
    glTexCoord2f(1,0); glVertex3f(-0.1*(r*0.25),(i+2)*h,0.95*(r*0.25));
    glTexCoord2f(0,0); glVertex3f(-0.0*(r*0.25),(i+2)*h,1.0*(r*0.25));
    glTexCoord2f(0,1); glVertex3f(-0.0*(r*0),(i+2.25)*h,1.0*(r*0));
    glTexCoord2f(1,1); glVertex3f(-0.1*(r*0),(i+2.25)*h,0.95*(r*0));
    
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
}

// Draw cactus 
// segment height (h), radius (r)
// number of segments (l), different bottom for branch (b) 
void drawCactus(double h, double r, int l, int b){
    int i=0;
    double x,y,z;
    // Enable textures
    glEnable(GL_TEXTURE_2D);
    glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
    glBindTexture(GL_TEXTURE_2D,texture[0]);
    if(b == 1){ // If it's a branch
        
        // Draw segment 1 of the bottom part
        glBegin(GL_QUAD_STRIP);
        
        glNormal3f(0,1,0);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0); glVertex3f(-2*r,-2.5*h,-0.7*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1); glVertex3f(-1.55*r,-2.5*h,-1.05*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(-1.6*r,-2.5*h,-1.6*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(-1.15*r,-2.5*h,-1.55*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(-0.7*r,-2.5*h,-2*r);
        
        glNormal3f(0,-1,0);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(-0.65*r,-3.5*h,-1.55*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(0.2*r,-3.5*h,-1.6*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(0.15*r,-3.5*h,-1.15*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(0.4*r,-3.5*h,-0.7*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(0.15*r,-3.5*h,-0.65*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(0.2*r,-3.5*h,0.2*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glTexCoord2f(0,1);glVertex3f(-0.65*r,-3.5*h,0.15*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(-0.7*r,-2.5*h,0.4*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(-1.05*r,-2.5*h,0.15*r);
        
        glNormal3f(0,1,0);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(-1.6*r,-2.5*h,0.2*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(0,1);glVertex3f(-1.55*r,-2.5*h,-0.65*r);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-3*r,-4*h,-3*r);
        glTexCoord2f(1,0);glVertex3f(-2*r,-2.5*h,-0.7*r);
        
        glEnd();
        
        // Draw segment 2 of the bottom part
        glBegin(GL_QUAD_STRIP);
        
        computeNormal(-2*r,-2.5*h,-0.7*r, -1.6*r,-2*h,-0.3*r, -1.55*r,-2.5*h,-1.05*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-2*r,-2.5*h,-0.7*r);
        glTexCoord2f(1,0);glVertex3f(-1.6*r,-2*h,-0.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.55*r,-2.5*h,-1.05*r);
        glTexCoord2f(0,1);glVertex3f(-1.15*r,-2*h,-0.65*r);
        
        computeNormal(-1.6*r,-2.5*h,-1.6*r, -1.2*r,-2*h,-1.2*r, -1.15*r,-2.5*h,-1.55*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.6*r,-2.5*h,-1.6*r);
        glTexCoord2f(1,0);glVertex3f(-1.2*r,-2*h,-1.2*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.15*r,-2.5*h,-1.55*r);
        glTexCoord2f(0,1);glVertex3f(-0.65*r,-2*h,-1.15*r);
        
        computeNormal(-0.7*r,-2.5*h,-2*r, -0.3*r,-2*h,-1.6*r, -0.65*r,-3.5*h,-1.55*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.7*r,-2.5*h,-2*r);
        glTexCoord2f(1,0);glVertex3f(-0.3*r,-2*h,-1.6*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.65*r,-3.5*h,-1.55*r);
        glTexCoord2f(0,1);glVertex3f(0.05*r,-2*h,-1.15*r);
        
        computeNormal(0.2*r,-3.5*h,-1.6*r, 0.6*r,-2*h,-1.2*r, 0.15*r,-3.5*h,-1.15*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.2*r,-3.5*h,-1.6*r);
        glTexCoord2f(1,0);glVertex3f(0.6*r,-2*h,-1.2*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.15*r,-3.5*h,-1.15*r);
        glTexCoord2f(0,1);glVertex3f(0.55*r,-2*h,-0.65*r);
        
        computeNormal(0.4*r,-3.5*h,-0.7*r, 1.0*r,-2*h,-0.3*r, 0.15*r,-3.5*h,-0.65*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.4*r,-3.5*h,-0.7*r);
        glTexCoord2f(1,0);glVertex3f(1.0*r,-2*h,-0.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.15*r,-3.5*h,-0.65*r);
        glTexCoord2f(0,1);glVertex3f(0.55*r,-2*h,0.05*r);
        
        computeNormal(0.2*r,-3.5*h,0.2*r, 0.6*r,-2*h,0.6*r, -0.65*r,-3.5*h,0.15*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.2*r,-3.5*h,0.2*r);
        glTexCoord2f(1,0);glVertex3f(0.6*r,-2*h,0.6*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.65*r,-3.5*h,0.15*r);
        glTexCoord2f(0,1);glVertex3f(0.05*r,-2*h,0.55*r);
        
        computeNormal(-0.7*r,-2.5*h,0.4*r, -0.3*r,-2*h,1.0*r, -1.05*r,-2.5*h,0.15*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.7*r,-2.5*h,0.4*r);
        glTexCoord2f(1,0);glVertex3f(-0.3*r,-2*h,1.0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.05*r,-2.5*h,0.15*r);
        glTexCoord2f(0,1);glVertex3f(-0.65*r,-2*h,0.55*r);
        
        computeNormal(-1.6*r,-2.5*h,0.2*r, -1.2*r,-2*h,0.6*r, -1.55*r,-2.5*h,-0.65*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.6*r,-2.5*h,0.2*r);
        glTexCoord2f(1,0);glVertex3f(-1.2*r,-2*h,0.6*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.55*r,-2.5*h,-0.65*r);
        glTexCoord2f(0,1);glVertex3f(-1.15*r,-2*h,0.05*r);
        
        computeNormal(-2*r,-2.5*h,-0.7*r, -1.6*r,-2*h,-0.3*r, -1.55*r,-2.5*h,-1.05*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-2*r,-2.5*h,-0.7*r);
        glTexCoord2f(1,0);glVertex3f(-1.6*r,-2*h,-0.3*r);
        
        glEnd();
        
        // Draw segment 3 of the bottom part
        glBegin(GL_QUAD_STRIP);
        
        computeNormal(-1.6*r,-2*h,-0.3*r, -1.3*r,-1*h,0*r, -1.15*r,-2*h,-0.65*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.6*r,-2*h,-0.3*r);
        glTexCoord2f(1,0);glVertex3f(-1.3*r,-1*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.15*r,-2*h,-0.65*r);
        glTexCoord2f(0,1);glVertex3f(-0.85*r,-1*h,-0.35*r);
        
        computeNormal(-1.2*r,-2*h,-1.2*r, -0.9*r,-1*h,-0.9*r, -0.65*r,-2*h,-1.15*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.2*r,-2*h,-1.2*r);
        glTexCoord2f(1,0);glVertex3f(-0.9*r,-1*h,-0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.65*r,-2*h,-1.15*r);
        glTexCoord2f(0,1);glVertex3f(-0.35*r,-1*h,-0.85*r);
        
        computeNormal(-0.3*r,-2*h,-1.6*r, 0*r,-1*h,-1.3*r, 0.05*r,-2*h,-1.15*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.3*r,-2*h,-1.6*r);
        glTexCoord2f(1,0);glVertex3f(0*r,-1*h,-1.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.05*r,-2*h,-1.15*r);
        glTexCoord2f(0,1);glVertex3f(0.35*r,-1*h,-0.85*r);
        
        computeNormal(0.6*r,-2*h,-1.2*r, 0.9*r,-1*h,-0.9*r, 0.55*r,-2*h,-0.65*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.6*r,-2*h,-1.2*r);
        glTexCoord2f(1,0);glVertex3f(0.9*r,-1*h,-0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.55*r,-2*h,-0.65*r);
        glTexCoord2f(0,1);glVertex3f(0.85*r,-1*h,-0.35*r);
        
        computeNormal(1.0*r,-2*h,-0.3*r, 1.3*r,-1*h,0*r, 0.55*r,-2*h,0.05*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(1.0*r,-2*h,-0.3*r);
        glTexCoord2f(1,0);glVertex3f(1.3*r,-1*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.55*r,-2*h,0.05*r);
        glTexCoord2f(0,1);glVertex3f(0.85*r,-1*h,0.35*r);
        
        computeNormal(0.6*r,-2*h,0.6*r, 0.9*r,-1*h,0.9*r, 0.05*r,-2*h,0.55*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.6*r,-2*h,0.6*r);
        glTexCoord2f(1,0);glVertex3f(0.9*r,-1*h,0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.05*r,-2*h,0.55*r);
        glTexCoord2f(0,1);glVertex3f(0.35*r,-1*h,0.85*r);
        
        computeNormal(-0.3*r,-2*h,1.0*r, 0*r,-1*h,1.3*r, -0.65*r,-2*h,0.55*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.3*r,-2*h,1.0*r);
        glTexCoord2f(1,0);glVertex3f(0*r,-1*h,1.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.65*r,-2*h,0.55*r);
        glTexCoord2f(0,1);glVertex3f(-0.35*r,-1*h,0.85*r);
        
        computeNormal(-1.2*r,-2*h,0.6*r, -0.9*r,-1*h,0.9*r, -1.15*r,-2*h,0.05*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.2*r,-2*h,0.6*r);
        glTexCoord2f(1,0);glVertex3f(-0.9*r,-1*h,0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-1.15*r,-2*h,0.05*r);
        glTexCoord2f(0,1);glVertex3f(-0.85*r,-1*h,0.35*r);
        
        computeNormal(-1.6*r,-2*h,-0.3*r, -1.3*r,-1*h,0*r, -1.15*r,-2*h,-0.65*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.6*r,-2*h,-0.3*r);
        glTexCoord2f(1,0);glVertex3f(-1.3*r,-1*h,0*r);
        
        glEnd();
        
        // Draw segment 3 of the bottom part
        glBegin(GL_QUAD_STRIP);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.3*r,-1*h,0*r);
        glTexCoord2f(1,0);glVertex3f(-1.4*r,0*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.85*r,-1*h,-0.35*r);
        glTexCoord2f(0,1);glVertex3f(-0.85*r,0*h,-0.35*r);
        
        computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.9*r,-1*h,-0.9*r);
        glTexCoord2f(1,0);glVertex3f(-1*r,0*h,-1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.35*r,-1*h,-0.85*r);
        glTexCoord2f(0,1);glVertex3f(-0.35*r,0*h,-0.85*r);
        
        computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0*r,-1*h,-1.3*r);
        glTexCoord2f(1,0);glVertex3f(0*r,0*h,-1.4*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.35*r,-1*h,-0.85*r);
        glTexCoord2f(0,1);glVertex3f(0.35*r,0*h,-0.85*r);
        
        computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.9*r,-1*h,-0.9*r);
        glTexCoord2f(1,0);glVertex3f(1*r,0*h,-1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.85*r,-1*h,-0.35*r);
        glTexCoord2f(0,1);glVertex3f(0.85*r,0*h,-0.35*r);
        
        computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(1.3*r,-1*h,0*r);
        glTexCoord2f(1,0);glVertex3f(1.4*r,0*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.85*r,-1*h,0.35*r);
        glTexCoord2f(0,1);glVertex3f(0.85*r,0*h,0.35*r);
        
        computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0.9*r,-1*h,0.9*r);
        glTexCoord2f(1,0);glVertex3f(1*r,0*h,1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(0.35*r,-1*h,0.85*r);
        glTexCoord2f(0,1);glVertex3f(0.35*r,0*h,0.85*r);
        
        computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(0*r,-1*h,1.3*r);
        glTexCoord2f(1,0);glVertex3f(0*r,0*h,1.4*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.35*r,-1*h,0.85*r);
        glTexCoord2f(0,1);glVertex3f(-0.35*r,0*h,0.85*r);
        
        computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-0.9*r,-1*h,0.9*r);
        glTexCoord2f(1,0);glVertex3f(-1*r,0*h,1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1);glVertex3f(-0.85*r,-1*h,0.35*r);
        glTexCoord2f(0,1);glVertex3f(-0.85*r,0*h,0.35*r);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0);glVertex3f(-1.3*r,-1*h,0*r);
        glTexCoord2f(1,0);glVertex3f(-1.4*r,0*h,0*r);
        
        glEnd();
        
 
    }
    
    for(i = 0; i<l; i+=2){
        glBegin(GL_QUAD_STRIP);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1.4*r,i*h,0*r);
        glTexCoord2f(1,0); glVertex3f(-1.3*r,(i+1)*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.85*r,i*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.85*r,(i+1)*h,-0.35*r);
        
        computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1*r,i*h,-1*r);
        glTexCoord2f(1,0); glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,i*h,-0.85*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+1)*h,-0.85*r);
        
        computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0*r,i*h,-1.4*r);
        glTexCoord2f(1,0); glVertex3f(0*r,(i+1)*h,-1.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.35*r,i*h,-0.85*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+1)*h,-0.85*r);
        
        computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(1*r,i*h,-1*r);
        glTexCoord2f(1,0); glVertex3f(0.9*r,(i+1)*h,-0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.85*r,i*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.85*r,(i+1)*h,-0.35*r);
        
        computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(1.4*r,i*h,0*r);
        glTexCoord2f(1,0); glVertex3f(1.3*r,(i+1)*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.85*r,i*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.85*r,(i+1)*h,0.35*r);
        
        computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(1*r,i*h,1*r);
        glTexCoord2f(1,0); glVertex3f(0.9*r,(i+1)*h,0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.35*r,i*h,0.85*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+1)*h,0.85*r);
        
        computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0*r,i*h,1.4*r);
        glTexCoord2f(1,0); glVertex3f(0*r,(i+1)*h,1.3*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,i*h,0.85*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+1)*h,0.85*r);
        
        computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1*r,i*h,1*r);
        glTexCoord2f(1,0); glVertex3f(-0.9*r,(i+1)*h,0.9*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.85*r,i*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.85*r,(i+1)*h,0.35*r);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1.4*r,i*h,0*r);
        glTexCoord2f(1,0); glVertex3f(-1.3*r,(i+1)*h,0*r);
        
        glEnd();
        
        
        
        glBegin(GL_QUAD_STRIP);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1.3*r,(i+1)*h,0*r);
        glTexCoord2f(1,0); glVertex3f(-1.4*r,(i+2)*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.85*r,(i+1)*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.85*r,(i+2)*h,-0.35*r);
        
        computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
        glTexCoord2f(1,0); glVertex3f(-1*r,(i+2)*h,-1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,(i+1)*h,-0.85*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+2)*h,-0.85*r);
        
        computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0*r,(i+1)*h,-1.3*r);
        glTexCoord2f(1,0); glVertex3f(0*r,(i+2)*h,-1.4*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.35*r,(i+1)*h,-0.85*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+2)*h,-0.85*r);
        
        computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0.9*r,(i+1)*h,-0.9*r);
        glTexCoord2f(1,0); glVertex3f(1*r,(i+2)*h,-1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.85*r,(i+1)*h,-0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.85*r,(i+2)*h,-0.35*r);
        
        computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(1.3*r,(i+1)*h,0*r);
        glTexCoord2f(1,0); glVertex3f(1.4*r,(i+2)*h,0*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.85*r,(i+1)*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(0.85*r,(i+2)*h,0.35*r);
        
        computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0.9*r,(i+1)*h,0.9*r);
        glTexCoord2f(1,0); glVertex3f(1*r,(i+2)*h,1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(0.35*r,(i+1)*h,0.85*r);
        glTexCoord2f(0,1); glVertex3f(0.35*r,(i+2)*h,0.85*r);
        
        computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(0*r,(i+1)*h,1.3*r);
        glTexCoord2f(1,0); glVertex3f(0*r,(i+2)*h,1.4*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.35*r,(i+1)*h,0.85*r);
        glTexCoord2f(0,1); glVertex3f(-0.35*r,(i+2)*h,0.85*r);
        
        computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-0.9*r,(i+1)*h,0.9*r);
        glTexCoord2f(1,0); glVertex3f(-1*r,(i+2)*h,1*r);
        
        glColor3f(0,0.3,0);
        glTexCoord2f(1,1); glVertex3f(-0.85*r,(i+1)*h,0.35*r);
        glTexCoord2f(0,1); glVertex3f(-0.85*r,(i+2)*h,0.35*r);
        
        computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
        glNormal3f(x,y,z);
        
        glColor3f(0,0.5,0);
        glTexCoord2f(0,0); glVertex3f(-1.3*r,(i+1)*h,0*r);
        glTexCoord2f(1,0); glVertex3f(-1.4*r,(i+2)*h,0*r);
        
        glEnd();
        

        
    }
    
    // Draw segment 1 of the cactus's top part
    glBegin(GL_QUAD_STRIP);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1.4*r,i*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-1.3*r,(i+1)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.85*r,i*h,-0.35*r);
    glTexCoord2f(0,1);glVertex3f(-0.85*r,(i+1)*h,-0.35*r);
    
    computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1*r,i*h,-1*r);
    glTexCoord2f(1,0);glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.35*r,i*h,-0.85*r);
    glTexCoord2f(0,1);glVertex3f(-0.35*r,(i+1)*h,-0.85*r);
    
    computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,i*h,-1.4*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+1)*h,-1.3*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.35*r,i*h,-0.85*r);
    glTexCoord2f(0,1);glVertex3f(0.35*r,(i+1)*h,-0.85*r);
    
    computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(1*r,i*h,-1*r);
    glTexCoord2f(1,0);glVertex3f(0.9*r,(i+1)*h,-0.9*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.85*r,i*h,-0.35*r);
    glTexCoord2f(0,1);glVertex3f(0.85*r,(i+1)*h,-0.35*r);
    
    computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(1.4*r,i*h,0*r);
    glTexCoord2f(1,0);glVertex3f(1.3*r,(i+1)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glTexCoord2f(1,1);glVertex3f(0.85*r,i*h,0.35*r);
    glTexCoord2f(0,1);glVertex3f(0.85*r,(i+1)*h,0.35*r);
    
    computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(1*r,i*h,1*r);
    glTexCoord2f(1,0);glVertex3f(0.9*r,(i+1)*h,0.9*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.35*r,i*h,0.85*r);
    glTexCoord2f(0,1);glVertex3f(0.35*r,(i+1)*h,0.85*r);
    
    computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,i*h,1.4*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+1)*h,1.3*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.35*r,i*h,0.85*r);
    glTexCoord2f(0,1);glVertex3f(-0.35*r,(i+1)*h,0.85*r);
    
    computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1*r,i*h,1*r);
    glTexCoord2f(1,0);glVertex3f(-0.9*r,(i+1)*h,0.9*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.85*r,i*h,0.35*r);
    glTexCoord2f(0,1);glVertex3f(-0.85*r,(i+1)*h,0.35*r);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1.4*r,i*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-1.3*r,(i+1)*h,0*r);
    
    glEnd();
    

    
    // Draw segment 2 of the cactus's top part
    glBegin(GL_QUAD_STRIP);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1.3*r,(i+1)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-0.8*r,(i+2)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.85*r,(i+1)*h,-0.35*r);
    glTexCoord2f(0,1);glVertex3f(-0.65*r,(i+2)*h,-0.15*r);
    
    computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
    glTexCoord2f(1,0);glVertex3f(-0.4*r,(i+2)*h,-0.4*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.35*r,(i+1)*h,-0.85*r);
    glTexCoord2f(0,1);glVertex3f(-0.15*r,(i+2)*h,-0.65*r);
    
    computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,(i+1)*h,-1.3*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2)*h,-0.8*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.35*r,(i+1)*h,-0.85*r);
    glTexCoord2f(0,1);glVertex3f(0.15*r,(i+2)*h,-0.65*r);
    
    computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0.9*r,(i+1)*h,-0.9*r);
    glTexCoord2f(1,0);glVertex3f(0.4*r,(i+2)*h,-0.4*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.85*r,(i+1)*h,-0.35*r);
    glTexCoord2f(0,1);glVertex3f(0.65*r,(i+2)*h,-0.15*r);
    
    computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(1.3*r,(i+1)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(0.8*r,(i+2)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.85*r,(i+1)*h,0.35*r);
    glTexCoord2f(0,1);glVertex3f(0.65*r,(i+2)*h,0.15*r);
    
    computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0.9*r,(i+1)*h,0.9*r);
    glTexCoord2f(1,0);glVertex3f(0.4*r,(i+2)*h,0.4*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.35*r,(i+1)*h,0.85*r);
    glTexCoord2f(0,1);glVertex3f(0.15*r,(i+2)*h,0.65*r);
    
    computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,(i+1)*h,1.3*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2)*h,0.8*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.35*r,(i+1)*h,0.85*r);
    glTexCoord2f(0,1);glVertex3f(-0.15*r,(i+2)*h,0.65*r);
    
    computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.9*r,(i+1)*h,0.9*r);
    glTexCoord2f(1,0);glVertex3f(-0.4*r,(i+2)*h,0.4*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.85*r,(i+1)*h,0.35*r);
    glTexCoord2f(0,1);glVertex3f(-0.65*r,(i+2)*h,0.15*r);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-1.3*r,(i+1)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-0.8*r,(i+2)*h,0*r);
    
    glEnd();
    

    
    // Draw segment 3 of the cactus's top part
    glBegin(GL_QUAD_STRIP);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.8*r,(i+2)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-0*r,(i+2.4)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.65*r,(i+2)*h,-0.15*r);
    glTexCoord2f(0,1);glVertex3f(-0*r,(i+2.4)*h,-0*r);
    
    computeNormal(-1*r,i*h,-1*r, -0.9*r,(i+1)*h,-0.9*r, -0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.4*r,(i+2)*h,-0.4*r);
    glTexCoord2f(1,0);glVertex3f(-0*r,(i+2.4)*h,-0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.15*r,(i+2)*h,-0.65*r);
    glTexCoord2f(0,1);glVertex3f(-0*r,(i+2.4)*h,-0*r);
    
    computeNormal(0*r,i*h,-1.4*r, 0*r,(i+1)*h,-1.3*r, 0.35*r,i*h,-0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,(i+2)*h,-0.8*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2.4)*h,-0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.15*r,(i+2)*h,-0.65*r);
    glTexCoord2f(0,1);glVertex3f(0*r,(i+2.4)*h,-0*r);
    
    computeNormal(1*r,i*h,-1*r, 0.9*r,(i+1)*h,-0.9*r, 0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0.4*r,(i+2)*h,-0.4*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2.4)*h,-0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.65*r,(i+2)*h,-0.15*r);
    glTexCoord2f(0,1);glVertex3f(0*r,(i+2.4)*h,-0*r);
    
    computeNormal(1.4*r,i*h,0*r, 1.3*r,(i+1)*h,0*r, 0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0.8*r,(i+2)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2.4)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.65*r,(i+2)*h,0.15*r);
    glTexCoord2f(0,1);glVertex3f(0*r,(i+2.4)*h,0*r);
    
    computeNormal(1*r,i*h,1*r, 0.9*r,(i+1)*h,0.9*r, 0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0.4*r,(i+2)*h,0.4*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2.4)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(0.15*r,(i+2)*h,0.65*r);
    glTexCoord2f(0,1);glTexCoord2f(0,1);glVertex3f(0*r,(i+2.4)*h,0*r);
    
    computeNormal(0*r,i*h,1.4*r, 0*r,(i+1)*h,1.3*r, -0.35*r,i*h,0.85*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(0*r,(i+2)*h,0.8*r);
    glTexCoord2f(1,0);glVertex3f(0*r,(i+2.4)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.15*r,(i+2)*h,0.65*r);
    glTexCoord2f(0,1);glVertex3f(-0*r,(i+2.4)*h,0*r);
    
    computeNormal(-1*r,i*h,1*r, -0.9*r,(i+1)*h,0.9*r, -0.85*r,i*h,0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.4*r,(i+2)*h,0.4*r);
    glTexCoord2f(1,0);glVertex3f(-0*r,(i+2.4)*h,0*r);
    
    glColor3f(0,0.3,0);
    glTexCoord2f(1,1);glVertex3f(-0.65*r,(i+2)*h,0.15*r);
    glTexCoord2f(0,1);glVertex3f(-0*r,(i+2.4)*h,0*r);
    
    computeNormal(-1.4*r,i*h,0*r, -1.3*r,(i+1)*h,0*r, -0.85*r,i*h,-0.35*r, &x,&y,&z);
    glNormal3f(x,y,z);
    
    glColor3f(0,0.5,0);
    glTexCoord2f(0,0);glVertex3f(-0.8*r,(i+2)*h,0*r);
    glTexCoord2f(1,0);glVertex3f(-0*r,(i+2.4)*h,0*r);
    
    glEnd();
    
    
    glDisable(GL_TEXTURE_2D);


    // float Emission1[] = {0.25,0.25,0.25,1};
    // float Emission2[] = {0,0,0,1};
    // glMaterialfv(GL_FRONT,GL_EMISSION,Emission1);
    //double ran = (rand() / (double)RAND_MAX);
    glNormal3f(1,1,1);


    for(i = 0; i<l; i+=2){
        // Draw spines
        glColor3f(0.5,0.5,0.5);
        if(i > 0){
            glBegin(GL_LINES);
            glVertex3f(-1.4*r,i*h,0*r);
            glVertex3f(-1.5*r,i*h,0.1*r);
            
            glVertex3f(-1*r,i*h,-1*r);
            glVertex3f(-1.1*r,i*h,-1.1*r);
            
            glVertex3f(0*r,i*h,-1.4*r);
            glVertex3f(0.1*r,i*h,-1.5*r);
            
            glVertex3f(1*r,i*h,-1*r);
            glVertex3f(1.1*r,i*h,-1.1*r);
            
            glVertex3f(1.4*r,i*h,0*r);
            glVertex3f(1.5*r,i*h,0.1*r);
            
            glVertex3f(1*r,i*h,1*r);
            glVertex3f(1.1*r,i*h,1.1*r);
            
            glVertex3f(0*r,i*h,1.4*r);
            glVertex3f(0.1*r,i*h,1.5*r);
            
            glVertex3f(-1*r,i*h,1*r);
            glVertex3f(-1.1*r,i*h,1.1*r);
            
            glEnd();
        }

        // Draw more spines
        glBegin(GL_LINES);
        
        glVertex3f(-1.3*r,(i+1)*h,0*r);
        glVertex3f(-1.4*r,(i+1)*h,0*r);
        
        glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
        glVertex3f(-1*r,(i+1)*h,-1*r);
        
        glVertex3f(0*r,(i+1)*h,-1.3*r);
        glVertex3f(0*r,(i+1)*h,-1.4*r);
        
        glVertex3f(0.9*r,(i+1)*h,-0.9*r);
        glVertex3f(1*r,(i+1)*h,-1*r);
        
        glVertex3f(1.3*r,(i+1)*h,0*r);
        glVertex3f(1.4*r,(i+1)*h,0*r);
        
        glVertex3f(0.9*r,(i+1)*h,0.9*r);
        glVertex3f(1*r,(i+1)*h,1*r);
        
        glVertex3f(0*r,(i+1)*h,1.3*r);
        glVertex3f(0*r,(i+1)*h,1.4*r);
        
        glVertex3f(-0.9*r,(i+1)*h,0.9*r);
        glVertex3f(-1*r,(i+1)*h,1*r);
        
        glEnd();
    }

        // Add spines
    glBegin(GL_LINES);
    
    glVertex3f(-1.4*r,i*h,0*r);
    glVertex3f(-1.5*r,(i+0.05)*h,0.1*r);
    
    glVertex3f(-1*r,i*h,-1*r);
    glVertex3f(-1.1*r,(i+0.05)*h,-1.1*r);
    
    glVertex3f(0*r,i*h,-1.4*r);
    glVertex3f(0.1*r,(i+0.05)*h,-1.5*r);
    
    glVertex3f(1*r,i*h,-1*r);
    glVertex3f(1.1*r,(i+0.05)*h,-1.1*r);
    
    glVertex3f(1.4*r,i*h,0*r);
    glVertex3f(1.5*r,(i+0.05)*h,0.1*r);
    
    glVertex3f(1*r,i*h,1*r);
    glVertex3f(1.1*r,(i+0.05)*h,1.1*r);
    
    glVertex3f(0*r,i*h,1.4*r);
    glVertex3f(0.1*r,(i+0.05)*h,1.5*r);
    
    glVertex3f(-1*r,i*h,1*r);
    glVertex3f(-1.1*r,(i+0.05)*h,1.1*r);
    
    glEnd();

        // Add more spines
    glBegin(GL_LINES);
    
    glVertex3f(-1.3*r,(i+1)*h,0*r);
    glVertex3f(-1.4*r,(i+1.15)*h,0.1*r);
    
    glVertex3f(-0.9*r,(i+1)*h,-0.9*r);
    glVertex3f(-1*r,(i+1.15)*h,-1*r);
    
    glVertex3f(0*r,(i+1)*h,-1.3*r);
    glVertex3f(0.1*r,(i+1.15)*h,-1.4*r);
    
    glVertex3f(0.9*r,(i+1)*h,-0.9*r);
    glVertex3f(1*r,(i+1.15)*h,-1*r);
    
    glVertex3f(1.3*r,(i+1)*h,0*r);
    glVertex3f(1.4*r,(i+1.15)*h,0.1*r);
    
    glVertex3f(0.9*r,(i+1)*h,0.9*r);
    glVertex3f(1*r,(i+1.15)*h,1*r);
    
    glVertex3f(0*r,(i+1)*h,1.3*r);
    glVertex3f(0.1*r,(i+1.15)*h,1.4*r);
    
    glVertex3f(-0.9*r,(i+1)*h,0.9*r);
    glVertex3f(-1*r,(i+1.15)*h,1*r);
    
    glEnd();

    // Add some more spines
    glBegin(GL_LINES);
    
    glVertex3f(-0.8*r,(i+2)*h,0*r);
    glVertex3f(-0.9*r,(i+2.25)*h,0.1*r);
    
    glVertex3f(-0.4*r,(i+2)*h,-0.4*r);
    glVertex3f(-0.5*r,(i+2.25)*h,-0.5*r);
    
    glVertex3f(0*r,(i+2)*h,-0.8*r);
    glVertex3f(0.1*r,(i+2.25)*h,-0.9*r);
    
    glVertex3f(0.4*r,(i+2)*h,-0.4*r);
    glVertex3f(0.5*r,(i+2.25)*h,-0.5*r);
    
    glVertex3f(0.8*r,(i+2)*h,0*r);
    glVertex3f(0.9*r,(i+2.25)*h,0.1*r);
    
    glVertex3f(0.4*r,(i+2)*h,0.4*r);
    glVertex3f(0.5*r,(i+2.25)*h,0.5*r);
    
    glVertex3f(0*r,(i+2)*h,0.8*r);
    glVertex3f(0.1*r,(i+2.25)*h,0.9*r);
    
    glVertex3f(-0.4*r,(i+2)*h,0.4*r);
    glVertex3f(-0.5*r,(i+2.25)*h,0.5*r);
    
    glEnd();
    
    // Add spines at the very top of the cactus
    glBegin(GL_LINES);
    
    glVertex3f(-0*r,(i+2.4)*h,0*r);
    glVertex3f(-0.1*r,(i+2.6)*h,0.1*r);
    
    glVertex3f(-0*r,(i+2.4)*h,-0*r);
    glVertex3f(-0.1*r,(i+2.6)*h,-0.1*r);
    
    glVertex3f(0*r,(i+2.4)*h,-0*r);
    glVertex3f(0.1*r,(i+2.6)*h,-0.1*r);
    
    glVertex3f(0*r,(i+2.4)*h,-0*r);
    glVertex3f(0.1*r,(i+2.6)*h,-0.1*r);
    
    glVertex3f(0*r,(i+2.4)*h,0*r);
    glVertex3f(0.1*r,(i+2.6)*h,0.1*r);
    
    glVertex3f(0*r,(i+2.4)*h,0*r);
    glVertex3f(0.1*r,(i+2.6)*h,0.1*r);
    
    glVertex3f(0*r,(i+2.4)*h,0*r);
    glVertex3f(0.1*r,(i+2.6)*h,0.1*r);
    
    glVertex3f(-0*r,(i+2.4)*h,0*r);
    glVertex3f(-0.1*r,(i+2.6)*h,0.1*r);
    
    glEnd();
}

// Draw a cluster of floating cactai, sand buildings and drifters
// Drifter 1 rotation angle (dr1)
// Drifter 2 rotation angle (dr2)
void cluster(float dr1, float dr2){

    // // Draw building 1
    glPushMatrix();
    glTranslatef(0,-25,0);
    drawFloatingBase(10,10,90);
    drawBuilding(0.5, 2.5, 20);
    glPopMatrix();
 
    // Draw building 2
    glPushMatrix();
    glTranslatef(-3,-25,-3);
    drawBuilding(0.5, 2, 10);
    glPopMatrix();
    
    // Draw building 3
    glPushMatrix();
    glTranslatef(3,-25,3);
    drawBuilding(0.5, 2, 5);
    glPopMatrix();
    
    // Draw building 4
    glPushMatrix();
    glTranslatef(3,-25,-3);
    drawBuilding(0.5, 2, 7);
    glPopMatrix();
    
    // Draw building 5
    glPushMatrix();
    glTranslatef(-3,-25,3);
    drawBuilding(0.5, 2, 3);
    glPopMatrix();

    // Draw empty base
    glPushMatrix();
    glTranslatef(-15,-30,10);
    drawFloatingBase(2,3,120);
    glPopMatrix();

    // Draw empty base
    glPushMatrix();
    glTranslatef(-20,-33,10);
    drawFloatingBase(3,4,120);
    glPopMatrix();

    // Draw cactus 1
    glPushMatrix();
    glTranslatef(12,-28,12);
    drawFloatingBase(3,4,90);
    drawCactus(1, 1, 18, 0);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(14.75,-24,12);
    glRotatef(45,0,1,0);
    drawCactus(0.5, 0.75, 14, 1);
    glPopMatrix();
    
    // Draw drifter 1
    drawDrifter(-16,-5,-12,2.5,dr1,1,dr1);
    
    // Draw cactus 2
    glPushMatrix();
    glTranslatef(-16,-20,-12);
    drawFloatingBase(6,7,0);
    drawCactus(1, 3, 2, 0);
    glPopMatrix();
    
    // Draw cactus 3
    glPushMatrix();
    glTranslatef(5,-19,-20);
    drawFloatingBase(3,5,0);
    drawCactus(1, 1.5, 4, 0);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(5,-15,-23);
    glRotatef(140,0,1,0);
    drawCactus(0.5, 0.5, 14, 1);
    glPopMatrix();
    
    // Draw cactus 4
    glPushMatrix();
    glTranslatef(14,-5,-5);
    drawFloatingBase(5,10,160);
    drawCactus(0.75, 2, 16, 0);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(18.5,0,-5);
    glRotatef(40,0,1,0);
    drawCactus(0.35, 1, 14, 1);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(11,8,-5);
    glRotatef(230,0,1,0);
    drawCactus(0.3, 0.75, 8, 1);
    glPopMatrix();   
    
    // Draw cactus 5
    glPushMatrix();
    glTranslatef(0,0,-20);
    drawFloatingBase(7,7,120);
    drawCactus(0.5, 2, 2, 0);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(0,2,-20);
    drawCactus(0.75, 1, 2, 0);
    glPopMatrix();
    
    // Draw branch
    glPushMatrix();
    glTranslatef(3,0,-23);
    drawCactus(0.5, 1, 2, 0);
    glPopMatrix();

    // Draw drifter 2
    drawDrifter(-5,22.5,20,2.5,dr2,2,dr2);
    
    // Draw cactus 6
    glPushMatrix();
    glTranslatef(-5,-35,20);
    drawFloatingBase(6,8,120);
    drawCactus(1, 3, 10, 0);
    glPopMatrix();
}

/*
* OpenGL (GLUT) calls this routine to display the scene
* Source: Example code from class
*/
void display(){
    // Erase the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    // Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
    // Undo previous transformations
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(xPos, yPos, zPos, xPos+directionX, yPos+directionY, zPos+directionZ, 0, 0.5, 0);    
    // Smooth shading
    glShadeModel(GL_SMOOTH);
    // Light switch
    if (light)
    {
        // Translate intensity to color vectors
        float Ambient[]  = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
        float Diffuse[]  = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
        float Specular[] = {0.0,0.0,0.0,1.0};
        // Light position
        float Position[] = {distance*Cos(zh),ylight*Sin(zh*3)*2,distance*Sin(zh),1.0};
        // Draw light position as ball (still no lighting here)
        glColor3f(1,1,1);
        ball(Position[0],Position[1],Position[2],0.5,0.9,1,0.4);
        // OpenGL should normalize normal vectors
        glEnable(GL_NORMALIZE);
        // Enable lighting
        glEnable(GL_LIGHTING);
        // Location of viewer for specular calculations
        glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
        // glColor sets ambient and diffuse color materials
        glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
        // Enable light 0
        glEnable(GL_LIGHT0);
        // Set ambient, diffuse, specular components and position of light 0
        glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
        glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
        glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
        glLightfv(GL_LIGHT0,GL_POSITION,Position);
    }
    else
        glDisable(GL_LIGHTING);
    
    // Draw the world
    glPushMatrix();
    glRotated(Sin(fmod(time,360.0)/5)*360,0,1,0);

    // Draw The Thinker
    glColor3f(1,1,1);
    glPushMatrix();
    glTranslated(0,10,0);
    glRotated(Sin(fmod(10*time,360.0))*180,0,1,0);
    glRotated(-5,1,0,0);
    drawFloatingBase(4,5,30);
    glScaled(2,2,2);
    glCallList(obj);
    glPopMatrix();

    // Draw the star sphere
    drawStars();
 
    // Draw cluster 1
    cluster(fmod(120*time,360.0),fmod(90*time,360.0));
    
    // Draw cluster 2
    glPushMatrix();
    glTranslated(-35,35,0);
    glRotated(90,0,1,0);
    cluster(fmod(-120*time,360.0),fmod(-90*time,360.0));
    glPopMatrix();
    
    // Draw cluster 3
    glPushMatrix();
    glTranslated(35,55,35);
    glRotated(180,0,1,0);
    cluster(fmod(130*time,360.0),fmod(80*time,360.0));
    glPopMatrix();

    // Draw cluster 4
    glPushMatrix();
    glTranslated(-35,-20,35);
    glRotated(270,0,1,0);
    cluster(fmod(-130*time,360.0),fmod(-80*time,360.0));
    glPopMatrix();

    // Draw cluster 5
    glPushMatrix();
    glTranslated(35,55,-35);
    glRotated(45,0,1,0);
    cluster(fmod(100*time,360.0),fmod(70*time,360.0));
    glPopMatrix();
 
    // Draw cluster 6
    glPushMatrix();
    glTranslated(0,-50,0);
    glRotated(130,0,1,0);
    cluster(fmod(120*time,360.0),fmod(-90*time,360.0));
    glPopMatrix();

    glPopMatrix();

    glDisable(GL_LIGHTING);
    //  Display control
    
    glWindowPos2i(5,5);
    if(first_time){
        Print("[!] Press spacebar to see keyboard controls.");
    }
    //Print("aX=%.1f|aY=%.1f|dX=%.1f|dY=%.1f|dZ=%.1f|Xp=%.1f|Yp=%.1f|Zp=%.1f|fov=%d",angleX,angleY,directionX,directionY,directionZ,xPos,yPos,zPos,fov);
    if(displayControls == 1){
        glWindowPos2i(5,150);
        Print("Show/hide keyboard control: spacebar");
        glWindowPos2i(5,130);
        Print("Preset locations: 1,2,3,4,5");
        glWindowPos2i(5,110);
        Print("Move: w / s | Strafe: a / d");
        glWindowPos2i(5,90);
        Print("Turn: left / right arrow");
        glWindowPos2i(5,70);
        Print("Move up / down: up / down arrow");
        glWindowPos2i(5,50);
        Print("Pause: p | Change fov: - / +");         
        glWindowPos2i(5,30);
        Print("Light: l | Change light elevation: [ ]");
        glWindowPos2i(5,10);
        Print("Ambient light: z / x | Diffuse light: c / v");
    }
    
    // Render the scene and make it visible
    ErrCheck("display");
    glFlush();
    glutSwapBuffers();
}

/*
* GLUT calls this routine when it's idle
* Source: Example code from class
*/
void idle(){
    // Elapsed time in seconds
    time = glutGet(GLUT_ELAPSED_TIME)/1000.0;
    zh = fmod(45*time,360.0);
    if(bang==1){
        if(fov>50){
            expansion /= 1.14;            
            fov -= expansion;
            if(expansion < 0.00001){
              bang = 0;
              fov = 50;  
            } 
        }
        if(fov<=50 && bang==1){
            fov = 50;
            bang = 0;
        }            
    }
    if(bang==1){
        Project(fov,asp,dim);
    }
    // Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
* GLUT calls this routine when an arrow key is pressed
* Source: Example code from class
*/
void special(int key,int x,int y){
    // Right arrow key - increase angle by 5 degrees
    if (key == GLUT_KEY_RIGHT){
        angleY += 0.05f;
        directionX = sin(angleY);
        directionZ = -cos(angleY);
    } 
    // Left arrow key - decrease angle by 5 degrees
    else if (key == GLUT_KEY_LEFT){
        angleY -= 0.05f;
        directionX = sin(angleY);
        directionZ = -cos(angleY);
    }   
    // Up arrow key - increase elevation by 5 degrees
    else if (key == GLUT_KEY_UP){
        yPos += 1;
    }
    // Down arrow key - decrease elevation by 5 degrees
    else if (key == GLUT_KEY_DOWN){
        yPos -= 1;
    }
    glutPostRedisplay();
}

/*
* GLUT calls this routine when a key is pressed
* Source: Example code from class
*/
void key(unsigned char ch,int x,int y){

    // Exit on ESC
    if (ch == 27){
        exit(0);
    }
    else if (ch == 'a' || ch == 'a'){
        double a = angleY - M_PI/2;
        double dX, dZ;
        dX = sin(a);
        dZ = -cos(a);
        xPos += dX;
        zPos += dZ;
    }
    else if(ch == 'd' || ch == 'd'){
        double a = angleY + M_PI/2;
        double dX, dZ;
        dX = sin(a);
        dZ = -cos(a);
        xPos += dX;
        zPos += dZ;
    }
    else if(ch == 'w' || ch == 'W'){
        if(sqrt((zPos*zPos)+(xPos*xPos))>100){
            xPos += directionX * 3;
            zPos += directionZ * 3;
            yPos += directionY * 3;    
        }
        else{
            xPos += directionX * 1;
            zPos += directionZ * 1;
            yPos += directionY * 1;
        } 
    }
    else if(ch == 's' || ch == 'S'){
        if(sqrt((zPos*zPos)+(xPos*xPos))>100){
            xPos -= directionX * 3;
            zPos -= directionZ * 3;
            yPos -= directionY * 3;    
        }
        else{
            xPos -= directionX * 1;
            zPos -= directionZ * 1;
            yPos -= directionY * 1;
        }
    }
    // Preset view location 1
    else if (ch == '1'){
        directionX=0;
        directionY=0;
        directionZ=-1;
        angleX = 0.0;
        angleY = 0.0;        
        xPos = 0;
        yPos = 0;
        zPos = 242;        
        fov = 50;
    }
    // Preset view location 2
    else if (ch == '2'){
        directionX=0;
        directionY=0;
        directionZ=-1;
        angleX = 0.0;
        angleY = 0.0;        
        xPos = 0;
        yPos = 0;
        zPos = 92;        
        fov = 50;
    }
    // Preset view location 3
    else if (ch == '3'){
        directionX=0;
        directionY=0;
        directionZ=-1;
        angleX = 0.0;
        angleY = 0.0;        
        xPos = 0;
        yPos = -22;
        zPos = 18.5;        
        fov = 61;
    }
    // Preset view location 4
    else if (ch == '4'){
        directionX=0;
        directionY=0;
        directionZ=-1;
        angleX = 0.0;
        angleY = 0.0;        
        xPos = 0;
        yPos = -49;
        zPos = 38.5;        
        fov = 61;
    }
    // Preset view location 5
    else if (ch == '5'){
        directionX=0;
        directionY=0;
        directionZ=-1;
        angleX = 0.0;
        angleY = 0.0;        
        xPos = 0;
        yPos = 13;
        zPos = 15.1;        
        fov = 58;
    }
    else if(ch == 32){
        if(first_time){
            first_time = 0;
        }
        if(displayControls)displayControls = 0;
        else displayControls = 1;
    }
    // Toggle lighting
    else if (ch == 'l' || ch == 'L')
    light = 1-light;
    // Toggle light movement
    else if (ch == 'p' || ch == 'P')
    move = 1-move;
    // Change field of view angle
    else if (ch == '-' && ch > 1)
    fov--;
    else if (ch == '+' && ch < 179)
    fov++;
    // Light elevation
    else if (ch=='[')
    ylight -= 1;
    else if (ch==']')
    ylight += 1;
    // Ambient level
    else if ((ch=='x' || ch=='X') && ambient > 0)
    ambient -= 5;
    else if ((ch=='z' || ch=='Z') && ambient < 100)
    ambient += 5;
    // Diffuse level
    else if ((ch=='c' || ch=='C') && diffuse > 0)
    diffuse -= 5;
    else if ((ch=='v' || ch=='V') && diffuse < 100)
    diffuse += 5;

    Project(fov,asp,dim);
    // Animate if requested
    glutIdleFunc(move?idle:NULL);
    // Tell GLUT it is necessary to redisplay the scene
    glutPostRedisplay();
}

/*
* GLUT calls this routine when the window is resized
* Source: Example code from class
*/
void reshape(int width,int height)
{
    // Ratio of the width to the height of the window
    asp = (height > 0) ? (double)width/height : 1;
    // Set the viewport to the entire window
    glViewport(0,0, width,height);
    // Set projection
    Project(fov,asp,dim);
}

/*
* Start up GLUT and tell it what to do
* Source: Example code from class
*/
int main(int argc,char* argv[])
{
    int i;
    directionX = sin(angleY);
    directionZ = -cos(angleY);
    // Generate and store some random numbers for drawing stars
    for(i = 0; i < 3000; i++){
        stars[i][0] = 360 * (rand() / (double)RAND_MAX);
        stars[i][1] = 360 * (rand() / (double)RAND_MAX);
    }
    // Initialize GLUT
    glutInit(&argc,argv);
    // Request double buffered, true color window with Z buffering at 600x600
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    //glutInitWindowSize(800,500);
    glutCreateWindow("Introspection");
    glutFullScreen();
    // Set callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutSpecialFunc(special);
    glutKeyboardFunc(key);
    
    glutIdleFunc(idle);
    // Load texture
    texture[0] = LoadTexBMP("sand.bmp");
   
    Mix_Music* music;
    //  Initialize audio
    if (Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096)) Fatal("Cannot initialize audio\n");
    //  Load music
    music = Mix_LoadMUS("sayuri.ogg");
    if (!music) Fatal("Cannot load thewall.ogg\n");
    //  Play (looping)
    Mix_PlayMusic(music,-1);
    // SDL event loop
    obj = LoadOBJ("theThinker.obj");
    ErrCheck("init");
    glutMainLoop();
    //  Shut down SDL
    Mix_CloseAudio();
    ErrCheck("init");
    return 0;
}
