#pragma once

#include <ranges>
#include <utility>
#include "core/utils/member_data.h"
#include "core/concepts/qualified.h"
#include "core/concepts/range_of.h"
#include "core/utils/range_utils/find_interval.h"
#include "core/utils/range_utils/vector_assign.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/type_traits/special_functions_properties.h"

#include "concepts.h"
#include "auxiliary/interpolatee_validation.h"
#include "auxiliary/find_index_and_relpos.h"
#include "auxiliary/integrate_impl.h"
#include "auxiliary/interval_at.h"
#include "auxiliary/json_deserializer_impl.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] generic_linear
// -----------------------------------------------------------------------------
    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, std::semiregular Less = std::ranges::less>
        requires 
            distance_measurable<std::ranges::range_reference_t<Xs>> && 
            distance_measurable<std::ranges::range_reference_t<Ys>> &&
            std::strict_weak_order<const Less&, std::ranges::range_reference_t<Xs>, std::ranges::range_reference_t<Xs>>
    class generic_linear {
    private:
        using this_type = generic_linear;
        using spfn_props = special_function_properties<util::member_data<Xs>, util::member_data<Ys>, Less>;
        using grid_type = std::ranges::range_value_t<const Xs>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        generic_linear() = delete;
        constexpr generic_linear(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_constructible_v)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr generic_linear(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <
            cpt::constructible_to<util::member_data<Xs>> AXs, 
            cpt::constructible_to<util::member_data<Ys>> AYs
        >
        constexpr generic_linear(AXs&& xs, AYs&& ys, Less less)
            : grids_(std::forward<AXs>(xs)),
              values_(std::forward<AYs>(ys)),
              less_(std::move(less))
        {
            egret_detail::interp1d_impl::interpolatee_validation(grids_.get(), values_.get(), less_);
        }

        template <
            cpt::constructible_to<util::member_data<Xs>> AXs, 
            cpt::constructible_to<util::member_data<Ys>> AYs
        >
        constexpr generic_linear(AXs&& xs, AYs&& ys)
            requires std::is_default_constructible_v<Less>
            : generic_linear(std::forward<AXs>(xs), std::forward<AYs>(ys), Less{})
        {
        }

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&)
            noexcept(spfn_props::are_nothrow_move_assignable_v)
            requires spfn_props::are_move_assignable_v = default;

    // -------------------------------------------------------------------------
    //  interpolatotion1d behavior
    //
        constexpr const Xs& grids() const noexcept { return grids_.get(); }
        constexpr const Ys& values() const noexcept { return values_.get(); }

        template <distance_measurable_from<grid_type> X>
            requires std::strict_weak_order<const Less&, const grid_type&, const X&>
        constexpr auto operator()(const X& x) const
        {
            const auto [idx, w] = egret_detail::interp1d_impl::find_index_and_relpos(grids_.get(), x, less_);
            const auto [ylit, yrit] = egret_detail::interp1d_impl::interval_at(idx, values_.get());
            return *ylit * (1 - w) + *yrit * w;
        }

    // -------------------------------------------------------------------------
    //  der
    //
        template <distance_measurable_from<grid_type> X>
            requires std::strict_weak_order<const Less&, const grid_type&, const X&>
        constexpr auto der1(const X& x) const
        {
            const auto [xlit, xrit] = util::find_interval(grids_.get(), x, less);
            const auto idx = std::ranges::distance(std::ranges::begin(grids_.get()), xlit);
            const auto [ylit, yrit] = egret_detail::interp1d_impl::interval_at(idx, values_.get());
            return (*yrit - *ylit) / interp1d::distance(*xlit, *xrit);
        }

        template <distance_measurable_from<grid_type> X>
            requires std::predicate<const Less&, const grid_type&, const X&>
        constexpr auto der2(const X& x) const
        {
            using result_type = decltype(this->der1(x));
            return static_cast<result_type>(0);
        }

    // -------------------------------------------------------------------------
    //  integrate
    //
        template <distance_measurable_from<grid_type> X>
            requires 
                std::strict_weak_order<const Less&, const grid_type&, const X&> &&
                std::constructible_from<X, std::ranges::range_reference_t<const Xs>>
        constexpr auto integrate(const X& from, const X& to) const
            -> std::common_type_t<distance_result_t<X>, std::ranges::range_value_t<Ys>>
        {
            using result_t = std::common_type_t<distance_result_t<X>, std::ranges::range_value_t<Ys>>;

            struct partial_integrator_t {
                constexpr result_t operator()() const
                {
                    auto average = (**ylit + **yrit) / 2;
                    auto dist = interp1d::distance(static_cast<X>(**xlit), static_cast<X>(**xrit));
                    return static_cast<result_t>(std::move(dist) * std::move(average));
                }
                constexpr result_t operator()(const X& f, const X& t) const
                {
                    auto fdist = interp1d::distance(**xlit, f);
                    auto tdist = interp1d::distance(**xlit, t);
                    auto interval_length = interp1d::distance(**xlit, **xrit);
                    const auto w_average = (std::move(fdist) + std::move(tdist)) / (2 * std::move(interval_length));
                    auto average = **ylit * (1 - w_average) + **yrit * w_average;   // = (this(f) + this(t)) / 2
                    auto dist = interp1d::distance(f, t);
                    return static_cast<result_t>(std::move(dist) * std::move(average));
                }

                const std::ranges::iterator_t<const Xs>* xlit;
                const std::ranges::iterator_t<const Xs>* xrit;
                const std::ranges::iterator_t<const Ys>* ylit;
                const std::ranges::iterator_t<const Ys>* yrit;
            };
            // for x < grids.front()
            const auto xbeg = std::ranges::begin(grids_.get());
            const auto xnxt = std::ranges::next(xbeg);
            const auto ybeg = std::ranges::begin(values_.get());
            const auto ynxt = std::ranges::next(ybeg);
            const auto left_extrap = partial_integrator_t {
                std::addressof(xbeg), std::addressof(xnxt),
                std::addressof(ybeg), std::addressof(ynxt),
            };

            // for grids.back() <= x
            const auto xlst = std::ranges::next(xbeg, std::ranges::distance(grids_.get()) - 2);
            const auto xlstnxt = std::ranges::next(xlst);
            const auto ylst = std::ranges::next(ybeg, std::ranges::distance(values_.get()) - 2);
            const auto ylstnxt = std::ranges::next(ylst);
            const auto right_extrap = partial_integrator_t {
                std::addressof(xlst), std::addressof(xlstnxt),
                std::addressof(ylst), std::addressof(ylstnxt),
            };

            // for grids.front() <= x < grids.back() with partial interval
            const auto internal_partial = [](
                const auto& xlit, const auto& xrit, const auto& ylit, const auto& yrit,
                const X& f, const X& t
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    std::addressof(xlit), std::addressof(xrit),
                    std::addressof(ylit), std::addressof(yrit),
                };
                return integrator(f, t);
            };

            // for grids.front() <= x < grids.back() with full interval
            const auto internal_full = [](
                const auto& xlit, const auto& xrit, const auto& ylit, const auto& yrit
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    std::addressof(xlit), std::addressof(xrit),
                    std::addressof(ylit), std::addressof(yrit),
                };
                return integrator();
            };

            const auto& [f, t] = std::minmax(from, to, less_);
            const int sign = less_(to, from) ? -1 : 1;
            return static_cast<result_t>(
                sign * egret_detail::interp1d_impl::integrate_impl<result_t>(
                    left_extrap, internal_partial, internal_full, right_extrap,
                    grids_.get(), values_.get(), f, t, less_
                )
            );
        }

    // -------------------------------------------------------------------------
    //  get
    //
        constexpr const Less& less() const noexcept { return less_; }

    protected:
        util::member_data<Xs> grids_;
        util::member_data<Ys> values_;
        Less less_;

    }; // class generic_linear

    template <typename Xs, typename Ys>
    generic_linear(Xs, Ys) -> generic_linear<Xs, Ys>;

    template <typename Xs, typename Ys>
    generic_linear(std::reference_wrapper<Xs>, Ys) -> generic_linear<const Xs&, Ys>;

    template <typename Xs, typename Ys>
    generic_linear(Xs, std::reference_wrapper<Ys>) -> generic_linear<Xs, const Ys&>;

    template <typename Xs, typename Ys>
    generic_linear(std::reference_wrapper<Xs>, std::reference_wrapper<Ys>) -> generic_linear<const Xs&, const Ys&>;

    template <typename Xs, typename Ys, typename Less>
    generic_linear(Xs, Ys, Less) -> generic_linear<Xs, Ys, Less>;

    template <typename Xs, typename Ys, typename Less>
    generic_linear(std::reference_wrapper<Xs>, Ys, Less) -> generic_linear<const Xs&, Ys, Less>;

    template <typename Xs, typename Ys, typename Less>
    generic_linear(Xs, std::reference_wrapper<Ys>, Less) -> generic_linear<Xs, const Ys&, Less>;

    template <typename Xs, typename Ys, typename Less>
    generic_linear(std::reference_wrapper<Xs>, std::reference_wrapper<Ys>, Less) -> generic_linear<const Xs&, const Ys&, Less>;

