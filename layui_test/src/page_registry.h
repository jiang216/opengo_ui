#pragma once

#include "ui/ui/widget.h"

#include <functional>

namespace ui {
class Font;
struct Theme;
class ScrollBox;
class Code;
class Scene;
}  // namespace ui

// 页面宿主：向内容区（ScrollBox）添加本页控件；切页时框架释放旧页全部控件
struct PageHost {
    ui::ScrollBox* content = nullptr;  // 预览区（第一个 tab）
    ui::Code* code = nullptr;          // "查看代码" tab（页面写入示例源码）
    ui::ScrollBox* help = nullptr;     // "帮助" tab（页面写入 API 说明）
    ui::Font* font = nullptr;
    const ui::Theme* theme = nullptr;
    ui::Scene* scene = nullptr;  // 框架级 Scene：页面可订阅 PostDraw/Frame 等（切页须 unsubscribe）
    void* owner = nullptr;  // 框架上下文（存储应用级状态，如 carousel/upload 推进列表）
};

// 单个 demo 页面
struct PageDef {
    int order;           // 全局导航顺序（同分组连续）；跨 TU 静态注册顺序不确定，用 order 保证侧栏确定性
    const char* group;   // 侧栏分组名（布局 / 基本元素 / 组件示例）
    const char* title;   // 侧栏菜单项 / 页面标题
    const char* tip;     // 悬停提示
    void (*build)(PageHost&);
    void (*destroy)(PageHost&) = nullptr;  // 可空：切页释放前清理本页根级弹出等（UI-033）
    // —— 双列侧栏（镜像 site-demo-table-nav，仅数据表格用）——
    // board_anchor：true 表示本页是某板块在主导航的锚点项（如「数据表格」）。
    //   - 选中锚点 → 显示第二列子菜单（该板块子页）+ 显示第一个子页。
    //   - board == nullptr → 非板块子页（普通主导航项，不触发第二列）。
    //   - board != nullptr 且 !board_anchor → 属某板块的子页：不进主导航，只在第二列列出。
    bool board_anchor = false;
    const char* board = nullptr;  // 所属板块名（第二列子菜单标题），如 "数据表格"
    int seg = 0;                  // 第二列内分段号（镜像 <hr> 间隔），1..N
    const char* subtitle = nullptr;  // 侧栏英文副标题（新镜像 docs/2 菜单），可空
};

namespace page {
void register_page(const PageDef& def);
const std::vector<PageDef>& all_pages();
}  // namespace page
