//Akaki Kuumeri

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<GL/glut.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WINDOW_NAME "test3"
#define astcount 100
#define SW 600
#define SH 500
#define NEAR 3.0
#define FAR 30.0
#define TEXHEIGHT 650
#define TEXWIDTH 650
GLuint TextureHandle[3];

using namespace cv;
using namespace std;

void OnIdle(void);
void init(void);
void glut_display(void);
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void draw_pyramid(void);
void draw_orbit(double r);
void set_texture(void);

double Angle1 = 3.0;
double Angle2 =0.1;
double Distance = 10.0;
bool LeftButtonOn = false;
bool RightButtonOn = false;
double mytime = 500.0;
double timerate = 0.6;
double astangle1[astcount];
double astangle2[astcount];
double astangle3[astcount];
double astr[astcount];
double astspeed[astcount];
double aststart[astcount];
double astsize[astcount];

Mat frame, baseblur, highlights, bokeh, mediums, temp;

int blurvalue = 20;
int dilation_size = 3;
int dilation_size2 = 3;
int granularity = 3;
int opacity = 10;
int overlap = 13;//how much to take from the previous z slice.
int focus = 8000;
int bokehthreshold=150;
  
int no_passes = 8;

GLUquadricObj *Ring;

Mat flipped;
Mat img(SH, SW, CV_8UC3);
Mat depth(SH, SW, CV_32F);
Mat result(SH, SW, CV_8UC3);
Mat depthtemp(SH, SW, CV_32F);

int main (int argc, char *argv[])
{
  namedWindow("Controls",1);
    createTrackbar("Focus", "Controls", &focus, 10000, 0);
  createTrackbar("Blur size", "Controls", &blurvalue, 40, 0);
  createTrackbar("Granularity", "Controls", &granularity, 20, 0 );
  createTrackbar("Depth overlap", "Controls", &overlap, 100, 0);
  createTrackbar("Number of Passes", "Controls", &no_passes, 20, 0);
    createTrackbar("Bokeh Threshold", "Controls", &bokehthreshold, 300, 0);
  //namedWindow("Bokeh", 1);
  
  
  for (int i = 0; i < astcount; i++){
    astangle1[i] = (rand() % 50)*0.1 - 0.05;
    astangle2[i] = (rand() % 50)*0.1 - 0.05;
    astangle3[i] = rand() % 360;
    astr[i] = (rand()%40)*0.01 + 3.7;
    astspeed[i] = astr[i] / 8.0;
    aststart[i] = rand() % 360;
    astsize[i] = (rand()%10)*0.002;
  }
  Ring = gluNewQuadric();
  gluQuadricDrawStyle(Ring, GLU_FILL);
  gluQuadricOrientation( Ring,GLU_OUTSIDE);




  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);

  
  
  glutInitWindowSize(SW,SH);
  glutCreateWindow(WINDOW_NAME);
  //glutCreateWindow("Hello");
  init();
  
  glutIdleFunc(OnIdle);
  glutDisplayFunc(glut_display);
  glutKeyboardFunc(glut_keyboard);
  glutMouseFunc(glut_mouse);
  glutMotionFunc(glut_motion);
  glutPassiveMotionFunc(glut_motion);

  glutMainLoop();

  return 0;
}

