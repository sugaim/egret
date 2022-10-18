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
#include "auxiliary/interval_at.h"
#include "auxiliary/json_deserializer_impl.h"

namespace egret::math::interp1d {
// -----------------------------------------------------------------------------
//  [class] generic_pwconst
// -----------------------------------------------------------------------------
    template <
        std::ranges::forward_range Xs, std::ranges::forward_range Ys, 
        cpt::non_reference P = double, std::semiregular Less = std::ranges::less
    >
        requires 
            distance_measurable<std::ranges::range_reference_t<Xs>> && 
            std::strict_weak_order<const Less&, std::ranges::range_reference_t<Xs>, std::ranges::range_reference_t<Xs>> &&
            std::strict_weak_order<const Less&, const P&, double>
    class generic_pwconst {
    private:
        using this_type = generic_pwconst;
        using spfn_props = special_function_properties<util::member_data<Xs>, util::member_data<Ys>, P, Less>;
        using grid_type = std::ranges::range_value_t<const Xs>;
        using value_type = std::ranges::range_value_t<const Ys>;
        using partition_type = P;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtors and assigns
    //
        generic_pwconst() = delete;
        constexpr generic_pwconst(const this_type&)
            noexcept(spfn_props::are_nothrow_copy_constructible_v)
            requires spfn_props::are_copy_constructible_v = default;
        constexpr generic_pwconst(this_type&&)
            noexcept(spfn_props::are_nothrow_move_constructible_v)
            requires spfn_props::are_move_constructible_v = default;

        template <
            cpt::constructible_to<util::member_data<Xs>> AXs, 
            cpt::constructible_to<util::member_data<Ys>> AYs,
            cpt::constructible_to<P> AP
        >
        constexpr generic_pwconst(AXs&& xs, AYs&& ys, AP&& partition, bool is_right_continuous, Less less)
            : grids_(std::forward<AXs>(xs)),
              values_(std::forward<AYs>(ys)),
              partition_(std::forward<AP>(partition)),
              is_right_continuous_(is_right_continuous),
              less_(std::move(less))
        {
            egret_detail::interp1d_impl::interpolatee_validation(grids_.get(), values_.get(), less_);
            if (std::invoke(less_, partition_, 0.) || std::invoke(less_, 1., partition_)) {
                throw exception("partition point defining pwconst interpolation must be between 0 and 1.");
            }
        }

        template <
            cpt::constructible_to<util::member_data<Xs>> AXs, 
            cpt::constructible_to<util::member_data<Ys>> AYs,
            cpt::constructible_to<P> AP
        >
        constexpr generic_pwconst(AXs&& xs, AYs&& ys, AP&& partition, bool is_right_continuous = true)
            requires std::is_default_constructible_v<Less>
            : generic_pwconst(std::forward<AXs>(xs), std::forward<AYs>(ys), std::forward<AP>(partition), is_right_continuous, Less{})
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

        template <typename X>
            requires
                std::strict_weak_order<const Less&, const grid_type&, const X&> &&
                std::strict_weak_order<const Less&, distance_result_t<grid_type, X>, const P&>
        constexpr auto operator()(const X& x) const
            -> std::ranges::range_reference_t<const Ys>
        {
            const auto [idx, w] = egret_detail::interp1d_impl::find_index_and_relpos(grids_.get(), x, less_);
            const auto [ylit, yrit] = egret_detail::interp1d_impl::interval_at(idx, values_.get());
            if (is_right_continuous_) {
                // p <= w implies right point
                return !std::invoke(less_, w, partition_) ? *yrit : *ylit;
            }
            else {
                // w <= p implies left point
                return !std::invoke(less_, partition_, w) ? *ylit : *yrit;
            }
        }

    // -------------------------------------------------------------------------
    //  der
    //
        constexpr value_type der1(auto&& x) const
        {
            return static_cast<value_type>(0);
        }

        constexpr value_type der2(auto&& x) const
        {
            return static_cast<value_type>(0);
        }

