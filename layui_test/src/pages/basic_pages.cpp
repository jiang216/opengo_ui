#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

constexpr float kCW = 1000.f;

void title(PageHost& host, const char* text) {
    auto* t = new ui::FieldsetTitle(800, 26, text);
    t->set_font(host.font);
    t->set_theme(host.theme);
    host.content->add_child(t);
}

void para(PageHost& host, const char* text) {
    auto* l = new ui::Label(0, 0, kCW, 0, text);
    l->set_font(host.font);
    l->set_theme(host.theme);
    l->set_color(host.theme->text_secondary);
    l->set_wrap(true);
    host.content->add_child(l);
}

void code_block(PageHost& host, const char* code, float h = 150.f) {
    ui::Code* c = new ui::Code(0, 0, kCW - 40.f, h);
    c->set_font(host.font);
    c->set_theme(host.theme);
    c->set_lang("JavaScript");
    c->set_code(code);
    host.content->add_child(c);
}

// —— 开始使用 ——
void build_start(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "layui（谐音「类 UI」）是一款采用自身模块规范编写的前端 UI 框架，"
        "遵循原生 HTML/CSS/JS 的开发方式，极易上手，拿来即用。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    title(host, "获取 layui");
    para(host, "获取方式：官网首页下载；npm 安装（npm i layui）；GitHub / Gitee 仓库克隆。");
    code_block(host,
        "npm install layui\n"
        "# 或\n"
        "git clone https://github.com/layui/layui.git");

    title(host, "目录结构");
    code_block(host,
        "layui\n"
        "├─ css          // 核心样式库（layui.css）\n"
        "├─ font         // 字体图标库（iconfont.ttf 等）\n"
        "├─ layui.js     // 基础框架（模块化核心）\n"
        "└─ layui.all.js // 包含全部模块的合并文件");

    title(host, "经典模块化写法");
    code_block(host,
        "<link href=\"./layui/css/layui.css\" rel=\"stylesheet\">\n"
        "<script src=\"./layui/layui.js\"></script>\n"
        "\n"
        "layui.use(['layer', 'form'], function(){\n"
        "  var layer = layui.layer,\n"
        "      form  = layui.form;\n"
        "  layer.msg('Hello layui');\n"
        "});");

    if (host.help) {
        const char* lines[] = {
            "开始使用：下载 → 引入 css/js → layui.use 按需加载模块。",
            "本页为文档摘要占位，完整教程内容后续补全。",
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

// —— 底层方法 ——
void build_base(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "底层方法指 layui() 基础框架暴露的最小工具集，不依赖任何组件模块。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    title(host, "常用方法一览");
    para(host,
        "layui.$(selector)：jQuery 风格选择器（内置轻量实现）；\n"
        "layui.each(obj, fn)：遍历对象/数组；\n"
        "layui.url(href)：解析 URL 为 {hash, search, href, pathname…}；\n"
        "layui.data(table, settings)：本地存储 localStorage 封装；\n"
        "layui.device(key)：设备与浏览器信息探测；\n"
        "layui.stope(e)：阻止事件冒泡；\n"
        "layui.type(value)：严格类型判断（array/null/date 等细分）。");
    code_block(host,
        "// 遍历数组\n"
        "layui.each([1, 2, 3], function(i, v){ console.log(i, v); });\n"
        "\n"
        "// 本地存储：写入 / 读取\n"
        "layui.data('test', {key: 'name', value: '贤心'});\n"
        "var name = layui.data('test', {key: 'name'}).name;\n"
        "\n"
        "// 设备信息\n"
        "console.log(layui.device().ie); // 是否 ie 及版本");

    if (host.help) {
        const char* lines[] = {
            "底层方法均为 layui.* 命名空间静态函数，无需 layui.use 即可调用。",
            "本页为文档摘要占位，完整 API 后续补全。",
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

// —— 模块系统 ——
void build_modules(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "layui 以模块为核心组织代码：内置模块按需加载，业务模块按 layui 规范扩展。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    title(host, "使用模块");
    code_block(host,
        "layui.use(['layer', 'table', 'form'], function(){\n"
        "  var layer = layui.layer;\n"
        "  var table = layui.table;\n"
        "});");

    title(host, "内置模块清单");
    para(host,
        "基础：layer 弹出层 / laydate 日期 / laypage 分页 / laytpl 模板引擎；\n"
        "数据：table 数据表格 / form 表单 / upload 上传 / transfer 穿梭框 / tree 树；\n"
        "交互：dropdown 下拉 / colorpicker 取色 / slider 滑块 / rate 评分；\n"
        "展示：carousel 轮播 / flow 流加载 / util 工具 / code 代码修饰。");

    title(host, "扩展自定义模块");
    code_block(host,
        "// 定义模块\n"
        "layui.define(function(exports){\n"
        "  exports('mymod', {hello: function(){}});\n"
        "});\n"
        "\n"
        "// 使用模块\n"
        "layui.use('mymod', function(){\n"
        "  layui.mymod.hello();\n"
        "});");

    if (host.help) {
        const char* lines[] = {
            "模块系统：layui.use 加载、layui.define 定义、base/extensions 配置路径。",
            "本页为文档摘要占位，完整规范后续补全。",
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

// —— 组件构建 ——
void build_component(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "面向参与 layui 源码贡献的开发者：从源码构建发行包。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    title(host, "构建步骤");
    code_block(host,
        "git clone https://github.com/layui/layui.git\n"
        "cd layui && npm install\n"
        "gulp build   # 或 npm run build");

    title(host, "源码目录");
    para(host,
        "src/ 为源码目录（css/less 与 js/模块），dist/ 为构建产物；\n"
        "gulp 任务负责 less 编译、js 合并与压缩、图标字体拷贝等。");

    if (host.help) {
        const char* lines[] = {"组件构建：node/gulp 工具链，产物输出 dist/。本页为摘要占位。"};
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

// —— 国际化 ——
void build_i18n(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "layui 自 2.8 起提供国际化支持：日期、分页等组件文案可按语言包切换。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    title(host, "切换语言");
    code_block(host,
        "// laydate 语言切换示例\n"
        "laydate.render({\n"
        "  elem: '#date',\n"
        "  lang: 'en'   // cn 中文（默认）/ en 英文\n"
        "});");

    title(host, "说明");
    para(host,
        "语言包随发行包提供，也可通过 layui.i18n 相关接口注册自定义词条；\n"
        "本克隆工程的对应能力映射：LaydateBox::set_lang（cn/en）。");

    if (host.help) {
        const char* lines[] = {"国际化：lang 配置 + 语言包注册。本页为摘要占位。"};
        for (const char* line : lines) {
            ui::Label* label = new ui::Label(0, 0, 800, 0, line);
            label->set_font(host.font);
            label->set_theme(host.theme);
            label->set_color(host.theme->text);
            host.help->add_child(label);
        }
    }
}

// —— 更新日志 ——
void build_versions(PageHost& host) {
    auto* bq = new ui::BlockQuote(kCW, 0,
        "镜像站版本 2.13.x 系列。以下为主要版本节点摘录，完整日志见官方 GitHub Releases。");
    bq->set_font(host.font); bq->set_theme(host.theme);
    host.content->add_child(bq);

    struct V { const char* ver; const char* note; };
    static const V kVersions[] = {
        {"2.13.8", "当前镜像版本；组件细节修复与体验优化。"},
        {"2.13.0", "2.13 系列起点；下拉/表格交互增强。"},
        {"2.12.0", "主题定制体系完善，深色模式适配。"},
        {"2.11.0", "layer 弹层动画与层级管理重构。"},
        {"2.10.0", "table 大数据渲染优化。"},
        {"2.9.0",  "新增若干工具类与图标扩充（190+ 图标）。"},
        {"2.8.0",  "国际化支持；默认主题色调整为 #16baaa。"},
    };
    for (const V& v : kVersions) {
        std::string txt = std::string("v") + v.ver + "　" + v.note;
        auto* l = new ui::Label(0, 0, kCW, 0, txt.c_str());
        l->set_font(host.font);
        l->set_theme(host.theme);
        l->set_color(host.theme->text_secondary);
        host.content->add_child(l);
    }

    if (host.help) {
        const char* lines[] = {"更新日志：完整变更见 GitHub Releases 页。本页为摘要占位。"};
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

// —— 注册：基础组 6 页（负 order，前置于布局组）——

static const bool kRegistered = []() {
    page::register_page({-9, "基础", "开始使用", "获取 layui / 目录结构 / 快速上手",
                         [](PageHost& h) { build_start(h); }, nullptr, false, nullptr, 0, "Getting Started"});
    page::register_page({-8, "基础", "底层方法", "$选择器/each/url/data/device 等底层工具",
                         [](PageHost& h) { build_base(h); }, nullptr, false, nullptr, 0, "Base Function"});
    page::register_page({-7, "基础", "模块系统", "use/define 与内置模块清单",
                         [](PageHost& h) { build_modules(h); }, nullptr, false, nullptr, 0, "Modules"});
    page::register_page({-6, "基础", "组件构建", "从源码构建发行包（gulp）",
                         [](PageHost& h) { build_component(h); }, nullptr, false, nullptr, 0, "Component"});
    page::register_page({-5, "基础", "国际化", "语言包与 lang 配置",
                         [](PageHost& h) { build_i18n(h); }, nullptr, false, nullptr, 0, "i18n"});
    page::register_page({-4, "基础", "更新日志", "主要版本节点摘录",
                         [](PageHost& h) { build_versions(h); }, nullptr, false, nullptr, 0, "Versions"});
    return true;
}();
