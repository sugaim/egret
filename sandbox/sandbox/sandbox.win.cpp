#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/assertions/exception.h"
#include "egret/fittings/yc/constraints/ois_constraint.h"
#include "egret/chrono/daycounters.h"
#include "core/chrono/calendar.h"
#include "egret/fittings/yc/constraints/any_evaluator.h"
#include "core/chrono/stopwatch.h"

namespace  {
    egret::chrono::calendar_server get_calsrv()
    {
        const char* path = EGRET_SOLUTION_DIRECTORY R"(sandbox\sandbox\data\calendars)";
        auto src = egret::chrono::json_directory_calendar_source(path);
        return egret::chrono::calendar_server(std::move(src));
    }

    nlohmann::json parse_json(const char* path)
    {
        auto fs = std::ifstream(path);
        return nlohmann::json::parse(fs);
    }

} // namespace 

int main()
{
    try {
        const auto calsrv = get_calsrv();

        using tp = egret::fit::yc::ois_constraint<std::string, std::string, egret::chrono::daycounter, egret::chrono::calendar_identifier>;

        egret::chrono::stopwatch sw;
        sw.start();
        const char* path = EGRET_SOLUTION_DIRECTORY R"(sandbox\sandbox\data\ois.json)";
        const auto j = parse_json(path);
        sw.stop(); 
        std::cout << "1: " << sw.microseconds() << std::endl;
        sw.reset();
        sw.start();
        auto obj1 = j.get<tp>();
        sw.stop(); 
        std::cout << "2: " << sw.microseconds() << std::endl;
        sw.reset();
        sw.start();
        const auto objx = from_dto(obj1, calsrv);
        sw.stop(); 
        std::cout << "3: " << sw.microseconds() << std::endl;
        sw.reset();
        sw.start();
        const auto objxz = from_dto(obj1, calsrv);
        sw.stop(); 
        std::cout << "4: " << sw.microseconds() << std::endl;
        sw.reset();
        sw.start();
        const auto objxx = from_dto(std::move(obj1), calsrv);
        sw.stop(); 
        std::cout << "5: " << sw.microseconds() << std::endl;

        std::cout << egret::cpt::yield_curve_evaluator<decltype(objxx), std::string, egret::model::any_yield_curve<double>> << std::endl;
        //const auto any = egret::fit::yc::any_evaluator<double, std::string>(obj2);
    }
    catch (const egret::exception& e) {
        std::cout << e.what() << std::endl;

        for (const auto& st : e.stacktrace()) {
            std::cout << st << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}
