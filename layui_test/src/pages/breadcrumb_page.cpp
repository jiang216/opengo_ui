#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/breadcrumb.h"
#include "ui/ui/code.h"
#include "ui/ui/label.h"
#include "ui/ui/navbar.h"
#include "ui/ui/navmenu.h"
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

ui::Breadcrumb* add_breadcrumb(PageHost& host) {
    ui::Breadcrumb* bc = new ui::Breadcrumb(0, 0, 800, 26.f);
    bc->set_font(host.font);
    bc->set_theme(host.theme);
    host.content->add_child(bc);
    return bc;
}

void build_nav(PageHost& host) {
    // ── 水平导航菜单 ──
    add_section(host, "水平导航菜单");
    {
        auto* nav = new ui::NavBar(0, 0, 800, 40.f);
        nav->set_font(host.font);
        nav->set_theme(host.theme);
        nav->add_item("选中");
        nav->set_active(0);
        nav->add_item("常规");
        nav->add_item("导航");
        nav->add_item("子级");
        nav->add_child("菜单1");
        nav->add_child("菜单2");
        nav->add_child("菜单3");
        nav->add_item("选项");
        nav->add_child("选项1");
        nav->add_child("选项2");
        nav->add_child("选项3");
        nav->add_item("演示");
        host.content->add_child(nav);
    }

    // ── 导航带徽章和图片 ──
    add_section(host, "导航带徽章和图片");
    {
        auto* nav = new ui::NavBar(0, 0, 800, 40.f);
        nav->set_font(host.font);
        nav->set_theme(host.theme);
        nav->set_bar_disabled(true);
        nav->add_item("\xe5\xb8\xa6\xe5\xbe\xbd\xe7\xab\xa0");  // 带徽章
        nav->set_badge("9");
        nav->add_item("\xe5\xb0\x8f\xe5\x9c\x86\xe7\x82\xb9");  // 小圆点
        nav->set_dot_badge();
        nav->add_item("");  // 图片项（无文字）
        nav->set_image("37326744.png");
        nav->add_child("\xe5\xad\x90\xe7\xba\xa7\xe8\x8f\x9c\xe5\x8d\x95");  // 子级菜单
        nav->add_child("\xe6\xa8\xaa\xe7\xba\xbf\xe9\x9a\x94\xe6\x96\xad");  // 横线隔断
        nav->add_child("\xe9\x80\x80\xe5\x87\xba");  // 退出
        host.content->add_child(nav);
    }

    {
        auto* bq = new ui::BlockQuote(800, 0, "\xe8\xa5\xbf\xe8\xaf\x84\xe8\xbf\x90\xe9\x80\x9a\xe8\xbf\x87\xe9\x85\x8d\xe7\xbd\xae lay-bar=\"disabled\" \xe5\xb1\x9e\xe6\x80\xa7\xef\xbc\x8c\xe7\xa6\x81\xe7\x94\xa8\xe4\xba\x86\xe6\xbb\x91\xe5\x9d\x97\xe8\xb7\x9f\xe9\x9a\x8f\xe5\x8a\x9f\xe8\x83\xbd");  // 该导航通过配置 lay-bar="disabled" 属性，禁用了滑块跟随功能
        bq->set_font(host.font);
        bq->set_theme(host.theme);
        host.content->add_child(bq);
    }

    // ── 更多导航主题 ──
    add_section(host, "更多导航主题");
    {
        auto* nav_cyan = new ui::NavBar(0, 0, 800, 40.f);
        nav_cyan->set_font(host.font);
        nav_cyan->set_theme(host.theme);
        nav_cyan->set_bg_color({0.f, 0.72f, 0.84f, 1.f});  // cyan
        nav_cyan->add_item("\xe8\x97\x8f\xe9\x9d\x92\xe5\xaf\xbc\xe8\x88\xaa");  // 藏青导航
        nav_cyan->add_item("\xe4\xba\xa7\xe5\x93\x81");  // 产品
        nav_cyan->add_item("\xe5\xa4\xa7\xe6\x95\xb0\xe6\x8d\xae");  // 大数据
        nav_cyan->add_item("\xe8\xa7\xa3\xe5\x86\xb3\xe6\x96\xb9\xe6\xa1\x88");  // 解决方案
        nav_cyan->add_child("\xe7\xa7\xbb\xe5\x8a\xa8\xe6\xa8\xa1\xe5\x9d\x97");  // 移动模块
        nav_cyan->add_child("\xe5\x90\x8e\xe5\x8f\xb0\xe6\xa8\xa1\xe7\x89\x88");  // 后台模版
        nav_cyan->add_child("\xe7\x94\xb5\xe5\x95\x86\xe5\xb9\xb3\xe5\x8f\xb0");  // 电商平台
        nav_cyan->add_item("\xe7\xa4\xbe\xe5\x8c\xba");  // 社区
        host.content->add_child(nav_cyan);
    }
    {
        auto* nav_green = new ui::NavBar(0, 0, 800, 40.f);
        nav_green->set_font(host.font);
        nav_green->set_theme(host.theme);
        nav_green->set_bg_color({0.25f, 0.55f, 0.35f, 1.f});  // green
        nav_green->add_item("\xe5\xa2\xa8\xe7\xbb\xbf\xe5\xaf\xbc\xe8\x88\xaa");  // 墨绿导航
        nav_green->add_item("\xe4\xba\xa7\xe5\x93\x81");
        nav_green->add_item("\xe5\xa4\xa7\xe6\x95\xb0\xe6\x8d\xae");
        nav_green->add_item("\xe8\xa7\xa3\xe5\x86\xb3\xe6\x96\xb9\xe6\xa1\x88");
        nav_green->add_child("\xe7\xa7\xbb\xe5\x8a\xa8\xe6\xa8\xa1\xe5\x9d\x97");
        nav_green->add_child("\xe5\x90\x8e\xe5\x8f\xb0\xe6\xa8\xa1\xe7\x89\x88");
        nav_green->add_child("\xe7\x94\xb5\xe5\x95\x86\xe5\xb9\xb3\xe5\x8f\xb0");
        nav_green->add_item("\xe7\xa4\xbe\xe5\x8c\xba");
        host.content->add_child(nav_green);
    }
    {
        auto* nav_blue = new ui::NavBar(0, 0, 800, 40.f);
        nav_blue->set_font(host.font);
        nav_blue->set_theme(host.theme);
        nav_blue->set_bg_color({0.2f, 0.5f, 0.85f, 1.f});  // blue
        nav_blue->set_bar_disabled(true);
        nav_blue->add_item("\xe8\x89\xb3\xe8\x93\x9d\xe5\xaf\xbc\xe8\x88\xaa");  // 艳蓝导航
        nav_blue->add_item("\xe4\xba\xa7\xe5\x93\x81");
        nav_blue->add_item("\xe5\xa4\xa7\xe6\x95\xb0\xe6\x8d\xae");
        nav_blue->add_item("\xe8\xa7\xa3\xe5\x86\xb3\xe6\x96\xb9\xe6\xa1\x88");
        nav_blue->add_child("\xe7\xa7\xbb\xe5\x8a\xa8\xe6\xa8\xa1\xe5\x9d\x97");
        nav_blue->add_child("\xe5\x90\x8e\xe5\x8f\xb0\xe6\xa8\xa1\xe7\x89\x88");
        nav_blue->add_child("\xe7\x94\xb5\xe5\x95\x86\xe5\xb9\xb3\xe5\x8f\xb0");
        nav_blue->add_item("\xe7\xa4\xbe\xe5\x8c\xba");
        host.content->add_child(nav_blue);
    }

    // ── 垂直导航菜单 ──
    add_section(host, "垂直导航菜单");
    {
        auto* nav1 = new ui::NavMenu(0, 0, 200.f, 0.f);
        nav1->set_font(host.font);
        nav1->set_theme(host.theme);
        nav1->add_group("\xe9\xbb\x98\xe8\xae\xa4\xe5\xb1\x95\xe5\xbc\x80");  // 默认展开
        nav1->add_item("\xe9\x80\x89\xe9\xa1\xb9\xe4\xb8\x80");  // 选项一
        nav1->add_item("\xe9\x80\x89\xe9\xa1\xb9\xe4\xba\x8c");  // 选项二
        nav1->add_item("\xe9\x80\x89\xe9\xa1\xb9\xe4\xb8\x89");  // 选项三
        nav1->add_item("\xe8\xb7\xb3\xe8\xbd\xac\xe9\xa1\xb9");  // 跳转项
        nav1->add_group("\xe8\xa7\xa3\xe5\x86\xb3\xe6\x96\xb9\xe6\xa1\x88");  // 解决方案
        nav1->add_item("\xe7\xa7\xbb\xe5\x8a\xa8\xe6\xa8\xa1\xe5\x9d\x97");
        nav1->add_item("\xe5\x90\x8e\xe5\x8f\xb0\xe6\xa8\xa1\xe7\x89\x88");
        nav1->add_item("\xe7\x94\xb5\xe5\x95\x86\xe5\xb9\xb3\xe5\x8f\xb0");
        nav1->add_group("\xe4\xba\x91\xe5\xb8\x82\xe5\x9c\xba");  // 云市场
        nav1->add_group("\xe7\xa4\xbe\xe5\x8c\xba");  // 社区
        nav1->set_active(0);
        nav1->expand_all();
        host.content->add_child(nav1);
    }

    // ── 侧边固定导航菜单 ──
    add_section(host, "侧边固定导航菜单");
    {
        auto* bq = new ui::BlockQuote(800, 0, "\xe5\x9b\xbe\xe5\x9c\x88\xef\xbc\x8c\xe5\xb0\xb1\xe5\xb7\xa6\xe8\xbe\xb9\xe9\x82\xa3\xe5\xae\xb6\xe4\xbc\x81\xef\xbc\x81");  // 囖，就左边那家伙！
        bq->set_font(host.font);
        bq->set_theme(host.theme);
        host.content->add_child(bq);
    }

    // ── 默认面包屑 ──
    add_section(host, "默认面包屑");
    {
        ui::Breadcrumb* bc = add_breadcrumb(host);
        bc->add_item("\xe9\xa6\x96\xe9\xa1\xb5");  // 首页
        bc->add_item("\xe6\xbc\x94\xe7\xa4\xba");  // 演示
        bc->add_current("\xe5\xaf\xbc\xe8\x88\xaa\xe5\x85\x83\xe7\xb4\xa0");  // 导航元素
    }

    // ── 自定义分隔符的面包屑 ──
    add_section(host, "自定义分隔符的面包屑");
    {
        ui::Breadcrumb* bc = add_breadcrumb(host);
        bc->set_separator(" \xe2\x80\x94 ");
        bc->add_item("\xe9\xa6\x96\xe9\xa1\xb5");  // 首页
        bc->add_item("\xe5\x9b\xbd\xe9\x99\x85\xe6\x96\xb0\xe9\x97\xbb");  // 国际新闻
        bc->add_item("\xe4\xba\x9a\xe5\xa4\xaa\xe5\x9c\xb0\xe5\x8c\xba");  // 亚太地区
        bc->add_current("\xe6\xad\xa3\xe6\x96\x87");  // 正文
    }

    // ── 门户频道面包屑 ──
    add_section(host, "\xe8\xbf\x98\xe5\x8f\xaf\xe4\xbb\xa5\xe7\x94\xa8\xe4\xba\x8e\xe9\x97\xa8\xe6\x88\xb7\xe9\xa2\x91\xe9\x81\x93\xe7\x9a\x84\xe9\x9d\xa2\xe5\x8c\x85\xe5\xb1\x91");  // 还可以用于门户频道的面包屑
    {
        ui::Breadcrumb* bc = add_breadcrumb(host);
        bc->set_separator(" | ");
        bc->add_item("\xe5\xa8\xb1\xe4\xb9\x90");  // 娱乐
        bc->add_item("\xe5\x85\xab\xe5\x8d\xa6");  // 八卦
        bc->add_item("\xe4\xbd\x93\xe8\x82\xb2");  // 体育
        bc->add_item("\xe6\x90\x9e\xe7\xac\x91");  // 搞笑
        bc->add_item("\xe8\xa7\x86\xe9\xa2\x91");  // 视频
        bc->add_item("\xe6\xb8\xb8\xe6\x88\x8f");  // 游戏
        bc->add_item("\xe7\xbb\xbc\xe8\x89\xba");  // 综艺
    }

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// --- NavBar 水平导航 ---\n"
            "auto* nav = new ui::NavBar(0, 0, 800, 40.f);\n"
            "nav->set_font(host.font);\n"
            "nav->set_theme(host.theme);\n"
            "nav->add_item(\"选中\");\n"
            "nav->set_active(0);  // layui-this\n"
            "nav->add_item(\"子级\");\n"
            "nav->add_child(\"菜单1\");  // 下拉子项\n"
            "nav->set_bg_color({0, 0.72f, 0.84f, 1});  // cyan 主题\n"
            "\n"
            "// --- Breadcrumb 面包屑 ---\n"
            "auto* bc = new ui::Breadcrumb(0, 0, 800, 26.f);\n"
            "bc->set_separator(\" — \");\n"
            "bc->add_item(\"首页\");\n"
            "bc->add_current(\"正文\");\n"
            "\n"
            "// --- NavMenu 垂直导航 ---\n"
            "auto* tree = new ui::NavMenu(0, 0, 200, 200);\n"
            "tree->add_group(\"默认展开\");\n"
            "tree->add_item(\"选项一\");\n"
            "tree->expand_all();");
    }
    if (host.help) {
        const char* lines[] = {
            "NavBar（横向导航）API",
            "  add_item(text, cb)：追加顶级导航项",
            "  add_child(text, cb)：给最后项追加下拉子菜单",
            "  set_active(n)：设置选中项（layui-this）",
            "  set_badge(text) / set_dot_badge()：徽章",
            "  set_bg_color({r,g,b,a})：彩色主题",
            "  set_bar_disabled(bool)：禁用滑块跟随",
            "",
            "Breadcrumb（面包屑）API",
            "  add_item(text)：链接项 | add_current(text)：当前页 cite",
            "  set_separator(str)：分隔符，默认 \" / \"",
            "",
            "NavMenu（垂直导航）API",
            "  add_group(title)：追加一级组",
            "  add_item(text, cb)：给最后组追加子项",
            "  expand_all()：展开所有组",
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

static void build(PageHost& host) { build_nav(host); }

static const PageDef kBreadcrumbPage{34, "展示", "导航/面包屑",
                                     "\xe5\xaf\xbc\xe8\x88\xaa/\xe9\x9d\xa2\xe5\x8c\x85\xe5\xb1\x91", build, nullptr, false, nullptr, 0, "Nav"};

static const bool kRegistered = []() {
    page::register_page(kBreadcrumbPage);
    return true;
}();
