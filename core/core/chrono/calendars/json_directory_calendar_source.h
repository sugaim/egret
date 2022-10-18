#pragma once

#include "core/auto_link.h"
#include <vector>
#include <chrono>
#include <optional>
#include <filesystem>

namespace egret::chrono {
// -----------------------------------------------------------------------------
//  [class] json_directory_calendar_source
// -----------------------------------------------------------------------------    
    /**
     * @brief directory based calendar source.
     * @details
     *  directory is assumed to have '{calendar_name}.json' files
     *  where the json files are assumed to obey calendar_source.schema.json.
    */
    class json_directory_calendar_source {
    private:
        using this_type = json_directory_calendar_source;

    public:
    // -------------------------------------------------------------------------
    //  ctors, dtor and assigns
    //
        json_directory_calendar_source() = delete;
        json_directory_calendar_source(const this_type&) = default;
        json_directory_calendar_source(this_type&&) = default;

        json_directory_calendar_source(std::filesystem::path directory_path, std::string ext);
        explicit json_directory_calendar_source(std::filesystem::path directory_path);

        this_type& operator =(const this_type&) = default;
        this_type& operator =(this_type&&) noexcept = default;

    // -------------------------------------------------------------------------
    //  calendar source behavior
    //
        std::optional<std::vector<std::chrono::sys_days>> get_additional_holidays(std::string_view code) const;
        std::optional<std::vector<std::chrono::sys_days>> get_additional_businessdays(std::string_view code) const;

    // -------------------------------------------------------------------------
    //  get
    //
        const auto& directory_path() const noexcept { return dirpath_; }
        const auto& file_extention() const noexcept { return ext_; }

    private:
        std::filesystem::path dirpath_;
        std::string ext_;

    }; // class json_directory_calendar_source

} // namespace egret::chrono
