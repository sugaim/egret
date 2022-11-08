#pragma once

#include <vector>
#include <nlohmann/json_fwd.hpp>
#include "concepts.h"
#include "core/math/algebra/concepts.h"
#include "core/utils/range_utils/vector_assign.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/type_traits/special_functions_properties.h"
#include "core/concepts/constructible_to.h"
#include "core/assertions/assertion.h"
#include "slopes/concepts.h"
#include "auxiliary/find_index_and_relpos.h"
#include "auxiliary/interpolatee_validation.h"
#include "auxiliary/integrate_impl.h"
#include "auxiliary/json_deserializer_impl.h"
#include "slopes/any_slope_generator.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] cspline
// -----------------------------------------------------------------------------
    template <
        distance_measurable X, distance_measurable Y,
        slope_generator<
            std::ranges::iterator_t<const std::vector<X>>, 
            std::ranges::sentinel_t<const std::vector<X>>,
            std::ranges::iterator_t<const std::vector<Y>>,
            std::back_insert_iterator<std::vector<Y>>
        > SlopeGenerator = any_slope_generator<
            std::ranges::iterator_t<const std::vector<X>>, 
            std::ranges::sentinel_t<const std::vector<X>>,
            std::ranges::iterator_t<const std::vector<Y>>,
            std::back_insert_iterator<std::vector<Y>>
        >,
        typename Less = std::ranges::less
    >
    class cspline {
    private:
        using this_type = cspline;
        using spfn_props = special_function_properties<std::vector<X>, std::vector<Y>, SlopeGenerator, Less>;

        static_assert(
            relpos_computable<X>, 
            "Relative position must be defined for grids.");
        static_assert(
            std::strict_weak_order<const Less&, const X&, const X&>, 
            "Less must be a comparison of grids");
        static_assert(
            std::common_with<relpos_t<X>, Y>, 
            "Grid and value must have a common type as result type.");
        static_assert(
            cpt::module<std::common_type_t<relpos_t<X>, Y>, relpos_t<X>>, 
            "Relative position type of grids must be a module coefficient of reuslt type.");
            
    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        cspline() = delete;
        cspline(const this_type&)
            requires spfn_props::are_copy_constructible_v = default;
        cspline(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <
            cpt::constructible_to<std::vector<X>> AXs, 
            cpt::constructible_to<std::vector<Y>> AYs,
            cpt::constructible_to<SlopeGenerator> SG,
            cpt::constructible_to<Less> ALess
        >
        cspline(AXs&& xs, AYs&& ys, SG&& slope_generator, ALess&& less)
            : grids_(std::forward<AXs>(xs)),
              values_(std::forward<AYs>(ys)),
              slope_generator_(std::forward<SG>(slope_generator)),
              less_(std::forward<ALess>(less)),
              coefficients_(),
              slope_buffer_()
        {
            egret_detail::interp1d_impl::interpolatee_validation(grids_, values_, less_);
            this->_calculate_coefficients();
        }

        template <
            cpt::constructible_to<std::vector<X>> AXs, 
            cpt::constructible_to<std::vector<Y>> AYs,
            cpt::constructible_to<SlopeGenerator> SG
        >
        cspline(AXs&& xs, AYs&& ys, SG&& slope_generator)
            requires std::is_default_constructible_v<Less>
            : cspline(std::forward<AXs>(xs), std::forward<AYs>(ys), std::forward<SG>(slope_generator), Less{})
        {
        }

        this_type& operator =(const this_type&)
            requires spfn_props::are_copy_assignable_v = default;
        this_type& operator =(this_type&&)
            noexcept(spfn_props::are_nothrow_move_assignable_v)
            requires spfn_props::are_move_assignable_v = default;

    // -------------------------------------------------------------------------
    //  interpolatotion1d behavior
    //
        const std::vector<X>& grids() const noexcept { return grids_; }
        const std::vector<Y>& values() const noexcept { return values_; }

        template <relpos_computable_from<X> AX>
            requires 
                std::strict_weak_order<const Less&, const X&, const AX&> &&
                std::common_with<relpos_t<AX, X>, Y> &&
                cpt::module<std::common_type_t<relpos_t<AX, X>, Y>, relpos_t<AX, X>>
        auto operator()(const AX& x) const
            -> std::common_type_t<relpos_t<AX, X>, Y>
        {
            const auto [idx, w] = egret_detail::interp1d_impl::find_index_and_relpos(grids_, x, less_);
            return static_cast<std::common_type_t<relpos_t<AX, X>, Y>>(
                coefficients_[4 * idx]
                + coefficients_[4 * idx + 1] * w
                + coefficients_[4 * idx + 2] * (w * w / 2)
                + coefficients_[4 * idx + 3] * (w * w * w / 6)
            );
        }

    // -------------------------------------------------------------------------
    //  mutable behavior
    //
        template <cpt::forward_range_of<X> Xs, cpt::forward_range_of<Y> Ys>
        void initialize(Xs&& xs, Ys&& ys)
        {
            egret_detail::interp1d_impl::interpolatee_validation(xs, ys, less_);
            util::vector_assign(grids_, std::forward<Xs>(xs));
            util::vector_assign(values_, std::forward<Ys>(ys));
            this->_calculate_coefficients();
        }

        template <typename AY>
            requires std::is_assignable_v<Y&, AY>
        void update(std::size_t i, AY&& value)
        {
            egret::assertion(i < values_.size(), "Assigned to out of range element. [size={}, index={}]", values_.size(), i);
            values_[i] = std::forward<AY>(value);
            this->_calculate_coefficients();
        }

    // -------------------------------------------------------------------------
    //  der
    //
        template <relpos_computable_from<X> AX>
            requires 
                std::strict_weak_order<const Less&, const X&, const AX&> &&
                std::common_with<relpos_t<AX, X>, Y> &&
                cpt::module<std::common_type_t<relpos_t<AX, X>, Y>, relpos_t<AX, X>>
        auto der1(const X& x) const
            -> std::common_type_t<relpos_t<AX, X>, Y>
        {
            const auto [xlit, xrit] = util::find_interval(grids_, x, less_);
            const auto idx = static_cast<std::size_t>(xlit - grids_.begin());
            const auto xdist = interp1d::distance(*xlit, *xrit);
            const auto w = interp1d::distance(*xlit, x) / xdist;
            return static_cast<std::common_type_t<relpos_t<AX, X>, Y>>(
                (
                    coefficients_[4 * idx + 1]
                    + coefficients_[4 * idx + 2] * w
                    + coefficients_[4 * idx + 3] * (w * w / 2)
                ) / xdist
            );
        }

        template <relpos_computable_from<X> AX>
            requires 
                std::strict_weak_order<const Less&, const X&, const AX&> &&
                std::common_with<relpos_t<AX, X>, Y> &&
                cpt::module<std::common_type_t<relpos_t<AX, X>, Y>, relpos_t<AX, X>>
        auto der2(const X& x) const
            -> std::common_type_t<relpos_t<AX, X>, Y>
        {
            const auto [xlit, xrit] = util::find_interval(grids_, x, less_);
            const auto idx = static_cast<std::size_t>(xlit - grids_.begin());
            const auto xdist = interp1d::distance(*xlit, *xrit);
            const auto w = interp1d::distance(*xlit, x) / xdist;
            return static_cast<std::common_type_t<relpos_t<AX, X>, Y>>(
                (
                    coefficients_[4 * idx + 2]
                    + coefficients_[4 * idx + 3] * w
                ) / (xdist * xdist)
            );
        }

    // -------------------------------------------------------------------------
    //  integrate
    //
        template <distance_measurable AX>
            requires 
                std::strict_weak_order<const Less&, const X&, const AX&> &&
                std::constructible_from<AX, const X&> &&
                cpt::module<std::common_type_t<relpos_t<AX, X>, Y>, relpos_t<AX, X>> &&
                cpt::vector<std::common_type_t<distance_result_t<AX>, Y>, distance_result_t<AX>>
        constexpr auto integrate(const AX& from, const AX& to) const
            -> std::common_type_t<distance_result_t<X>, Y>
        {
            using result_t = std::common_type_t<distance_result_t<AX>, Y>;

            struct partial_integrator_t {
                constexpr result_t operator()() const
                {
                    auto dist = interp1d::distance(static_cast<AX>(self->grids()[idx]), static_cast<AX>(self->grids()[idx + 1]));
                    return static_cast<result_t>(std::move(dist) * (
                        self->coefficients_[4 * idx]
                        + self->coefficients_[4 * idx + 1] / 2
                        + self->coefficients_[4 * idx + 2] / 6
                        + self->coefficients_[4 * idx + 3] / 24
                    ));
                }
                constexpr result_t operator()(const AX& f, const AX& t) const
                {
                    auto interval_length = interp1d::distance(self->grids()[idx], self->grids()[idx + 1]);
                    const auto wf = interp1d::distance(self->grids()[idx], f) / interval_length;
                    const auto wt = interp1d::distance(self->grids()[idx], t) / interval_length;
                    const auto wsum = wt + wf;
                    const auto wtwf = wt * wf;
                    auto dist = interp1d::distance(f, t);
                    return static_cast<result_t>(std::move(dist) * (
                        self->coefficients_[4 * idx]
                        + self->coefficients_[4 * idx + 1] * (wsum / 2)
                        + self->coefficients_[4 * idx + 2] * ((wsum * wsum - wtwf) / 6)
                        + self->coefficients_[4 * idx + 3] * (wsum * (wsum * wsum - 2 * wtwf) / 24)                    
                    ));
                }

                const std::size_t idx;
                const this_type* self;
            };
            // for x < grids.front()
            const auto left_extrap = partial_integrator_t {
                0, 
                this
            };

            // for grids.back() <= x
            const auto right_extrap = partial_integrator_t {
                static_cast<std::size_t>(static_cast<long>(grids_.size()) - 2), 
                this
            };

            // for grids.front() <= x < grids.back() with partial interval
            const auto internal_partial = [this](
                const auto& xlit, const auto&, const auto&, const auto&,
                const X& f, const X& t
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    static_cast<std::size_t>(xlit - this->grids().begin()),
                    this
                };
                return integrator(f, t);
            };

            // for grids.front() <= x < grids.back() with full interval
            const auto internal_full = [this](
                const auto& xlit, const auto&, const auto&, const auto&
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    static_cast<std::size_t>(xlit - this->grids().begin()),
                    this
                };
                return integrator();
            };

            const auto& [f, t] = std::minmax(from, to, less_);
            const auto sign = static_cast<result_t>(less_(to, from) ? -1 : 1);
            return static_cast<result_t>(
                sign * egret_detail::interp1d_impl::integrate_impl<result_t>(
                    left_extrap, internal_partial, internal_full, right_extrap,
                    grids_, values_, f, t, less_
                )
            );
        }

    // -------------------------------------------------------------------------
    //  get
    //
        const SlopeGenerator& slope_generator() const noexcept { return slope_generator_; }
        const Less& less() const noexcept { return less_; }

    private:
        std::vector<X> grids_;
        std::vector<Y> values_;
        SlopeGenerator slope_generator_;
        Less less_;

        std::vector<Y> coefficients_;
        std::vector<Y> slope_buffer_;

        void _calculate_coefficients()
        {
            slope_buffer_.clear();
            slope_buffer_.reserve(grids_.size());
            slope_generator_.generate(grids_.begin(), grids_.end(), values_.begin(), std::back_inserter(slope_buffer_));

            egret::assertion(
                slope_buffer_.size() == grids_.size(), 
                "Fail to generate slopes for cubic spline because of result slope size. "
                "[expected={}, actual={}]",
                grids_.size(), slope_buffer_.size()
            );
            const auto n_interval = static_cast<std::size_t>(static_cast<long>(slope_buffer_.size()) - 1);
            coefficients_.clear();
            coefficients_.reserve(4 * n_interval);

            for (std::size_t i = 0; i < n_interval; ++i) {
                const auto dx = interp1d::distance(grids_[i], grids_[i + 1]);
                const auto dy = values_[i + 1] - values_[i];
                const auto ml = slope_buffer_[i];
                const auto mr = slope_buffer_[i + 1];
                coefficients_.push_back(values_[i]);
                coefficients_.push_back(ml * dx);
                coefficients_.push_back(3 * dy - (mr + 2 * ml) * dx);
                coefficients_.push_back((mr + ml) * dx - 2 * dy);
            }
            slope_buffer_.clear();
        }

    }; // class cspline

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename SG>
    cspline(Xs, Ys, SG) 
        -> cspline<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, SG>;

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename SG, typename Less>
    cspline(Xs, Ys, SG, Less)
        -> cspline<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, SG, Less>;
        
} // namespace egret::math::interp1d

namespace nlohmann {
    template <typename X, typename Y, typename SG, typename Less>
    struct adl_serializer<egret::math::interp1d::cspline<X, Y, SG, Less>> {
        using target_type = egret::math::interp1d::cspline<X, Y, SG, Less>;

        template <typename Json>
            //requires std::is_default_constructible_v<Less> && requires (const Json& j) {
            //    { j.template get<X>() } -> std::convertible_to<X>;
            //    { j.template get<Y>() } -> std::convertible_to<Y>;
            //    { j.template get<SG>() } -> std::convertible_to<SG>;
            //}
        static target_type from_json(const Json& j)
        {
            namespace impl = egret_detail::interp1d_impl;
            auto [xs, ys] = impl::recover_knots<X, Y>(j);
            auto slope_gen = ("slope_generator" >> egret::util::j2obj::get<SG>)(j);
            return target_type{std::move(xs), std::move(ys), std::move(slope_gen)};
        }

        template <typename Json>
            //requires requires (Json& j, const X& x, const Y& y, const SG& sg) {
            //    j = x;
            //    j = y;
            //    j = sg;
            //}
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
            j["slope_generator"] = obj.slope_generator();
        }
    };

} // namespace nlohmann