// -----------------------------------------------------------------------------
//  [class] linear
// -----------------------------------------------------------------------------
    template <distance_measurable X, distance_measurable Y, std::semiregular Less = std::ranges::less>
        requires std::predicate<const Less&, const X&, const X&>
    class linear : public generic_linear<std::vector<X>, std::vector<Y>, Less> {
    private:
        using super_type = generic_linear<std::vector<X>, std::vector<Y>, Less>;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        using super_type::super_type;
        using super_type::operator =;

    // -------------------------------------------------------------------------
    //  mutable behavior
    //
        template <cpt::forward_range_of<X> Xs, cpt::forward_range_of<Y> Ys>
        void initialize(Xs&& xs, Ys&& ys)
        {
            egret_detail::interp1d_impl::interpolatee_validation(xs, ys, super_type::less_);
            util::vector_assign(super_type::grids_, std::forward<Xs>(xs));
            util::vector_assign(super_type::values_, std::forward<Ys>(ys));
        }

        template <typename AY>
            requires std::is_assignable_v<Y&, AY>
        void update(std::size_t i, AY&& value)
        {
            egret::assertion(super_type::values_.size() < i, "Assigned to out of range element. [size={}, index={}]", super_type::values_.size(), i);
            super_type::values_[i] = std::forward<AY>(value);
        }

    // -------------------------------------------------------------------------
    //  as_immutable
    //
        super_type& as_immutable() & noexcept { return static_cast<super_type&>(*this); }
        const super_type& as_immutable() const & noexcept { return static_cast<const super_type&>(*this); }
        super_type&& as_immutable() && noexcept { return static_cast<super_type&&>(*this); }
        const super_type&& as_immutable() const && noexcept { return static_cast<const super_type&&>(*this); }

    }; // class linear

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys>
    linear(Xs, Ys) 
        -> linear<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>>;

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename Less>
    linear(Xs, Ys, Less)
        -> linear<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, Less>;

} // namespace egret::math::interp1d

