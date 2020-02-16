#pragma once

#include <memory>
#include <functional>
#include "cm_ctors.h"
#include "type_checks.h"

//not sure how to say ... some libraries want pointer to pointer on call,
//but we want to use shared_ptr .. so, here is wrapper around shared_ptr
//which does temporary pointer

template <class Src>
class shared_ptr_tmp_wrapper
{
public:
    static_assert(types_ns::issharedptr<Src>::value, "Expected std::shared_ptr");
    using SrcPtr = typename Src::element_type*;
    using Del = std::function<void(SrcPtr)>;

    STACK_ONLY;
    NO_COPYMOVE(shared_ptr_tmp_wrapper);
    shared_ptr_tmp_wrapper() = delete;
    shared_ptr_tmp_wrapper(Src& shared, Del del):
        refp(&shared),
        refd(del)
    {

    }

    operator SrcPtr*()
    {
        tmp = refp->get();
        return &tmp;
    }

    ~shared_ptr_tmp_wrapper()
    {
        *refp = Src(tmp, std::move(refd));
    }

private:
    SrcPtr tmp{nullptr};
    Src* refp{nullptr};
    Del  refd;
};
