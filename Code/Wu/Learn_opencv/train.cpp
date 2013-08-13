#include <map>
#include <utility>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <boost/range.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/ml/ml.hpp>

namespace Fs = boost::filesystem;

const auto data_file = "sample.train";

const auto attr_count = 14;
const auto class_count = 7;

const int test_array[] = { 0, 9, 17, 23, 33, 39, 53 };
const int test_count = std::extent<decltype(test_array)>::value;

auto equals = [](float l, float r){ return std::abs(l - r) < FLT_EPSILON ? 1 : 0; };

// This function reads data and responses from the file <filename>
static int
read_num_class_data( const char* filename, int var_count,
                     CvMat** data, CvMat** responses )
{
    const int M = 1024;
    FILE* f = fopen( filename, "rt" );
    CvMemStorage* storage;
    CvSeq* seq;
    char buf[M+2];
    float* el_ptr;
    CvSeqReader reader;
    int i, j;

    if( !f )
        return 0;

    el_ptr = new float[var_count+1];
    storage = cvCreateMemStorage();
    seq = cvCreateSeq( 0, sizeof(*seq), (var_count+1)*sizeof(float), storage );

    for(;;)
    {
        char* ptr;
        if( !fgets( buf, M, f ) || !strchr( buf, ',' ) )
            break;
        el_ptr[0] = buf[0];
        ptr = buf+2;
        for( i = 1; i <= var_count; i++ )
        {
            int n = 0;
            sscanf( ptr, "%f%n", el_ptr + i, &n );
            ptr += n + 1;
        }
        if( i <= var_count )
            break;
        cvSeqPush( seq, el_ptr );
    }
    fclose(f);

    *data = cvCreateMat( seq->total, var_count, CV_32F );
    *responses = cvCreateMat( seq->total, 1, CV_32F );

    cvStartReadSeq( seq, &reader );

    for( i = 0; i < seq->total; i++ )
    {
        const float* sdata = (float*)reader.ptr + 1;
        float* ddata = data[0]->data.fl + var_count*i;
        float* dr = responses[0]->data.fl + i;

        for( j = 0; j < var_count; j++ )
            ddata[j] = sdata[j];
        *dr = sdata[-1];
        CV_NEXT_SEQ_ELEM( seq->elem_size, reader );
    }

    cvReleaseMemStorage( &storage );
    delete[] el_ptr;
    return 1;
}

std::pair<cv::Mat, cv::Mat> read_data()
{
    CvMat* raw_data = nullptr;
    CvMat* raw_response = nullptr;
    if (!read_num_class_data(data_file, attr_count, &raw_data, &raw_response)) {
        throw std::runtime_error(std::string("cannot open file ") + data_file);
    }

    return std::make_pair(cv::Mat(raw_data), cv::Mat(raw_response));
}

void test_rtree(const cv::Mat& data, const cv::Mat& response)
{
    // create training data and response data.
    auto nsamples = data.rows;
    auto ntests = test_count;
    auto ntrains = nsamples - ntests;

    // prepare random trees
    cv::RandomTrees rt;
    cv::RandomTreeParams par(25, 
                             5,
                             0,
                             false,
                             15,
                             nullptr,
                             true,
                             5,
                             10,
                             0.01f,
                             CV_TERMCRIT_ITER
                             );

    // create type mask
    cv::Mat var_type(attr_count + 1, 1, CV_8U);
    var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
    var_type.at<uchar>(attr_count) = CV_VAR_CATEGORICAL;
    
    // set sample index
    cv::Mat samples_idx(1, nsamples, CV_8U, 1);
    std::for_each(std::begin(test_array), std::end(test_array), [&](int line){
        samples_idx.at<uchar>(line) = uchar(0);
    });

    // train the classifier
    rt.train(data, CV_ROW_SAMPLE, response, cv::Mat(), samples_idx, var_type, cv::Mat(), par);
    
    double train_hit = 0;
    double test_hit = 0;
    
    // predict
    for (int row = 0; row < nsamples; ++row) {
        cv::Mat sample = data.row(row);
        auto res = rt.predict(sample);
        assert(res != 0);
        auto is_right = std::abs(res - response.at<float>(row, 0)) <= FLT_EPSILON ? 1 : 0;
        std::count(std::begin(test_array), std::end(test_array), row)? 
            test_hit += is_right:
            train_hit += is_right;
    }
    
    std::cout
        << "Random trees:\n"
        << "Number of samples: " << nsamples
        << "\nNumber of test: " << ntests
        << "\nNumber of trees: " << rt.get_tree_count()
        << "\nRecognition rate: train = " << (100 * train_hit / ntrains)
        << "; test = " << (100 * test_hit / ntests)
        << "\n\n";
}

