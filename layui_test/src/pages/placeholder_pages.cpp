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

// 与镜像站 docs/2 侧栏对齐的缺失页（UI-106）：仅注册导航项 + 占位内容，
// 官方示例详情后续逐页补全。order 按镜像侧栏顺序插入既有编号体系：
//   基础组整体前置于「布局」(0/1)，取负值；通用/表单/展示组内按镜像顺序插空，
//   受影响的已实现页 order 同步顺延（anim 11→12、checkbox 21→23、
//   breadcrumb 33→34、menubar 34→35、tabs 35→36、badge 36→37、auxiliar 37→38、
//   card 38→40、progress 39→41、timeline 40→42、tree 41→44、carousel 42→45、
//   flow 43→46、text 44→47）。
struct PlaceholderDef {
    int order;
    const char* group;
    const char* title;
    const char* subtitle;
};
const PlaceholderDef kPlaceholders[] = {
    // —— 基础（Getting Started）：镜像整组缺失 ——
    {-9, "基础", "开始使用", "Getting Started"},
    {-8, "基础", "底层方法", "Base Function"},
    {-7, "基础", "模块系统", "Modules"},
    {-6, "基础", "组件构建", "Component"},
    {-5, "基础", "国际化", "i18n"},
    {-4, "基础", "更新日志", "Versions"},
    // —— 通用：颜色在按钮前、图标在按钮与动画之间（镜像顺序：颜色/按钮/图标/动画）——
    {9, "通用", "颜色", "Color"},
    {11, "通用", "图标", "Icon"},
    // —— 表单：镜像顺序 表单组件/输入框/选择框/复选框/单选框 ——
    {21, "表单", "输入框", "Input / Textarea"},
    {22, "表单", "选择框", "Select"},
    {24, "表单", "单选框", "Radio"},
    // —— 展示：镜像顺序 分页→树形表格→导航…辅助→公共类→面板…时间线→固定条→树组件 ——
    {33, "展示", "树形表格", "TreeTable"},
    {39, "展示", "公共类", "Class"},
    {43, "展示", "固定条", "Fixbar"},
};
const bool kRegistered = []() {
    for (const PlaceholderDef& p : kPlaceholders) {
        page::register_page({p.order, p.group, p.title, "官方示例内容待后续补全",
                             build_placeholder, nullptr, false, nullptr, 0, p.subtitle});
    }
    return true;
}();

}  // namespace
