#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/image.h"
#include "ui/ui/label.h"
#include "ui/ui/panel.h"
#include "ui/ui/progress.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/table.h"
#include "ui/ui/theme.h"
#include "ui/ui/upload.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

// 通用纵向间距空白（模拟 demo 之间的 margin-top）
void add_gap(PageHost& host, float h = 20.f) {
    ui::Panel* spacer = new ui::Panel(0, 0, 800, h);
    spacer->set_theme(host.theme);
    host.content->add_child(spacer);
}

// 小节标题（layui-field-title legend）
void add_fieldset_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 40.f, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
}

// 一行按钮组（按钮高度 38，模拟 layui-btn-container 内联）
void add_btn_group(PageHost& host, const std::vector<std::pair<const char*, const char*>>& btns) {
    ui::HBox* row = new ui::HBox(0, 0, 800, 44.f);
    row->set_padding(0.f);
    row->set_spacing(12.f);
    row->set_valign(ui::Valign::Center);
    row->set_theme(host.theme);
    for (auto& [txt, cls] : btns) {
        ui::Button* b = new ui::Button(0, 0, 0.f, 38.f, txt);
        b->set_font(host.font);
        b->set_theme(host.theme);
        b->add_class(cls);
        row->add_child(b);
    }
    host.content->add_child(row);
}

// ── 1. 常规使用：普通图片上传（要 test1 + demo1 预览 + demo 进度条）──────────
void build_demo1(PageHost& host) {
    add_fieldset_title(host, "常规使用：普通图片上传");

    ui::Upload* upload = new ui::Upload(0, 0, 800, 120.f);
    upload->set_font(host.font);
    upload->set_theme(host.theme);

    ui::Button* btn = new ui::Button(0, 0, 100.f, 38.f, "上传图片");
    btn->set_font(host.font);
    btn->set_theme(host.theme);
    btn->add_class("layui-btn");
    btn->set_on_click([upload]() { upload->add_file("photo_preview.jpg", 1024 * 256); });
    host.content->add_child(btn);

    // 预览区 + 进度条（layui-upload-list 内 img + 进度条）
    ui::Panel* list = new ui::Panel(0, 0, 800, 100.f);
    list->set_theme(host.theme);

    ui::Image* img = new ui::Image(10.f, 8.f, 50.f, 50.f);
    img->load_from_file("logo.png");
    list->add_child(img);

    ui::Progress* bar = new ui::Progress(70.f, 8.f, 250.f, 14.f);
    bar->set_font(host.font);
    bar->set_theme(host.theme);
    bar->set_value(0.68f);
    bar->set_show_text(true);
    list->add_child(bar);

    ui::Label* txt = new ui::Label(70.f, 30.f, 500, 0, "上传成功");
    txt->set_font(host.font);
    txt->set_theme(host.theme);
    txt->set_color(host.theme->text);
    list->add_child(txt);

    host.content->add_child(list);
    host.content->add_child(upload);
}

// ── 2. 高级应用：制作一个多文件列表（testList 表格 + testListAction 开始上传）──
void build_demo2(PageHost& host) {
    add_fieldset_title(host, "高级应用：制作一个多文件列表");

    ui::Upload* upload = new ui::Upload(0, 0, 800, 160.f);
    upload->set_font(host.font);
    upload->set_theme(host.theme);

    ui::Button* pick = new ui::Button(0, 0, 120.f, 38.f, "选择多文件");
    pick->set_font(host.font);
    pick->set_theme(host.theme);
    pick->add_class("layui-btn layui-btn-normal");
    pick->set_on_click([upload]() {
        static int seq = 0;
        for (int i = 0; i < 3; ++i) {
            ++seq;
            char name[64];
            std::snprintf(name, sizeof(name), "document_%02d.pdf", seq);
            upload->add_file(name, 1024 * (seq % 8 + 1) * 128);
        }
    });
    host.content->add_child(pick);

    // 多文件列表表格（文件名/大小/上传进度/操作）
    ui::Table* table = new ui::Table(0, 0, 800, 220.f);
    table->set_font(host.font);
    table->set_theme(host.theme);
    table->add_column("文件名", 300.f);
    table->add_column("大小", 120.f);
    table->add_column("上传进度", 260.f);
    table->add_button_column("操作", 102.f, {"重传", "删除"});
    table->add_row({"doc_01.pdf", "1.0kb", "22%", ""});
    table->add_row({"doc_02.pdf", "2.0kb", "60%", ""});
    table->add_row({"doc_03.pdf", "3.0kb", "100%", ""});
    host.content->add_child(table);

    ui::Button* action = new ui::Button(0, 0, 120.f, 38.f, "开始上传");
    action->set_font(host.font);
    action->set_theme(host.theme);
    action->add_class("layui-btn");
    host.content->add_child(action);
}

