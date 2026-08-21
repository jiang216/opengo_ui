#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/icon.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;

// layui-icon 全量类名（190 项，与 ui::layui_icons.h 码点表一一对应）
const char* const kIconNames[] = {
    "404", "about", "add-1", "add-circle", "add-circle-fine", "addition",
    "align-center", "align-left", "align-right", "android", "app", "at",
    "auz", "backspace", "bluetooth", "camera", "camera-fill", "carousel",
    "cart", "cart-simple", "cellphone", "cellphone-fine", "chart",
    "chart-screen", "chat", "chrome", "circle", "circle-dot", "clear",
    "code-circle", "cols", "component", "console", "date", "delete",
    "dialogue", "diamond", "disabled", "dollar", "down", "download-circle",
    "edge", "edit", "email", "engine", "error", "export", "eye",
    "eye-invisible", "face-cry", "face-smile", "face-smile-b",
    "face-smile-fine", "face-surprised", "female", "file", "file-b",
    "find-fill", "fire", "firefox", "flag", "folder", "folder-open",
    "fonts-clear", "fonts-code", "fonts-del", "fonts-html", "fonts-i",
    "fonts-strong", "fonts-u", "form", "friends", "gift", "gitee", "github",
    "group", "headset", "heart", "heart-fill", "help", "home", "ie", "ios",
    "key", "layer", "layim-download", "layim-theme", "layim-uploadfile",
    "layouts", "leaf", "left", "light", "link", "list", "loading",
    "loading-1", "location", "lock", "log", "login-qq", "login-wechat",
    "login-weibo", "logout", "male", "menu-fill", "mike", "moon", "more",
    "more-vertical", "music", "mute", "next", "note", "notice", "ok",
    "password", "pause", "picture", "picture-fine", "play", "praise", "prev",
    "prev-circle", "print", "question", "radio", "rate", "rate-half",
    "rate-solid", "read", "reduce-circle", "refresh", "refresh-1",
    "refresh-3", "release", "reply-fill", "return", "right", "rmb", "rss",
    "screen-full", "screen-restore", "search", "senior", "service", "set",
    "set-fill", "set-sm", "share", "shrink-right", "slider", "snowflake",
    "speaker", "spread-left", "star", "star-fill", "subtraction", "success",
    "survey", "table", "tabs", "template", "template-1", "templeate-1",
    "test", "theme", "time", "tips", "tips-fill", "top", "transfer", "tread",
    "tree", "triangle-d", "triangle-r", "unlink", "up", "upload",
    "upload-circle", "upload-drag", "user", "username", "util", "vercode",
    "video", "voice", "water", "website", "wifi", "windows",
};
constexpr int kIconCount = (int)(sizeof(kIconNames) / sizeof(kIconNames[0]));

// 单元格：图标 + 类名（VBox）
ui::VBox* cell(PageHost& h, const char* name) {
    auto* v = new ui::VBox(0, 0, 188.f, 0);
    v->set_padding(6.f);
    v->set_spacing(4.f);
    v->set_theme(h.theme);
    auto* ic = new ui::Icon(0, 0, 176.f, 22.f, name);
    ic->set_font(h.font);
    v->add_child(ic);
    std::string label = std::string("layui-icon-") + name;
    auto* l = new ui::Label(0, 0, 176.f, 0, label.c_str());
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text_muted);
    v->add_child(l);
    return v;
}