namespace nlohmann {
    template <typename X, typename Y, typename Less>
    struct adl_serializer<egret::math::interp1d::linear<X, Y, Less>> {
        using target_type = egret::math::interp1d::linear<X, Y, Less>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            const auto sz = egret_detail::interp1d_impl::get_knots_size(j);
            std::vector<X> xs;
            std::vector<Y> ys;
            xs.reserve(sz);
            ys.reserve(sz);

            egret_detail::interp1d_impl::recover_knots<X, Y>(j, std::back_inserter(xs), std::back_inserter(ys));
            return target_type{std::move(xs), std::move(ys)};
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
        }
    };

    template <typename Xs, typename Ys, typename Less>
    struct adl_serializer<egret::math::interp1d::generic_linear<Xs, Ys, Less>> {
        using target_type = egret::math::interp1d::generic_linear<Xs, Ys, Less>;

        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
        }
    };

    template <typename X, typename Y, typename Less>
    struct adl_serializer<egret::math::interp1d::generic_linear<std::vector<X>, std::vector<Y>, Less>>
        : adl_serializer<egret::math::interp1d::linear<X, Y, Less>> {
        using target_type = egret::math::interp1d::generic_linear<std::vector<X>, std::vector<Y>, Less>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            using super_type = adl_serializer<egret::math::interp1d::linear<X, Y, Less>>;
            return target_type(super_type::from_json(j).as_immutable());
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
        }
    };

} // namespace nlohmann