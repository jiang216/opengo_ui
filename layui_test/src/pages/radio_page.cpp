#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/radiogroup.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

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
    l->set_align(2);
    return l;
}

ui::RadioGroup* rg(PageHost& h, float w) {
    auto* g = new ui::RadioGroup(0, 0, w, 28.f);
    g->set_font(h.font);
    g->set_theme(h.theme);
    g->set_option_height(24.f);
    g->set_spacing(12.f);
    return g;
}

void build_radio(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "单选框：组内互斥选择，支持默认选中、禁用单项、整组禁用与事件监听。"
        "本页控件为 ui::RadioGroup + ui::RadioButton。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 常规用法 ----
    auto* fs1 = new ui::Fieldset(kCW, 0, "常规用法");
    fs1->set_font(host.font);
    fs1->set_theme(host.theme);

    { auto* r = make_row(host, 32); r->add_child(lbl(host, "默认选中"));
      auto* g = rg(host, 400.f);
      g->add_option("男"); g->add_option("女");
      g->select(0);
      r->add_child(g); fs1->add_child(r); }
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "禁用项"));
      auto* g = rg(host, 400.f);
      g->add_option("标题"); g->add_option("作者"); g->add_option("禁用");
      g->select(0);
      g->set_option_enabled(2, false);
      r->add_child(g); fs1->add_child(r); }
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "整组禁用"));
      auto* g = rg(host, 400.f);
      g->add_option("写作"); g->add_option("阅读");
      g->select(0);
      g->set_enabled(false);
      r->add_child(g); fs1->add_child(r); }
    { auto* r = make_row(host, 40); r->add_child(lbl(host, "事件监听"));
      auto* g = rg(host, 400.f);
      g->add_option("男"); g->add_option("女"); g->add_option("保密");
      auto* s = new ui::Label(0, 0, 260.f, 0, "");
      s->set_font(host.font); s->set_theme(host.theme);
      s->set_color(host.theme->success);
      static const char* kNames[] = {"男", "女", "保密"};
      g->set_on_change([s](int i) {
          std::string txt = "选中了：";
          txt += (i >= 0 && i < 3) ? kNames[i] : "?";
          s->set_text(txt.c_str());
      });
      g->select(0);
      r->add_child(g); r->add_child(s); fs1->add_child(r); }
    host.content->add_child(fs1);

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "layui 单选框的边框风格（layui-form-pane）与主题色变体，"
        "随表单页方框风格一并呈现；此处保持常规风格。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/radio_page.cpp：克隆 layui 单选框文档\n"
            "\n"
            "ui::RadioGroup* rg = new ui::RadioGroup(0, 0, 400, 28);\n"
            "rg->set_font(host.font); rg->set_theme(host.theme);\n"
            "rg->set_option_height(24.f); rg->set_spacing(12.f);\n"
            "rg->add_option(\"男\"); rg->add_option(\"女\");\n"
            "rg->select(0);                       // 默认选中\n"
            "rg->set_option_enabled(2, false);    // 禁用单个选项\n"
            "rg->set_enabled(false);              // 整组禁用\n"
            "rg->set_on_change([](int i) {});     // 组内互斥回调");
    }
    if (host.help) {
        const char* lines[] = {
            "单选框 API 一览",
            "",
            "RadioGroup：单选框组（layui radio）",
            "  add_option(text)：追加选项（行高自动布局）",
            "  select(index)：选中并触发 on_change",
            "  selected()：当前选中下标",
            "  set_option_height / set_spacing：行高与间距",
            "  set_option_enabled(i, en)：禁用/启用单个选项",
            "  set_enabled(en)：整组禁用（传播到所有选项）",
            "  set_on_change(cb)：选中变化回调",
            "",
            "交互：点击或空格切换；组内互斥；禁用项不响应。",
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

static void build(PageHost& host) { build_radio(host); }

static const PageDef kRadioPage{24, "表单", "单选框", "单选框：默认选中/禁用项/整组禁用/事件", build, nullptr, false, nullptr, 0, "Radio"};

static const bool kRegistered = []() {
    page::register_page(kRadioPage);
    return true;
}();
