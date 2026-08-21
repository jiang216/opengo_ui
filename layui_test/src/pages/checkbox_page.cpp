#include "../page_registry.h"

#include "ui/gl/font.h"
#include "ui/gl/renderer.h"
#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/checkbox.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/panelwindow.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/switch.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;

ui::HBox* make_row(PageHost& h, float rh) {
    auto* r = new ui::HBox(0, 0, kCW, rh);
    r->set_padding(0.f);
    r->set_spacing(12.f);
    r->set_theme(h.theme);
    return r;
}

ui::Label* section_title(PageHost& h, const char* t) {
    auto* l = new ui::Label(0, 0, kCW, 0, t);
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text);
    return l;
}

// ─────────────────────────────────────────────────────────────
// 自定义内容控件（对应镜像站 lay-skin=none / lay-checkbox 外部 CSS 实现）
// ─────────────────────────────────────────────────────────────

// 浏览器卡片（lay-skin-checkcard + lay-check-dot）：图标 + 标题 + 描述 + 右上勾角。
// 状态取自所属 CheckBox（选中绿边淡绿底、禁用灰底）。
class BrowserCard : public ui::Widget {
public:
    BrowserCard(ui::CheckBox* owner, uint32_t icon_cp, const char* name, const char* desc)
        : owner_(owner), icon_cp_(icon_cp), name_(name ? name : ""), desc_(desc ? desc : "") {}

    void set_font(ui::Font* f) { font_ = f; }

    void on_draw(ui::Renderer& r) override {
        static const ui::Theme kDefault;
        const bool checked = owner_->checked();
        const bool dis = !owner_->enabled;

        // 卡片底：白 / 选中淡绿 rgb(22 183 119 / 10%) / 禁用 #eee
        ui::Vec4 bg{1, 1, 1, 1};
        if (dis) bg = ui::Vec4{0.933f, 0.933f, 0.933f, 1.0f};
        else if (checked) bg = ui::Vec4{0.086f, 0.718f, 0.467f, 0.10f};
        r.draw_rounded_rect(bounds_, 10.f, bg);

        // 边框：白/选中/禁用均 #e5e5e5，悬停 #16b777
        ui::Vec4 border{0.898f, 0.898f, 0.898f, 1.0f};
        if (!dis && (checked || owner_->hovered())) border = kDefault.success;
        r.draw_rounded_rect_outline(bounds_, 10.f, 1.f, border);

        const float pad = 12.f;
        // 图标（30px）
        if (font_ && icon_cp_ && font_->icon_loaded()) {
            const float ic = pad + 15.f;
            const float iy = bounds_.y + pad + 15.f;
            const ui::Vec4 ic_col = dis ? ui::Vec4{0.76f, 0.76f, 0.76f, 1.0f}
                                        : ui::Vec4{0.13f, 0.13f, 0.13f, 1.0f};
            font_->draw_icon(r, icon_cp_, bounds_.x + ic, iy, 30.f, ic_col);
        }

        // 标题（16px 粗，Color #000）
        if (font_ && !name_.empty()) {
            const float tx = bounds_.x + pad + 30.f + 8.f;
            const float ty = bounds_.y + pad;
            font_->draw_text(r, name_.c_str(), tx, ty,
                             dis ? ui::Vec4{0.62f, 0.62f, 0.62f, 1.0f} : ui::Vec4{0, 0, 0, 1});
        }
        // 描述（13px，#5f5f5f / 禁用 #c2c2c2）
        if (font_ && !desc_.empty()) {
            const float tx = bounds_.x + pad + 30.f + 8.f;
            const float ty = bounds_.y + pad + 24.f;
            const ui::Vec4 dc = dis ? ui::Vec4{0.76f, 0.76f, 0.76f, 1.0f}
                                    : ui::Vec4{0.37f, 0.37f, 0.37f, 1.0f};
            font_->draw_text(r, desc_.c_str(), tx, ty, dc);
        }

        // 右上勾角（lay-check-dot）：选中绿色三角 + 白对勾；禁用灰
        if (checked) {
            const ui::Vec4 corner = dis ? ui::Vec4{0.82f, 0.82f, 0.82f, 1.0f} : kDefault.success;
            const float cx = bounds_.x + bounds_.w;
            const float cy = bounds_.y;
            const ui::Vec2 tri[3] = {
                {cx - 18.f, cy}, {cx, cy}, {cx, cy + 18.f},
            };
            r.draw_polygon(tri, 3, corner);
            if (font_ && font_->icon_loaded()) {
                font_->draw_icon(r, ui::layui_icon::ok, cx - 9.f, cy + 9.f, 11.f,
                                 ui::Vec4{1, 1, 1, 1});
            }
        }
    }

private:
    ui::CheckBox* owner_;
    uint32_t icon_cp_;
    std::string name_, desc_;
    ui::Font* font_ = nullptr;
};