void build_icon(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "layui-icon 图标库（190 项）：字形来自 iconfont.ttf 私用区码点，"
        "由 ui::Icon + ui::layui_icon 码点表渲染。用法 class=\"layui-icon layui-icon-xxx\"。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 彩色图标 ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "彩色图标（set_color）");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        auto* r = new ui::HBox(0, 0, kCW, 30.f);
        r->set_padding(0.f);
        r->set_spacing(28.f);
        r->set_theme(host.theme);
        struct C { const char* name; ui::Vec4 color; };
        static const C kColored[] = {
            {"heart-fill", {1.f, 0.34f, 0.13f, 1.f}},
            {"star-fill", {1.f, 0.72f, 0.f, 1.f}},
            {"like", {0.09f, 0.72f, 0.47f, 1.f}},
            {"notice", {0.12f, 0.62f, 1.f, 1.f}},
            {"diamond", {0.64f, 0.20f, 0.78f, 1.f}},
            {"fire", {0.18f, 0.25f, 0.34f, 1.f}},
        };
        for (const C& c : kColored) {
            if (!ui::layui_icon::from_class(c.name)) continue;  // 表中无此名则跳过
            auto* ic = new ui::Icon(0, 0, 24.f, 24.f, c.name);
            ic->set_font(host.font);
            ic->set_color(c.color);
            r->add_child(ic);
        }
        host.content->add_child(r);
    }

    // ---- 按钮图标（镜像 h2#icon 三组） ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "按钮图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        // 第一组：默认尺寸 + 默认主题（1 个文字+图标 + 4 个纯图标）
        {
            auto* r = new ui::HBox(0, 0, kCW, 38.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);

            auto* b1 = new ui::Button(0, 0, 80.f, 38.f, "按钮");
            b1->set_font(host.font);
            b1->set_theme(host.theme);
            b1->set_icon("down");
            r->add_child(b1);

            struct I { const char* icon; };
            static const I kDef[] = {{"left"}, {"right"}, {"edit"}, {"share"}};
            for (const I& i : kDef) {
                auto* b = new ui::Button(0, 0, 44.f, 38.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(i.icon);
                r->add_child(b);
            }
            host.content->add_child(r);
        }

        // 第二组：小号 + primary（5 个纯图标）
        {
            auto* r = new ui::HBox(0, 0, kCW, 30.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            struct I { const char* icon; };
            static const I kSmP[] = {{"left"}, {"right"}, {"edit"}, {"delete"}, {"share"}};
            for (const I& i : kSmP) {
                auto* b = new ui::Button(0, 0, 34.f, 30.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(i.icon);
                b->add_class("layui-btn layui-btn-primary layui-btn-sm");
                r->add_child(b);
            }
            host.content->add_child(r);
        }

        // 第三组：小号 + 混合主题（disabled/normal/warm/danger）
        {
            auto* r = new ui::HBox(0, 0, kCW, 30.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            struct I { const char* icon; const char* cls; };
            static const I kSmM[] = {
                {"delete", "layui-btn layui-btn-sm layui-btn-disabled"},
                {"left",   "layui-btn layui-btn-sm"},
                {"right",  "layui-btn layui-btn-sm layui-btn-warm"},
                {"edit",   "layui-btn layui-btn-sm layui-btn-danger"},
            };
            for (const I& i : kSmM) {
                auto* b = new ui::Button(0, 0, 34.f, 30.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(i.icon);
                b->add_class(i.cls);
                r->add_child(b);
            }
            host.content->add_child(r);
        }
    }

    // ---- 全部图标 ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "全部图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        constexpr int kPerRow = 5;
        for (int base = 0; base < kIconCount; base += kPerRow) {
            auto* r = new ui::HBox(0, 0, kCW, 0);
            r->set_padding(0.f);
            r->set_spacing(10.f);
            r->set_theme(host.theme);
            for (int i = base; i < base + kPerRow && i < kIconCount; ++i)
                r->add_child(cell(host, kIconNames[i]));
            host.content->add_child(r);
        }
    }

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/icon_page.cpp：克隆 layui 图标文档\n"
            "\n"
            "#include \"ui/ui/icon.h\"\n"
            "\n"
            "ui::Icon* ic = new ui::Icon(0, 0, 30, 30, \"star\");\n"
            "ic->set_font(host.font);   // iconfont.ttf 已并入 Font 私用区\n"
            "ic->set_color(theme.accent);\n"
            "\n"
            "// 码点查询：ui::layui_icon::from_class(\"star\") -> 0xE67A\n"
            "// 字体加载：Font::load_icon_font(\"iconfont.ttf\", 18.f)");
    }
    if (host.help) {
        const char* lines[] = {
            "图标 API 一览",
            "",
            "ui::Icon：单个私用区图标码点控件",
            "  构造 Icon(x, y, w, h, \"star\")：名字不含 layui-icon- 前缀",
            "  set_icon(name)：换图标；找不到则清空",
            "  set_color(Vec4)：着色（默认主题文字色）",
            "",
            "ui::layui_icon（layui_icons.h，自动生成勿手改）",
            "  from_class(name)：类名 -> 码点，找不到返回 0",
            "  常量表：ic_404 / about / star ... 共 190 项",
            "",
            "字体：exe 目录 iconfont.ttf 经 Font::load_icon_font 并入同一 Font，",
            "与中文文本共用（POST_BUILD 拷贝，UI-048 exe 目录回退）。",
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

static void build(PageHost& host) { build_icon(host); }

static const PageDef kIconPage{11, "通用", "图标", "layui-icon 图标库：按钮图标 + 彩色示例 + 190 项全量网格", build, nullptr, false, nullptr, 0, "Icon"};

static const bool kRegistered = []() {
    page::register_page(kIconPage);
    return true;
}();