    // -------------------------------------------------------------------------
    //  integrate
    //
        template <distance_measurable_from<grid_type> X>
            requires 
                std::strict_weak_order<const Less&, const grid_type&, const X&> &&
                std::strict_weak_order<const Less&, distance_result_t<grid_type, X>, const P&> &&
                std::constructible_from<X, std::ranges::range_reference_t<const Xs>>
        constexpr auto integrate(const X& from, const X& to) const
            -> std::common_type_t<distance_result_t<X>, std::ranges::range_value_t<Ys>>
        {
            using result_t = std::common_type_t<distance_result_t<X>, std::ranges::range_value_t<Ys>>;

            struct partial_integrator_t {
                constexpr result_t operator()() const
                {
                    auto average = (**ylit * *partition + **yrit * (1 - *partition));
                    auto dist = interp1d::distance(static_cast<X>(**xlit), static_cast<X>(**xrit));
                    return static_cast<result_t>(std::move(dist) * std::move(average));
                }
                constexpr result_t operator()(const X& f, const X& t) const
                {
                    const auto interval_dist = interp1d::distance(**xlit, **xrit);
                    if (auto wf = interp1d::distance(**xlit, f) / interval_dist; (*less)(*partition, wf)) {
                        return static_cast<result_t>(**yrit * interp1d::distance(f, t));
                    }
                    if (auto wt = interp1d::distance(**xlit, t) / interval_dist; (*less)(wt, *partition)) {
                        return static_cast<result_t>(**ylit * interp1d::distance(f, t));
                    }
                    const auto partition_dist = *partition * interval_dist;
                    return static_cast<result_t>(
                        static_cast<result_t>(**yrit * (interp1d::distance(**xlit, t) - partition_dist)) +
                        static_cast<result_t>(**ylit * (partition_dist - interp1d::distance(**xlit, f)))
                    );
                }

                const std::ranges::iterator_t<const Xs>* xlit;
                const std::ranges::iterator_t<const Xs>* xrit;
                const std::ranges::iterator_t<const Ys>* ylit;
                const std::ranges::iterator_t<const Ys>* yrit;
                const P* partition;
                const Less* less;
            };
            // for x < grids.front()
            const auto xbeg = std::ranges::begin(grids_.get());
            const auto xnxt = std::ranges::next(xbeg);
            const auto ybeg = std::ranges::begin(values_.get());
            const auto ynxt = std::ranges::next(ybeg);
            const auto left_extrap = partial_integrator_t {
                std::addressof(xbeg), std::addressof(xnxt),
                std::addressof(ybeg), std::addressof(ynxt),
                std::addressof(partition_), std::addressof(less_)
            };

            // for grids.back() <= x
            const auto xlst = std::ranges::next(xbeg, std::ranges::distance(grids_.get()) - 2);
            const auto xlstnxt = std::ranges::next(xlst);
            const auto ylst = std::ranges::next(ybeg, std::ranges::distance(values_.get()) - 2);
            const auto ylstnxt = std::ranges::next(ylst);
            const auto right_extrap = partial_integrator_t {
                std::addressof(xlst), std::addressof(xlstnxt),
                std::addressof(ylst), std::addressof(ylstnxt),
                std::addressof(partition_), std::addressof(less_)
            };

            // for grids.front() <= x < grids.back() with partial interval
            const auto internal_partial = [this](
                const auto& xlit, const auto& xrit, const auto& ylit, const auto& yrit,
                const X& f, const X& t
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    std::addressof(xlit), std::addressof(xrit),
                    std::addressof(ylit), std::addressof(yrit),
                    std::addressof(this->partition_), std::addressof(this->less_)
                };
                return integrator(f, t);
            };

