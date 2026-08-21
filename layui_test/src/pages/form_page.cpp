#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/checkbox.h"
#include "ui/ui/code.h"
#include "ui/ui/combobox.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/radiogroup.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/textarea.h"
#include "ui/ui/textbox.h"
#include "ui/ui/theme.h"

namespace {

// 内容区宽度（ScrollBox content_w = 1060 - 48 - 12 ≈ 1000）
constexpr float kCW = 1000.f;
// Fieldset 内边距
constexpr float kFP = 12.f;
// Fieldset 内子控件可用宽度
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
    l->set_align(2);  // 右对齐（layui layui-form-label text-align:right）
    return l;
}

ui::Label* hint(PageHost& h, const char* t, float w = 240.f) {
    auto* l = new ui::Label(0, 0, w, 0, t);
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text_muted);
    l->set_align(2);
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

ui::ComboBox* cb(PageHost& h, float w) {
    auto* c = new ui::ComboBox(0, 0, w, 38.f);
    c->set_font(h.font);
    c->set_theme(h.theme);
    return c;
}

float fw() { return kIW - 120.f - 12.f; }

void build_form1(PageHost& host, ui::Fieldset* fs) {
    // 1 单行输入框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "单行输入框"));
      r->add_child(tb(host, fw(), "请输入标题")); fs->add_child(r); }
    // 2 验证必填项
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "验证必填项"));
      r->add_child(tb(host, fw(), "请输入")); fs->add_child(r); }
    // 3 验证手机 + 验证邮箱（inline pair）
    { auto* r = make_row(host, 50);
      r->add_child(lbl(host, "验证手机", 90)); r->add_child(tb(host, 190));
      r->add_child(lbl(host, "验证邮箱", 90)); r->add_child(tb(host, 190));
      fs->add_child(r); }
    // 4 多规则验证 + 验证日期 + 验证链接（inline triple）
    { auto* r = make_row(host, 50);
      r->add_child(lbl(host, "多规则验证", 80)); r->add_child(tb(host, 130));
      r->add_child(lbl(host, "验证日期", 80)); r->add_child(tb(host, 130, "yyyy-MM-dd"));
      r->add_child(lbl(host, "验证链接", 80)); r->add_child(tb(host, 130));
      fs->add_child(r); }
    // 5 验证身份证
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "验证身份证"));
      r->add_child(tb(host, fw())); fs->add_child(r); }
    // 6 自定义验证（password + hint）
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "自定义验证"));
      r->add_child(tb(host, 190, "请输入密码", true));
      r->add_child(hint(host, "请填写6到12位密码"));
      fs->add_child(r); }
    // 7 范围（range: ￥ - ￥）
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "范围"));
      r->add_child(tb(host, 100, "￥"));
      auto* dash = new ui::Label(0, 0, 20.f, 0, "-");
      dash->set_font(host.font); dash->set_theme(host.theme);
      r->add_child(dash);
      r->add_child(tb(host, 100, "￥"));
      fs->add_child(r); }
    // 10 单行选择框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "单行选择框"));
      auto* c = cb(host, fw());
      c->add_option(""); c->add_option("写作"); c->add_option("阅读");
      c->add_option("游戏"); c->add_option("音乐"); c->add_option("旅行");
      c->select(1); r->add_child(c); fs->add_child(r); }
    // 11 分组选择框 + 搜索选择框（inline pair）
    { auto* r = make_row(host, 50);
      r->add_child(lbl(host, "分组选择框", 90));
      auto* c1 = cb(host, 190);
      c1->add_option("请选择问题"); c1->add_option("你工作的第一个城市");
      c1->add_option("你的工号"); c1->add_option("你最喜欢的老师");
      c1->select(0); r->add_child(c1);
      r->add_child(lbl(host, "搜索选择框", 90));
      auto* c2 = cb(host, 190);
      c2->add_option("直接选择或搜索选择"); c2->add_option("layer");
      c2->add_option("form"); c2->add_option("layim");
      c2->add_option("element"); c2->add_option("laytpl");
      c2->add_option("upload");
      c2->select(0); r->add_child(c2);
      fs->add_child(r); }
    // 12 联动选择框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "联动选择框"));
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
      r->add_child(hint(host, "此处只是演示联动排版，并未做联动交互"));
      fs->add_child(r); }
    // 11 复选框
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "复选框"));
      auto mk = [&](const char* t, bool chk) {
          auto* c = new ui::CheckBox(0, 0, 100.f, 24.f, t);
          c->set_font(host.font); c->set_theme(host.theme); c->set_checked(chk);
          r->add_child(c);
      };
      mk("写作", false); mk("阅读", true); mk("游戏", false);
      fs->add_child(r); }
    // 12 原始复选框（primary skin）
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "原始复选框"));
      auto mk = [&](const char* t, bool chk, bool en = true) {
          auto* c = new ui::CheckBox(0, 0, 100.f, 24.f, t);
          c->set_font(host.font); c->set_theme(host.theme);
          c->set_checked(chk); c->set_enabled(en);
          r->add_child(c);
      };
      mk("写作", true); mk("阅读", false); mk("游戏", false, false);
      fs->add_child(r); }
    // 13 开关-默认关
    { auto* r = make_row(host, 34); r->add_child(lbl(host, "开关-默认关"));
      auto* sw = new ui::Switch(0, 0, 90.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      r->add_child(sw); fs->add_child(r); }
    // 14 开关-默认开
    { auto* r = make_row(host, 34); r->add_child(lbl(host, "开关-默认开"));
      auto* sw = new ui::Switch(0, 0, 90.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme); sw->set_checked(true);
      r->add_child(sw); fs->add_child(r); }
    // 15 单选框
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "单选框"));
      auto* rg = new ui::RadioGroup(0, 0, 400.f, 28.f);
      rg->set_font(host.font); rg->set_theme(host.theme);
      rg->set_option_height(24.f); rg->set_spacing(12.f);
      rg->add_option("男"); rg->add_option("女"); rg->add_option("禁用");
      rg->select(0); rg->set_option_enabled(2, false);
      r->add_child(rg); fs->add_child(r); }
    // 16 普通文本域（textarea）
    { auto* r = make_row(host, 120.f); r->add_child(lbl(host, "普通文本域"));
      auto* ta = new ui::TextArea(0, 0, fw(), 100.f);
      ta->set_font(host.font); ta->set_theme(host.theme);
      ta->set_placeholder("请输入内容");
      r->add_child(ta); fs->add_child(r); }
    // 17 立即提交 + 重置
    { auto* r = make_row(host, 44.f);
      r->add_child(new ui::Label(0, 0, 120.f, 0, ""));
      auto* sub = new ui::Button(0, 0, 90.f, 38.f, "立即提交");
      sub->set_font(host.font); sub->set_theme(host.theme);
      sub->add_class("layui-btn");
      r->add_child(sub);
      auto* rst = new ui::Button(0, 0, 90.f, 38.f, "重置");
      rst->set_font(host.font); rst->set_theme(host.theme);
      rst->add_class("layui-btn layui-btn-primary");
      r->add_child(rst);
      fs->add_child(r); }
}

