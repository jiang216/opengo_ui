#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/combobox.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
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

ui::ComboBox* cb(PageHost& h, float w) {
    auto* c = new ui::ComboBox(0, 0, w, 38.f);
    c->set_font(h.font);
    c->set_theme(h.theme);
    return c;
}

float fw() { return kIW - 120.f - 12.f; }

void build_select(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "选择框：下拉选择、初始选中、禁用与联动排版。"
        "本页控件为 ui::ComboBox，点击展开选项面板、点击外部自动关闭。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 常规用法 ----
    auto* fs1 = new ui::Fieldset(kCW, 0, "常规用法");
    fs1->set_font(host.font);
    fs1->set_theme(host.theme);

    { auto* r = make_row(host, 50); r->add_child(lbl(host, "单选选择框"));
      auto* c = cb(host, fw());
      c->add_option(""); c->add_option("写作"); c->add_option("阅读");
      c->add_option("游戏"); c->add_option("音乐"); c->add_option("旅行");
      c->select(0);
      r->add_child(c); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "初始选中"));
      auto* c = cb(host, fw());
      c->add_option("请选择问题"); c->add_option("你工作的第一个城市");
      c->add_option("你的工号"); c->add_option("你最喜欢的老师");
      c->select(2);
      r->add_child(c); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "事件监听"));
      auto* c = cb(host, fw());
      c->add_option("请选择爱好"); c->add_option("写作"); c->add_option("阅读");
      c->add_option("游戏"); c->add_option("音乐");
      auto* s = new ui::Label(0, 0, 260.f, 0, "");
      s->set_font(host.font); s->set_theme(host.theme);
      s->set_color(host.theme->success);
      static const char* kNames[] = {"（空）", "写作", "阅读", "游戏", "音乐"};
      c->set_on_change([s](int i) {
          std::string txt = "选中了：";
          txt += (i >= 0 && i < 5) ? kNames[i] : "?";
          s->set_text(txt.c_str());
      });
      c->select(0);
      r->add_child(c); r->add_child(s); fs1->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "禁用状态"));
      auto* c = cb(host, fw());
      c->add_option("写作"); c->add_option("阅读"); c->add_option("游戏");
      c->select(0);
      c->set_enabled(false);
      r->add_child(c); fs1->add_child(r); }
    host.content->add_child(fs1);

    // ---- Fieldset 2: 联动选择框 ----
    auto* fs2 = new ui::Fieldset(kCW, 0, "联动选择框");
    fs2->set_font(host.font);
    fs2->set_theme(host.theme);

    { auto* r = make_row(host, 50); r->add_child(lbl(host, "省/市/区"));
      auto* p = cb(host, 150);
      p->add_option("请选择省"); p->add_option("浙江省");
      p->add_option("江西省"); p->add_option("福建省");
      p->select(1); r->add_child(p);
      auto* c = cb(host, 150);
      c->add_option("请选择市"); c->add_option("杭州");
      c->add_option("宁波"); c->add_option("温州");
      c->add_option("台州"); c->add_option("绍兴");
      c->select(0); r->add_child(c);
      auto* d = cb(host, 150);
      d->add_option("请选择县/区"); d->add_option("西湖区");
      d->add_option("余杭区"); d->add_option("临安市");
      d->select(0); r->add_child(d);
      auto* hint = new ui::Label(0, 0, 240.f, 0, "此处只是演示联动排版，并未做联动交互");
      hint->set_font(host.font); hint->set_theme(host.theme);
      hint->set_color(host.theme->text_muted);
      r->add_child(hint);
      fs2->add_child(r); }
    host.content->add_child(fs2);

    // ---- 底部引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "分组/搜索选择框依赖 layui form 的 option 分组与搜索过滤，"
        "当前以平铺选项近似呈现，后续按需扩展 ComboBox 能力。");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/select_page.cpp：克隆 layui 选择框文档\n"
            "\n"
            "ui::ComboBox* c = new ui::ComboBox(0, 0, w, 38.f);\n"
            "c->set_font(host.font); c->set_theme(host.theme);\n"
            "c->add_option(\"\");            // 空选项（layui 首项空白）\n"
            "c->add_option(\"写作\");\n"
            "c->select(0);                  // 初始选中并触发 on_change\n"
            "c->set_on_change([](int i) {});\n"
            "c->set_enabled(false);         // 禁用整组\n"
            "\n"
            "// 打开一个下拉时自动关闭其他下拉（全实例注册表）");
    }
    if (host.help) {
        const char* lines[] = {
            "选择框 API 一览",
            "",
            "ComboBox：下拉选择框（layui select）",
            "  add_option(text)：追加选项",
            "  select(index)：选中并触发 on_change",
            "  selected()：当前选中下标（-1 未选）",
            "  set_on_change(cb)：选项变化回调",
            "  set_enabled(false)：禁用（不展开面板）",
            "",
            "交互：点击展开 ScrollBox 选项面板；点击外部或 Esc 自动关闭；",
            "打开任一下拉时其余下拉自动收起。",
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

static void build(PageHost& host) { build_select(host); }

static const PageDef kSelectPage{22, "表单", "选择框", "选择框：单选/初始选中/事件/禁用 + 联动", build, nullptr, false, nullptr, 0, "Select"};

static const bool kRegistered = []() {
    page::register_page(kSelectPage);
    return true;
}();