            // for grids.front() <= x < grids.back() with full interval
            const auto internal_full = [this](
                const auto& xlit, const auto& xrit, const auto& ylit, const auto& yrit
            ) -> result_t
            {
                const auto integrator = partial_integrator_t {
                    std::addressof(xlit), std::addressof(xrit),
                    std::addressof(ylit), std::addressof(yrit),
                    std::addressof(this->partition_), std::addressof(this->less_)
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
        constexpr const P& partition_ratio() const noexcept { return partition_; }
        constexpr bool is_right_continuous() const noexcept { return is_right_continuous_; }
        constexpr const Less& less() const noexcept { return less_; }

    protected:
        util::member_data<Xs> grids_;
        util::member_data<Ys> values_;
        P partition_;
        bool is_right_continuous_;
        Less less_;

    }; // class generic_pwconst

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(Xs, Ys, P) -> generic_pwconst<Xs, Ys, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(Xs, Ys, P, bool) -> generic_pwconst<Xs, Ys, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(std::reference_wrapper<Xs>, Ys, P) -> generic_pwconst<const Xs&, Ys, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(std::reference_wrapper<Xs>, Ys, P, bool) -> generic_pwconst<const Xs&, Ys, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(Xs, std::reference_wrapper<Ys>, P) -> generic_pwconst<Xs, const Ys&, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(Xs, std::reference_wrapper<Ys>, P, bool) -> generic_pwconst<Xs, const Ys&, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(std::reference_wrapper<Xs>, std::reference_wrapper<Ys>, P) -> generic_pwconst<const Xs&, const Ys&, P>;

    template <typename Xs, typename Ys, typename P>
    generic_pwconst(std::reference_wrapper<Xs>, std::reference_wrapper<Ys>, P, bool) -> generic_pwconst<const Xs&, const Ys&, P>;

    template <typename Xs, typename Ys, typename P, typename Less>
    generic_pwconst(Xs, Ys, P, bool, Less) -> generic_pwconst<Xs, Ys, P, Less>;

    template <typename Xs, typename Ys, typename P, typename Less>
    generic_pwconst(std::reference_wrapper<Xs>, Ys, P, bool, Less) -> generic_pwconst<const Xs&, Ys, P, Less>;

    template <typename Xs, typename Ys, typename P, typename Less>
    generic_pwconst(Xs, std::reference_wrapper<Ys>, P, bool, Less) -> generic_pwconst<Xs, const Ys&, P, Less>;

    template <typename Xs, typename Ys, typename P, typename Less>
    generic_pwconst(std::reference_wrapper<Xs>, std::reference_wrapper<Ys>, P, bool, Less) -> generic_pwconst<const Xs&, const Ys&, P, Less>;

// -----------------------------------------------------------------------------
//  [class] pwconst
// -----------------------------------------------------------------------------
    template <
        distance_measurable X, distance_measurable Y, 
        cpt::non_reference P = double, std::semiregular Less = std::ranges::less
    >
        requires 
            std::predicate<const Less&, const X&, const X&> &&
            std::predicate<const Less&, const P&, double> &&
            std::predicate<const Less&, double, const P&>
    class pwconst : public generic_pwconst<std::vector<X>, std::vector<Y>, P, Less> {
    private:
        using super_type = generic_pwconst<std::vector<X>, std::vector<Y>, P, Less>;

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

    }; // class pwconst

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename P>
    pwconst(Xs, Ys, P) 
        -> pwconst<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, P>;

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename P>
    pwconst(Xs, Ys, P, bool) 
        -> pwconst<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, P>;

    template <std::ranges::forward_range Xs, std::ranges::forward_range Ys, typename P, typename Less>
    pwconst(Xs, Ys, P, bool, Less)
        -> pwconst<std::ranges::range_value_t<Xs>, std::ranges::range_value_t<Ys>, P, Less>;

} // namespace egret::math::interp1d

namespace nlohmann {
    template <typename X, typename Y, typename P, typename Less>
    struct adl_serializer<egret::math::interp1d::pwconst<X, Y, P, Less>> {
        using target_type = egret::math::interp1d::pwconst<X, Y, P, Less>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            namespace j2obj = egret::util::j2obj;
            const bool is_right_continuous = ("is_right_continuous" >> j2obj::boolean)(j);
            auto partition_ratio = ("partition_ratio" >> j2obj::get<P>)(j);
            const auto sz = egret_detail::interp1d_impl::get_knots_size(j);
            std::vector<X> xs;
            std::vector<Y> ys;
            xs.reserve(sz);
            ys.reserve(sz);

            egret_detail::interp1d_impl::recover_knots<X, Y>(j, std::back_inserter(xs), std::back_inserter(ys));
            return target_type{std::move(xs), std::move(ys), std::move(partition_ratio), is_right_continuous};
        }

        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
            j["is_right_continuous"] = obj.is_right_continuous();
            j["partition_ratio"] = obj.partition_ratio();
        }
    };

    template <typename Xs, typename Ys, typename P, typename Less>
    struct adl_serializer<egret::math::interp1d::generic_pwconst<Xs, Ys, P, Less>> {
        using target_type = egret::math::interp1d::generic_pwconst<Xs, Ys, P, Less>;

        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
            j["is_right_continuous"] = obj.is_right_continuous();
            j["partition_ratio"] = obj.partition_ratio();
        }
    };

    template <typename X, typename Y, typename P, typename Less>
    struct adl_serializer<egret::math::interp1d::generic_pwconst<std::vector<X>, std::vector<Y>, P, Less>>
        : adl_serializer<egret::math::interp1d::pwconst<X, Y, P, Less>> {
        using target_type = egret::math::interp1d::generic_pwconst<std::vector<X>, std::vector<Y>, P, Less>;

        template <typename Json>
        static target_type from_json(const Json& j)
        {
            using super_type = adl_serializer<egret::math::interp1d::pwconst<X, Y, P, Less>>;
            return target_type(super_type::from_json(j).as_immutable());
        }
        template <typename Json>
        static void to_json(Json& j, const target_type& obj)
        {
            namespace interp1d = egret::math::interp1d;
            namespace interp1d_impl = egret_detail::interp1d_impl;
            interp1d_impl::records_knots(j, interp1d::grids(obj), interp1d::values(obj));
            j["is_right_continuous"] = obj.is_right_continuous();
            j["partition_ratio"] = obj.partition_ratio();
        }
    };

} // namespace nlohmann
