#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(int argc, char *argv[]) {
  int INIT_TIME = 50;
  int width, height;
  double B_PARAM = 1.0 / 50.0;
  double T_PARAM = 1.0 / 200.0;
  double Zeta = 10.0;

  int blurvalue = 19;
  double bokehthreshold = 0.8;
  int bokehthresholdint = 253;
  double bokehthreshold2;
  int bokehthresholdint2 = 248;
  int dilation_size = 10;
  int dilation_size2 = 7;

  VideoCapture cap;
  Mat frame, result, baseblur, highlights, bokeh, mediums;
  Mat avg_img, sqm_img;
  Mat lower_img, upper_img, tmp_img;
  Mat dst_img, msk_img;

  if (argc >= 2) {
    cap.open(argv[1]);
  }else{
    cap.open(0);
  }
  if (!cap.isOpened()){
    printf("Can not open video.\n");
    exit(0);
  }

  cap >> frame;
  frame = imread("yakei.jpg");

  Size s = frame.size();

  //namedWindow("Input", 1);
  //namedWindow("FG", 1);
  namedWindow("Result",1);
  namedWindow("Controls",1);
  createTrackbar("Blur size", "Controls", &blurvalue, 40, 0);
  createTrackbar("Bokeh Threshold", "Controls", &bokehthresholdint, 255, 0);
  createTrackbar("Medium bokeh threshold", "Controls", &bokehthresholdint2, 255, 0 );
  createTrackbar("Bokeh Size", "Controls", &dilation_size, 50, 0);
  createTrackbar("Medium bokeh size", "Controls", &dilation_size2, 50, 0);
  //namedWindow("Bokeh", 1);

  bool loop_flag = true;
  while (loop_flag) {
    //cap>>frame;
    //if (frame.empty()) break;
    
    // BEGIN FILTER

    frame.convertTo(tmp_img, tmp_img.type());
    
    //GaussianBlur(frame, tmp_img, Size(0,0), 5, 5, 0);

    medianBlur(frame, baseblur, (blurvalue / 2) * 2 + 1);
    //brightness range
    baseblur.copyTo(result);
    
    Mat temp;
    //convert to grayscale for thresholding
    cvtColor(frame, temp, CV_BGR2YCrCb);
    vector<Mat> planes;
    split(temp, planes);
    Mat bnw = planes[0];

    bokehthreshold = bokehthresholdint;
    threshold(bnw, highlights, bokehthreshold, 0.5, THRESH_TOZERO);//take only highlights
    bokehthreshold2 = bokehthresholdint2;
    threshold(bnw, mediums, bokehthreshold2, 0.5, THRESH_TOZERO);
    threshold(mediums, mediums, bokehthreshold, 0.5, THRESH_TOZERO_INV);//also shave off too bright ones
    equalizeHist(highlights, highlights);
    equalizeHist(mediums, mediums);

    Mat dilationelement =
getStructuringElement(MORPH_ELLIPSE,
		      Size(2*dilation_size + 1, 2* dilation_size + 1 ),
                      Point( dilation_size, dilation_size) );
      
    dilate( highlights, temp, dilationelement);
    
    dilationelement = getStructuringElement(MORPH_ELLIPSE,
		      Size(2*dilation_size2 + 1, 2* dilation_size2 + 1 ),
		       Point( dilation_size2, dilation_size2) );
    dilate (mediums, mediums, dilationelement);
    
    //imshow("Bokeh", temp);
    
    Mat bokeh;
    temp.convertTo(bokeh, frame.type());
    
    //convert grayscale bokeh image to RGB:
    vector<Mat> channels;
    channels.push_back(temp);    
    channels.push_back(temp);
    channels.push_back(temp);
    merge(channels, bokeh);
    vector<Mat> channels2;
    channels2.push_back(mediums); 
    channels2.push_back(mediums);
    channels2.push_back(mediums);
    Mat mediumbokeh;
    merge(channels2, mediumbokeh);
    //RGB conversion over
    
    //blur bokeh a little bit:
    GaussianBlur(mediumbokeh, mediumbokeh, Size(0,0), 3, 3, 0);
    blur(bokeh, bokeh, Size(2,2));
    /*******************************

    NOTE FOR NEXT TIME:
    for the highligh blur, and maybe for medium blur as well,
make a loop that goes through a number of brightness values. For example
100% brightness to 98%, then 97% to 96% and so on, and apply seperate bokeh
effect on all. This way bokeh circles dont merge as much.
Maybe even add seperate sprites for the absolute white case only.

make the granularity and number of passes controllable.

    ******************************/ 


    //add(baseblur, bokeh, result);
    
    addWeighted(mediumbokeh, 0.3, baseblur, 1.0, 0.0, temp, -1);
    addWeighted(bokeh, 0.5, temp, 0.9, 0.0, result, -1);
    

    imshow("Result", result);
      
    char key = waitKey(10);
    if(key == 27) {
      loop_flag = false;
    }
  }
  return 0;
}
