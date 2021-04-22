
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/objdetect.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <fstream>
#include <chrono>

using namespace cv;
using namespace std;

// global variables
    int count = 0;
    bool timer_on = false;
    double total_time_in_s = 0; 
    float Navg = 0;
    std::chrono::steady_clock::time_point start;
    std::chrono::steady_clock::time_point end;
	

/*detects people in the area with rectangles and returns the number of people
detected to the main function*/

int detectAndDraw(const HOGDescriptor &hog, Mat &img)
{
    vector<Rect> found, found_filtered;
    double time = (double) getTickCount();
    hog.detectMultiScale(img, found, 0, Size(8,8), Size(16,16), 1.07, 2);
    time = (double) getTickCount() - time;
    cout << "detection time = " << (time*1000./cv::getTickFrequency()) << " ms" << endl;

    for(size_t i = 0; i < found.size(); i++ )
    {
        Rect r = found[i];
        size_t j;
        // Do not add small detections inside a bigger detection.
        for ( j = 0; j < found.size(); j++ )
            if ( j != i && (r & found[j]) == r )
                break;
        if ( j == found.size() )
        	found_filtered.push_back(r);
    }
    for (size_t i = 0; i < found_filtered.size(); i++)
    {
        Rect r = found_filtered[i];
        // The HOG detector returns slightly larger rectangles than the real objects,
        // so we slightly shrink the rectangles to get a nicer output.
        r.x += cvRound(r.width*0.1);
        r.width = cvRound(r.width*0.8);
        r.y += cvRound(r.height*0.07);
        r.height = cvRound(r.height*0.8);
        rectangle(img, r.tl(), r.br(), cv::Scalar(0,255,0), 3);
    }
    return found_filtered.size();
}

/*captures the frames from the camera and sends to detectAndDraw to get number of
 people entered in the area */

int main(int argc, char **argv) {

    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    cout << "Built with OpenCV " << CV_VERSION << endl;
    Mat cam_image;
    VideoCapture capture;
    	//opening the camera
    capture.open(0);
    
	// files to write data 
    ofstream N_file("Npeople.txt");
    ofstream T_file("Toccupied.txt");
    //int count = 0;
    //bool timer_on = false;
    //double total_time_in_s = 0;	
         //checks whether camera is opened or not
    if(capture.isOpened())	
    {
        cout << "Capture is opened" << endl;
        for(;;)
        {
            capture >> cam_image;
        //checks for the last frame to exit
            if(cam_image.empty())	
                break;

	    //returns the number of people detected from detect and draw function
            int num_people = detectAndDraw(hog, cam_image);		
            cout << "People count: " << num_people << endl;
	    //Nfile.open();
	    N_file << num_people << endl;
	    //Nfile.close();
	    Navg += num_people;
	    ++count;
	    
	    if(count = 10)
	    {
	    	Navg /= 10;
	    	count = 0;
	    		
	    }		
	    if (Navg >= 1)  // N was >=1 successively for 10 reads
	    {
	    	if( !timer_on)
	    	{
	    		// start timer
	    		start = std::chrono::steady_clock::now();
	    		timer_on = true;
			cout << "\n...........Turning on Timer!.........";
	    	}
	    }
	    else if (Navg == 0) // N was '0' successively for 10 reads
	    {
	    	if(timer_on)
	    	{
			cout << "\n......Turning off Timer!....";
	    		end = std::chrono::steady_clock::now();
	    		std::chrono::duration<double> elapsed_seconds = end-start;
	    		
	    		std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
	    		total_time_in_s += elapsed_seconds.count();
	    		//Tfile.open();
			T_file << total_time_in_s << endl;
			//Tfile.close();
	    		timer_on = false;		
	    	}
	    }

        //displays every frame to the user
            imshow("HOG Descriptor based People counter", cam_image);
            cout<<"press q to quit"<<endl;
            if((waitKey(30) & 0xff)== 'q')
            {
               cout<<"quit sucessfully"<<endl;
	       N_file.close();
	       T_file.close();
               return 0;
            }
        }
    }
    else
    {
	//executes when the camera is not opened and exits from the main	
        cout << "No capture" << endl;				
        cam_image = Mat::zeros(480, 640, CV_8UC1);
        imshow("Sample", cam_image);
        waitKey(0);
        return 0;
    }
    cout<<"video completed succesfully"<<endl;
    return 0;
}

