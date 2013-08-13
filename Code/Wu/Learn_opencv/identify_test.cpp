#include "Precompile.h"
#include "Leaf_identifier.h"
#include "Leaf_util.h"
#include <iterator>
#include <sstream>

namespace Fs = boost::filesystem;
namespace Rng = boost::range;

std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data();

void do_work()
try {
    auto test = load_data();

    int hit = 0;
    int partial_hit = 0;
    int miss = 0;
    auto leaf = Leaf::identifier();
    leaf->init("leaf.config");
    if (!leaf->is_ok()) {
        leaf->train(Leaf::MM_view(test));
    }

    assert(leaf->is_ok());
    for (auto iter = test.begin(); iter != test.end(); ++iter) {
        BOOST_LOG_TRIVIAL(debug) << "Test a new sample";
        auto ret = leaf->identify(iter->second);

        typedef decltype(*ret.begin())& value_ref;
        std::for_each(ret.begin(), ret.end(), [](value_ref v){
            BOOST_LOG_TRIVIAL(debug)
                << v.first << " => prob: " << v.second;
        });
        if (ret.empty()) {
            ++miss;
            BOOST_LOG_TRIVIAL(debug) << "No result";
        }
        else {
            auto max = std::max_element(ret.begin(), ret.end(), [](value_ref l, value_ref r){
                return l.second < r.second;
            });
            if (max->first == iter->first) ++hit;
            else if (Rng::count_if(ret, [&](value_ref v){ return v.first == iter->first; })) {
                ++partial_hit;
            }
            else {
                BOOST_LOG_TRIVIAL(warning) << "Failed to recogize";
            }
        }
        BOOST_LOG_TRIVIAL(debug) << "It should be " << iter->first;
    }

    auto size = test.size();
    BOOST_LOG_TRIVIAL(info) << "Total: " << size;
    BOOST_LOG_TRIVIAL(info) << "Hit: " << hit << " =>" << 100.f * hit / size << "%";
    BOOST_LOG_TRIVIAL(info) << "Partial: " << partial_hit << " =>" << 100.f * partial_hit / size << "%";
    BOOST_LOG_TRIVIAL(info) << "Miss: " << miss << " =>" << 100.f * miss / size << "%";
}
catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
}

void test_svm_param();
void test_etree();
void test_rt();

int main()
{
#if 1
    do_work();
#else
    test_svm_param();
    test_etree();
    test_rt();
#endif
    return 0;
}
//-----------------------------------------------------------
//-----------------------------------------------------------
std::vector<Leaf::Leaf_identifier::feature_vector>
    read_vectors(const Fs::path& p)
{
    assert(Fs::exists(p));
    std::ifstream in(p.string());
    assert(in);
    std::vector<Leaf::Leaf_identifier::feature_vector> fvs;
    std::string line;
    while (std::getline(in, line)) {
        Leaf::Leaf_identifier::feature_vector fv;
        std::istringstream iss(line);
        float f = 0.f;
        while (iss >> f) {
            fv.push_back(f);
        }
        if (fv.size() != 14) break;
        fvs.push_back(std::move(fv));
    }
    return fvs;
}

std::multimap<
    Leaf::Leaf_identifier::label,
    Leaf::Leaf_identifier::feature_vector
    >
    load_data()
{
    std::multimap<
        Leaf::Leaf_identifier::label,
        Leaf::Leaf_identifier::feature_vector
    > rs;
    std::map<std::string, Leaf::Leaf_identifier::label> name2id;
    Fs::path dir = "F:\\Projects\\Leaf_in_mind\\Yrui_leaf\\Ê÷Ò¶Êý¾Ý£¨new£©";
    assert(Fs::is_directory(dir));

    int id = 1;
    bool has_map = Fs::exists("name-map.map");
    if (has_map) {
        std::ifstream ifs("name-map.map");
        boost::archive::text_iarchive in(ifs);
        in >> name2id;
    }
    for (auto iter = Fs::directory_iterator(dir);
        iter != Fs::directory_iterator();
        ++iter) {
            auto& path = iter->path();
            if (path.extension() == ".txt") {
                auto label = 0;
                if (has_map) {
                    label = name2id[path.stem().string()];
                    assert(label != 0);
                }
                else {
                    label = id++;
                    name2id[path.stem().string()] = label;
                }
                auto v = read_vectors(path);
                BOOST_FOREACH(auto& f, v) {
                    rs.insert(std::make_pair(label, f));
                }
            }
    }

    if (!has_map) {
        std::ofstream ofs("name-map.map");
        assert(ofs);
        boost::archive::text_oarchive out(ofs);
        out << name2id;
    }

    return rs;
}

