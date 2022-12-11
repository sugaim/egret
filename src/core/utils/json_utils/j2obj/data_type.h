#pragma once

#include <nlohmann/json_fwd.hpp>
#include "core/utils/string_utils/parse.h"
#include "core/assertions/assertion.h"
#include "concepts.h"
#include "interface.h"
#include "transformed.h"

namespace egret::util::j2obj {
// -----------------------------------------------------------------------------
//  [class] string_t
//  [value] string
// -----------------------------------------------------------------------------
    class string_t : public json_deserializer_interface<string_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> const typename Json::string_t&
        {
            return j.template get_ref<const typename Json::string_t&>();
        }

        template <typename T, typename ...Args>
        constexpr auto parse_to(Args&& ...args) const
        {
            if constexpr (sizeof...(Args) != 0) {
                return this->transform(util::from_string<T>.bind(std::forward<Args>(args)...));
            }
            else {
                return this->transform(util::from_string<T>);
            }
        }
        
    }; // class string_t

    inline constexpr auto string = string_t {};

// -----------------------------------------------------------------------------
//  [class] boolean_t
//  [value] boolean
// -----------------------------------------------------------------------------
    class boolean_t : public json_deserializer_interface<boolean_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> typename Json::boolean_t
        {
            return j.template get<typename Json::boolean_t>();
        }
        
    }; // class boolean_t
    
    inline constexpr auto boolean = boolean_t {};

// -----------------------------------------------------------------------------
//  [class] integer_t
//  [value] integer
// -----------------------------------------------------------------------------
    class integer_t : public json_deserializer_interface<integer_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> typename Json::number_integer_t
        {
            return j.template get<typename Json::number_integer_t>();
        }
        
    }; // class integer_t
    
    inline constexpr auto integer = integer_t {};

// -----------------------------------------------------------------------------
//  [class] unsigned_integer_t
//  [value] unsigned_integer
// -----------------------------------------------------------------------------
    class unsigned_integer_t : public json_deserializer_interface<unsigned_integer_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> typename Json::number_unsigned_integer_t
        {
            return j.template get<typename Json::number_unsigned_integer_t>();
        }
        
    }; // class unsigned_integer_t
    
    inline constexpr auto unsigned_integer = unsigned_integer_t {};    
    
// -----------------------------------------------------------------------------
//  [class] number_t
//  [value] number
// -----------------------------------------------------------------------------
    class number_t : public json_deserializer_interface<number_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> typename Json::number_float_t
        {
            return j.template get<typename Json::number_float_t>();
        }
        
    }; // class number_t
    
    inline constexpr auto number = number_t {};    
    
// -----------------------------------------------------------------------------
//  [class] array_t
//  [value] array
// -----------------------------------------------------------------------------
    class array_t : public json_deserializer_interface<array_t> {
    private:
        template <typename C>
        struct range_transformer {
            template <std::ranges::range JsonArray>
                requires std::is_invocable_v<const C&, std::ranges::range_reference_t<const JsonArray&>>
            auto operator()(const JsonArray& js) const
                -> std::ranges::transform_view<const JsonArray&, C>
            {
                return std::ranges::transform_view<const JsonArray&, C>(js, static_cast<C>(converter));
            }

            std::remove_cvref_t<C> converter;
        };
        template <typename C>
        struct vector_transformer {
            template <std::ranges::range JsonArray>
                requires std::is_invocable_v<const C&, std::ranges::range_reference_t<const JsonArray&>>
            auto operator()(const JsonArray& js) const
                -> std::vector<std::remove_cvref_t<std::invoke_result_t<const C&, std::ranges::range_reference_t<const JsonArray&>>>>
            {
                return js | std::views::transform(converter) | std::ranges::to<std::vector>();
            }

            std::remove_cvref_t<C> converter;
        };

    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> const typename Json::array_t&
        {
            return j.template get_ref<const typename Json::array_t&>();
        }

        template <typename C>
        constexpr auto to_range_with(C&& converter) const
            -> transformed_deserializer<array_t, range_transformer<std::remove_cvref_t<C>>>
        {
            auto transformer = range_transformer<std::remove_cvref_t<C>>(std::forward<C>(converter));
            return this->transform(std::move(transformer));
        }
        template <typename T>
        constexpr auto to_range_of() const
            -> transformed_deserializer<array_t, range_transformer<j2obj::get_t<T>>>
        {
            return this->to_range_with(j2obj::get<T>);
        }

        template <typename C>
        constexpr auto to_vector_with(C&& converter) const
            -> transformed_deserializer<array_t, vector_transformer<std::remove_cvref_t<C>>>
        {
            auto transformer = vector_transformer<std::remove_cvref_t<C>>(std::forward<C>(converter));
            return this->transform(std::move(transformer));
        }
        template <typename T>
        constexpr auto to_vector_of() const
            -> transformed_deserializer<array_t, vector_transformer<j2obj::get_t<T>>>
        {
            return this->to_vector_with(j2obj::get<T>);
        }
        
    }; // class array_t
    
    inline constexpr auto array = array_t {};
    
// -----------------------------------------------------------------------------
//  [class] binary_decorder_t
// -----------------------------------------------------------------------------
    template <auto decorder>
    class binary_decorder_t : public json_deserializer_interface<binary_decorder_t<decorder>> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> Json
        {
            return decorder(j.get_binary());
        }
        
    }; // class binary_decorder_t

// -----------------------------------------------------------------------------
//  [class] binary_t
//  [value] binary
// -----------------------------------------------------------------------------
    class binary_t : public json_deserializer_interface<binary_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> const typename Json::binary_t&
        {
            return j.get_binary();
        }

        static constexpr auto decode_bson = binary_decorder_t<
            []<typename Json>(const Json& json) { return Json::from_bson(json.get_binary()); }>{};
        static constexpr auto decode_bjdata = binary_decorder_t<
            []<typename Json>(const Json& json) { return Json::from_bjdata(json.get_binary()); }>{};
        static constexpr auto decode_cbor = binary_decorder_t<
            []<typename Json>(const Json& json) { return Json::from_cbor(json.get_binary()); }>{};
        static constexpr auto decode_msgpack = binary_decorder_t<
            []<typename Json>(const Json& json) { return Json::from_msgpack(json.get_binary()); }>{};
        static constexpr auto decode_ubjson = binary_decorder_t<
            []<typename Json>(const Json& json) { return Json::from_ubjson(json.get_binary()); }>{};

    }; // class binary_t
    
    inline constexpr auto binary = binary_t {};
    
// -----------------------------------------------------------------------------
//  [class] object_t
//  [value] object
// -----------------------------------------------------------------------------
    class object_t : public json_deserializer_interface<object_t> {
    public:
        template <cpt::basic_json Json>
        auto operator()(const Json& j) const -> const typename Json::object_t&
        {
            return j.template get_ref<const typename Json::object_t&>();
        }
        
    }; // class object_t
    
    inline constexpr auto object = object_t {};
    
} // namespace egret::util::j2obj