// 标签徽章（lay-skin-tag layui-badge）：圆形胶囊，选中绿底白字，未选中红底白字
class TagPill : public ui::Widget {
public:
    TagPill(ui::CheckBox* owner, const char* text) : owner_(owner), text_(text ? text : "") {}

    void set_font(ui::Font* f) { font_ = f; }

    void on_draw(ui::Renderer& r) override {
        static const ui::Theme kDefault;
        const bool checked = owner_->checked();
        const ui::Vec4 bg = checked ? kDefault.success : kDefault.danger;
        r.draw_rounded_rect(bounds_, bounds_.h * 0.5f, bg);
        if (font_ && !text_.empty()) {
            const float tw = font_->measure_text(text_.c_str());
            const float tx = bounds_.x + (bounds_.w - tw) * 0.5f;
            const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
            font_->draw_text(r, text_.c_str(), tx, ty, ui::Vec4{1, 1, 1, 1});
        }
    }

private:
    ui::CheckBox* owner_;
    std::string text_;
    ui::Font* font_ = nullptr;
};

// 自定义标题模板（lay-checkbox）：普通文本 + 链接下划线（自定义<a><ins>标题模板</ins></a>）
class LinkTitle : public ui::Widget {
public:
    LinkTitle(ui::CheckBox* owner, const char* prefix, const char* link)
        : owner_(owner), prefix_(prefix ? prefix : ""), link_(link ? link : "") {}

    void set_font(ui::Font* f) { font_ = f; }

    void on_draw(ui::Renderer& r) override {
        static const ui::Theme kDefault;
        if (!font_) return;
        const float ty = bounds_.y + (bounds_.h - font_->line_height()) * 0.5f;
        float x = bounds_.x;
        const ui::Vec4 txt = owner_->enabled ? ui::Vec4{0.37f, 0.37f, 0.37f, 1.0f}
                                             : ui::Vec4{0.76f, 0.76f, 0.76f, 1.0f};
        if (!prefix_.empty()) {
            font_->draw_text(r, prefix_.c_str(), x, ty, txt);
            x += font_->measure_text(prefix_.c_str());
        }
        if (!link_.empty()) {
            font_->draw_text(r, link_.c_str(), x, ty, kDefault.info);
            const float lw = font_->measure_text(link_.c_str());
            r.draw_quad({x, ty + font_->line_height(), lw, 1.f}, kDefault.info);
        }
    }

private:
    ui::CheckBox* owner_;
    std::string prefix_, link_;
    ui::Font* font_ = nullptr;
};

// 心形图标 + 标题模板（lay-checkbox + lay-skin=tag 内容）
class HeartTitle : public ui::Widget {
public:
    HeartTitle(ui::CheckBox* owner) : owner_(owner) {}

    void set_font(ui::Font* f) { font_ = f; }

    void on_draw(ui::Renderer& r) override {
        static const ui::Theme kDefault;
        if (!font_) return;
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const ui::Vec4 col = owner_->enabled ? ui::Vec4{1, 1, 1, 1}
                                             : ui::Vec4{0.76f, 0.76f, 0.76f, 1.0f};
        if (font_->icon_loaded()) {
            font_->draw_icon(r, ui::layui_icon::heart, bounds_.x + 8.f, cy, 14.f, col);
        }
        font_->draw_text(r, "标题模板", bounds_.x + 20.f,
                         cy - font_->line_height() * 0.5f, col);
    }

private:
    ui::CheckBox* owner_;
    ui::Font* font_ = nullptr;
};

// 开关自定义标题（lay-checkbox + lay-skin=switch）：月亮/太阳图标双态（开=月亮，关=太阳）
class MoonLightTitle : public ui::Widget {
public:
    MoonLightTitle(ui::Switch* owner) : owner_(owner) {}

    void set_font(ui::Font* f) { font_ = f; }

