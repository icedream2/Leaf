#include "Precompile.h"
#include "Leaf_identifier_imp.h"
#include "Leaf_util.h"
#include <array>

/*!
 * \file    Identifier_recognition_imp.cpp
 * \ingroup Impl.
 * \brief   Recognition related implementation.
 */

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
namespace {

//! \name   boost::async defect
//! @{
inline int predictor_exec(std::function<int()> fn)
{
    return fn();
}

inline boost::unique_future<int>
    async(std::function<int()> fn)
{
    return boost::async(boost::bind(predictor_exec, fn));
}
//! @}

void correctify_result_set(Leaf::Leaf_identifier::result_set& rs)
{
    namespace Ad = boost::adaptors;
    // remove low prob result
    auto iter = rs.begin();
    while (iter != rs.end()) {
        if (iter->second < 0.1f) iter = rs.erase(iter);
        else ++iter;
    }

    auto val_range = rs | Ad::map_values;
    auto sum = std::accumulate(val_range.begin(), val_range.end(), 0.f);
    for (auto& item: rs) {
        item.second /= sum;
    }
}

/*!
 * \param   [in] candidate        vote result of the five major classifiers.
 * \param   [in] ann_probs        ann probabilities estimate.
 * \return  result_set of possbile results with coresponding probabilities.
 *
 * vote result and generate probabilities based result set.
 * the result may be like this:
 *      (7, 10%)
 *      (12, 20%)
 *      ...
 */
Leaf::Leaf_identifier::result_set
    vote_result(
        const std::array<int, 5>& candidate,
        std::map<Leaf::Leaf_identifier::label, float>& ann_probs
        )
{
    typedef Leaf::Leaf_identifier::result_set result_set;
    typedef Leaf::Leaf_identifier::label label;

    // vote and check
    std::map<label, std::size_t> vote;
    for (auto x: candidate) {
        ++vote[x];
    }

    result_set rs;

    typedef decltype(*vote.begin())& value_cref;
    auto vote_entry = [&](int count){
        return std::find_if(
                vote.begin(),
                vote.end(),
                [=](value_cref v){ return v.second == count; });
    };

    // process the vote and generate coresponding probabilities
    // note that:
    //      [3, 4) denote credible result
    //      [1, 3) denote incredible result
    //      (0, 1) denote candidate result
    switch (vote.size()) {
    case 1:     // the most happy result
        {
        // we only get one result, we can simply return it.
        // for precision, I check the probability of that result in ANN
            BOOST_LOG_TRIVIAL(debug) << "Sole result: " << vote.begin()->first;
            assert(ann_probs.find(vote.begin()->first) != ann_probs.end());
            ann_probs[vote.begin()->first] += 3.f;
            break;
        // note that when there are only one result, the ANN result is credible,
        // such that we can calculate probabilities based on ann_probs
        }

    case 2:
        {
        // possible result may be (4, 1) or (3, 2)
            auto vote_4 = vote_entry(4);
            if (vote_4 != vote.end()) {  // (4, 1)
                auto vote_1 = vote_entry(1);
                assert(vote_1 != vote.end());

                BOOST_LOG_TRIVIAL(debug)
                    << "(4, 1) result: ("
                    << vote_4->first << ", " << vote_1->first << ")";

                // no matter ANN works or not, the (4, 1) result is
                // likely to present a valid result.
                // if ANN works, probs += 2.f, 1.f otherwise
                ann_probs[vote_4->first] +=
                    (vote_4->first == candidate[4])?
                    3.f:
                    2.f;
            }
            else {  // (3, 2)
                // (3, 2) is not quite credible
                auto vote_3 = vote_entry(3);
                auto vote_2 = vote_entry(2);
                assert(vote_3 != vote.end());
                assert(vote_2 != vote.end());

                BOOST_LOG_TRIVIAL(debug)
                    << "(3, 2) result: ("
                    << vote_3->first << ", " << vote_2->first << ")";

                /*
                if (vote_3->first == candidate[4]) {  // ANN works
                    ann_probs[vote_3->first] += 1.f;
                    ann_probs[vote_2->first] += 0.5f;
                }
                else {  // ANN do not work
                    ann_probs[vote_3->first] += 1.5f;
                    ann_probs[vote_2->first] += 1.f;
                }
                */
                // I don't matter whether ANN works or not because I
                // think the candidate result may reside in (3, 2)
                ann_probs[vote_3->first] += 1.5f;
                ann_probs[vote_2->first] += 1.f;
            }

            break;
        }

    case 3:
        {
        // possible result: (3, 1, 1) or (2, 2, 1)
            auto vote_3 = vote_entry(3);
            if (vote_3 != vote.end()) {  // (3, 1, 1)
                // this may be a possible result
                BOOST_LOG_TRIVIAL(debug) << "(3, 1, 1) result => 3: " << vote_3->first;
                // remend all the probs
                for (auto& v: vote) ann_probs[v.first] += 0.5f;

                ann_probs[vote_3->first] += 1.f;
            }
            else {  // (2, 2, 1)
                // this one could be quite suspicious.
                BOOST_LOG_TRIVIAL(debug) << "(2, 2, 1) result";
                std::for_each(vote.begin(), vote.end(), [&](value_cref l){
                    if (l.second == 2) ann_probs[l.first] += 0.5f;
                });
            }
            break;
        }

    default:
        return rs;
    }

    // copy all possible result to result set
    for (auto& v: ann_probs) {
        if (v.second > 0.2f) rs.insert(v);
    }

    // correctify the probabilities
    correctify_result_set(rs);
    return rs;
}

}  // of namespace unnamed

