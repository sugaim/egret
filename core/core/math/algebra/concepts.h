#pragma once

#include <type_traits>
#include <concepts>
#include "core/concepts/qualified.h"
#include "core/concepts/non_void.h"

namespace egret::cpt {
// -----------------------------------------------------------------------------
//  [concept] subtractible_from
//  [concept] subtractible_with
//  [concept] subtractible
// -----------------------------------------------------------------------------
    template <typename T, typename U>
    concept subtractible_from = requires (const T& x, const U& y) {
        {y - x} -> cpt::non_void;
    };

    template <typename T, typename U>
    concept subtractible_with = 
        subtractible_from<T, U> && 
        subtractible_from<U, T>;

    template <typename T>
    concept subtractibleh = 
        subtractible_with<T, T>;

// -----------------------------------------------------------------------------
//  [concept] additive_group
//  [concept] strict_additive_group
// -----------------------------------------------------------------------------
    template <typename T>
    concept additive_group =
        std::is_convertible_v<std::remove_cvref_t<T>, int> &&
        requires (const T& x, const T& y) {
            { - x } -> std::convertible_to<std::remove_cvref_t<T>>;
            {x - y} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x + y} -> std::convertible_to<std::remove_cvref_t<T>>;
        };

    template <typename T>
    concept strict_additive_group =
        additive_group<T> &&
        requires (const T& x, const T& y) {
            { - x } -> cpt::qualified<T>;
            {x - y} -> cpt::qualified<T>;
            {x + y} -> cpt::qualified<T>;
        };

// -----------------------------------------------------------------------------
//  [concept] ring
//  [concept] strict_ring
// -----------------------------------------------------------------------------
    template <typename T>
    concept ring =
        additive_group<T> &&
        requires (const T& x, const T& y, int n) {
            {n + x} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x + n} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x - n} -> std::convertible_to<std::remove_cvref_t<T>>;
            {n - x} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x * n} -> std::convertible_to<std::remove_cvref_t<T>>;
            {n * x} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x * y} -> std::convertible_to<std::remove_cvref_t<T>>;
        };

    template <typename T>
    concept strict_ring =
        ring<T> &&
        strict_additive_group<T> &&
        requires (const T& x, const T& y, int n) {
            {n + x} -> cpt::qualified<T>;
            {x + n} -> cpt::qualified<T>;
            {x - n} -> cpt::qualified<T>;
            {n - x} -> cpt::qualified<T>;
            {x * n} -> cpt::qualified<T>;
            {n * x} -> cpt::qualified<T>;
            {x * y} -> cpt::qualified<T>;
        };

// -----------------------------------------------------------------------------
//  [concept] field
//  [concept] strict_field
// -----------------------------------------------------------------------------
    template <typename T>
    concept field =
        ring<T> &&
        requires (const T& x, const T& y, int n) {
            {x / y} -> std::convertible_to<std::remove_cvref_t<T>>;
            {x / n} -> std::convertible_to<std::remove_cvref_t<T>>;
            {n / x} -> std::convertible_to<std::remove_cvref_t<T>>;
        };

    template <typename T>
    concept strict_field =
        field<T> &&
        requires (const T& x, const T& y, int n) {
            {x / y} -> cpt::qualified<T>;
            {x / n} -> cpt::qualified<T>;
            {n / x} -> cpt::qualified<T>;
        };
        
// -----------------------------------------------------------------------------
//  [concept] module
//  [concept] strict_module
// -----------------------------------------------------------------------------
    template <typename T, typename C>
    concept module =
        additive_group<T> &&
        ring<C> &&
        requires (const C& c, const T& x, int n) {
            {x * n} -> std::convertible_to<T>;
            {n * x} -> std::convertible_to<T>;
            {c * x} -> std::convertible_to<T>;
            {x * c} -> std::convertible_to<T>;
        };

    template <typename T, typename C>
    concept strict_module =
        module<T, C> &&
        strict_module<T, C> &&
        strict_ring<C> &&
        requires (const C& c, const T& x, int n) {
            {x * n} -> cpt::qualified<T>;
            {n * x} -> cpt::qualified<T>;
            {c * x} -> cpt::qualified<T>;
            {x * c} -> cpt::qualified<T>;
        };
        
// -----------------------------------------------------------------------------
//  [concept] vector
//  [concept] strict_vector
// -----------------------------------------------------------------------------
    template <typename T, typename C>
    concept vector =
        module<T, C> &&
        field<C> &&
        requires (const C& c, const T& x, int n) {
            {x / n} -> std::convertible_to<T>;
            {x * c} -> std::convertible_to<T>;
        };

    template <typename T, typename C>
    concept strict_vector =
        vector<T, C> &&
        strict_module<T, C> &&
        strict_field<C> &&
        requires (const C& c, const T& x, int n) {
            {x / n} -> cpt::qualified<T>;
            {x * c} -> cpt::qualified<T>;
        };

} // namespace egret::cpt