    void on_draw(ui::Renderer& r) override {
        if (!font_ || !font_->icon_loaded()) return;
        const float cx = bounds_.x + bounds_.w * 0.5f;
        const float cy = bounds_.y + bounds_.h * 0.5f;
        const ui::Vec4 col = owner_->checked() ? ui::Vec4{1, 1, 1, 1}
                                               : ui::Vec4{0.6f, 0.6f, 0.6f, 1.0f};
        const uint32_t cp = owner_->checked() ? ui::layui_icon::moon : ui::layui_icon::light;
        font_->draw_icon(r, cp, cx, cy, 14.f, col);
    }

private:
    ui::Switch* owner_;
    ui::Font* font_ = nullptr;
};

// ─────────────────────────────────────────────────────────────
// 1. 默认风格
// ─────────────────────────────────────────────────────────────
void build_default(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "默认风格"));
    fs->add_child(section_title(host,
        "属性 title 可设置复选框标题；属性 checked 可设置默认选中；"
        "属性 disabled 可设置禁用状态。"));

    auto* r = make_row(host, 34);
    // 默认
    auto* c1 = new ui::CheckBox(0, 0, 100.f, 24.f, "默认");
    c1->set_font(host.font); c1->set_theme(host.theme);
    r->add_child(c1);
    // 选中（lay-text="选中" checked）
    auto* c2 = new ui::CheckBox(0, 0, 100.f, 24.f, "选中");
    c2->set_font(host.font); c2->set_theme(host.theme); c2->set_checked(true);
    r->add_child(c2);
    // 禁用
    auto* c3 = new ui::CheckBox(0, 0, 100.f, 24.f, "禁用");
    c3->set_font(host.font); c3->set_theme(host.theme); c3->set_enabled(false);
    r->add_child(c3);
    // 半选（indeterminate 只能动态设置）
    auto* c4 = new ui::CheckBox(0, 0, 100.f, 24.f, "半选");
    c4->set_font(host.font); c4->set_theme(host.theme); c4->set_indeterminate(true);
    r->add_child(c4);
    fs->add_child(r);

    fs->add_child(section_title(host,
        "注：半选属性 indeterminate 只能动态设置（此处演示 set_indeterminate(true)）。"));
}

// ─────────────────────────────────────────────────────────────
// 2. 标签风格
// ─────────────────────────────────────────────────────────────
void build_tag(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "标签风格"));
    fs->add_child(section_title(host, "通过 lay-skin=\"tag\" 可设置复选框的标签风格。"));

    auto* r = make_row(host, 30);
    auto mk = [&](const char* t, bool chk, bool en = true) {
        auto* c = new ui::CheckBox(0, 0, 130.f, 30.f, t);
        c->set_font(host.font); c->set_theme(host.theme);
        c->set_skin(ui::CheckBox::Skin::Tag);
        c->set_checked(chk); c->set_enabled(en);
        r->add_child(c);
    };
    mk("默认", false);
    mk("选中", true);
    mk("禁用", false, false);
    fs->add_child(r);
}

