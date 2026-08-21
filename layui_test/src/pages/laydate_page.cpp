#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/label.h"
#include "ui/ui/laydate.h"
#include "ui/ui/laydatebox.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

ui::Label* add_status(PageHost& host, const char* init_text) {
    ui::Label* status = new ui::Label(0, 0, 800, 0, init_text);
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text);
    host.content->add_child(status);
    return status;
}

ui::Label* add_note(PageHost& host, const char* text) {
    ui::Label* note = new ui::Label(0, 0, 800, 0, text);
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
    return note;
}

// 弹层输入框 demo 工厂：LaydateBox（input+popup）+ 可选 status 回调
ui::LaydateBox* add_box(PageHost& host, const char* label_text, const char* placeholder,
                        ui::Label* status, const char* status_fmt) {
    ui::LaydateBox* b = new ui::LaydateBox(0, 0, 280.f, 38.f);
    b->set_font(host.font);
    b->set_theme(host.theme);
    b->set_placeholder(placeholder);
    if (status && status_fmt) {
        b->set_on_change([status, status_fmt](const char* v) {
            char buf[128];
            snprintf(buf, sizeof(buf), status_fmt, v);
            status->set_text(buf);
        });
    }
    host.content->add_child(b);
    if (label_text && label_text[0]) {
        ui::Label* l = new ui::Label(0, 0, 800, 0, label_text);
        l->set_font(host.font);
        l->set_theme(host.theme);
        l->set_color(host.theme->text_muted);
        host.content->add_child(l);
    }
    return b;
}

// ---- 分组1：常规用法 ----
void demo_basic(PageHost& host) {
    add_section(host, "常规用法");
    ui::Label* s1 = add_status(host, "中文版");
    ui::LaydateBox* b1 = add_box(host, "", "yyyy-MM-dd", s1, "中文版选中：%s");
    b1->set_date(2026, 8, 18);

    ui::Label* s2 = add_status(host, "国际版");
    ui::LaydateBox* b2 = add_box(host, "", "yyyy-MM-dd", s2, "国际版选中：%s");
    b2->set_lang("en");
    b2->set_date(2026, 8, 18);
}

// ---- 分组2：其它选择器 ----
void demo_pickers(PageHost& host) {
    add_section(host, "其它选择器");
    ui::Label* s_year = add_status(host, "年选择器");
    ui::LaydateBox* y = add_box(host, "", "yyyy", s_year, "选中年份：%s");
    y->set_type("year");

    ui::Label* s_month = add_status(host, "年月选择器");
    ui::LaydateBox* m = add_box(host, "", "yyyy-MM", s_month, "选中年月：%s");
    m->set_type("month");

    ui::Label* s_time = add_status(host, "时间选择器");
    ui::LaydateBox* t = add_box(host, "", "HH:mm:ss", s_time, "选中时间：%s");
    t->set_type("time");

    ui::Label* s_dt = add_status(host, "日期时间选择器");
    ui::LaydateBox* dt = add_box(host, "", "yyyy-MM-dd HH:mm:ss", s_dt, "选中日期时间：%s");
    dt->set_type("datetime");
}

// ---- 分组3：范围选择 ----
void demo_range(PageHost& host) {
    add_section(host, "范围选择");
    ui::Label* s_date = add_status(host, "日期范围");
    ui::LaydateBox* d = add_box(host, "", " - ", s_date, "日期范围：%s");
    d->set_range(true);

    ui::Label* s_year = add_status(host, "年范围");
    ui::LaydateBox* y = add_box(host, "", " - ", s_year, "年范围：%s");
    y->set_type("year");
    y->set_range(true);

    ui::Label* s_month = add_status(host, "年月范围");
    ui::LaydateBox* m = add_box(host, "", " - ", s_month, "年月范围：%s");
    m->set_type("month");
    m->set_range(true);

    ui::Label* s_time = add_status(host, "时间范围");
    ui::LaydateBox* t = add_box(host, "", " - ", s_time, "时间范围：%s");
    t->set_type("time");
    t->set_range(true);

    ui::Label* s_dt = add_status(host, "日期时间范围");
    ui::LaydateBox* dt = add_box(host, "", " - ", s_dt, "日期时间范围：%s");
    dt->set_type("datetime");
    dt->set_range(true);
}

