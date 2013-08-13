#include "Precompile.h"
#include "Leaf_identifier.h"
#include "Leaf_identifier_imp.h"
#include <stdexcept>

/*!
 * \file    Leaf_identifier.cpp
 * \brief   Implementation of Leaf_identifier.
 * \ingroup Identify.
 */

namespace {

inline void validate_this(bool b)
{
    if (!b) {
        throw std::runtime_error("leaf_identifier does not init properly");
    }
}

inline void validate_this(void* p)
{
    return validate_this(p != nullptr);
}

}  // of namespace unnamed

Leaf::Leaf_identifier::Leaf_identifier()
    : imp_()
{
}

Leaf::Leaf_identifier::~Leaf_identifier()
{
    // delete nullptr is valid
    delete imp_;
    imp_ = nullptr;
}

void Leaf::Leaf_identifier::init(
        const std::string& config
    )
{
    if (!imp_) {
        imp_ = new Leaf_imp(config);
    }
}

bool Leaf::Leaf_identifier::is_init() const
{
    return imp_ != nullptr;
}

bool Leaf::Leaf_identifier::is_ok() const
{
    validate_this(imp_);
    return imp_->is_ok();
}

void Leaf::Leaf_identifier::clear()
{
    validate_this(imp_);
    return imp_->clear();
}

void Leaf::Leaf_identifier::train(
    const Leaf::Leaf_identifier::Feature_view& view
    )
{
    validate_this(imp_);
    return imp_->train(view);
}

Leaf::Leaf_identifier::result_set
    Leaf::Leaf_identifier::identify(
        const Leaf::Leaf_identifier::feature_vector& fv
        )
{
    validate_this(imp_);
    validate_this(is_ok());
    return imp_->identify(fv);
}

Leaf::Leaf_identifier* Leaf::identifier()
{
    static Leaf_identifier instance;
    return &instance;
}


//---------------------------------------------------------------------------
// End of File
//---------------------------------------------------------------------------