// ─────────────────────────────────────────────────────────────
// 3. 开关风格
// ─────────────────────────────────────────────────────────────
void build_switch(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "开关风格"));
    fs->add_child(section_title(host,
        "通过 lay-skin=\"switch\" 可设置复选框的开关风格。"
        "在 title 属性中通过 | 分隔符可设置两种状态下的不同标题。"));

    // 基本开关
    { auto* r = make_row(host, 34);
      auto* sw = new ui::Switch(0, 0, 60.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      r->add_child(sw); fs->add_child(r); }

    // ON|OFF 开关（checked）
    { auto* r = make_row(host, 34);
      auto* sw = new ui::Switch(0, 0, 90.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      sw->set_text("ON|OFF"); sw->set_checked(true);
      r->add_child(sw); fs->add_child(r); }

    // 开启|关闭
    { auto* r = make_row(host, 34);
      auto* sw = new ui::Switch(0, 0, 120.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      sw->set_text("开启|关闭");
      r->add_child(sw); fs->add_child(r); }

    // 禁用开关
    { auto* r = make_row(host, 34);
      auto* sw = new ui::Switch(0, 0, 60.f, 24.f);
      sw->set_font(host.font); sw->set_theme(host.theme);
      sw->set_enabled(false);
      r->add_child(sw); fs->add_child(r); }
}

// ─────────────────────────────────────────────────────────────
// 4. 自定义标题模板（lay-checkbox 绑定）
// ─────────────────────────────────────────────────────────────
void build_custom_title(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "自定义标题模板"));
    fs->add_child(section_title(host,
        "在 checkbox 元素后的相邻元素设置特定属性 lay-checkbox，"
        "可以与 checkbox 标题进行绑定（此处用 set_custom_content 实现）。"));

    // ① checkbox + 自定义<a href><ins>标题模板</ins></a>
    {
        auto* r = make_row(host, 34);
        auto* c1 = new ui::CheckBox(0, 0, 220.f, 24.f, "");
        c1->set_font(host.font); c1->set_theme(host.theme);
        auto* link = new LinkTitle(c1, "自定义", "标题模板");
        link->set_font(host.font);
        c1->set_custom_content(link);
        r->add_child(c1);
        fs->add_child(r);
    }

    // ② checkbox(tag) + 心形图标 + 标题模板
    {
        auto* r = make_row(host, 30);
        auto* c2 = new ui::CheckBox(0, 0, 140.f, 30.f, "");
        c2->set_font(host.font); c2->set_theme(host.theme);
        c2->set_skin(ui::CheckBox::Skin::Tag);
        auto* heart = new HeartTitle(c2);
        heart->set_font(host.font);
        c2->set_custom_content(heart);
        r->add_child(c2);
        fs->add_child(r);
    }

    // ③ checkbox(switch) + 月亮 | 太阳
    {
        auto* r = make_row(host, 34);
        auto* sw = new ui::Switch(0, 0, 80.f, 24.f);
        sw->set_font(host.font); sw->set_theme(host.theme);
        auto* ml = new MoonLightTitle(sw);
        ml->set_font(host.font);
        sw->set_custom_content(ml);
        r->add_child(sw);
        fs->add_child(r);
    }
}

// ─────────────────────────────────────────────────────────────
// 5. 自定义任意风格（lay-skin=none）
// ─────────────────────────────────────────────────────────────
void build_skin_none(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "自定义任意风格"));
    fs->add_child(section_title(host,
        "通过对 checkbox 元素设置 lay-skin=\"none\" 属性禁用默认样式，"
        "从而实现任意风格的多选组件。以下卡片/标签样式均为外部自主实现，并非 Layui 内置。"));

    // 卡片风格
    fs->add_child(section_title(host, "自定义“卡片风格”的多选组件"));

    auto make_card = [&](const char* name, const char* desc, uint32_t icon_cp,
                         bool checked, bool en) {
        ui::CheckBox* c = new ui::CheckBox(0, 0, 232.f, 100.f, "");
        c->set_font(host.font); c->set_theme(host.theme);
        c->set_skin(ui::CheckBox::Skin::None);
        c->set_checked(checked);
        c->set_enabled(en);
        auto* card = new BrowserCard(c, icon_cp, name, desc);
        card->set_font(host.font);
        c->set_custom_content(card);
        return c;
    };

    { auto* r = make_row(host, 100);
      r->set_spacing(8.f);
      r->add_child(make_card("Chrome", "由 Google 公司开发的网页浏览器，被大多数人所使用。",
                             ui::layui_icon::chrome, false, true));
      r->add_child(make_card("Edge", "由 Microsoft 开发的网页浏览器，基于 Chromeium 内核。",
                             ui::layui_icon::edge, true, true));
      r->add_child(make_card("Firefox", "由 Mozilla 开发的开放源代码的网页浏览器。",
                             ui::layui_icon::firefox, false, false));
      r->add_child(make_card("Internet Explorer", "由 Microsoft 出品的网页浏览器，俗称 IE。",
                             ui::layui_icon::ie, true, false));
      fs->add_child(r); }

    // 标签风格
    fs->add_child(section_title(host, "自定义“标签风格”的多选组件"));

    auto make_pill = [&](const char* t, bool checked) {
        ui::CheckBox* c = new ui::CheckBox(0, 0, 56.f, 22.f, "");
        c->set_font(host.font); c->set_theme(host.theme);
        c->set_skin(ui::CheckBox::Skin::None);
        c->set_checked(checked);
        auto* pill = new TagPill(c, t);
        pill->set_font(host.font);
        c->set_custom_content(pill);
        return c;
    };

    { auto* r = make_row(host, 28);
      r->set_spacing(8.f);
      r->add_child(make_pill("唱", true));
      r->add_child(make_pill("跳", true));
      r->add_child(make_pill("rap", false));
      r->add_child(make_pill("篮球", false));
      fs->add_child(r); }

    fs->add_child(section_title(host, "更多风格可自主实现，为避免影响文档其他重要版面的阅读，此处不做过多演示。"));
}