// now the boost algorithm can only be used in 2-class problems
// or we can recompose the samples to fit multi-class problems which
// require temperary process in identification.
void test_boost(const cv::Mat& data, const cv::Mat& response)
{
    const auto nsamples = data.rows;
    const auto ntrains = nsamples - test_count;

    // features + class -> response
    cv::Mat_<float> new_data(nsamples * class_count, attr_count + 1);
    cv::Mat_<int> new_response(nsamples * class_count, 1);
    
    std::cout << "Using boosting classifier:\n";
    for (int row = 0; row < nsamples; ++row) {
        auto cur_row = data.row(row);
        const auto cur_resp = response.at<float>(row);
        assert(cur_resp >= '1' && cur_resp <= '1' + class_count);
        for (int i = 0; i < class_count; ++i) {
            const auto new_row = row * class_count + i;
            auto new_data_row = new_data.row(new_row);
            assert(cur_row.cols + 1 == new_data_row.cols && cur_row.cols == attr_count);
            std::copy(cur_row.begin<float>(), cur_row.end<float>(), new_data_row.begin());
            assert(boost::range::equal(
                cv::Mat_<float>(new_data_row.colRange(0, attr_count)), 
                cv::Mat_<float>(cur_row)
                )
            );
            
            new_data_row.at<float>(attr_count) = i;

            assert(response.rows * class_count == new_response.rows);
            assert(response.cols == new_response.cols);
            new_response.at<int>(new_row) = (cur_resp == i + '1')? 1: -1;
            assert(new_response.at<int>(new_row) == 1 || new_response.at<int>(new_row) == -1);
        }
    }

    assert(std::count(new_response.begin(), new_response.end(), 1) == nsamples);
    
    // create type mask
    cv::Mat var_type(attr_count + 2, 1, CV_8U, cv::Scalar_<uchar>(CV_VAR_ORDERED));
    var_type.at<uchar>(attr_count) = CV_VAR_CATEGORICAL;
    var_type.at<uchar>(attr_count + 1) = CV_VAR_CATEGORICAL;
    
    // train the classifier
    std::cout << "train the classifier(may take a few minutes)...\n";
    cv::Boost bst;
    float priors[attr_count + 1];
    std::fill(std::begin(priors), std::end(priors), 1);
    priors[attr_count] = 10;
    bst.train(
        new_data, 
        CV_ROW_SAMPLE, 
        new_response, 
        cv::Mat(), 
        cv::Mat(), 
        var_type, 
        cv::Mat(),
        cv::BoostParams(cv::Boost::REAL, 10, 0.95, 5, false, priors)
        );

    // try predicting
    int hit = 0;
    cv::Mat_<float> sample(1, attr_count + 1);
    cv::Mat_<float> weak_resp(1, bst.get_weak_predictors()->total);
    for (int row = 0; row < nsamples; ++row) {
        int type = -1;
        auto cur_row = data.row(row);
        std::copy(cur_row.begin<float>(), cur_row.end<float>(), sample.begin());
        int best_class = 0;
        double max_sum = -1;
        for (int i = 0; i < class_count; ++i) {
            sample.at<float>(attr_count) = i;
            auto ret = bst.predict(sample, cv::Mat(), cv::Range::all(), false, true);
            if (max_sum < ret) {
                max_sum = ret;
                best_class = i + '1';
            }
        }
        assert('1' <= response.at<float>(row) && response.at<float>(row) <= '1' + class_count);
        hit += equals(best_class, response.at<float>(row));
    }

    std::cout
        << "Number of trees: " << bst.get_weak_predictors()->total << "\n"
        << "Recognition rate: " << 100. * hit / nsamples << "\n"
        << std::endl;
}