//----------------------------------------------------------
void get_sample_resp(
    const Leaf::Leaf_identifier::Feature_view& view,
    cv::Mat_<float>& sample,
    cv::Mat_<int>& response
    )
{
    std::size_t i = 0;
    for (auto iter = view.begin(), end = view.end(); iter != end; ++iter, ++i) {
        assert(i < view.size());

        auto row = sample.row(i);
        auto& v = *iter;

        // copy feature vector to coresponding row
        std::copy(v.second.begin(), v.second.end(), row.begin());
        // set response
        response.at<int>(i) = v.first;
    }
}

std::pair<cv::Mat_<float>, cv::Mat_<int>>
    adapt_to_mat(const Leaf::Leaf_identifier::Feature_view& view)
{
    cv::Mat_<float> sample(view.size(), 14);
    cv::Mat_<int> response(view.size(), 1);
    get_sample_resp(view, sample, response);
    return { sample, response };
}

auto smps = load_data();
auto mats = adapt_to_mat(Leaf::MM_view(smps));
auto& data = mats.first;
auto& response = mats.second;
int total = smps.size();
cv::RandomTreeParams par(25,
                    5,
                    0,
                    false,
                    15,
                    nullptr,
                    true,
                    5,
                    25,
                    0.01f,
                    CV_TERMCRIT_ITER
                    );

void test_svm_param()
{
    cv::SVM svm;
    cv::SVMParams params;
    params.kernel_type = cv::SVM::LINEAR;
    params.svm_type = cv::SVM::C_SVC;
    params.C = 1;
    svm.train(data, response, cv::Mat(), cv::Mat(), params);
    int total = smps.size();
    int hit {};
    int miss {};
    for (auto& s: smps) {
        auto ret = svm.predict(cv::Mat_<float>(s.second).t());
        ret == s.first? ++hit: ++miss;
    }
    std::cout
        << "SVM: \n"
        << "Hit: " << 100.f * hit / total << "%\n"
        << "Miss: " << 100.f * miss / total << "%\n\n";
}

void test_rt()
{
    cv::RandomTrees cls;
    cv::Mat var_type(14 + 1, 1, CV_8U);
    var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
    var_type.at<uchar>(14) = CV_VAR_CATEGORICAL;
    cls.train(
        data, CV_ROW_SAMPLE, response, {}, {}, var_type, {}, par
        );
    int hit {};
    int miss {};
    for (auto& s: smps) {
        auto ret = cls.predict(cv::Mat_<float>(s.second).t());
        ret == s.first? ++hit: ++miss;
    }
    std::cout
        << "RTree: \n"
        << "Hit: " << 100.f * hit / total << "%\n"
        << "Miss: " << 100.f * miss / total << "%\n\n";
}

void test_etree()
{
    CvERTrees cls;
    cv::Mat var_type(14 + 1, 1, CV_8U);
    var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
    var_type.at<uchar>(14) = CV_VAR_CATEGORICAL;
    cls.train(
        data, CV_ROW_SAMPLE, response, {}, {}, var_type, {}, par
        );
    int hit {};
    int miss {};
    for (auto& s: smps) {
        auto ret = cls.predict(cv::Mat_<float>(s.second).t());
        ret == s.first? ++hit: ++miss;
    }
    std::cout
        << "ERTree: \n"
        << "Hit: " << 100.f * hit / total << "%\n"
        << "Miss: " << 100.f * miss / total << "%\n\n";
}