void build_form2(PageHost& host, ui::Fieldset* fs) {
    // 赋值/取值 form: 输入框 + 密码框 + 选择框 + 复选框 + 开关 + 单选框 + 文本域 + buttons
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "输入框"));
      r->add_child(tb(host, fw(), "请输入标题")); fs->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "密码框"));
      r->add_child(tb(host, fw(), "请输入密码", true)); fs->add_child(r); }
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "选择框"));
      auto* c = cb(host, fw());
      c->add_option(""); c->add_option("写作"); c->add_option("阅读");
      c->add_option("游戏"); c->add_option("音乐");
      c->select(0); r->add_child(c); fs->add_child(r); }
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "复选框"));
      auto mk = [&](const char* t) {
          auto* c = new ui::CheckBox(0, 0, 100.f, 24.f, t);
          c->set_font(host.font); c->set_theme(host.theme);
          r->add_child(c);
      };
      mk("写作"); mk("阅读"); mk("发呆");
      fs->add_child(r); }
    { auto* r = make_row(host, 34); r->add_child(lbl(host, "开关"));
      auto* sw = new ui::Switch(0, 0, 90.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      r->add_child(sw); fs->add_child(r); }
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "单选框"));
      auto* rg = new ui::RadioGroup(0, 0, 400.f, 28.f);
      rg->set_font(host.font); rg->set_theme(host.theme);
      rg->set_option_height(24.f); rg->set_spacing(12.f);
      rg->add_option("男"); rg->add_option("女");
      rg->select(0);
      r->add_child(rg); fs->add_child(r); }
    { auto* r = make_row(host, 120.f); r->add_child(lbl(host, "文本域"));
      auto* ta = new ui::TextArea(0, 0, fw(), 100.f);
      ta->set_font(host.font); ta->set_theme(host.theme);
      ta->set_placeholder("请输入内容");
      r->add_child(ta); fs->add_child(r); }
    // 取值 + 赋值 + 提交 buttons
    { auto* r = make_row(host, 44.f);
      r->add_child(new ui::Label(0, 0, 120.f, 0, ""));
      auto* get = new ui::Button(0, 0, 90.f, 38.f, "赋值");
      get->set_font(host.font); get->set_theme(host.theme);
      get->add_class("layui-btn layui-btn-normal");
      r->add_child(get);
      auto* set = new ui::Button(0, 0, 90.f, 38.f, "取值");
      set->set_font(host.font); set->set_theme(host.theme);
      set->add_class("layui-btn layui-btn-normal");
      r->add_child(set);
      auto* sub = new ui::Button(0, 0, 90.f, 38.f, "立即提交");
      sub->set_font(host.font); sub->set_theme(host.theme);
      sub->add_class("layui-btn");
      r->add_child(sub);
      fs->add_child(r); }
}