void OnIdle(void){
    static int counter = 0;
    
    if (counter ==0){
        glBindTexture(GL_TEXTURE_2D, TextureHandle[0]);
    }else if(counter == 1000){
        glBindTexture(GL_TEXTURE_2D, TextureHandle[1]);
    }else if(counter == 2000){
        glBindTexture(GL_TEXTURE_2D, TextureHandle[2]);
    }
    
    counter++;
    if (counter > 3000) counter = 0;

    
    
  mytime += timerate;
  glutPostRedisplay();

  //get image:
  glPixelStorei(GL_PACK_ALIGNMENT, (img.step & 3)?1:4);
  glPixelStorei(GL_PACK_ROW_LENGTH, img.step/img.elemSize());
  glReadPixels(0, 0, img.cols, img.rows, GL_BGR_EXT, GL_UNSIGNED_BYTE, img.data);
  //get depth buffer:
  glPixelStorei(GL_PACK_ALIGNMENT, (depth.step & 3)?1:4);
  glPixelStorei(GL_PACK_ROW_LENGTH, depth.step/depth.elemSize());
  glReadPixels(0, 0, depth.cols, depth.rows, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data);
  //cv::Mat flipped(img);
  flip(img, img, 0);
  flip(depth, depth, 0);
  //imwrite("snapshot.png", img);
  img.convertTo(temp, CV_8UC3);
  depth.convertTo(depthtemp, CV_32F);
  
  //Mat thresholdresult(depthtemp);
  //threshold(depthtemp, thresholdresult, bokehthresholdint/10000.0, 1, THRESH_TOZERO);
  //imshow("thresholdresult",thresholdresult);
  //Mat tempmask;
Mat blurs;
blurs = Mat::zeros(SH,SW,CV_8UC3);	
  result = Mat::zeros(SH, SW, CV_8UC3);
//img.copyTo(result);	
  //thresholdresult.convertTo(tempmask, CV_8UC1);
  //temp.copyTo(result, tempmask);
  //boxFilter(temp, baseblur, -1, Size(blurvalue + 1, blurvalue + 1));
  
    
    //convert to grayscale
    Mat bnw1channel, bnwtemp;
    cvtColor(temp, bnwtemp, CV_BGR2YCrCb);
    vector<Mat> planes;
    split(temp, planes);
    bnw1channel = planes[0];
    
    double thrhigh= 1.0;
  double thrlow = 1.0;
    //FAR FOCUS:

  for (int i = 0; i < no_passes; i++) {
    //go through each pass and threshold out a slice
    thrhigh = thrlow + (overlap-10)/10000.0;;//take the previous low
      thrlow -= (thrlow - focus/10000.0)/granularity;//bring the bottom down, progressively slower
    Mat thresholdresult;
    thresholdresult = Mat::ones(SH, SW, CV_8UC1);
    threshold(depthtemp, thresholdresult, thrlow, 1, THRESH_TOZERO); //drop evrything below to zero
    threshold(thresholdresult, thresholdresult, thrhigh, 1, THRESH_TOZERO_INV); //drop evrything above to zero
    //equalizeHist(thresholdresult, thresholdresult);
    //use thresh as mask and copy only that portion of image:
    Mat tempmask;
    highlights = Mat::zeros(SH, SW, CV_8UC3);
    thresholdresult.convertTo(tempmask, CV_8UC1);
    temp.copyTo(highlights, tempmask);
      

    int blursize = 1.0*(no_passes-i)/no_passes*blurvalue+1;
    Mat blurresult;
   
    //blursize = (blursize % 2 == 1) ? blursize : blursize+1;
    boxFilter(highlights, blurresult, -1, Size(blursize, blursize));
    //medianBlur(blurresult, blurresult, blursize);
    //blur(dilationresult, dilationresult, Size(i,i));
    //GaussianBlur(dilationresult, dilationresult, Size(0,0), blursize, blursize, 0);
      
      int dsize = blursize/2;
      Mat dilationelement =
      getStructuringElement(MORPH_ELLIPSE,
                            Size(2*dsize + 1, 2* dsize + 1 ),
                            Point( dsize, dsize) );
      
      Mat dilationresult, dilationmask;// = Mat::zeros(SH, SW, CV_8UC1);
      
      bnw1channel.copyTo(dilationmask, tempmask);
      threshold(dilationmask, thresholdresult, bokehthreshold, 1, THRESH_TOZERO_INV);
      equalizeHist(thresholdresult, thresholdresult);
      if (sum(thresholdresult)[0] > 0) {//if any bokeh is found at all
          Mat dilatables;
          dilatables = Mat::zeros(SH, SW, CV_8UC3);
          highlights.copyTo(dilatables, thresholdresult);
      
          dilate(dilatables, dilationresult, dilationelement);
          if (blursize < 9) blur(dilationresult, dilationresult, Size(3,3));
          add(dilationresult, result, result);
      }
      add(blurresult, result, result);
  }
    {
    //NEXT THE PINTO
    thrlow = thrhigh - (overlap-10)/10000.0;//take the previous top
    thrhigh= overlap/10000.0;//prepare for the for loop
    for (int i = 0; i < no_passes; i++){
        thrhigh += (focus/10000.0 - thrhigh)/granularity;//get the final near focus thresh
    }
    Mat thresholdresult;
    thresholdresult = Mat::ones(SH, SW, CV_8UC1);
    threshold(depthtemp, thresholdresult, thrhigh-(overlap-10)/10000.0, 1, THRESH_TOZERO); //drop evrything below to zero
    threshold(thresholdresult, thresholdresult, thrlow, 1, THRESH_TOZERO_INV); //drop evrything above to zero
    //equalizeHist(thresholdresult, thresholdresult);
    //use thresh as mask and copy only that portion of image:
    Mat tempmask;
    highlights = Mat::zeros(SH, SW, CV_8UC3);
    thresholdresult.convertTo(tempmask, CV_8UC1);
    temp.copyTo(highlights, tempmask);
    
        //no blurring
    
        add(highlights, result, result);
    }
    
    
    //NEXT THE NEAR BLUR
    thrhigh= overlap/10000.0;//start from 0
    
    
    for (int i = 0; i < no_passes; i++) {
        //go through each pass and threshold out a slice
        thrlow = thrhigh - (overlap-10)/10000.0;//take the previous top
        thrhigh += (focus/10000.0 - thrhigh)/granularity;//bring the threshold half the way nearer to the focus
        Mat thresholdresult;
        thresholdresult = Mat::ones(SH, SW, CV_8UC1);
        threshold(depthtemp, thresholdresult, thrlow, 1, THRESH_TOZERO); //drop evrything below to zero
        threshold(thresholdresult, thresholdresult, thrhigh, 1, THRESH_TOZERO_INV); //drop evrything above to zero
        //equalizeHist(thresholdresult, thresholdresult);
        //use thresh as mask and copy only that portion of image:
        Mat tempmask;
        highlights = Mat::zeros(SH, SW, CV_8UC3);
        thresholdresult.convertTo(tempmask, CV_8UC1);
        temp.copyTo(highlights, tempmask);
        
        //Mat dilationresult;
        //dilate(highlights, dilationresult, dilationelement);
        int blursize = 1.0*(no_passes-i)/no_passes*blurvalue+1;
        Mat blurresult;
        //blursize = (blursize % 2 == 1) ? blursize : blursize+1;
        boxFilter(highlights, blurresult, -1, Size(blursize, blursize));
        //blur(dilationresult, dilationresult, Size(i,i));
        //GaussianBlur(dilationresult, dilationresult, Size(0,0), blursize, blursize, 0);
        add(blurresult, result, result);
    }
	//add(result, blurs, result);
  
  imshow("test3", result);
  imshow("z buffer", depth);
  waitKey(1);
  //imshow("Hello", flipped);
  
}


