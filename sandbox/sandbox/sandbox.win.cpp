// sandbox.win.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "core/assertions/exception.h"
#include "core/utils/json_utils/j2obj.h"
#include "core/utils/string_utils/parse.h"
#include "core/chrono/calendars/json_directory_calendar_source.h"
#include "core/chrono/calendars/json_map_calendar_source.h"
#include "core/chrono/calendars/calendar_server.h"
#include "core/chrono/stopwatch.h"
#include "core/math/interp1d.h"
#include "core/utils/json_utils/chrono.h"
#include "egret/markets/daycounters/daycounter.h"
#include "egret/instruments/cashflows/floating_rate_cf.h"
#include "egret/instruments/cashflows/ois_cf.h"
#include "core/math/interp1d/any.h"
#include "core/math/interp1d.h"
#include "egret/instruments/cashflows/fixed_rate_cf.h"
#include "egret/fittings/yc/constraints/composite_evaluator.h"

struct hogehoge {
    int x;
    int y;
};

int main()
{
    try {
        const auto cfs = nlohmann::json::parse(R"(
            [
                {
                    "discount_tag": "tona",
                    "notional": 100,
                    "accrual_start": "2020-10-10",
                    "accrual_end": "2020-11-10",
                    "payment_date": "2020-11-10",
                    "cashout_date": "2020-11-10",
                    "accrual_daycounter": "ACT365F",
                    "rate": "10bp"
                },
                {
                    "discount_tag": "tona",
                    "notional": 100,
                    "accrual_start": "2020-10-10",
                    "accrual_end": "2020-11-10",
                    "payment_date": "2020-11-10",
                    "cashout_date": "2020-11-10",
                    "accrual_daycounter": "ACT365F",
                    "rate": "10bp"
                }
            ]
        )");

        using cf_t = egret::inst::cfs::fixed_rate_cf<std::string, egret::mkt::daycounter>;
        using result_t = egret::fit::yc::composite_evaluator<cf_t>;

        const auto obj = cfs.get<result_t>();


        //const auto j = nlohmann::json::parse(R"({
        //    "is_right_continuous": true,
        //    "partition_ratio": 0.9,
        //    "slope_generator": {
        //    },
        //    "knots": [
        //        {
        //            "grid": "2022-09-11",
        //            "value": 1
        //        },
        //        {
        //            "grid": "2022-09-01",
        //            "value": 2
        //        },
        //        {
        //            "grid": "2022-09-21",
        //            "value": 2
        //        }
        //    ]
        //})");
        //using namespace std::chrono_literals;
        //const auto xxx = j.get<egret::math::interp1d::central_difference>();
        //const auto obj = j.get<egret::math::interp1d::cspline<std::chrono::sys_days, double, egret::math::interp1d::central_difference>>();
        ////auto any = egret::math::interp1d::any_mutable(obj);
        ////std::cout << any.is_1st_ord_differentiable() << std::endl;
        //const auto ymd = 2022y/9/5;
        //const auto yy = obj.integrate(std::chrono::sys_days(ymd), std::chrono::sys_days(ymd) + std::chrono::days(15));
        //std::cout << yy << std::endl;
        ////auto jj = nlohmann::json();
        ////jj = obj;

        ////std::cout << jj.dump(4) << std::endl;

        //const auto f = egret::math::interp1d::cspline(
        //    obj.grids(),
        //    obj.values(),
        //    egret::math::interp1d::forward_difference()
        //);

        //std::cout << f.integrate(std::chrono::sys_days(ymd), std::chrono::sys_days(ymd) + std::chrono::days(15)) << std::endl;

        //const auto j = nlohmann::json::parse(R"({
        //    "rate_tag": "hogehoge",
        //    "notional": 344,
        //    "accrual_start": "2022-10-10",
        //    "accrual_end": "2022-10-10",
        //    "fixing_date": "2022-10-10",
        //    "payment_date": "2022-10-10",
        //    "accrual_daycounter": "act365f",
        //    "gearing": 1.0,
        //    "spread": 0.042
        //})");

        //const auto cf = j.get<egret::inst::float_rate_cf<std::string, egret::mkt::daycounter>>();

        //nlohmann::json jjj;
        //jjj = cf;

        //std::cout << jjj << std::endl;

        //const auto j = nlohmann::json::parse(R"({
        //    "rate_tag": "hogehoge",
        //    "notional": 344,
        //    "accrual_start": "2022-10-10",
        //    "accrual_end": "2022-10-10",
        //    "payment_date": "2022-10-10",
        //    "accrual_daycounter": "act365f",
        //    "compound_convention": {
        //        "compound_method": "spread_exclusive",
        //        "lookback_period": 5,
        //        "lockout_period": 0
        //    },
        //    "accrual_calendar": {
        //        "code": "TKY",
        //        "combination_type": "all_open"
        //    },
        //    "reference_rate_calendar": {
        //        "code": "TKY",
        //        "combination_type": "all_open"
        //    },
        //    "gearing": 4.2,
        //    "spread": "42bp"
        //})");

        //const auto cf = j.get<egret::inst::ois_coupon_cf_dto>();

        
        //std::ifstream f ( R"(\temp\cal.json)" );
        //const auto json = nlohmann::json::parse(f);

        //const auto src = json.get<egret::chrono::json_map_calendar_source<>>();
        //const auto srv = egret::chrono::calendar_server(src);

        //egret::chrono::stopwatch sw;
        //sw.start();
        //const auto cal = srv.get({.codes = {"TKY"}});
        //sw.stop();

        //std::cout << sw.milliseconds() << std::endl;

        //using namespace std::chrono_literals;
        //constexpr std::chrono::sys_days st = 2022y/10/5;
        //constexpr std::chrono::sys_days ed = 2022y/10/12;
        //std::cout << egret::chrono::count_businessdays(cal, st, ed) << std::endl;

        //using namespace std::string_view_literals;
        //constexpr auto ff = egret::util::parse<std::chrono::sys_days>;// egret::util::from_string<std::chrono::sys_days>.bind("%F"sv);
        //const auto xx = egret::util::from_string<std::chrono::sys_days>.bind("%F")("2022-01-10");
        //std::cout << xx << std::endl;
        ////std::chrono::sys_days d{};
        ////std::chrono::parse("%F", d);
        //constexpr auto deser = egret::util::j2obj::object.from_properties<int>(
        //    "value" >> egret::util::j2obj::string.transform(egret::util::from_string<std::chrono::sys_days>.bind("%F"))
        //);
    }
    catch (const egret::exception& e) {
        std::cout << e.what() << std::endl;

        for (const auto& st : e.stacktrace()) {
            std::cout << st << std::endl;
        }
    }
}

// プログラムの実行: Ctrl + F5 または [デバッグ] > [デバッグなしで開始] メニュー
// プログラムのデバッグ: F5 または [デバッグ] > [デバッグの開始] メニュー

// 作業を開始するためのヒント: 
//    1. ソリューション エクスプローラー ウィンドウを使用してファイルを追加/管理します 
//   2. チーム エクスプローラー ウィンドウを使用してソース管理に接続します
//   3. 出力ウィンドウを使用して、ビルド出力とその他のメッセージを表示します
//   4. エラー一覧ウィンドウを使用してエラーを表示します
//   5. [プロジェクト] > [新しい項目の追加] と移動して新しいコード ファイルを作成するか、[プロジェクト] > [既存の項目の追加] と移動して既存のコード ファイルをプロジェクトに追加します
//   6. 後ほどこのプロジェクトを再び開く場合、[ファイル] > [開く] > [プロジェクト] と移動して .sln ファイルを選択します
