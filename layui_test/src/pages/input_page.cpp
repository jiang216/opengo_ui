#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/textarea.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"

namespace {

// 内容区宽度（ScrollBox content_w = 1060 - 48 - 12 ≈ 1000）
constexpr float kCW = 1000.f;
constexpr float kFP = 12.f;
constexpr float kIW = kCW - 2.f * kFP;

ui::HBox* make_row(PageHost& h, float rh) {
    auto* r = new ui::HBox(0, 0, kIW, rh);
    r->set_padding(0.f);
    r->set_spacing(12.f);
    r->set_theme(h.theme);
    return r;
}

ui::Label* lbl(PageHost& h, const char* t, float w = 120.f) {
    auto* l = new ui::Label(0, 0, w, 0, t);
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text);
    l->set_align(2);  // 右对齐（layui layui-form-label）
    return l;
}

ui::TextBox* tb(PageHost& h, float w, const char* ph = nullptr, bool pw = false) {
    auto* t = new ui::TextBox(0, 0, w, 38.f);
    t->set_font(h.font);
    t->set_theme(h.theme);
    if (ph) t->set_placeholder(ph);
    if (pw) t->set_password(true);
    return t;
}

float fw() { return kIW - 120.f - 12.f; }

void build_input(PageHost& host) {
    // ---- 顶部引用块（镜像 docs 输入框页导语）----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "输入框与文本域：普通输入框、必填项、密码框、初始赋值，以及多行文本域 textarea。"
        "本页控件为 ui::TextBox / ui::TextArea 一比一克隆 layui-input / layui-textarea 行为。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 常规用法 ----
    auto* fs1 = new ui::Fieldset(kCW, 0, "常规用法");
    fs1->set_font(host.font);
    fs1->set_theme(host.theme);

    { auto* r = make_row(host, 50); r->add_child(lbl(host, "单行输入框"));
      r->add_child(tb(host, fw(), "请输入标题")); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "验证必填项"));
      r->add_child(tb(host, fw(), "请输入")); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "初始值"));
      auto* t = tb(host, fw());
      t->set_text("贤心");
      fs1->add_child(r); r->add_child(t); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "密码框"));
      r->add_child(tb(host, fw(), "请输入密码", true)); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "即时监听"));
      auto* t = tb(host, fw(), "输入内容试试");
      auto* s = new ui::Label(0, 0, 260.f, 0, "");
      s->set_font(host.font); s->set_theme(host.theme);
      s->set_color(host.theme->success);
      t->set_on_change([s](const char* v) {
          std::string txt = "实时值：";
          txt += (v && *v) ? v : "（空）";
          s->set_text(txt.c_str());
      });
      r->add_child(t); r->add_child(s); fs1->add_child(r); }
    host.content->add_child(fs1);

    // ---- Fieldset 2: 文本域 ----
    auto* fs2 = new ui::Fieldset(kCW, 0, "文本域");
    fs2->set_font(host.font);
    fs2->set_theme(host.theme);

    { auto* r = make_row(host, 120.f); r->add_child(lbl(host, "普通文本域"));
      auto* ta = new ui::TextArea(0, 0, fw(), 100.f);
      ta->set_font(host.font); ta->set_theme(host.theme);
      ta->set_placeholder("请输入内容");
      r->add_child(ta); fs2->add_child(r); }
    { auto* r = make_row(host, 120.f); r->add_child(lbl(host, "初始值文本域"));
      auto* ta = new ui::TextArea(0, 0, fw(), 100.f);
      ta->set_font(host.font); ta->set_theme(host.theme);
      ta->set_text("人生似修行。");
      r->add_child(ta); fs2->add_child(r); }
    host.content->add_child(fs2);

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "TextBox 支持字符输入 / 退格 / Delete / Home / End、超宽横向滚动；"
        "TextArea 支持 Enter 换行与纵向滚动。更多交互（选择/剪贴板）见帮助页。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/input_page.cpp：克隆 layui 输入框文档\n"
            "\n"
            "ui::TextBox* t = new ui::TextBox(0, 0, w, 38.f);\n"
            "t->set_font(host.font); t->set_theme(host.theme);\n"
            "t->set_placeholder(\"请输入标题\");   // 空文本灰色占位\n"
            "t->set_text(\"贤心\");                // 初始值\n"
            "t->set_password(true);              // 密码掩码 '*'\n"
            "t->set_on_change([](const char* v) {});\n"
            "\n"
            "ui::TextArea* ta = new ui::TextArea(0, 0, w, 100.f);\n"
            "ta->set_placeholder(\"请输入内容\");\n"
            "ta->set_text(\"人生似修行。\");");
    }
    if (host.help) {
        const char* lines[] = {
            "输入框 API 一览",
            "",
            "TextBox：单行输入框（layui-input）",
            "  set_placeholder(text)：空文本灰色占位提示",
            "  set_password(on)：'*' 掩码显示",
            "  set_text / text()：赋值取值（程序赋值也触发 on_change）",
            "  set_on_change(cb)：文本变化回调；set_on_enter(cb)：回车回调",
            "  键盘：字符 / Backspace / Delete / 左右 / Home / End",
            "  选择：Shift+方向拖选、鼠标拖选、Ctrl+A/C/X/V（UI-101）",
            "",
            "TextArea：多行文本域（layui-textarea）",
            "  set_placeholder / set_text / set_on_change",
            "  Enter 换行、上下键与翻页、自动纵向滚动",
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

static void build(PageHost& host) { build_input(host); }

static const PageDef kInputPage{21, "表单", "输入框", "输入框：单行/密码/初始值/即时监听 + 文本域", build, nullptr, false, nullptr, 0, "Input / Textarea"};

static const bool kRegistered = []() {
    page::register_page(kInputPage);
    return true;
}();
