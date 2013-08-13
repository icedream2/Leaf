#include <map>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include "ml.h"

#define NUMBER_OF_TRAINING_SAMPLES 8
#define ATTRIBUTES_PER_SAMPLE	13
#define NUMBER_OF_TESTING_SAMPLES 2
#define NUMBER_OF_CLASSES 10
std::string sample="zhang.txt";
std::string test="zhang2.txt";
std::vector<std::vector<float>> readfile(std::string path)
{
	std::cout<<"Reading file: "<<std::endl;
	std::ifstream in(path);
    assert(in);
	std::vector<std::vector<float>> fvs;
    std::string line;
	int i=0;
    while (std::getline(in, line) && !in.bad()) { 
		std::vector<float> fv;
        std::istringstream instr(line);
        float d = 0;
        while (instr >> d) {
            fv.push_back(d); 
        }
		std::cout<<++i<<std::endl;
        assert(fv.size() == ATTRIBUTES_PER_SAMPLE+1);
        fvs.push_back(fv);
    }
	return fvs;
}
int read_data_to_mat(std::vector<std::vector<float>>data, cv::Mat mdata, cv::Mat classes,int n_samples ) 
{
	std::cout<<"Reading vector: "<<std::endl;
	int number_of_data=data.size();
	for(int i=0;i<number_of_data;i++)
	{
		std::vector<float>temp=data[i];
		for(int j=0;j<ATTRIBUTES_PER_SAMPLE;j++)
		{
			 mdata.at<float>(i, j) = temp[j]; 
		}
		classes.at<float>(i,0)=temp[ATTRIBUTES_PER_SAMPLE];
	}
	return 1;
}
int main()
{
	 //定义训练数据与标签矩阵 
	cv::Mat training_data = cv::Mat(NUMBER_OF_TRAINING_SAMPLES, ATTRIBUTES_PER_SAMPLE, CV_32FC1);  
    cv::Mat training_classifications = cv::Mat(NUMBER_OF_TRAINING_SAMPLES, 1, CV_32FC1);  
  
    //定义测试数据矩阵与标签  
    cv::Mat testing_data = cv::Mat(NUMBER_OF_TESTING_SAMPLES, ATTRIBUTES_PER_SAMPLE, CV_32FC1);  
    cv::Mat testing_classifications = cv::Mat(NUMBER_OF_TESTING_SAMPLES, 1, CV_32FC1);

	cv::Mat var_type = cv::Mat(ATTRIBUTES_PER_SAMPLE + 1, 1, CV_8U );  
    var_type.setTo(cv::Scalar(CV_VAR_NUMERICAL) ); // all inputs are numerical

	var_type.at<uchar>(ATTRIBUTES_PER_SAMPLE, 0) = CV_VAR_CATEGORICAL; 
	double result;
	std::cout<<"Starting read data"<<std::endl;
	if(read_data_to_mat(readfile(sample), training_data, training_classifications, NUMBER_OF_TRAINING_SAMPLES)
	&& read_data_to_mat(readfile(test), testing_data, testing_classifications, NUMBER_OF_TESTING_SAMPLES))
	{
		float priors[] = {1,1,1,1,1,1,1,1,1,1};
		CvRTParams params = CvRTParams(25, // max depth  
										   5, // min sample count  
										   0, // regression accuracy: N/A here  
										   false, // compute surrogate split, no missing data  
										   15, // max number of categories (use sub-optimal algorithm for larger numbers)  
										   priors, // the array of priors  
										   false,  // calculate variable importance  
										   4,       // number of variables randomly selected at node and used to find the best split(s).  
										   100,  // max number of trees in the forest  
										   0.01f,               // forrest accuracy  
										   CV_TERMCRIT_ITER |   CV_TERMCRIT_EPS // termination cirteria  
										  );
		CvRTrees* rtree = new CvRTrees;  
		std::cout<<"Training..."<<std::endl; 
        rtree->train(training_data, CV_ROW_SAMPLE, training_classifications,  
                     cv::Mat(), cv::Mat(), var_type, cv::Mat(), params);  
		std::cout<<"Testing..."<<std::endl;

		cv::Mat test_sample;  
        int correct_class = 0;  
        int wrong_class = 0;  
        int false_positives [NUMBER_OF_CLASSES] = {0,0,0,0,0,0,0,0,0,0};  
  
		for (int tsample = 0; tsample < NUMBER_OF_TESTING_SAMPLES; tsample++)  
        {  
  
            // extract a row from the testing matrix  
            test_sample = testing_data.row(tsample);  
       
            result = rtree->predict(test_sample, cv::Mat());  
  
            printf("Testing Sample %i -> class result (digit %d)\n", tsample, (int) result);  
  
            // if the prediction and the (true) testing classification are the same  
            // (N.B. openCV uses a floating point decision tree implementation!)  
            if (fabs(result - testing_classifications.at<float>(tsample, 0))  
                    >= FLT_EPSILON)  
            {  
                // if they differ more than floating point error => wrong class  
                wrong_class++;  
                false_positives[(int) result]++;  
            }  
            else  
            {  
                // otherwise correct  
                correct_class++;  
            }  
			printf("\tCorrect classification: %d (%g%%)\n"  
                "\tWrong classifications: %d (%g%%)\n",   
                correct_class, (double) correct_class*100/NUMBER_OF_TESTING_SAMPLES,  
                wrong_class, (double) wrong_class*100/NUMBER_OF_TESTING_SAMPLES);  
  
			for (int i = 0; i < NUMBER_OF_CLASSES; i++)  
			{  
				printf( "\tClass (digit %d) false postives  %d (%g%%)\n", i,  
						false_positives[i],  
						(double) false_positives[i]*100/NUMBER_OF_TESTING_SAMPLES);  
			}  
  

		}
	}
	system("pause");
	return 0;
}