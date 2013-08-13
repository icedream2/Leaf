#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include<iostream>
#include"GCApplication.h"
#include"Leafparse.h"
using namespace std;
using namespace cv;
void show_img1(const std::string& name, const Mat& img)
{
    imshow(name, img);
    waitKey();
}

static void help()
{
    cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
            "and then grabcut will attempt to segment it out.\n"
            "Call:\n"
            "./grabcut <image_name>\n"
        "\nSelect a rectangular area around the object you want to segment\n" <<
        "\nHot keys: \n"
        "\tESC - quit the program\n"
        "\tr - restore the original image\n"
        "\tn - next iteration\n"
        "\n"
        "\tleft mouse button - set rectangle\n"
        "\n"
        "\tCTRL+left mouse button - set GC_BGD pixels\n"
        "\tSHIFT+left mouse button - set CG_FGD pixels\n"
        "\n"
        "\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
        "\tSHIFT+right mouse button - set CG_PR_FGD pixels\n" << endl;
}
GCApplication gcapp;

void on_mouse( int event, int x, int y, int flags, void* param )
{
    gcapp.mouseClick( event, x, y, flags, param );
}

int main( int argc, char** argv )
{

    string filename = "5.jpg";
    Mat image = imread( filename, 1 );
    if( image.empty() )
    {
        cout << "\n Durn, couldn't read image filename " << filename << endl;
        return 1;
    }

    help();

    const string winName = "image";
    cvNamedWindow( winName.c_str(), CV_WINDOW_AUTOSIZE );
    cvSetMouseCallback( winName.c_str(), on_mouse, 0 );

    gcapp.setImageAndWinName( image, winName );
    gcapp.showImage();

    for(;;)
    {
        int c = cvWaitKey(0);
        switch( (char) c )
        {
                case '\x1b':
                    cout << "Exiting ..." << endl;
                    goto exit_main;
                case 'r':
                    cout << endl;
                    gcapp.reset();
                    gcapp.showImage();
                    break;

				case 'd':{
					Mat im=gcapp.getImage();
					show_img1("!!!!",im);
					for(int i=0;i<im.rows;i++){
						for(int j=0;j<im.cols;j++){
							if(im.at<Vec3b>(Point(j,i))[0]==0&&im.at<Vec3b>(Point(j,i))[1]==0&&im.at<Vec3b>(Point(j,i))[2]==0){
								im.at<Vec3b>(Point(j,i))[0]=255;
								im.at<Vec3b>(Point(j,i))[1]=255;
								im.at<Vec3b>(Point(j,i))[2]=255;
							}
							/*if(j==539){
								int p;
								p++;
							}*/
						}
					}
					show_img1("!!!!",im);
					Mat img(im.rows, im.cols,CV_8UC1);
					cvtColor(im,img,CV_BGR2GRAY);
					show_img1("!!!!",img);
					auto feature=Leaf::img_analysis(img);
					for(int i=0;i<feature.size();i++)
						cout<<feature[i]<<endl;
					system("pause");
					goto exit_main;
				}
                case 'n':{
                    int iterCount = gcapp.getIterCount();
                    cout << "<" << iterCount << "... ";
                    int newIterCount = gcapp.nextIter();
                    if( newIterCount > iterCount )
                    {
                        gcapp.showImage();
                        cout << iterCount << ">" << endl;
                    }
                    else
                        cout << "rect must be determined>" << endl;
                    break;
				}
        }
    }

exit_main:
    cvDestroyWindow( winName.c_str() );
    return 0;
}