// ---- 分组4：自定义格式 ----
void demo_format(PageHost& host) {
    add_section(host, "自定义格式");
    ui::Label* s_f1 = add_status(host, "请选择日期（yyyy年MM月dd日）");
    ui::LaydateBox* f1 = add_box(host, "", "yyyy年MM月dd日", s_f1, "选中：%s");
    f1->set_format("yyyy年MM月dd日");

    ui::Label* s_f2 = add_status(host, "请选择日期（dd/MM/yyyy）");
    ui::LaydateBox* f2 = add_box(host, "", "dd/MM/yyyy", s_f2, "选中：%s");
    f2->set_format("dd/MM/yyyy");

    ui::Label* s_f3 = add_status(host, "请选择月份（yyyyMMdd）");
    ui::LaydateBox* f3 = add_box(host, "", "yyyyMMdd", s_f3, "选中：%s");
    f3->set_format("yyyyMMdd");

    ui::Label* s_f4 = add_status(host, "请选择时间（H点m分）");
    ui::LaydateBox* f4 = add_box(host, "", "H点m分", s_f4, "选中：%s");
    f4->set_type("time");
    f4->set_format("H点m分");

    ui::Label* s_f5 = add_status(host, "请选择范围（month, ~）");
    ui::LaydateBox* f5 = add_box(host, "", " ~ ", s_f5, "选中：%s");
    f5->set_type("month");
    f5->set_range(true);
    f5->set_range_separator(" ~ ");
    f5->set_format("yyyy-MM");

    ui::Label* s_f6 = add_status(host, "请选择范围（datetime, 到）");
    ui::LaydateBox* f6 = add_box(host, "", "开始 到 结束", s_f6, "选中：%s");
    f6->set_type("datetime");
    f6->set_range(true);
    f6->set_range_separator(" 到 ");
    f6->set_format("yyyy年M月d日H时m分s秒");
}

// ---- 分组5：公历节日和自定义重要日子 ----
void demo_calendar(PageHost& host) {
    add_section(host, "公历节日和自定义重要日子");
    ui::Label* s17 = add_status(host, "开启公历节日");
    ui::LaydateBox* b17 = add_box(host, "", "yyyy-MM-dd", s17, "选中：%s");
    b17->set_calendar(true);

    ui::Label* s18 = add_status(host, "自定义重要日");
    ui::LaydateBox* b18 = add_box(host, "", "yyyy-MM-dd", s18, "选中：%s");
    b18->set_marks({
        {"0-10-14", "生日"},
        {"0-12-31", "跨年"},
        {"0-0-10", "工资"},
        {"0-0-15", "月中"},
        {"2017-8-15", ""},
        {"2099-10-14", "呵呵"},
    });
}

// ---- 分组6：控制可选的日期与时间 ----
void demo_limit(PageHost& host) {
    add_section(host, "控制可选的日期与时间");
    add_note(host, "限定可选日期：2016-10-14 到 2080-10-14");
    ui::Label* s_l1 = add_status(host, "限定可选日期");
    ui::LaydateBox* l1 = add_box(host, "", "yyyy-MM-dd", s_l1, "选中：%s");
    l1->set_min("2016-10-14");
    l1->set_max("2080-10-14");

    add_note(host, "前后若干天可选：前几天(-7)到后7天(+7)");
    ui::Label* s_l2 = add_status(host, "前后若干天可选");
    ui::LaydateBox* l2 = add_box(host, "", "yyyy-MM-dd", s_l2, "选中：%s");
    l2->set_min_offset(-7);
    l2->set_max_offset(7);

    add_note(host, "限定可选时间：09:30:00 到 17:30:00（仅清空/确定按钮）");
    ui::Label* s_l3 = add_status(host, "限定可选时间");
    ui::LaydateBox* l3 = add_box(host, "", "HH:mm:ss", s_l3, "选中：%s");
    l3->set_type("time");
    l3->set_min("09:30:00");
    l3->set_max("17:30:00");
    l3->set_btns({"clear", "confirm"});
}

