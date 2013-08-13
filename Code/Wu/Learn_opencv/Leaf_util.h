#ifndef YRUI_LEAF_UTIL_H_
#define YRUI_LEAF_UTIL_H_

#include "Feature_view.h"
#include <algorithm>

/*!
 * \file        Leaf_util.h
 * \ingroup     Identify.
 * \brief       std::multimap version of Feature_view.
 */

namespace Leaf {

namespace Detail {

typedef std::multimap<
        Leaf_identifier::label,
        Leaf_identifier::feature_vector
    > imp_type;

}  // of namespace Detail

class MM_iterator :
    public Leaf_identifier::Feature_view::
           Const_iterator_proxy::Const_view_iterator {
public:
    MM_iterator(Detail::imp_type::const_iterator iter) : iter_(iter) {}

    virtual void incr() override { ++iter_; }
    virtual value_type deref() override
    {
        return value_type(iter_->first, iter_->second);
    }

    virtual bool equals(const Const_view_iterator* iter) override
    {
        if (auto imp = dynamic_cast<const MM_iterator*>(iter)) {
            return imp->iter_ == iter_;
        }
        return false;
    }

private:
    Detail::imp_type::const_iterator iter_;
};

class MM_view : public Leaf_identifier::Feature_view {
public:
    MM_view(Detail::imp_type& m) : mm_(m) {}

    virtual std::size_t size() const override { return mm_.size(); }
    virtual bool empty() const override { return mm_.empty(); }

    virtual std::vector<Leaf_identifier::feature_vector>
        get(Leaf_identifier::label l) const override
    {
        std::vector<Leaf_identifier::feature_vector> fvs;

        auto range = mm_.equal_range(l);
        std::for_each(range.first, range.second, [&](decltype(*range.first)& v){
            fvs.push_back(v.second);
        });

        return fvs;
    }

    virtual Const_iterator_proxy begin() const override
    {
        return Const_iterator_proxy(new MM_iterator(mm_.begin()));
    }

    virtual Const_iterator_proxy end() const override
    {
        return Const_iterator_proxy(new MM_iterator(mm_.end()));
    }

private:
    Detail::imp_type& mm_;
};

}

#endif  //!YRUI_LEAF_UTIL_H_