// ─────────────────────────────────────────────────────────────
// 6. 复选框事件
// ─────────────────────────────────────────────────────────────
void build_event(PageHost& host, ui::Fieldset* fs) {
    fs->add_child(section_title(host, "复选框事件"));
    fs->add_child(section_title(host,
        "checkbox 事件在复选框选中或取消选中时触发。"
        "默认/标签风格用 form.on('checkbox(filter)')，开关风格用 form.on('switch(filter)')。"));

    ui::Label* status = new ui::Label(0, 0, kCW, 0, "checked 状态: false");
    status->set_font(host.font);
    status->set_theme(host.theme);
    status->set_color(host.theme->text_secondary);

    auto* r = make_row(host, 34);
    auto* c = new ui::CheckBox(0, 0, 200.f, 24.f, "是否同意");
    c->set_font(host.font); c->set_theme(host.theme);
    c->set_on_change([status](bool checked) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "checked 状态: %s", checked ? "true" : "false");
        status->set_text(buf);
    });
    r->add_child(c);
    fs->add_child(r);
    fs->add_child(status);

    // 提交按钮（form.on('submit') → layer.alert 字段结果）
    auto* r2 = make_row(host, 44);
    auto* btn = new ui::Button(0, 0, 90.f, 38.f, "确认");
    btn->set_font(host.font); btn->set_theme(host.theme);
    btn->add_class("layui-btn");
    r2->add_child(btn);
    fs->add_child(r2);

    // 弹窗展示字段（镜像 layer.alert(JSON.stringify(field))）
    auto* content = host.content;
    auto* font = host.font;
    auto* theme = host.theme;
    btn->set_on_click([content, font, theme, c, btn]() {
        ui::Widget* root = content->root();
        ui::PanelWindow* win = new ui::PanelWindow(0, 0, 300.f, 160.f, "当前填写的字段值");
        win->set_font(font);
        win->set_theme(theme);
        const char* val = c->checked() ? "on" : "off";
        char txt[128];
        std::snprintf(txt, sizeof(txt), "{ \"agreement\": \"%s\" }", val);
        ui::Label* body = new ui::Label(16, 20, 260.f, 80.f, txt);
        body->set_wrap(true);
        body->set_font(font);
        body->set_theme(theme);
        body->set_color(theme->text);
        win->add_child(body);
        win->set_on_close([root]() { root->close_popups(); });
        const ui::Rect rb = root->bounds();
        const float cx = rb.x + rb.w * 0.5f;
        const float cy = rb.y + rb.h * 0.5f;
        win->set_bounds(cx - 150.f, cy - 80.f, 300.f, 160.f);
        root->add_child(win);
        root->open_popup(btn, win);
        win->layout();
    });
}