// ── 3. 上传多张图片（test2 + demo2 预览 blockquote）──────────────────────────
void build_demo3(PageHost& host) {
    add_fieldset_title(host, "上传多张图片");

    ui::Button* btn = new ui::Button(0, 0, 120.f, 38.f, "多图片上传");
    btn->set_font(host.font);
    btn->set_theme(host.theme);
    btn->add_class("layui-btn");
    host.content->add_child(btn);

    // 预览缩略图（layui-quote-nm）
    ui::BlockQuote* quote = new ui::BlockQuote(800, 120.f, "预览图：", true);
    quote->set_font(host.font);
    quote->set_theme(host.theme);
    for (int i = 0; i < 3; ++i) {
        ui::Image* img = new ui::Image(8.f + i * 60.f, 28.f, 52.f, 52.f);
        img->load_from_file("logo.png");
        quote->add_child(img);
    }
    host.content->add_child(quote);
}

// ── 4. 指定允许上传的文件类型（test3~test6）──────────────────────────────────
void build_demo4(PageHost& host) {
    add_fieldset_title(host, "指定允许上传的文件类型");
    add_btn_group(host, {{"上传文件", "layui-btn"},
                         {"只允许压缩文件", "layui-btn layui-btn-primary"},
                         {"上传视频", "layui-btn"},
                         {"上传音频", "layui-btn"}});
    ui::Label* note = new ui::Label(0, 4, 800, 0,
                                    "accept: file / exts: zip|rar|7z / accept: video / accept: audio");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 5. 设定文件大小限制（test7，size: 60）────────────────────────────────────
void build_demo5(PageHost& host) {
    add_fieldset_title(host, "设定文件大小限制");
    add_btn_group(host, {{"上传图片", "layui-btn layui-btn-danger"}});
    ui::Label* note = new ui::Label(0, 4, 800, 0, "这里以限制 60KB 为例（size: 60，单位 KB）");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 6. 同时绑定多个元素（demoMore：上传A/B/C）────────────────────────────────
void build_demo6(PageHost& host) {
    add_fieldset_title(host, "同时绑定多个元素，并将属性设定在元素上");
    add_btn_group(host, {{"上传A", "layui-btn"}, {"上传B", "layui-btn"}, {"上传C", "layui-btn"}});
    ui::Label* note = new ui::Label(0, 4, 800, 0,
                                    "每个按钮独立配置 url / size / accept（lay-data 属性）");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 7. 选完文件后不自动上传（test8 + test9）──────────────────────────────────
void build_demo7(PageHost& host) {
    add_fieldset_title(host, "选完文件后不自动上传");

    ui::Upload* upload = new ui::Upload(0, 0, 800, 100.f);
    upload->set_font(host.font);
    upload->set_theme(host.theme);

    ui::Button* choose = new ui::Button(0, 0, 120.f, 38.f, "选择文件");
    choose->set_font(host.font);
    choose->set_theme(host.theme);
    choose->add_class("layui-btn layui-btn-normal");
    choose->set_on_click([upload]() { upload->add_file("manual_upload.bin", 1024 * 1024); });
    host.content->add_child(choose);

    ui::Button* go = new ui::Button(0, 0, 120.f, 38.f, "开始上传");
    go->set_font(host.font);
    go->set_theme(host.theme);
    go->add_class("layui-btn");
    host.content->add_child(go);

    host.content->add_child(upload);
    ui::Label* note = new ui::Label(0, 4, 800, 0, "auto: false，bindAction 指向「开始上传」，选择后不自动上传");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 8. 拖拽上传（test10 拖拽区域）────────────────────────────────────────────
void build_demo8(PageHost& host) {
    add_fieldset_title(host, "拖拽上传");

    ui::Panel* drag = new ui::Panel(0, 0, 800, 120.f);
    drag->set_theme(host.theme);
    drag->add_class("layui-upload-drag");
    ui::Label* zone = new ui::Label(0, 0, 800, 0,
                                    "点击上传，或将文件拖拽到此处");
    zone->set_font(host.font);
    zone->set_theme(host.theme);
    zone->set_color(host.theme->text_secondary);
    drag->add_child(zone);
    host.content->add_child(drag);
    ui::Label* note = new ui::Label(0, 4, 800, 0, "elem: '#test10'，layui-upload-drag 拖拽区域");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 9. 绑定原始文件域（test20）───────────────────────────────────────────────
void build_demo9(PageHost& host) {
    add_fieldset_title(host, "绑定原始文件域");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
                                    "elem: '#test20'，绑定原生 <input type=\"file\" name=\"file\"> 元素。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);
}

// ── 主构建：页首引语 + 全部 9 个 demo + 代码/帮助 ─────────────────────────────
void build_upload(PageHost& host) {
    // 页首提示（layui-elem-quote）
    ui::BlockQuote* intro = new ui::BlockQuote(800, 40.f, "提示：以下示例的部分上传接口由第三方网站 http://httpbin.org 提供。"
                                                             " 未配置接口的示例，每次上传都会报「请求上传接口出现异常」，属正常现象。");
    intro->set_font(host.font);
    intro->set_theme(host.theme);
    host.content->add_child(intro);

    build_demo1(host);
    add_gap(host, 10.f);
    build_demo2(host);
    add_gap(host, 30.f);
    build_demo3(host);
    add_gap(host, 10.f);
    build_demo4(host);
    add_gap(host, 10.f);
    build_demo5(host);
    add_gap(host, 10.f);
    build_demo6(host);
    add_gap(host, 10.f);
    build_demo7(host);
    add_gap(host, 10.f);
    build_demo8(host);
    add_gap(host, 10.f);
    build_demo9(host);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/upload_page.cpp — 一比一克隆 layui upload.html 全部 9 个 demo\n"
            "\n"
            "// 1. 常规使用：普通图片上传\n"
            "ui::Upload* upload = new ui::Upload(0, 0, 800, 120.f);\n"
            "upload->add_file(\"photo_preview.jpg\", 1024 * 256);\n"
            "\n"
            "// 2. 高级应用：多文件列表（Table：文件名/大小/进度/操作）\n"
            "ui::Table* table = new ui::Table(0, 0, 800, 220.f);\n"
            "table->add_column(\"文件名\", 300.f);\n"
            "table->add_column(\"大小\", 120.f);\n"
            "table->add_column(\"上传进度\", 260.f);\n"
            "table->add_button_column(\"操作\", 102.f, {\"重传\", \"删除\"});\n"
            "\n"
            "// 3. 上传多张图片（BlockQuote 预览缩略图）\n"
            "// 4. 指定允许上传的文件类型：accept file / exts zip|rar|7z / video / audio\n"
            "// 5. 设定文件大小限制：size: 60（KB）\n"
            "// 6. 同时绑定多个元素：lay-data url/size/accept\n"
            "// 7. 选完文件后不自动上传：auto: false + bindAction\n"
            "// 8. 拖拽上传：layui-upload-drag\n"
            "// 9. 绑定原始文件域：elem: '#test20'\n"
            "\n"
            "// 所有 Upload 组件每帧调用 advance(dt) 推进上传进度");
    }
    if (host.help) {
        const char* lines[] = {
            "文件上传（Upload）API — 一比一克隆 upload.html 全部 9 个 demo",
            "",
            "1. 常规使用：普通图片上传",
            "   上传图片按钮 + 预览区 + 进度条 + 状态",
            "2. 高级应用：制作一个多文件列表",
            "   多文件表格（文件名/大小/上传进度/操作列）+ 开始上传",
            "3. 上传多张图片",
            "   多图片上传按钮 + blockquote 预览缩略图网格",
            "4. 指定允许上传的文件类型",
            "   accept: file / exts: zip|rar|7z / accept: video / accept: audio",
            "5. 设定文件大小限制",
            "   size: 60（限制文件大小，单位 KB）",
            "6. 同时绑定多个元素",
            "   每个按钮独立配置 url / size / accept（lay-data 属性）",
            "7. 选完文件后不自动上传",
            "   auto: false + bindAction 手动触发上传",
            "8. 拖拽上传",
            "   layui-upload-drag 拖拽区域",
            "9. 绑定原始文件域",
            "   elem: 原生 <input type=\"file\">",
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
    build_upload(host);
}

static const PageDef kUploadPage{53, "交互", "文件上传", "文件上传：选择/列表/进度", build, nullptr, false, nullptr, 0, "Upload"};

static const bool kRegistered = []() {
    page::register_page(kUploadPage);
    return true;
}();