void init(void) {
  glClearColor(0.0,0.0,0.0,0.0);
    
    glGenTextures(3, TextureHandle);
    
    for (int i = 0; i < 3; i++){
        glBindTexture(GL_TEXTURE_2D, TextureHandle[i]);
        
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXWIDTH, TEXHEIGHT,
                     0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    
    set_texture();
}

void glut_keyboard(unsigned char key, int x, int y){
  switch(key){
  case 'q':
    exit(0);
  }
  glutPostRedisplay();

}

void glut_mouse(int button, int state, int x , int y){
  if (button == GLUT_LEFT_BUTTON){
    if (state == GLUT_UP){
      LeftButtonOn = false;
    } else if (state == GLUT_DOWN){
      LeftButtonOn = true;
    }
  }

  if (button == GLUT_RIGHT_BUTTON){
    if (state == GLUT_UP){
      RightButtonOn = false;
    } else if (state == GLUT_DOWN){
      RightButtonOn = true;
    }
  }
}
void glut_motion(int x, int y){
  static int px = -1, py = -1;

  if (LeftButtonOn == true){
    if(px >= 0 && py >= 0){
      Angle1 +=(double) - (x - px) /20;
      Angle2 += (double) (y - py)/20;
    }
    px = x;
    py = y;

  }else if (RightButtonOn){
    if (px >= 0 && py >= 0){
      Distance += (double)(y -py)/20;
    }
    px = x;
    py = y;
  }else{
    px = -1;
    py = -1;
  }

  glutPostRedisplay();
}

void glut_display(void){
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0,1.0 * SW / SH ,NEAR,FAR);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  gluLookAt(Distance*cos(Angle2) * sin(Angle1),
	    Distance * sin(Angle2),
	    Distance * cos(Angle2) * cos(Angle1),
	    0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
    
    glTranslatef(2.0,0.0,0.0);
    
  //sun
  glPushMatrix();
  glColor3d(1.0,1.0,0.2);
  glutSolidSphere(0.7, 30, 20);
  //draw_pyramid();//for reference
  glTranslatef(0.0,-0.7,0.0);
  glScalef(0.3,0.3,0.3);
  draw_pyramid();
  glPopMatrix();

//extra pyramids
  glPushMatrix();
  glScalef(0.3, 0.3, 0.3);
  glTranslated(-100.0, -2.0, -100.0);
  for (int i = 0; i < 50; i ++){
    glTranslatef(4.0,0.0,0.0);
      for (int j=0; j< 50; j++) {
          glTranslatef(0.0, 0.0, 4.0);
          draw_pyramid();
      }
      glTranslatef(0.0, 0.0, -200.0);
  }
  glPopMatrix();

  //venus
  glPushMatrix();
draw_orbit(1.3);
  glColor3d(0.4,0.2,0.7);
  glRotatef(mytime*2.0, 0.0, 1.0, 0.0);
  glTranslatef(1.3, 0.0, 0.0);
  glutSolidSphere(0.03, 15,10);
glPopMatrix();

  //earth
  glPushMatrix();
  draw_orbit(2.0);
  glRotatef(mytime, 0.0, 1.0, 0.0);
  glTranslatef(2.0,0.0,0.0);
  //glRotatef(time, 1.0, 0.0, 1.0);
  glColor3d(0.0,1.0,0.5);
  glutSolidSphere(0.1,15,10);
  //glPopMatrix();
  //moon
  glRotatef(mytime*12, 0.0, 1.0, 0.0);//12 months in a year
  draw_orbit(0.4);
  glTranslatef(0.4, 0.0, 0.0);
  glColor3d(0.2,0.2,0.2);
  glutSolidSphere(0.02, 10,10);
  glPopMatrix();

//mars
  glPushMatrix();
  glRotatef(mytime*0.7, 0.0, 1.0, 0.0);
draw_orbit(3.0);
  glTranslatef(3.0,0.0,0.0);
  glColor3d(8.0,0.0,0.0);
  glutSolidSphere(0.08,15,10);
  //glPopMatrix();
  glPopMatrix();

  //asteroids
  for (int i = 0; i <astcount;i++){
    glPushMatrix();
    glRotatef(astangle1[i], 1.0,0.0,0.0);
    glRotated(astangle2[i], 0.0, 0.0, 1.0);
    glRotatef(mytime * astspeed[i] + aststart[i], 0.0, 1.0, 0.0);
    glTranslatef(astr[i], 0.0, 0.0);
    glColor3d(0.5, 0.3, 0.3);
    glutSolidSphere(astsize[i], 3, 3);
    glPopMatrix();
  }
    //stars
    for (int i = 0; i <astcount*2;i++){
        glPushMatrix();
        glRotatef(astangle1[i]*180.0, 1.0,0.0,0.0);
        glRotated(astangle2[i]*180.0, 0.0, 0.0, 1.0);
        glRotatef(aststart[i], 0.0, 1.0, 0.0);
        glTranslatef(7.0, 0.0, 0.0);
        glColor3d(0.2, 0.2, 0.5);
        glutSolidSphere(0.02, 3, 3);
        glPopMatrix();
    }
  

  //ringed planet
  glPushMatrix();
draw_orbit(5.0);
  glRotatef(mytime*0.3, 0.0, 1.0, 0.0);
  glTranslatef(5.0, 0.0, 0.0);
  glColor3d(0.4, 0.2, 0.3);
  glutSolidSphere(0.3, 20, 15);
  glPushMatrix();
  glColor3d(0.2,0.7, 0.2);
  glRotatef(80, 1.0, 0.0, 0.0);
  gluDisk(Ring, 0.5, 0.65, 30, 2);
  glPopMatrix();//back to the planet, next moons
  glPushMatrix();
  
  glRotatef(40, 1.0, 0.0, 0.0);
  glRotatef(mytime*2, 0.0, 1.0, 0.0);
  draw_orbit(0.4);
  glColor3d(0.4,0.7,0.1);
  glTranslatef(0.4, 0.0, 0.0);
  glutSolidSphere(0.05, 10, 5);
  glPopMatrix();

  glPushMatrix();
  glRotatef(8, 0.0, 0.0, 1.0);
  glRotatef(mytime*1.5, 0.0, 1.0, 0.0);
  draw_orbit(0.9);
  glColor3d(0.9,1.0,0.3);
  glTranslatef(0.9, 0.0, 0.0);
  glutSolidSphere(0.08, 10, 5);
  glPopMatrix();


  //
  // teapot plant
  //
  glPopMatrix();
  glPushMatrix();
 glRotatef(10,0.0,0.0,1.0);
  glRotatef((mytime-300.0)*0.03, 0.0, 1.0, 0.0);
  draw_orbit(8.0);
  glTranslatef(8.0, 0.0, 0.0);
  glColor3d(0.1, 0.4, 0.7);
  glutSolidTeapot(0.17);
  glPushMatrix();
  //moons
  
  glRotatef(50, 0.0, 0.0, 1.0);
  glRotatef(mytime*1.2, 0.0, 1.0, 0.0);
  draw_orbit(0.4);
  glTranslatef(0.4, 0.0, 0.0);
 glColor3d(0.4,0.7,0.1);
  glutSolidSphere(0.03, 10, 5);
  glPopMatrix();

  glPushMatrix();
  
  glRotatef(15, 0.0, 0.0, 1.0);
  glRotatef(mytime*0.9, 0.0, 1.0, 0.0);
  draw_orbit(0.5);
  glTranslatef(0.5, 0.0, 0.0);
 glColor3d(0.9,0.1,0.4);
  glutSolidSphere(0.02, 10, 5);
  glPopMatrix();

  glPopMatrix();

    
    //draw billboad
    glEnable(GL_TEXTURE_2D);
    
    
    GLdouble pointA[] = {-10.0, 3.0,-3.0};
    GLdouble pointB[] = {-10.0, -3.0, -3.0};
    GLdouble pointC[] = {-10.0, -3.0, 3.0};
    GLdouble pointD[] = {-10.0, 3.0, 3.0};
    glBegin(GL_POLYGON);
    glTexCoord2d(1.0,0.0);
    glVertex3dv(pointA);
    glTexCoord2d(0.0,0.0);
    glVertex3dv(pointB);
    glTexCoord2d(0.0,1.0);
    glVertex3dv(pointC);
    glTexCoord2d(1.0,1.0);
    glVertex3dv(pointD);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);

  
  
  /* glPushMatrix();
  glTranslatef(-1.0,3.0,0.0);
  glRotatef(-30, 0.0, 0.0, 1.0);
  glColor3f(1.0,1.0,1.0);
  glutWireTeapot(1.0);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, -2.0, 0.0);
  draw_pyramid();
  glPopMatrix();
  */
  glFlush();
  glDisable(GL_DEPTH_TEST);
  
  glutSwapBuffers();
}


