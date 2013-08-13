#ifndef YRUI_LEAF_FEATURE_VIEW_H_
#define YRUI_LEAF_FEATURE_VIEW_H_

#include "Leaf_identifier.h"
#include <memory>
#include <cassert>

/*!
 * \file    Feature_view.h
 * \ingroup Identify.
 * \brief   Define Feature_view interface.
 */

namespace Leaf {

class Leaf_identifier::Feature_view {
public:
    typedef Leaf_identifier::label label;
    typedef Leaf_identifier::feature_vector feature_vector;
    typedef const std::pair<label, const feature_vector&> value_type;

    class Const_iterator_proxy :
        public std::iterator<
                            std::input_iterator_tag,
                            value_type
        > {
    public:
        class Const_view_iterator {
        public:
            typedef Const_iterator_proxy::value_type value_type;
            typedef Const_iterator_proxy::reference reference;

            virtual ~Const_view_iterator() {}

            virtual void incr() = 0;
            virtual value_type deref() = 0;

            virtual bool equals(const Const_view_iterator* iter) = 0;
        };

        Const_iterator_proxy(Const_view_iterator* iter)
            : iter_(iter)
        {
        }

        Const_iterator_proxy& operator++()
        {
            assert(iter_);
            iter_->incr();
            return *this;
        }

        /*!
         * \note    value_type of this object cannot be provided by the imp type.
         */
        value_type operator*() const
        {
            assert(iter_);
            return iter_->deref();
        }

        bool operator==(const Const_iterator_proxy& other) const
        {
            assert(iter_);
            assert(other.iter_);
            return iter_->equals(other.iter_.get());
        }

        bool operator!=(const Const_iterator_proxy& other) const
        {
            return !(*this == other);
        }

    private:
        std::shared_ptr<Const_view_iterator> iter_;
    };

    typedef Const_iterator_proxy const_iterator;
    typedef const_iterator::reference reference;

    virtual ~Feature_view() {}

    virtual std::size_t size() const = 0;
    virtual bool empty() const = 0;

    virtual std::vector<feature_vector> get(label l) const = 0;

    virtual const_iterator begin() const = 0;
    virtual const_iterator end() const = 0;
};

}  // of namespace Leaf

#endif  //!YRUI_LEAF_FEATURE_VIEW_H_