Leaf::Leaf_identifier::Leaf_imp::result_set
    Leaf::Leaf_identifier::Leaf_imp::primary_filter_(
        const cv::Mat& sample
        )
{
    BOOST_LOG_TRIVIAL(trace) << "Primary identification";

    assert(k_ != 0);
    assert(class_count_ != 0);
    assert(attr_count_ != 0);

    // launch all tasks

    auto task1 = async([&]{ return cvRound(ert_.predict(sample)); });
    auto task2 = async([&]{ return cvRound(bayes_.predict(sample)); });
    auto task3 = async([&]{ return cvRound(rt_.predict(sample)); });

    cv::Mat_<float> nearest(1, k_);
    cv::Mat_<float> dist(1, k_);
    cv::Mat_<float> result(1, 1);
    auto task4 = async([&]{
        return cvRound(knn_.find_nearest(sample, k_, result, nearest, dist));
    });

    std::map<label, float> ann_probs;
    cv::Mat_<float> ann_resp(1, class_count_);
    auto task5 = async([&]()->int{
        ann_.predict(sample, ann_resp);
        cv::Point max_loc;
        cv::minMaxLoc(ann_resp, nullptr, nullptr, nullptr, &max_loc);

        /*
        auto iter = std::find_if(
            label2num_.begin(),
            label2num_.end(),
            [=](decltype(*label2num_.begin())& v){
                return max_loc.x == v.second;
        });
        */
        auto iter = label2num_.begin();
        while (iter != label2num_.end()) {
            if (max_loc.x == iter->second) break;
            ++iter;
        }

        assert(iter != label2num_.end());
        return iter->first;
    });

    // wait for finish
    std::array<int, 5> ret {
        task1.get(),
        task2.get(),
        task3.get(),
        task4.get(),
        task5.get()
    };

    // fill ann_probs
    assert(!samples_.empty());
    for (auto iter = samples_.begin(); iter != samples_.end(); ++iter) {
        auto label = iter->first;
        auto prob = ann_resp.at<float>(label2num_[label]);
        ann_probs.insert(std::make_pair(label, prob));
    }

    BOOST_LOG_TRIVIAL(debug) << "ERTrees predict: " << ret[0];
    BOOST_LOG_TRIVIAL(debug) << "Bayes predict: " << ret[1];
    BOOST_LOG_TRIVIAL(debug) << "RandomTrees predict: " << ret[2];
    BOOST_LOG_TRIVIAL(debug) << "KNN predict: " << ret[3];
    BOOST_LOG_TRIVIAL(debug) << "ANN predict: " << ret[4];

    for (int i = 0; i < dist.cols; ++i) {
        BOOST_LOG_TRIVIAL(debug)
            << "KNN neighbors: "
            << label(nearest.at<float>(0, i))
            << " dist: "
            << dist.at<float>(0, i);
    }
    for (auto iter = ann_probs.begin(); iter != ann_probs.end(); ++iter) {
        BOOST_LOG_TRIVIAL(debug)
            << "ANN: "
            << iter->first
            << " probablities: "
            << iter->second;
    }

#ifndef YRUI_TEST
    // if this sample has been trained, return directly.
    if (*dist.begin() == 0) {
        result_set rs;
        rs.insert(std::make_pair(
            static_cast<label>(*nearest.begin()),
            1.f
            ));
        return rs;
    }
#endif

    return vote_result(ret, ann_probs);
}