void build_form3(PageHost& host, ui::Fieldset* fs) {
    // 方框风格表单（layui layui-form-pane 风格）
    // 1 长输入框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "长输入框"));
      r->add_child(tb(host, fw(), "请输入标题")); fs->add_child(r); }
    // 2 短输入框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "短输入框"));
      r->add_child(tb(host, 200, "请输入")); fs->add_child(r); }
    // 3 日期选择 + 行内表单（inline pair）
    { auto* r = make_row(host, 50);
      r->add_child(lbl(host, "日期选择", 90)); r->add_child(tb(host, 190));
      r->add_child(lbl(host, "行内表单", 90)); r->add_child(tb(host, 190));
      fs->add_child(r); }
    // 4 密码 + hint
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "密码"));
      r->add_child(tb(host, 200, "请输入密码", true));
      r->add_child(hint(host, "请务必填写用户名"));
      fs->add_child(r); }
    // 5 范围
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "范围"));
      r->add_child(tb(host, 100, "￥"));
      auto* dash = new ui::Label(0, 0, 20.f, 0, "-");
      dash->set_font(host.font); dash->set_theme(host.theme);
      r->add_child(dash);
      r->add_child(tb(host, 100, "￥"));
      fs->add_child(r); }
    // 6 单行选择框
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "单行选择框"));
      auto* c = cb(host, fw());
      c->add_option(""); c->add_option("写作"); c->add_option("阅读");
      c->add_option("游戏"); c->add_option("音乐"); c->add_option("旅行");
      c->select(1); r->add_child(c); fs->add_child(r); }
    // 7 行内选择框（省/市/区）
    { auto* r = make_row(host, 50); r->add_child(lbl(host, "行内选择框"));
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
      fs->add_child(r); }
    // 8 开关-开
    { auto* r = make_row(host, 34); r->add_child(lbl(host, "开关-开"));
      auto* sw = new ui::Switch(0, 0, 90.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme); sw->set_checked(true);
      r->add_child(sw); fs->add_child(r); }
    // 9 单选框
    { auto* r = make_row(host, 32); r->add_child(lbl(host, "单选框"));
      auto* rg = new ui::RadioGroup(0, 0, 400.f, 28.f);
      rg->set_font(host.font); rg->set_theme(host.theme);
      rg->set_option_height(24.f); rg->set_spacing(12.f);
      rg->add_option("男"); rg->add_option("女"); rg->add_option("禁用");
      rg->select(0); rg->set_option_enabled(2, false);
      r->add_child(rg); fs->add_child(r); }
    // 10 文本域
    { auto* r = make_row(host, 120.f); r->add_child(lbl(host, "文本域"));
      auto* ta = new ui::TextArea(0, 0, fw(), 100.f);
      ta->set_font(host.font); ta->set_theme(host.theme);
      ta->set_placeholder("请输入内容");
      r->add_child(ta); fs->add_child(r); }
    // 11 跳转式提交
    { auto* r = make_row(host, 44.f);
      auto* sub = new ui::Button(0, 0, 120.f, 38.f, "跳转式提交");
      sub->set_font(host.font); sub->set_theme(host.theme);
      sub->add_class("layui-btn");
      r->add_child(sub);
      fs->add_child(r); }
}

