#pragma once

#include <iterator>
#include <memory>
#include <typeindex>
#include "core/concepts/different_from.h"
#include "core/utils/maybe.h"
#include "concepts.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] any_slope_generator
// -----------------------------------------------------------------------------
    template <
        std::forward_iterator XIt, std::sentinel_for<XIt> XSn, 
        std::forward_iterator YIt,
        std::output_iterator<std::iter_value_t<YIt>> OIt
    >
    class any_slope_generator {
    private:
        using this_type = any_slope_generator;

        struct base {
            virtual ~base() = default;
            virtual OIt generate(XIt xit, const XSn& xsn, YIt yit, OIt oit) const = 0;
            virtual std::type_index type() const noexcept = 0;
            virtual const void* pointer() const noexcept = 0;
        };

        template <typename C>
        struct concrete final : base {
            concrete(C obj) : obj_(std::move(obj)) {}
            OIt generate(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
            { 
                return obj_.generate(std::move(xit), xsn, std::move(yit), std::move(oit)); 
            }
            std::type_index type() const noexcept { return typeid(C); }
            const void* pointer() const noexcept { return std::addressof(obj_); }

            C obj_;
        };

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        any_slope_generator() = delete;
        any_slope_generator(const this_type&) noexcept = default;
        any_slope_generator(this_type&&) noexcept = default;

        template <slope_generator<XIt, XSn, YIt, OIt> C>
            requires cpt::different_from<C, this_type>
        any_slope_generator(C&& obj)
            : obj_(std::make_shared<concrete<std::remove_cvref_t<C>>>(std::forward<C>(obj)))
        {
        }

        this_type& operator =(const this_type&) noexcept = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  slope_generator
    //
        OIt generate(XIt xit, const XSn& xsn, YIt yit, OIt oit) const
        {
            return obj_->generate(std::move(xit), xsn, std::move(yit), std::move(oit));
        }

    // -------------------------------------------------------------------------
    //  type_erasure behavior
    //
        std::type_index type() const noexcept { return obj_->type(); }

        template <slope_generator<XIt, XSn, YIt, OIt> C>
        util::maybe<const C&> as() const noexcept
        {
            if (this->type() == typeid(C)) {
                return util::maybe<const C&>(*reinterpret_cast<const C*>(obj_->pointer()));
            }
            else {
                return util::maybe<const C&>();
            }
        }
        
    private:
        std::shared_ptr<const base> obj_;

    }; // class any_slope_generator

} // namespace egret::math::interp1d