void draw_pyramid(void){
  GLdouble point0[] = {0.0,1.0,0.0};
  GLdouble pointA[] = {1.5,-1.0,1.5};
  GLdouble pointB[] = {-1.5, -1.0, 1.5};
  GLdouble pointC[] = {-1.5, -1.0, -1.5};
  GLdouble pointD[] = {1.5, -1.0, -1.5};

  glColor3d(1.0,0.0,0.0);
  glBegin(GL_TRIANGLES);
  glVertex3dv(point0);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glEnd();

  glColor3d(1.0,1.0,0.0);
  glBegin(GL_TRIANGLES);
  glVertex3dv(point0);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glEnd();

  glColor3d(0.0,1.0,1.0);
  glBegin(GL_TRIANGLES);
  glVertex3dv(point0);
  glVertex3dv(pointC);
  glVertex3dv(pointD);
  glEnd();

  glColor3d(1.0,0.0,1.0);
  glBegin(GL_TRIANGLES);
  glVertex3dv(point0);
  glVertex3dv(pointD);
  glVertex3dv(pointA);
  glEnd();

  glColor3d(1.0,1.0,1.0);
  glBegin(GL_POLYGON);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glVertex3dv(pointD);
  glEnd();
}

void draw_orbit(double r){
   if (LeftButtonOn || RightButtonOn){
     glPushMatrix();
     glRotatef(90, 1.0, 0.0, 0.0);
     glColor3d(1.0,1.0,1.0);
     gluDisk(Ring, r-0.03,r,100,2);
     glPopMatrix();
   }
  }

void set_texture(void) {
    for (int i = 0; i < 3; i++){
        char inputfile[256];
        
        switch (i) {
            case 0:
                sprintf(inputfile, "flower1.jpg");
                break;
            case 1:
                sprintf(inputfile, "flower2.jpg");
                break;
            case 2:
                sprintf(inputfile, "flower3.jpg");
                break;
        }
        
        cv::Mat input;
        input = cv::imread(inputfile, 1);
        glBindTexture(GL_TEXTURE_2D, TextureHandle[i]);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        (TEXWIDTH - input.cols)/2,
                        (TEXHEIGHT - input.rows)/2,
                        input.cols, input.rows,
                        GL_RGB, GL_UNSIGNED_BYTE, input.data);
    }
}
