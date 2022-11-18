#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/assertions/exception.h"
#include "egret/fittings/yc/constraints/ois_constraint.h"
#include "core/chrono/stopwatch.h"

int main()
{
    try {
        const char* path = EGRET_SOLUTION_DIRECTORY R"(sandbox\sandbox\data\ois.json)";
        auto fs = std::ifstream(path);
        std::cout << fs.is_open() << std::endl;
        auto j = nlohmann::json {};
        fs >> j;

        using tp = egret::fit::yc::ois_constraint<std::string, std::string, std::string, std::string>;

        egret::chrono::stopwatch sw;
        sw.start();
        [[maybe_unused]] const tp obj = j.get<tp>();
        sw.stop(); 
        std::cout << sw.microseconds() << std::endl;
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