// ---- 分组7：同时绑定多个 ----
void demo_multi(PageHost& host) {
    add_section(host, "同时绑定多个");
    ui::Label* s_m1 = add_status(host, "多输入框 1");
    ui::LaydateBox* m1 = add_box(host, "", "yyyy-MM-dd", s_m1, "输入框1：%s");
    ui::Label* s_m2 = add_status(host, "多输入框 2");
    ui::LaydateBox* m2 = add_box(host, "", "yyyy-MM-dd", s_m2, "输入框2：%s");
    ui::Label* s_m3 = add_status(host, "多输入框 3");
    ui::LaydateBox* m3 = add_box(host, "", "yyyy-MM-dd", s_m3, "输入框3：%s");
}

// ---- 分组8：其它功能示例 ----
void demo_features(PageHost& host) {
    add_section(host, "其它功能示例");
    ui::Label* s19 = add_status(host, "初始赋值：1989-10-14");
    ui::LaydateBox* b19 = add_box(host, "", "yyyy-MM-dd", s19, "初始赋值选中：%s");
    b19->set_value("1989-10-14");

    ui::Label* s20 = add_status(host, "选中后的回调（done）");
    ui::LaydateBox* b20 = add_box(host, "", "yyyy-MM-dd", s20, "你选择的日期是：%s");
    b20->set_on_done([s20](const char* val, int y, int m, int d, int, int, int) {
        char buf[128];
        snprintf(buf, sizeof(buf), "done 回调：%s（%d-%02d-%02d）", val, y, m, d);
        s20->set_text(buf);
    });

    ui::Label* s21 = add_status(host, "日期切换的回调（change）");
    ui::LaydateBox* b21 = add_box(host, "", "yyyy-MM-dd", s21, "change 回调：%s");

    ui::Label* s22 = add_status(host, "不出现底部栏");
    ui::LaydateBox* b22 = add_box(host, "", "yyyy-MM-dd", s22, "选中：%s");
    b22->set_show_bottom(false);

    ui::Label* s23 = add_status(host, "只出现确定按钮");
    ui::LaydateBox* b23 = add_box(host, "", "yyyy-MM-dd", s23, "选中：%s");
    b23->set_btns({"confirm"});

    ui::Label* s27 = add_status(host, "日期只读");
    ui::LaydateBox* b27 = add_box(host, "", "yyyy-MM-dd", s27, "选中：%s");
    b27->set_readonly(true);
    b27->set_date(2026, 8, 18);
}

// ---- 分组9：其它主题 ----
void demo_theme(PageHost& host) {
    add_section(host, "其它主题");
    ui::Label* s29 = add_status(host, "墨绿主题");
    ui::LaydateBox* b29 = add_box(host, "", "yyyy-MM-dd", s29, "选中：%s");
    b29->set_theme_style("molv");

    ui::Label* s30 = add_status(host, "自定义颜色主题");
    ui::LaydateBox* b30 = add_box(host, "", "yyyy-MM-dd", s30, "选中：%s");
    b30->set_theme_style("#393D49");

    ui::Label* s31 = add_status(host, "格子主题");
    ui::LaydateBox* b31 = add_box(host, "", "yyyy-MM-dd", s31, "选中：%s");
    b31->set_theme_style("grid");
}

// ---- 分组10：直接嵌套显示（static） ----
void demo_static(PageHost& host) {
    add_section(host, "直接嵌套显示");
    ui::Laydate* n1 = new ui::Laydate(0, 0, 280.f, 340.f);
    n1->set_font(host.font);
    n1->set_theme(host.theme);
    host.content->add_child(n1);

    ui::Laydate* n2 = new ui::Laydate(0, 0, 280.f, 340.f);
    n2->set_font(host.font);
    n2->set_theme(host.theme);
    n2->set_lang("en");
    host.content->add_child(n2);

    ui::Laydate* n3 = new ui::Laydate(0, 0, 280.f, 300.f);
    n3->set_font(host.font);
    n3->set_theme(host.theme);
    n3->set_type("month");
    host.content->add_child(n3);

    ui::Laydate* n4 = new ui::Laydate(0, 0, 280.f, 220.f);
    n4->set_font(host.font);
    n4->set_theme(host.theme);
    n4->set_type("time");
    host.content->add_child(n4);
}

