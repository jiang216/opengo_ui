#include "../page_registry.h"

#include "ui/ui/label.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"

namespace {

// 未克隆页面的占位内容：预览提示 + 帮助说明（layui 官方示例内容待补全）
void build_placeholder(PageHost& host) {
    ui::Label* note = new ui::Label(0, 0, 620, 0,
                                    "该页面尚未克隆。\n本项目逐页克隆 layui 官方 demo 网站，"
                                    "此页面的官方示例内容待后续补全，敬请期待。");
    note->set_font(host.font);
    note->set_theme(host.theme);
    note->set_color(host.theme->text_secondary);
    note->set_wrap(true);
    host.content->add_child(note);

    if (host.help) {
        ui::Label* h = new ui::Label(0, 0, 620, 0, "该页面尚未克隆，暂无 API 说明。");
        h->set_font(host.font);
        h->set_theme(host.theme);
        h->set_color(host.theme->text);
        h->set_wrap(true);
        host.help->add_child(h);
    }
}

// UI-106 注册的 14 个占位页已全部由真实内容页取代：
//   基础组 6 页 → basic_pages.cpp（-9..-4）
//   颜色 → color_page.cpp（9）、图标 → icon_page.cpp（11）
//   输入框 → input_page.cpp（21）、选择框 → select_page.cpp（22）、单选框 → radio_page.cpp（24）
//   树形表格 → treetable_page.cpp（33）、公共类 → class_page.cpp（39）、固定条 → fixbar_page.cpp（43）
// 本文件保留占位机制：后续对齐镜像站新增页面时，向 kPlaceholders 追加条目即可。
struct PlaceholderDef {
    int order;
    const char* group;
    const char* title;
    const char* subtitle;
};
// 哨兵条目（title == nullptr 时跳过）：MSVC 不支持零长数组，追加真实占位页时
// 在哨兵前插入 {order, group, title, subtitle} 即可。
const PlaceholderDef kPlaceholders[] = {
    {0, nullptr, nullptr, nullptr},
};
const bool kRegistered = []() {
    for (const PlaceholderDef& p : kPlaceholders) {
        if (!p.title) continue;
        page::register_page({p.order, p.group, p.title, "官方示例内容待后续补全",
                             build_placeholder, nullptr, false, nullptr, 0, p.subtitle});
    }
    return true;
}();

}  // namespace
