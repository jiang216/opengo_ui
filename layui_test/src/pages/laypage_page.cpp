#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/label.h"
#include "ui/ui/laypage.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

#include <algorithm>
#include <cstdio>
#include <string>

namespace {

void add_section(PageHost& host, const char* text) {
    ui::Label* label = new ui::Label(0, 0, 800, 0, text);
    label->set_font(host.font);
    label->set_theme(host.theme);
    label->set_color(host.theme->text_secondary);
    host.content->add_child(label);
}

// 把私有用区码点（layui 图标）编码为 UTF-8，用作上一页/下一页文本
std::string u8cp(uint32_t cp) {
    std::string s;
    if (cp < 0x80) {
        s.push_back((char)cp);
    } else if (cp < 0x800) {
        s.push_back((char)(0xC0 | (cp >> 6)));
        s.push_back((char)(0x80 | (cp & 0x3F)));
    } else {
        s.push_back((char)(0xE0 | (cp >> 12)));
        s.push_back((char)(0x80 | ((cp >> 6) & 0x3F)));
        s.push_back((char)(0x80 | (cp & 0x3F)));
    }
    return s;
}

ui::Vec4 rgb(int r, int g, int b) { return ui::Vec4{r / 255.f, g / 255.f, b / 255.f, 1.0f}; }

ui::Laypage* new_page(PageHost& host) {
    ui::Laypage* page = new ui::Laypage(0, 0, 800, 28.f);
    page->set_font(host.font);
    page->set_theme(host.theme);
    host.content->add_child(page);
    return page;
}

void build_laypage(PageHost& host) {
    // 总页数低于页码总数 → 全部页码直显
    add_section(host, "总页数低于页码总数（count 50）");
    new_page(host)->set_count(50);

    // 总页数大于页码总数 → 折叠省略号 + 切页回调
    add_section(host, "总页数大于页码总数（count 70，折叠省略号 + jump 回调）");
    {
        ui::Label* status = new ui::Label(0, 0, 800, 0, "当前页：1");
        status->set_font(host.font);
        status->set_theme(host.theme);
        status->set_color(host.theme->text);
        host.content->add_child(status);
        ui::Laypage* page = new_page(host);
        page->set_count(70);
        page->set_on_change([status](int p) {
            char buf[32];
            std::snprintf(buf, sizeof(buf), "当前页：%d（jump: curr=%d）", p + 1, p + 1);
            status->set_text(buf);
        });
    }

    // 自定义主题 - 颜色随意定义
    add_section(host, "自定义主题 - 颜色随意定义（count 100）");
    {
        ui::Laypage* a = new_page(host);
        a->set_count(100);
        a->set_theme_color(rgb(0x1E, 0x9F, 0xFF));
        ui::Laypage* b = new_page(host);
        b->set_count(100);
        b->set_theme_color(rgb(0xFF, 0x57, 0x22));
        ui::Laypage* c = new_page(host);
        c->set_count(100);
        c->set_theme_color(rgb(0xFF, 0xB8, 0x00));
    }

    // 自定义首页/尾页/上一页/下一页文本
    add_section(host, "自定义首页、尾页、上一页、下一页文本（count 100）");
    {
        ui::Laypage* page = new_page(host);
        page->set_count(100);
        page->set_first_text("首页");
        page->set_last_text("尾页");
        page->set_prev_text(u8cp(ui::layui_icon::prev).c_str());
        page->set_next_text(u8cp(ui::layui_icon::next).c_str());
    }

    // 不显示首页尾页
    add_section(host, "不显示首页尾页（count 100）");
    {
        ui::Laypage* page = new_page(host);
        page->set_count(100);
        page->set_show_first(false);
        page->set_show_last(false);
    }

    // 开启 HASH（URL 无 hash 环境，等价：默认布局 + 指定起始页）
    add_section(host, "开启 HASH（count 500，模拟初始页=8）");
    {
        ui::Laypage* page = new_page(host);
        page->set_count(500);
        page->set_page(7);  // 0 基第 8 页
    }

    // 只显示上一页、下一页
    add_section(host, "只显示上一页、下一页（count 50）");
    {
        ui::Laypage* page = new_page(host);
        page->set_count(50);
        page->set_layout_str(std::vector<std::string>{"prev", "next"});
    }

    // 显示完整功能
    add_section(host, "显示完整功能（count 100：count/prev/page/next/limit/refresh/skip）");
    {
        ui::Laypage* page = new_page(host);
        page->set_limits(std::vector<int>{10, 20, 30});
        page->set_count(100);
        page->set_layout_str(std::vector<std::string>{"count", "prev", "page", "next",
                                                     "limit", "refresh", "skip"});
    }

    // 自定义排版
    add_section(host, "自定义排版（count 1000）");
    {
        ui::Laypage* demo8 = new_page(host);
        demo8->set_count(1000);
        demo8->set_layout_str(std::vector<std::string>{"limit", "prev", "page", "next"});
        ui::Laypage* demo9 = new_page(host);
        demo9->set_count(1000);
        demo9->set_layout_str(std::vector<std::string>{"prev", "next", "page"});
        ui::Laypage* demo10 = new_page(host);
        demo10->set_count(1000);
        demo10->set_layout_str(std::vector<std::string>{"page", "count"});
    }

    // 自定义每页条数的选择项
    add_section(host, "自定义每页条数的选择项（count 1000，limit 100，limits 100/300/500）");
    {
        ui::Laypage* page = new_page(host);
        page->set_limit(100);
        page->set_limits(std::vector<int>{100, 300, 500});
        page->set_count(1000);
        page->set_layout_str(std::vector<std::string>{"limit", "prev", "page", "next"});
    }

    // 将一段已知数组分页展示
    add_section(host, "将一段已知数组分页展示（count 37，每页 10）");
    {
        const char* cities[] = {
            "北京", "上海", "广州", "深圳", "杭州", "长沙", "合肥", "宁夏", "成都", "西安",
            "南昌", "上饶", "沈阳", "济南", "厦门", "福州", "九江", "宜春", "赣州", "宁波",
            "绍兴", "无锡", "苏州", "徐州", "东莞", "佛山", "中山", "成都", "武汉", "青岛",
            "天津", "重庆", "南京", "九江", "香港", "澳门", "台北"};
        constexpr int n = sizeof(cities) / sizeof(cities[0]);
        constexpr int per = 10;

        ui::Label* list = new ui::Label(0, 0, 800, 0, "");
        list->set_font(host.font);
        list->set_theme(host.theme);
        list->set_color(host.theme->text);
        host.content->add_child(list);

        ui::Laypage* page = new_page(host);
        page->set_count(n);
        page->set_limit(per);
        page->set_on_change([list, cities, n, per](int p0) {
            const int start = p0 * per;
            const int end = std::min(n, start + per);
            std::string s;
            char buf[16];
            std::snprintf(buf, sizeof(buf), "第 %d/%d 页：", p0 + 1, (n + per - 1) / per);
            s = buf;
            for (int i = start; i < end; ++i) {
                if (i > start) s += "、";
                s += cities[i];
            }
            list->set_text(s.c_str());
        });
    }

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/laypage_page.cpp：克隆 layui laypage.html 全部 demo\n"
            "\n"
            "ui::Laypage* p = new ui::Laypage(0, 0, 800, 28.f);\n"
            "p->set_font(host.font);\n"
            "p->set_theme(host.theme);\n"
            "\n"
            "p->set_count(100);                       // 数据总数→页数=ceil(count/limit)\n"
            "p->set_limit(10);\n"
            "p->set_theme_color(Vec4{0.12f,0.62f,1.0f,1});   // 自定义当前页主题色\n"
            "p->set_show_first(false);               // 不显示首页尾页\n"
            "p->set_layout_str({\"count\",\"prev\",\"page\",\"next\",\"limit\",\"refresh\",\"skip\"});\n"
            "p->set_limits({10,20,30});              // limit 下拉候选\n"
            "p->set_on_change([](int p0){ /* 0 基翻页回调 */ });\n"
            "host.content->add_child(p);");
    }
    if (host.help) {
        const char* lines[] = {
            "分页（Laypage）API",
            "  set_count(n)：数据总数，页数=ceil(n/limit)；set_total(pg)：直接设页数",
            "  set_limit(n)：每页条数；set_limits(v)：limit 下拉候选（布局含 limit 时）",
            "  set_layout_str({...})：count/prev/page/next/limit/refresh/skip/first/last",
            "  set_show_first()/set_show_last()：首页/尾页显隐；set_*_text()：自定义按钮文本",
            "  set_theme_color(c)：当前页主题色；set_page(p)：0 基当前页",
            "  set_on_change(int) / set_on_jump(curr,limit) / set_on_refresh()：回调",
            "外观：项高 28；页码≤7 全显，否则首页/当前±1/末页 + 省略号；默认 #16baaa 当前页",
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
    build_laypage(host);
}

static const PageDef kLaypagePage{32, "展示", "分页", "分页：镜像 layui laypage 全部 demo（主题/排版/limit/skip/数组分页）", build, nullptr, false, nullptr, 0, "Laypage"};

static const bool kRegistered = []() {
    page::register_page(kLaypagePage);
    return true;
}();