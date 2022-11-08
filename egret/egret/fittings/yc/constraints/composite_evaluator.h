#pragma once

#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "core/utils/json_utils/j2obj.h"
#include "egret/models/curves/concepts.h"
#include "concepts.h"

namespace egret::fit::yc {
// -----------------------------------------------------------------------------
//  [class] composite_evaluator
// -----------------------------------------------------------------------------
    template <typename Component>
    class composite_evaluator {
    private:
        using this_type = composite_evaluator;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        composite_evaluator() = delete;
        composite_evaluator(const this_type&) = default;
        composite_evaluator(this_type&&) noexcept = default;

        explicit composite_evaluator(std::vector<Component> comps) noexcept
            : comps_(std::move(comps))
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  yield_curve_evaluator behavior
    //
        template <typename RateTag, cpt::yield_curve Curve>
            requires cpt::yield_curve_evaluator<Component, RateTag, Curve>
        auto evaluate(const std::chrono::sys_days& vdt, const std::map<RateTag, Curve>& curves) const
        {
            using result_t = evaluate_result_t<Component, RateTag, Curve>;
            result_t result {0};
            for (const auto& comp : comps_) {
                result += yc::evaluate(comp, vdt, curves);
            }
            return result;
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const std::vector<Component>& components() const noexcept { return comps_; }

    private:
        std::vector<Component> comps_;

    }; // class composite_evaluator

} // namespace egret::fit::yc

namespace nlohmann {
    template <typename Component>
    struct adl_serializer<egret::fit::yc::composite_evaluator<Component>> {

        using target_type = egret::fit::yc::composite_evaluator<Component>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            return target_type(j2obj::array.to_vector_of<Component>()(j));
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            j = obj.components();
        }
    };

} // namespace nlohmann