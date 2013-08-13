#ifndef YRUI_LEAF_IDENTIFIER_IMP_H_
#define YRUI_LEAF_IDENTIFIER_IMP_H_

#include "Precompile.h"
#include "Leaf_identifier.h"
#include <boost/property_tree/ptree.hpp>

/*!
 * \defgroup    Impl    Implementation of Leaf.
 * \ingroup     Identify.
 */

/*!
 * \file     Leaf_identifier_imp.h.
 * \ingroup  Impl.
 * \brief    Implementation interface of class Leaf_identifier.
 */
namespace Leaf {

/*!
 * \brief       Identifier implementation class.
 */
class Leaf_identifier::Leaf_imp {
//! \name   Types
//! @{
public:
    typedef Leaf_identifier::feature_vector feature_vector;
    typedef Leaf_identifier::label label;
    typedef Leaf_identifier::result_entry result_entry;
    typedef Leaf_identifier::result_set result_set;
//! @}

//! \name   Structors
//! @{
public:
    /*!
     * \brief   construct a impl object using specific config file.
     * If there are any existing classifiers, load them, otherwise read training set
     * and train classifiers concurrently.
     */
    Leaf_imp(const std::string& config);
//! @}

//! \name   Operation
//! @{
public:
    /*!
     * \brief   return if the identifier is trained.
     */
    bool is_ok() const { return is_trained_; }

    /*!
     * \brief   train the identifier.
     * \pre     !is_ok().
     * \post    is_ok().
     */
    void train(const Leaf_identifier::Feature_view&);

    void clear();

    result_set identify(const feature_vector& fv);
//! @}

//! \name   Inner helpers
//! @{
private:
    /*!
     * \brief   init the instance.
     */
    void init_(const std::string& config);

    /*!
     * \brief   top level filter operation using pre-trained classifiers.
     */
    result_set primary_filter_(const cv::Mat& fv);

    /*!
     * \brief   elaborate to predict probabilities.
     */
    result_set secondary_predict_(
            const cv::Mat& fv,
            const result_set& rs
            );

    /*!
     * \brief   calculate the result from secondary_predict_() and
     *          verify their probabilites.
     */
    result_set finnal_verify_(
            const cv::Mat& fv,
            const result_set& rs
            );
    /*!
     * \brief   clear all existing classifiers and train.
     */
    void train_imp_(const cv::Mat_<float>& sample, const cv::Mat_<int>& response);

    void save_();

    template <class Range>
    std::multimap<label, feature_vector>
        retrieve_portion_sample_(const Range& r);
//! @}

//! \name   Representation
//! @{
private:
    bool is_trained_;

    int class_count_;
    int attr_count_;
    int k_;

    std::string config_file_;
    boost::property_tree::ptree config_;

    //cv::SVM svm_;
    cv::ERTrees ert_;
    cv::NormalBayesClassifier bayes_;
    cv::RandomTrees rt_;
    cv::KNearest knn_;
    cv::NeuralNet_MLP ann_;

    std::map<label, int> label2num_;
    std::multimap<label, feature_vector> samples_;
//! @}
};

template <class Range>
std::multimap<
    Leaf_identifier::label,
    Leaf_identifier::feature_vector
    >
    Leaf_identifier::Leaf_imp::retrieve_portion_sample_(const Range& r)
{
    std::multimap<label, feature_vector> ret;
    for (auto l: r) {
        auto range = samples_.equal_range(l);
        ret.insert(range.first, range.second);
    }
    return ret;
}

void get_sample_resp(
    const Leaf::Leaf_identifier::Feature_view& view,
    cv::Mat_<float>& sample,
    cv::Mat_<int>& response
    );

}  // of namespace Leaf


#endif  //!YRUI_LEAF_IDENTIFIER_IMP_H_
