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

// layui 官方 demo 侧栏完整树（克隆自 layui.com/demo），全部 30 页已实现。
// 占位机制（build_placeholder）保留备用：新增未实现页时把 order/group/title/tip 加回下方数组即可。

// struct PlaceholderDef { int order; const char* group; const char* title; const char* tip; };
// const PlaceholderDef kPlaceholders[] = {
//     {46, "组件示例", "新页面", "待实现"},
// };
// const bool kRegistered = []() {
//     for (const PlaceholderDef& p : kPlaceholders) {
//         page::register_page({p.order, p.group, p.title, p.tip, build_placeholder});
//     }
//     return true;
// }();

}  // namespace