// mlp is too limited when taking number of classes into consideration, thus
// I simply ignore it.
// while it can be used in farther process when serveral result is produced by previous operation.
void test_mlp(const cv::Mat& data, const cv::Mat& response)
{
    const auto nsamples = data.rows;

    cv::Mat_<float> new_response(nsamples, class_count, 0.f);
    assert(std::all_of(new_response.begin(), new_response.end(), [](float f){ return f == 0.f; }));
    
    // 1. unroll the response
    for (int row = 0; row < data.rows; ++row) {
        cv::Mat_<float> cur_row = new_response.row(row);
        const auto label = cvRound(response.at<float>(row) - '1');
        cur_row.at<float>(label) = 1.f;
    }

    // 2. train
    int layer_sz[] = { attr_count, 100, 100, class_count };
    CvANN_MLP mlp(cv::Mat_<int>(1, 4, layer_sz));
    
    std::cout << "Mlp training(it may take a few minute)...\n";
    mlp.train(
        data, new_response, 
        cv::Mat(), cv::Mat(), 
        cv::ANN_MLP_TrainParams(
            cv::TermCriteria(CV_TERMCRIT_ITER, 1000, 0.01),
            cv::ANN_MLP_TrainParams::BACKPROP,
            0.001
            )
        );

    // 3. predict
    cv::Mat_<float> mlp_resp(1, class_count);
    int hit = 0;
    for (int row = 0; row < nsamples; ++row) {
        auto sample_row = data.row(row);
        mlp.predict(sample_row, mlp_resp);
        cv::Point max_loc;
        cv::minMaxLoc(mlp_resp, nullptr, nullptr, nullptr, &max_loc);
        auto best_class = max_loc.x + '1';
        hit += equals(best_class, response.at<float>(row));
    }

    std::cout << "Recognition rate: " << 100.f * hit / nsamples << "\n\n";
}

void test_knearest(const cv::Mat& data, const cv::Mat& response, int k)
{
    const auto nsamples = data.rows;

    // 1. train the classifier
    cv::KNearest kn(data, response);

    // 2. predict
    cv::Mat_<float> nearest(nsamples, k);
    cv::Mat_<float> result(1, nsamples);
    kn.find_nearest(data, k, result, nearest, cv::Mat());
    int true_resp = 0;
    int accuracy = 0;
    for (int i = 0; i < nsamples; ++i) {
        if (result.at<float>(0, i) == response.at<float>(i)) ++true_resp;
        cv::Mat_<float> all_result = nearest.row(i);
        std::for_each(all_result.begin(), all_result.end(), [&](float f){
            if (f == response.at<float>(i)) ++accuracy;
        });
    }

    std::cout << "Knearest with K: " << k
        << "\ntrue_resp: " << 100.f * true_resp / nsamples
        << "\naccuracy: " << 100.f * accuracy / (nsamples * k)
        << "\n\n";
}

void test_nbayes(const cv::Mat& data, const cv::Mat& response)
{
    const auto nsamples = data.rows;

    cv::NormalBayesClassifier bayer(data, response);
    
    cv::Mat_<float> result(1, nsamples, 0.f);
    bayer.predict(data, &result);
    
    int true_resp = 0;
    for (int i = 0; i < nsamples; ++i) {
        if (result.at<float>(0, i) == response.at<float>(i)) ++true_resp;
    }

    std::cout
        << "Naive bayes: \n"
        << "true resp = " << 100.f * true_resp / nsamples
        << "\n\n";
}

void test_svm(const cv::Mat& data, const cv::Mat& response)
{
    const auto nsamples = data.rows;
    
    cv::SVMParams params;
    params.kernel_type = cv::SVM::LINEAR;
    params.svm_type = cv::SVM::C_SVC;
    params.C = 1;

    // 1. train
    cv::SVM svm;
    svm.train(data, response, cv::Mat(), cv::Mat(), params);

    // 2. predict
    cv::Mat_<float> result;
    svm.predict(data, result);
    
    int true_resp = 0;
    for (int i = 0; i < nsamples; ++i) {
        if (result.at<float>(i) == response.at<float>(i)) ++true_resp;
    }

    std::cout
        << "SVM true_resp: "
        << 100.f * true_resp / nsamples
        << "\n\n";
}

int main()
try {
    auto mats = read_data();
    auto& data = mats.first;
    auto& response = mats.second;
    
    test_rtree(data, response);
    test_boost(data, response);
    test_mlp(data, response);
    test_knearest(data, response, 5);
    test_knearest(data, response, 10);
    test_nbayes(data, response);
    test_svm(data, response);

    return 0;
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
    return -1;
}
//----------------------------------------------------------------