void build_form(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "鉴于小伙伴的普遍反馈，先温馨提醒两个常见\u201c问题\u201d："
        "1. 为什么select/checkbox/radio没显示？"
        "2. 动态添加的表单元素如何更新？");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- Fieldset 1: 表单集合演示 ----
    auto* fs1 = new ui::Fieldset(kCW, 0, "表单集合演示");
    fs1->set_font(host.font);
    fs1->set_theme(host.theme);
    build_form1(host, fs1);
    host.content->add_child(fs1);

    // ---- 引用块 ----
    auto* bq2 = new ui::BlockQuote(kCW, 0,
        "由于未能做出动态效果未及呈现的展示，请参见相关文档自行了解其使用过程，以趋近完美");
    bq2->set_font(host.font);
    bq2->set_theme(host.theme);
    host.content->add_child(bq2);

    // ---- Fieldset 2: 赋值和取值 ----
    auto* fs2 = new ui::Fieldset(kCW, 0, "赋值和取值");
    fs2->set_font(host.font);
    fs2->set_theme(host.theme);
    build_form2(host, fs2);
    host.content->add_child(fs2);

    // ---- Fieldset 3: 方框风格的表单集合 ----
    auto* fs3 = new ui::Fieldset(kCW, 0, "方框风格的表单集合");
    fs3->set_font(host.font);
    fs3->set_theme(host.theme);
    build_form3(host, fs3);
    host.content->add_child(fs3);

    // ---- 底部引用块 ----
    auto* bq3 = new ui::BlockQuote(kCW, 0,
        "更多我们未能呈现的示例，还有待您在阅读文档、以及不断使用的过程，去深入挖掘。");
    bq3->set_font(host.font);
    bq3->set_theme(host.theme);
    host.content->add_child(bq3);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/form_page.cpp（节选）：克隆 layui form.html\n"
            "\n"
            "ui::TextBox* tb = new ui::TextBox(0, 0, w, 38.f);\n"
            "tb->set_font(host.font); tb->set_theme(host.theme);\n"
            "tb->set_placeholder(\"请输入标题\"); tb->set_on_change(cb);\n"
            "\n"
            "ui::ComboBox* cb = new ui::ComboBox(0, 0, w, 38.f);\n"
            "cb->add_option(\"写作\"); cb->select(0);\n"
            "cb->set_on_change([](int i) {});\n"
            "\n"
            "ui::CheckBox* c = new ui::CheckBox(0, 0, 100, 24, \"写作\");\n"
            "c->set_checked(true); c->set_on_change([](bool v) {});\n"
            "\n"
            "ui::RadioGroup* rg = new ui::RadioGroup(0, 0, 400, 28);\n"
            "rg->add_option(\"男\"); rg->select(0);\n"
            "\n"
            "ui::Switch* sw = new ui::Switch(0, 0, 90, 24);\n"
            "sw->set_checked(true); sw->set_on_change([](bool on) {});\n"
            "\n"
            "ui::TextArea* ta = new ui::TextArea(0, 0, w, 100);\n"
            "ta->set_placeholder(\"请输入内容\");\n"
            "\n"
            "ui::Button* btn = new ui::Button(0, 0, 90, 38, \"立即提交\");\n"
            "btn->add_class(\"layui-btn\"); btn->set_on_click([]() {});");
    }
    if (host.help) {
        const char* lines[] = {
            "表单元素 API 一览（克隆 layui form.html 预览）",
            "",
            "TextBox：单行输入框",
            "  set_placeholder / set_password / set_on_change",
            "  字符/退格/Delete/Home/End，聚焦光标 + 超宽横向滚动",
            "",
            "TextArea：多行文本框",
            "  set_placeholder / set_text / set_on_change",
            "  Enter 换行，上下键/翻页，自动纵向滚动",
            "",
            "ComboBox：下拉选择框",
            "  add_option / select / set_on_change",
            "  点击展开下拉面板，点击外部自动关闭",
            "",
            "CheckBox：复选框",
            "  set_checked / set_enabled / set_on_change",
            "  点击或空格切换",
            "",
            "RadioGroup：单选框组",
            "  add_option / select / set_option_enabled / set_on_change",
            "  组内互斥，点击或空格切换选中",
            "",
            "Switch：开关",
            "  set_checked / set_on_change",
            "  点击或 Enter/Space 切换开/关",
            "",
            "Button：按钮",
            "  add_class(\"layui-btn\") / add_class(\"layui-btn-primary\")",
            "  set_on_click / set_icon / preferred_height",
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

static void build(PageHost& host) { build_form(host); }

static const PageDef kFormPage{20, "表单", "表单", "表单：输入框/下拉/复选/单选/开关/文本框/按钮", build, nullptr, false, nullptr, 0, "Form"};

static const bool kRegistered = []() {
    page::register_page(kFormPage);
    return true;
}();
