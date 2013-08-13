#include "Precompile.h"
#include "Leaf_identifier_imp.h"
#include "Leaf_util.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <numeric>

/*!
 * \file    Leaf_identifier_imp.cpp
 * \ingroup Impl.
 * \brief   Implementation of Leaf_imp.
 */

namespace Fs = boost::filesystem;
namespace Pt = boost::property_tree;
namespace Rn = boost::range;
namespace Ad = boost::adaptors;

namespace Config {

/*!
 * \brief   Define keys used in the config file.
 */
enum Config_key {
    class_count,
    attr_count,
    knn_k,
    classifiers,
    samples,

    ert,
    knn,
    ann,
    randomtrees,
    bayes
};

namespace Detail {
const char* _path_value[] = {
    "leaf-config.class-count",
    "leaf-config.attr-count",
    "leaf-config.classifiers.knn.k",
    "leaf-config.classifiers",

    "leaf-config.samples",

    "ert.cls-file",
    "knn.cls-file",
    "ann.cls-file",
    "randomtrees.cls-file",
    "bayes.cls-file"
};

const char* _cls_file[] = {
    "ert-cls.csv",
    "knn-cls.csv",
    "ann-cls.csv",
    "randomtrees-cls.csv",
    "bayes-cls.csv"
};
}

template <Config_key key>
inline const char* path()
{
    static_assert(
        key < std::extent<decltype(Detail::_path_value)>::value,
        "bad config key"
        );
    return Detail::_path_value[key];
}

template <Config_key key>
inline const char* value()
{
    static_assert(ert <= key && key <= bayes, "this key does not have value");
    return Detail::_cls_file[key - ert];
}

template <>
inline const char* value<samples>()
{
    return "leaf-samples.db";
}

}  // of namespace config

namespace {

void init_log()
{
    namespace Log = boost::log;
    namespace expr = Log::expressions;

    auto sink1 = Log::add_console_log();

    // \todo    move log file name to config file.
    auto sink2 = Log::add_file_log("leaf_identify.log");

    Log::add_common_attributes();

    auto format = expr::stream
        << expr::attr<unsigned>("LineID")
        << ":<" << Log::trivial::severity
        << ">" << expr::smessage;

    sink1->set_formatter(format);
    sink2->set_formatter(format);
}

struct Seq_generator {
    Seq_generator(): i_() {}

    int operator()() { return i_++; }

private:
    int i_;
};

}  // of namespace unnamed