// ---- 汇总构建 ----
void build_laydate(PageHost& host) {
    demo_basic(host);
    demo_pickers(host);
    demo_range(host);
    demo_format(host);
    demo_calendar(host);
    demo_limit(host);
    demo_multi(host);
    demo_features(host);
    demo_theme(host);
    demo_static(host);

    // ---- 说明 ----
    add_section(host, "说明");
    add_note(host,
        "本页一键复制 layui laydate.html：测试1-31 为「输入框+点击弹出」交互（LaydateBox），测试-n1..n4 为直接嵌套显示（Laydate 内嵌）。\n"
        "类型：set_type(\"year\"/\"month\"/\"date\"/\"time\"/\"datetime\")\n"
        "范围：set_range(true)，连续点两个值\n"
        "格式：set_format(\"yyyy年MM月dd日\"/\"dd/MM/yyyy\"/\"H点m分\")\n"
        "节日/标注：set_calendar(true) + set_marks({{\"0-10-14\",\"生日\"}})\n"
        "限定：set_min/set_max(\"2016-10-14\") 或 set_min_offset(-7)/set_max_offset(7)\n"
        "主题：set_theme_style(\"molv\"/\"grid\"/\"#393D49\")\n"
        "底部栏：set_show_bottom(false)、set_btns({\"clear\",\"confirm\"})\n"
        "国际化：set_lang(\"en\")；只读：set_readonly(true)");

    // ---- 代码 ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/laydate_page.cpp（节选）：克隆 layui laydate.html\n"
            "\n"
            "// 弹层模式（input + popup）：LaydateBox\n"
            "ui::LaydateBox* box = new ui::LaydateBox(0, 0, 280.f, 38.f);\n"
            "box->set_font(host.font); box->set_theme(host.theme);\n"
            "box->set_placeholder(\"yyyy-MM-dd\");\n"
            "box->set_type(\"datetime\");\n"
            "box->set_range(true);\n"
            "box->set_format(\"yyyy年MM月dd日\");\n"
            "box->set_marks({{\"0-10-14\", \"生日\"}});\n"
            "box->set_theme_style(\"molv\");\n"
            "box->set_btns({\"confirm\"});\n"
            "box->set_on_change([](const char* v) { /* 选中回调 */ });\n"
            "host.content->add_child(box);\n"
            "\n"
            "// 静态内嵌模式（position:static）：Laydate\n"
            "ui::Laydate* d = new ui::Laydate(0, 0, 280.f, 300.f);\n"
            "d->set_font(host.font); d->set_theme(host.theme);\n"
            "d->set_type(\"month\");\n"
            "host.content->add_child(d);");
    }

    // ---- 帮助 ----
    if (host.help) {
        const char* lines[] = {
            "日期时间选择器：两种呈现模式",
            "",
            "【弹层模式（input+popup）LaydateBox】对应 laydate.html test1..test31",
            "  set_type(\"year\"/\"month\"/\"date\"/\"time\"/\"datetime\")：类型",
            "  set_range(true)：范围选择（连续点两个）",
            "  set_format(\"yyyy/MM/dd H点m分\")：自定义格式（y/M/d/H/m/s 占位）",
            "  set_min/set_max(\"2016-10-14\")：绝对日期限定（时间类型为 \"09:30:00\"）",
            "  set_min_offset(-7)/set_max_offset(7)：相对今天偏移（天）",
            "  set_calendar(true)：公历节日",
            "  set_marks({{\"0-10-14\",\"生日\"},{2017-8-15,\"\"}})：自定义标注",
            "  set_theme_style(\"molv\"/\"grid\"/\"#393D49\")：墨绿/格子/自定义色",
            "  set_show_bottom(false)：隐藏底部栏",
            "  set_btns({\"clear\",\"confirm\"})：自定义底部按钮",
            "  set_lang(\"en\")：英文界面",
            "  set_readonly(true)：只读",
            "  set_value(\"1989-10-14\")：初始赋值",
            "  set_on_change(cb)：值变化回调 on_change(字符串)",
            "  set_on_done(cb)：确定回调",
            "",
            "【静态内嵌模式（position:static）Laydate】对应 test-n1..n4",
            "  直接作为 Widget 放入树内显示：set_type/set_lang 等同样生效",
        };
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

}  // namespace

static void build(PageHost& host) {
    build_laydate(host);
}

static const PageDef kLaydatePage{51, "交互", "日期与时间选择",
                                  "日期时间选择器：弹层/static 两种模式 40 demo", build, nullptr, false, nullptr, 0, "Laydate"};

static const bool kRegistered = []() {
    page::register_page(kLaydatePage);
    return true;
}();