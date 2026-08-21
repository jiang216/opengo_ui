#include "../page_registry.h"

#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/label.h"
#include "ui/ui/laytpl.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/textarea.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

#include <memory>
#include <string>

namespace {

const char* kDefaultData = R"({
  "title": "Layui常用模块"
  ,"list": [
    {
      "modname": "弹层"
      ,"alias": "layer"
      ,"site": "layer.layui.com"
    }
    ,{
      "modname": "表单"
      ,"alias": "form"
    }
    ,{
      "modname": "分页"
      ,"alias": "laypage"
    }
    ,{
      "modname": "日期"
      ,"alias": "laydate"
    }
    ,{
      "modname": "上传"
      ,"alias": "upload"
    }
  ]
})";

const char* kDefaultTpl = R"(<h3>{{ d.title }}</h3>
<ul>
{{#  layui.each(d.list, function(index, item){ }}
  <li>
    <span>{{ item.modname }}</span>
    <span>{{ item.alias }}：</span>
    <span>{{ item.site || '' }}</span>
  </li>
{{#  }); }}
{{#  if(d.list.length === 0){ }}
  无数据
{{#  } }} 
</ul>)";

void add_fieldset_title(PageHost& host, const char* text) {
    ui::FieldsetTitle* ft = new ui::FieldsetTitle(800, 40.f, text);
    ft->set_font(host.font);
    ft->set_theme(host.theme);
    host.content->add_child(ft);
}

// 单行分组标签（数据 / 模版 / 视图结果）
void add_caption(PageHost& host, const char* text) {
    ui::Label* l = new ui::Label(0, 0, 200, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    host.content->add_child(l);
}

void build_template(PageHost& host) {
    add_fieldset_title(host, "修改下面的数据和模版，可直接呈现渲染后的结果");

    // 数据 / 模版两个 textarea 并排（左数据、右模版）
    ui::HBox* editors = new ui::HBox(0, 0, 800, 230.f);
    editors->set_padding(0.f);
    editors->set_spacing(12.f);
    editors->set_theme(host.theme);
    host.content->add_child(editors);

    ui::TextArea* data_ta = new ui::TextArea(0, 0, 388.f, 230.f);
    data_ta->set_font(host.font);
    data_ta->set_theme(host.theme);
    data_ta->set_placeholder("数据（JSON）");
    data_ta->set_text(kDefaultData);
    editors->add_child(data_ta);

    ui::TextArea* tpl_ta = new ui::TextArea(0, 0, 400.f, 230.f);
    tpl_ta->set_font(host.font);
    tpl_ta->set_theme(host.theme);
    tpl_ta->set_placeholder("模版（laytpl 语法）");
    tpl_ta->set_text(kDefaultTpl);
    editors->add_child(tpl_ta);

    // 视图结果
    add_caption(host, "视图结果");
    ui::Code* view = new ui::Code(0, 0, 800, 180.f);
    view->set_font(host.font);
    view->set_theme(host.theme);
    host.content->add_child(view);

    // 渲染（把 data 与 tpl 文本喂给 ui::LayTpl 引擎，结果写入 view）
    auto rerender = [data_ta, tpl_ta, view]() {
        ui::LayTplValue data;
        std::string derr;
        if (!ui::laytpl_parse_json(data_ta->text().c_str(), data, &derr)) {
            view->set_code(std::string("JSON 解析错误：").append(derr).c_str());
            return;
        }
        ui::LayTpl tpl(tpl_ta->text());
        if (!tpl.ok()) {
            view->set_code(("模板编译错误：" + tpl.last_error()).c_str());
            return;
        }
        std::string html = tpl.render(data);
        if (html.empty() && !tpl.last_error().empty()) {
            view->set_code(("渲染错误：" + tpl.last_error()).c_str());
            return;
        }
        view->set_code(html.c_str());
    };
    rerender();

    data_ta->set_on_change([rerender](const char*) { rerender(); });
    tpl_ta->set_on_change([rerender](const char*) { rerender(); });

    // 说明
    add_caption(host, "说明");
    ui::Label* note = new ui::Label(0, 0, 800, 0,
        "laytpl 是 JavaScript 模板引擎：模板与数据共存，可直接在模板中处理逻辑。"
        "语法见文档：{{ d.field }} 输出（不转义）、{{= d.field }} 输出（转义）、"
        "{{# JS 语句 }} 逻辑处理、{{! 区域 !}} 过滤不解析；分隔符可用 laytpl.config 自定义。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_muted);
    host.content->add_child(note);

    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// 克隆 laytpl.html：数据 + 模版 → 渲染视图（实时）\n"
            "ui::LayTplValue data;\n"
            "ui::laytpl_parse_json(json_text, data);   // 解析 JSON 数据\n"
            "\n"
            "ui::LayTpl tpl(template_text);            // 解析 laytpl 模板\n"
            "std::string html = tpl.render(data);      // 渲染出结果\n"
            "\n"
            "// 模板语法（与 layui.laytpl 一致）：\n"
            "//   {{ d.field }}     输出字段（不转义 html）\n"
            "//   {{= d.field }}    输出字段（转义 html）\n"
            "//   {{# JS 语句 }}    逻辑（if/each 等）\n"
            "//   {{! 区域 !}}      过滤该区域不解析\n"
            "//   laytpl.config(\"<%%\", \"%%>\")  自定义分隔符\n"
            "\n"
            "// 页面上两个文本框可实时编辑数据/模版，结果即时刷新（TextArea.on_change → rerender）");
    }
    if (host.help) {
        const char* lines[] = {
            "模板引擎（laytpl）API",
            "  LayTplValue：数据节点（对象/数组/标量），可 laytpl_parse_json(json) 解析",
            "  LayTpl tpl(str)：编译模板；tpl.render(data) 返回渲染串",
            "  {{ d.field }} 输出（不转义）；{{= d.field }} 输出（转义 html）",
            "  {{# JS 语句 }} 逻辑处理：{{# if(cond){ }} ... {{# } }}、{{# layui.each(list, fn) }}",
            "  {{! 区域 !}} 过滤不解析；laytpl.config(open, close) 自定义分隔符",
            "  本页为实时沙箱：改数据/模版立即刷新视图，1:1 克隆 laytpl.html",
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
    build_template(host);
}

static const PageDef kTemplatePage{60, "其他", "模板引擎", "模板引擎：数据+模版渲染视图", build, nullptr, false, nullptr, 0, "Laytpl"};

static const bool kRegistered = []() {
    page::register_page(kTemplatePage);
    return true;
}();