//-----------------------------------------------------------------------
void Leaf::get_sample_resp(
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

//-----------------------------------------------------------------------
Leaf::Leaf_identifier::Leaf_imp::Leaf_imp(const std::string& config)
    : is_trained_(), class_count_(), attr_count_(), k_()
{
    init_log();
    init_(config);
}

void Leaf::Leaf_identifier::Leaf_imp::clear()
{
    BOOST_LOG_TRIVIAL(trace) << "Clear all existing classifiers";
    samples_.clear();
    label2num_.clear();

    ert_.clear();
    rt_.clear();
    bayes_.clear();
    knn_.clear();
    ann_.clear();

    is_trained_ = false;
}

void Leaf::Leaf_identifier::Leaf_imp::init_(
    const std::string& config_file
    )
{
    BOOST_LOG_TRIVIAL(trace) << "Instantiate identifier";
    // read config file
    try {
        using namespace Config;
        Pt::xml_parser::read_xml(config_file, config_);

        // read global identifier properties
        class_count_ = config_.get<int>(path<class_count>());
        attr_count_ = config_.get<int>(path<attr_count>());
        k_ = config_.get<int>(path<knn_k>());

        // check if we have pre-trained classifiers
        auto cls = config_.get_child(path<classifiers>());
        auto smps = config_.get_optional<std::string>(path<samples>());
        auto ert_file = cls.get_optional<std::string>(path<ert>());
        auto ann_file = cls.get_optional<std::string>(path<ann>());
        auto bayes_file = cls.get_optional<std::string>(path<bayes>());
        auto rt_file = cls.get_optional<std::string>(path<randomtrees>());
        if (ert_file && ann_file && bayes_file && rt_file && smps) {
            {
                assert(Fs::exists(smps.get()));
                std::ifstream ifs(smps.get());
                boost::archive::text_iarchive in(ifs);
                in >> samples_ >> label2num_;
            }

            // train classifiers
            auto str = [](decltype(ert_file)& f){ return f.get().c_str(); };
            ert_.load(str(ert_file));
            ann_.load(str(ann_file));
            bayes_.load(str(bayes_file));
            rt_.load(str(rt_file));

            // train knn in a special way
            MM_view view(samples_);
            cv::Mat_<float> sample(view.size(), attr_count_);
            cv::Mat_<int> response(view.size(), 1);
            get_sample_resp(view, sample, response);
            knn_.train(sample, response);

            is_trained_ = true;
            BOOST_LOG_TRIVIAL(trace) << "Load existing classifiers";
        }
    }
    catch (Pt::ptree_error&) {
        throw std::runtime_error("Bad config file");
    }

    config_file_ = config_file;
}

void Leaf::Leaf_identifier::Leaf_imp::train(
    const Leaf::Leaf_identifier::Feature_view& view
    )
{
    //assert(!is_trained_);
    BOOST_LOG_TRIVIAL(trace) << "Train classifiers";
    if (is_trained_) {
        // clear all
        this->clear();
    }

    assert(!is_trained_);
    assert(attr_count_);
    if (view.empty()) return;

    cv::Mat_<float> sample(view.size(), attr_count_);
    cv::Mat_<int> response(view.size(), 1);
    get_sample_resp(view, sample, response);

    // copy all in view to samples_
    samples_.insert(view.begin(), view.end());
    assert(samples_.size() == sample.rows);

    train_imp_(sample, response);
    save_();

    is_trained_ = true;
}

void Leaf::Leaf_identifier::Leaf_imp::save_()
{
    using namespace Config;
    BOOST_LOG_TRIVIAL(trace) << "Save trained classifiers";

    auto r1 = boost::async([&]{ ann_.save(value<ann>()); });
    auto r3 = boost::async([&]{ ert_.save(value<ert>()); });
    auto r4 = boost::async([&]{ rt_.save(value<randomtrees>()); });
    auto r5 = boost::async([&]{ bayes_.save(value<bayes>()); });

    auto& cls = config_.get_child(path<classifiers>());
    cls.put(path<ann>(), value<ann>());
    cls.put(path<ert>(), value<ert>());
    cls.put(path<randomtrees>(), value<randomtrees>());
    cls.put(path<bayes>(), value<bayes>());

    {
        std::ofstream ofs(value<samples>());
        boost::archive::text_oarchive out(ofs);
        out << samples_ << label2num_;
        config_.put(path<samples>(), value<samples>());

        Pt::xml_writer_settings<char> setting;
        Pt::xml_parser::write_xml(
            config_file_,
            config_,
            std::locale(),
            setting
            );
    }

    r1.get(); r3.get(); r4.get(); r5.get();
}

void Leaf::Leaf_identifier::Leaf_imp::train_imp_(
    const cv::Mat_<float>& data,
    const cv::Mat_<int>& response
    )
{
    assert(!is_trained_);
    BOOST_LOG_TRIVIAL(trace) << "Train all top-level classifiers";

    // train svm
    auto r1 = boost::async([&]{
        BOOST_LOG_TRIVIAL(trace) << "Train ERTrees";
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
        // create type mask
        cv::Mat var_type(attr_count_ + 1, 1, CV_8U);
        var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
        var_type.at<uchar>(attr_count_) = CV_VAR_CATEGORICAL;

        ert_.train(data, CV_ROW_SAMPLE, response, {}, {}, var_type, {}, par);
    });

    // train bayes
    auto r2 = boost::async([&]{
        BOOST_LOG_TRIVIAL(trace) << "Train Bayes";
        bayes_.train(data, response);
    });

    // train rt
    auto r3 = boost::async([&]{
        BOOST_LOG_TRIVIAL(trace) << "Train Random Trees";
        //! \todo   it should config in a config file.
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
        // create type mask
        cv::Mat var_type(attr_count_ + 1, 1, CV_8U);
        var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
        var_type.at<uchar>(attr_count_) = CV_VAR_CATEGORICAL;

        rt_.train(data, CV_ROW_SAMPLE, response, cv::Mat(), cv::Mat(), var_type, cv::Mat(), par);
    });

    // train knn
    auto r4 = boost::async([&]{
        BOOST_LOG_TRIVIAL(trace) << "Train KNN";
        knn_.train(data, response);
    });

    // train ann
    auto r5 = boost::async([&]{
        BOOST_LOG_TRIVIAL(trace) << "Train ANN";

        int layer_sz[] = { attr_count_, 100, 100, class_count_ };
        ann_.create(cv::Mat_<int>(1, 4, layer_sz));

        cv::Mat_<float> new_response(data.rows, class_count_, 0.f);
        assert(std::all_of(new_response.begin(), new_response.end(), [](float x){ return x == 0; }));

        // generate numbers used in ann
        Seq_generator gen;
        BOOST_FOREACH(auto label, samples_ | Ad::map_keys | Ad::uniqued) {
            label2num_[label] = gen();
        }

        for (int row = 0; row < data.rows; ++row) {
            cv::Mat_<float> cur_row = new_response.row(row);
            auto label = response.at<int>(row);
            cur_row.at<float>(label2num_[label]) = 1.f;
        }

        ann_.train(
            data, new_response,
            cv::Mat(), cv::Mat(),
            cv::ANN_MLP_TrainParams(
                cv::TermCriteria(CV_TERMCRIT_ITER, 1000, 0.01),
                cv::ANN_MLP_TrainParams::BACKPROP,
                0.001
                )
            );
    });

    r1.get(); r2.get(); r3.get(); r4.get(); r5.get();
}

Leaf::Leaf_identifier::Leaf_imp::result_set
    Leaf::Leaf_identifier::Leaf_imp::identify(
       const Leaf::Leaf_identifier::Leaf_imp::feature_vector& fv
    )
{
    BOOST_LOG_TRIVIAL(trace) << "Identify a new feature vector";
    auto vec = cv::Mat(fv).t();
    auto first = primary_filter_(vec);
    auto second = secondary_predict_(vec, first);
    return finnal_verify_(vec, second);
}

//------------------------------------------------------------------
// End of File
//------------------------------------------------------------------