//! \todo   filter the lower probabilities
Leaf::Leaf_identifier::Leaf_imp::result_set
    Leaf::Leaf_identifier::Leaf_imp::secondary_predict_(
        const cv::Mat& vec,
        const Leaf::Leaf_identifier::Leaf_imp::result_set& rs
    )
{
    BOOST_LOG_TRIVIAL(trace) << "Probability estimation";

    assert(vec.cols == attr_count_);
    if (rs.empty()) return rs;

    namespace Ad = boost::adaptors;
    // retrieve target sample data
    auto class_count = rs.size();
    auto samples = retrieve_portion_sample_(rs | Ad::map_keys | Ad::uniqued);
    cv::Mat_<float> data(samples.size(), attr_count_);
    cv::Mat_<int> response(samples.size(), 1);
    get_sample_resp(MM_view(samples), data, response);

    BOOST_LOG_TRIVIAL(trace)
        << "Training samples: "
        << class_count << " kind => " << data.rows;
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
    cv::Mat_<uchar> var_type(attr_count_ + 1, 1);
    var_type.setTo(cv::Scalar(CV_VAR_ORDERED));
    var_type.at<uchar>(attr_count_) = CV_VAR_CATEGORICAL;
    auto ret1 = async([&]()->int{
        BOOST_LOG_TRIVIAL(trace) << "RandomTrees";
        cv::RandomTrees rt;
        rt.train(data, CV_ROW_SAMPLE, response, {}, {}, var_type, {}, par);
        return cvRound(rt.predict(vec));
    });

    auto ret2 = async([&]()->int{
        BOOST_LOG_TRIVIAL(trace) << "ERandomTrees";
        cv::ERTrees ert;
        ert.train(data, CV_ROW_SAMPLE, response, {}, {}, var_type, {}, par);
        return cvRound(ert.predict(vec));
    });
    /*
    // It always crash in a unpreditable way
    auto ret3 = async([&]()->int{
        BOOST_LOG_TRIVIAL(trace) << "ANN";
        cv::NeuralNet_MLP nn;
        cv::Mat_<float> ann_resp(1, class_count);

        int layer_sz[] { attr_count_, 100, 100, class_count };
        nn.create(cv::Mat_<int>(1, 4, layer_sz));

        cv::Mat_<float> new_response(data.rows, class_count, 0.f);
        assert(std::all_of(
            new_response.begin(),
            new_response.end(),
            [](float x){ return x == 0; }
        ));

        for (int row = 0; row < data.rows; ++row) {
            cv::Mat_<float> cur_row = new_response.row(row);
            auto label = response.at<int>(row);
            assert(label != 0);
            assert(label2num_.count(label));
            cur_row.at<float>(label2num_[label]) = 1.f;
        }

        nn.train(
            data, new_response, {}, {},
            cv::ANN_MLP_TrainParams(
                cv::TermCriteria(CV_TERMCRIT_ITER, 1000, 0.01),
                cv::ANN_MLP_TrainParams::BACKPROP,
                0.001
                )
            );
        nn.predict(vec, ann_resp);
        cv::Point max_loc;
        cv::minMaxLoc(ann_resp, nullptr, nullptr, nullptr, &max_loc);

        auto iter = label2num_.begin();
        while (iter != label2num_.end()) {
            if (max_loc.x == iter->second) break;
            ++iter;
        }

        assert(iter != label2num_.end());
        return iter->first;
    });
    */

    result_set nrs(rs);
    nrs[ret1.get()] += 1.f;
    nrs[ret2.get()] += 1.f;
    //nrs[ret3.get()] += 1.f;
    correctify_result_set(nrs);

    BOOST_LOG_TRIVIAL(trace) << "Secondary result";
    for (auto& item: nrs) {
        BOOST_LOG_TRIVIAL(trace) << item.first << ": " << item.second;
    }
    return nrs;
}

// using binary classification to estimate the probabilities
Leaf::Leaf_identifier::Leaf_imp::result_set
    Leaf::Leaf_identifier::Leaf_imp::finnal_verify_(
        const cv::Mat& fv,
        const Leaf::Leaf_identifier::Leaf_imp::result_set& rs
    )
{
    BOOST_LOG_TRIVIAL(trace) << "Probability verification";
    result_set nrs(rs);

    cv::Mat_<float> data(samples_.size(), attr_count_);
    cv::Mat_<int> response(samples_.size(), 1);
    get_sample_resp(MM_view(samples_), data, response);
    cv::Mat var_type(attr_count_ + 1, 1, CV_8U, cv::Scalar_<uchar>(CV_VAR_ORDERED));
    var_type.at<uchar>(attr_count_) = CV_VAR_CATEGORICAL;
    for (auto& item: nrs) {
        cv::Boost bst;
        auto kind = item.first;
        auto new_resp = response.clone();
        assert(new_resp.rows == data.rows && new_resp.cols == 1);
        for (auto& x: new_resp) {
            x = (x == kind)? 1: -1;
        }
        assert(std::count(new_resp.begin(), new_resp.end(), 1) == samples_.count(item.first));
        bst.train(
            data,
            CV_ROW_SAMPLE,
            new_resp,
            {}, {},
            var_type,
            {},
            cv::BoostParams(cv::Boost::REAL, 10, 0.95, 5, false, nullptr)
            );
        if (bst.predict(fv) == 1) item.second += 1.f;
    }

    correctify_result_set(nrs);

    BOOST_LOG_TRIVIAL(trace) << "Final result";

    for (auto& item: nrs) {
        BOOST_LOG_TRIVIAL(trace) << item.first << ": " << item.second;
    }
    return nrs;
}

//---------------------------------------------------------------
// End of File
//---------------------------------------------------------------
