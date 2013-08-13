#ifndef YRUI_LEAF_IDENTIFIER_H_
#define YRUI_LEAF_IDENTIFIER_H_

#include <vector>
#include <map>
#include <string>

/*!
 * \defgroup    Identify    The identifier module of Leaf.
 */

/*!
 * \file        Leaf_identifier.h
 * \ingroup     Identify
 * \brief       The main module of the Leaf, which is used to identify leaves based
 *              on existing feature vectors.
 */

namespace Leaf {

/*!
 * \brief   the main identifier class.
 * \note    singleton.
 *
 * This object is used to parse the feature vector based on existing
 * dataset. It will not take the concret type of what to identify and
 * simply load feature vectors and classify the incoming sample vector
 * using machine learning theory.
 *
 * Generally speaking, this object is stuff-independent.
 *
 * Currently, this object will use a hard-coded way to load training set
 * and classifiers. The utimate goal is config these as a config file.
 */
class Leaf_identifier {
//! \name   Types
//! @{
public:
    /*!
     * \brief   feature vector used by identify functions.
     */
    typedef std::vector<float> feature_vector;

    /*!
     * \brief   labels used by classification.
     */
    typedef std::size_t label;

    /*!
     * \brief   multiple result set.
     */
    typedef std::map<label, float> result_set;

    /*!
     * \brief   result entry returned by the identify functions in the form
     *          of (result_label, probability)
     */
    typedef result_set::value_type result_entry;

    /*!
     * \brief   proxy classes used to iterate over sample datas.
     */
    class Feature_view;

//! @}

//! \name   Structors
//! @{
private:
    /*!
     * \brief   singleton getter.
     */
    friend Leaf_identifier* identifier();

    Leaf_identifier();

    //! \name   deleted
    //! @{
    Leaf_identifier(const Leaf_identifier&);
    Leaf_identifier(Leaf_identifier&&);

    Leaf_identifier& operator=(const Leaf_identifier&);
    Leaf_identifier& operator=(Leaf_identifier&&);
    //! @}

public:
    ~Leaf_identifier();
//! @}

//! \name   Operations
//! @{
public:
    /*!
     * \brief   initialization routine of the identifier module.
     * \param   [in] config     config file path.
     * \note    this function should be called before any call to others, and
     *          it can only be called once, the extra calls will make no sense.
     *
     */
    void init(const std::string& config);

    /*!
     * \brief   return if the identifier is initialized.
     */
    bool is_init() const;

    /*!
     * \brief   return if the identifier is usable(namely is trained).
     * \pre     is_init().
     */
    bool is_ok() const;

    /*!
     * \brief   train classifiers using feature vectors specified in the Feature_view.
     * \pre     !is_ok().
     * \post    is_ok().
     * \note    the data will be stored.
     */
    void train(const Feature_view&);

    /*!
     * \brief   clear trained classifiers.
     * \post    !is_ok();
     */
    void clear();

    /*!
     * \brief   parse \a fv, and return possible result.
     * \throw   std::runtime_error if this object is not initialized.
     * \pre     the classifier is properly trained.
     */
    result_set identify(const feature_vector& fv);
//! @}

//! \name   Implementation
//! @{
private:
    class Leaf_imp;
    Leaf_imp* imp_;
//! @}
};

Leaf_identifier* identifier();

}  // of namespace Leaf

#endif  //!YRUI_LEAF_IDENTIFIER_H_
