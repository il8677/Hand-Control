#include <iostream>
#include <opencv2/objdetect.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <thread>
#include <string>
#include <cstring>
#include <signal.h>
#include <stdio.h>
#include <sstream> //for std::stringstream

#include <cmath>

using namespace std;
using namespace cv;
#include "conf.cpp"
#define MAXPOINTS 20

// Computes the bearing in degrees from the point A(a1,a2) to
// the point B(b1,b2). Note that A and B are given in terms of
// screen coordinates.
double bearing(Point a, Point b){ //Adapted from stack overflow
    double b1 = b.x;
    double b2 = b.y;

    double a1 = a.x;
    double a2 = a.y;
    static const double TWOPI = 6.2831853071795865;
    static const double RAD2DEG = 57.2957795130823209;
    // if (a1 = b1 and a2 = b2) throw an error
    double theta = atan2(b1 - a1, a2 - b2);
    if (theta < 0.0)
        theta += TWOPI;
    return RAD2DEG * theta;
}


int keyboard;

struct Line{
    Point * a;
    Point * b;

    bool taken;

    Line(Point* aa, Point* bb){
        a=aa; b=bb;
    }

    float dist(){
        return norm(a-b);
    }
};

bool go = true;
String hand_cascade_name = "hand.xml";
CascadeClassifier hand_cascade = CascadeClassifier(hand_cascade_name);

Mat dispframe;
Mat frame;
void sighandler(int sig)
{
    imwrite("../HANDCONTROL_DEBUG_OUT.jpg", frame);
    exit(1);
}
vector<Point> positions;

vector<Line> lines;
int findClosestPoint(int index, double * theDistance){
    signal(SIGINT, &sighandler);
    int smallest;
    double smallestdist = 10000;
    for( size_t i = index; i < positions.size(); i++){
        double iteratorDist = norm(positions[i] - positions[index]);
        if(index == i) {
            continue;
        };
        if(iteratorDist < smallestdist){
            smallestdist=iteratorDist;
            smallest = i;
        }
    }
    *theDistance =  smallestdist;
    return smallest;
}

bool findChain(){

}

int main(){
    bool first=true;
    int lineClusterIndex = 0;

    vector<Line> superlines;

    namedWindow("main");

    if(!hand_cascade.load( hand_cascade_name )){
         cerr<<"Error loading " << hand_cascade_name<<endl;
         exit(1);
     };

     VideoCapture capture;
     if(!capture.open(0)){
         cerr<<"Camera is ded\n";
         exit(1);
     }
    while((char)keyboard!='q'){
        lines.clear();

        if((char)keyboard == 'c'){
            positions.clear();
            superlines.clear();
            first =true;
            lineClusterIndex = 0;
        }
        capture.read(frame);
        flip(frame, dispframe,1);
        frame = dispframe;

        std::vector<Rect> hands;

        Mat frame_gray;
        Mat flipframe_gray;
        cvtColor( frame, frame_gray, CV_BGR2GRAY );
        equalizeHist( frame_gray, frame_gray );


        hand_cascade.detectMultiScale( frame_gray, hands, 1.1, 2, 0, Size(150, 150) );
        if(doubleCheck){
            flip(frame_gray, flipframe_gray,1);
            std::vector<Rect> flippedHands;
            hand_cascade.detectMultiScale( flipframe_gray, flippedHands, 1.1, 2, 0, Size(150, 150) );

            for(size_t i = 0; i < flippedHands.size(); i++){

            }
        }
        for( size_t i = 0; i < hands.size(); i++ )
        {
            Point right(hands[i].x + hands[i].width, hands[i].y);
            Point left(hands[i].x, hands[i].y+hands[i].height);
            if(drawRectangles){
                rectangle(frame, right, left, Scalar(0,255,255));
            }
            positions.push_back(Point(hands[i].x,hands[i].y));

        }
        for( size_t i = 0; i < positions.size(); i++){//TODO: only execute when new positions are added
            if(drawPoints){
                circle(frame, positions[i], 2,Scalar(0,255,0),2);
            }
            if(drawPointMemorySlot){
                Point * address = &positions[i];
                std::stringstream ss;
                ss << address;
                putText(dispframe, ss.str(), Point(positions[i].x,positions[i].y+10),FONT_HERSHEY_COMPLEX_SMALL,0.8, cvScalar(0,0,250), 1);
            }
            #define DISTANCETHRESHOLD 200
            double closestPointDistance;
            int closestPointIndex = findClosestPoint(i, &closestPointDistance);
            if(closestPointDistance <= DISTANCETHRESHOLD){
                lines.push_back(Line(&positions[i],&positions[closestPointIndex]));


                if(drawLines){
                    arrowedLine(dispframe, *lines.back().a, *lines.back().b, Scalar(255,0,0));
                }
            }
        }



        if(drawSuperLines){
            for(size_t i = 0; i < superlines.size(); i++){
                arrowedLine(dispframe, *superlines[i].a, *superlines[i].b, Scalar(255,255,0));
            }
        }

        if(positions.size() > MAXPOINTS){
            positions.erase(positions.begin());
        }
        imshow("main", frame);

        //if(writefile){imwrite("../HANDCONTROL_DEBUG_OUT.jpg", frame);}
        keyboard=waitKey(30);
    }
    go=false;
    destroyAllWindows();
    return 0;
}
