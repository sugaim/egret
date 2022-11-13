#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/concepts.h"
#include "concepts.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [class] scaled_evaluator
// -----------------------------------------------------------------------------
    template <typename Base>
    class scaled_evaluator {
    private:
        using this_type = scaled_evaluator;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        scaled_evaluator() = delete;
        scaled_evaluator(const this_type&)
            requires std::is_copy_constructible_v<Base> = default;
        scaled_evaluator(this_type&&)
            noexcept(std::is_nothrow_move_constructible_v<Base>)
            requires std::is_move_constructible_v<Base> = default;

        scaled_evaluator(const Base& base, double scale)
            requires std::is_copy_constructible_v<Base>
            : base_(base), scale_(scale)
        {
        }

        scaled_evaluator(Base&& base, double scale)
            noexcept(std::is_nothrow_move_constructible_v<Base>)
            requires std::is_move_constructible_v<Base>
            : base_(std::move(base)), scale_(scale)
        {
        }

        this_type& operator =(const this_type&)
            requires std::is_copy_assignable_v<Base> = default;
        this_type& operator =(this_type&&)
            noexcept(std::is_nothrow_move_assignable_v<Base>)
            requires std::is_move_assignable_v<Base> = default;

    // -------------------------------------------------------------------------
    //  constraint_component behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires cpt::yield_curve_evaluator<Base, RateTag, Curve>
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
            -> evaluate_result_t<Base, RateTag, Curve>
        {
            using result_t = evaluate_result_t<Base, RateTag, Curve>;
            return result_t {scale_ * yc::evaluate(base_, vdt, curves)};
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const Base& base_component() const noexcept { return base_; }
        double multiplier() const noexcept { return scale_; }
    
    private:
        Base base_;
        double scale_;

    }; // class scaled_component

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename Base>
    struct adl_serializer<egret::fit::yc::scaled_evaluator<Base>> {
        
        using target_type = egret::fit::yc::scaled_evaluator<Base>;

        template <egret::cpt::deserializable_json_with<egret::util::j2obj::get_t<Base>> Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            constexpr auto deser = j2obj::construct<target_type>(
                "base" >> j2obj::get<Base>,
                "multiplier" >> j2obj::number
            );
            return deser(j);
        }
        template <typename Json>
            requires std::is_assignable_v<Json&, const Base&>
        static void to_json(Json& j, const target_type& obj)
        {
            j["base"] = obj.base_component();
            j["multiplier"] = obj.multiplier();
        }
    };

} // namespace nlohmann