// ─────────────────────────────────────────────────────────────
void build_checkbox(PageHost& host) {
    // ---- 顶部说明 ----
    auto* bq = new ui::BlockQuote(kCW, 0,
        "复选框组件是对 <input type=\"checkbox\"> 元素的美化替代，"
        "在原有的特性基础上，对 UI 进行了加强。");
    bq->set_font(host.font);
    bq->set_theme(host.theme);
    host.content->add_child(bq);

    // ---- Fieldset 1: 默认风格 ----
    auto* fs1 = new ui::Fieldset(kCW, 0, "默认风格");
    fs1->set_font(host.font);
    fs1->set_theme(host.theme);
    build_default(host, fs1);
    host.content->add_child(fs1);

    // ---- Fieldset 2: 标签风格 ----
    auto* fs2 = new ui::Fieldset(kCW, 0, "标签风格");
    fs2->set_font(host.font);
    fs2->set_theme(host.theme);
    build_tag(host, fs2);
    host.content->add_child(fs2);

    // ---- Fieldset 3: 开关风格 ----
    auto* fs3 = new ui::Fieldset(kCW, 0, "开关风格");
    fs3->set_font(host.font);
    fs3->set_theme(host.theme);
    build_switch(host, fs3);
    host.content->add_child(fs3);

    // ---- Fieldset 4: 自定义标题模板 ----
    auto* fs4 = new ui::Fieldset(kCW, 0, "自定义标题模板");
    fs4->set_font(host.font);
    fs4->set_theme(host.theme);
    build_custom_title(host, fs4);
    host.content->add_child(fs4);

    // ---- Fieldset 5: 自定义任意风格 ----
    auto* fs5 = new ui::Fieldset(kCW, 0, "自定义任意风格");
    fs5->set_font(host.font);
    fs5->set_theme(host.theme);
    build_skin_none(host, fs5);
    host.content->add_child(fs5);

    // ---- Fieldset 6: 复选框事件 ----
    auto* fs6 = new ui::Fieldset(kCW, 0, "复选框事件");
    fs6->set_font(host.font);
    fs6->set_theme(host.theme);
    build_event(host, fs6);
    host.content->add_child(fs6);

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/checkbox_page.cpp：克隆 layui form/checkbox.html\n"
            "\n"
            "// 默认风格\n"
            "ui::CheckBox* c = new ui::CheckBox(0, 0, 100, 24, \"默认\");\n"
            "c->set_font(host.font); c->set_theme(host.theme);\n"
            "\n"
            "// 半选（indeterminate 只能动态设置）\n"
            "ui::CheckBox* ind = new ui::CheckBox(0, 0, 100, 24, \"半选\");\n"
            "ind->set_indeterminate(true);\n"
            "\n"
            "// 标签风格（lay-skin=tag）\n"
            "ui::CheckBox* tag = new ui::CheckBox(0, 0, 130, 30, \"标签\");\n"
            "tag->set_skin(ui::CheckBox::Skin::Tag);\n"
            "\n"
            "// 开关风格（lay-skin=switch，| 分隔双态文本）\n"
            "ui::Switch* sw = new ui::Switch(0, 0, 90, 24);\n"
            "sw->set_text(\"ON|OFF\"); sw->set_checked(true);\n"
            "\n"
            "// 自定义标题模板（lay-checkbox 绑定）\n"
            "ui::CheckBox* cl = new ui::CheckBox(0, 0, 220, 24, \"\");\n"
            "cl->set_custom_content(link_widget);  // 自定义内容控件\n"
            "\n"
            "// 自定义任意风格（lay-skin=none）\n"
            "ui::CheckBox* card = new ui::CheckBox(0, 0, 232, 100, \"\");\n"
            "card->set_skin(ui::CheckBox::Skin::None);\n"
            "card->set_custom_content(card_widget);  // 卡片自定义内容\n"
            "\n"
            "// 事件\n"
            "c->set_on_change([](bool checked) {\n"
            "    // checked 状态变化\n"
            "});");
    }
    if (host.help) {
        const char* lines[] = {
            "复选框 API 一览（克隆 layui form/checkbox.html）",
            "",
            "CheckBox：复选框",
            "  set_text / set_checked / set_enabled / set_on_change",
            "  set_indeterminate(true) — 半选态（只能动态设置）",
            "  set_skin(Skin::Primary/Tag/None) — 默认/标签/无样式",
            "  set_custom_content(widget) — lay-checkbox 自定义内容绑定",
            "  点击或空格切换选中态；选中后自动清除半选",
            "",
            "Switch：开关风格复选框",
            "  set_text(\"ON|OFF\") — | 分隔双态文本（开=左段 关=右段）",
            "  set_custom_content(widget) — 开关自定义标题（如月亮/太阳图标）",
            "  set_checked / set_on_change",
            "  点击或 Enter/Space 切换",
            "",
            "自定义任意风格（Skin::None）：",
            "  结合 set_custom_content 可实现卡片/标签等多选组件",
            "  卡片/标签外观属外部自主实现（非 layui 内置）",
            "",
            "事件：",
            "  set_on_change(callback) — 选中/取消选中时触发",
            "  callback 参数: bool checked",
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

static void build(PageHost& host) { build_checkbox(host); }

static const PageDef kCheckboxPage{
    23, "表单", "复选框",
    "复选框：默认/标签/全部/事件/自定义",
    build, nullptr, false, nullptr, 0, "Checkbox"
};

static const bool kRegistered = []() {
    page::register_page(kCheckboxPage);
    return true;
}();