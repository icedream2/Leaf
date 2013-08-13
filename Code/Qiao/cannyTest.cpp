#include "cv.h"
#include "highgui.h"
char wndname[] = "Edge";
char tbarname[] = "Threshold";
int edge_thresh = 1;
IplImage *image = 0, *cedge = 0, *gray = 0, *edge = 0;
// ����������� callback ����
void on_trackbar(int h)
{
    cvSmooth( gray, edge, CV_BLUR, 3, 3, 0 );
    cvNot( gray, edge );
    // �ԻҶ�ͼ����б�Ե���
    cvCanny(gray, edge, (float)edge_thresh, (float)edge_thresh*3, 3);
    cvZero( cedge );
    // copy edge points
    cvCopy( image, cedge, edge );
    // ��ʾͼ��
    cvShowImage(wndname, cedge);
}
int main(  )
{
    char* filename = "zhang.jpg";
    
    if( (image = cvLoadImage( filename, 1)) == 0 )
        return -1;
    // Create the output image
    cedge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 3);
    // ����ɫͼ��ת��Ϊ�Ҷ�ͼ��
    gray = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    edge = cvCreateImage(cvSize(image->width,image->height), IPL_DEPTH_8U, 1);
    cvCvtColor(image, gray, CV_BGR2GRAY);
    // Create a window
    cvNamedWindow(wndname, 1);
    // create a toolbar 
    cvCreateTrackbar(tbarname, wndname, &edge_thresh, 100, on_trackbar);
    // Show the image
    on_trackbar(1);
    // Wait for a key stroke; the same function arranges events processing
    cvWaitKey(0);
    cvReleaseImage(&image);
    cvReleaseImage(&gray);
    cvReleaseImage(&edge);
    cvDestroyWindow(wndname);
    return 0;
}
