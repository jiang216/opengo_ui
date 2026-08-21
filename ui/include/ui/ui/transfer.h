#pragma once

#include "ui/ui/widget.h"

#include <functional>
#include <string>
#include <vector>

namespace ui {
class Font;
class Renderer;
class TextBox;
struct Theme;

// 穿梭方向：本次搬移数据的目标面板（ToRight = 左→右，ToLeft = 右→左）
enum class TransferDirection { ToRight, ToLeft };

// 穿梭框（M17d，layui-transfer 克隆）：左右两个列表，选中项经中间按钮整体左/右搬移，
// 点击项选中（高亮），标题含计数。支持初始右侧数据、禁用项、搜索过滤、穿梭回调、getData。
class Transfer : public Widget {
public:
    Transfer(float x, float y, float w, float h);

    void set_font(Font* font) { font_ = font; }
    void set_theme(const Theme* theme) { theme_ = theme; }
    void set_left_label(const char* s) { left_label_ = s ? s : ""; }
    void set_right_label(const char* s) { right_label_ = s ? s : ""; }

    // 加入左侧列表项；disabled 项不可选中/搬移（layui data.disabled）
    void add_item(const char* name, bool disabled = false);
    // 预置右侧列表项（layui value：初始右侧数据集合）
    void add_right_item(const char* name, bool disabled = false);
    // 清空两侧列表（实例 reload 重建用）
    void clear();

    // 显示搜索框（layui showSearch）：各面板头部输入实时过滤
    void set_show_search(bool on);
    bool show_search() const { return show_search_; }

    // 穿梭回调：direction 为本次搬移的目标面板；items 为被搬移的数据标题
    void set_on_change(std::function<void(TransferDirection, const std::vector<std::string>&)> cb) {
        on_change_ = std::move(cb);
    }

    // getData：右侧当前数据集合
    const std::vector<std::string>& right_items() const { return right_.items; }

    bool on_mouse_down(float x, float y, int button) override;
    bool on_mouse_scroll(float x, float y, float dx, float dy) override;
    void on_layout() override;
    void on_draw(Renderer& renderer) override;

private:
    struct Side {
        std::vector<std::string> items;
        std::vector<bool> disabled;
        std::vector<bool> sel;
        std::string filter;  // 搜索过滤关键字（showSearch）
        float scroll = 0.f;  // 列表滚动偏移（large 数据集超出面板高度时滚轮可达）
    };

    // 命中某面板某行 → 返回该项在 items 中的真实下标（含过滤映射 + 滚动偏移）；未命中返回 -1
    int hit_row(const Side& s, const Rect& panel, float x, float y, bool search_on) const;
    // 过滤后可见项的真实下标集合（顺序保持；无过滤时返回全量）
    std::vector<int> visible_rows(const Side& s) const;
    // 某面板列表可视区高度（标题条 + 搜索区之下）
    float list_view_h(const Rect& panel, bool search_on) const;
    // 内容总高超过可视高度时，把 scroll 夹紧到有效范围
    void clamp_scroll(Side& s, const Rect& panel, bool search_on);
    void move(TransferDirection dir);
    void emit_change(TransferDirection dir, const std::vector<std::string>& moved);

    Side left_, right_;
    std::string left_label_ = "待选", right_label_ = "已选";
    bool show_search_ = false;
    TextBox* search_l_ = nullptr;
    TextBox* search_r_ = nullptr;
    Font* font_ = nullptr;
    const Theme* theme_ = nullptr;
    std::function<void(TransferDirection, const std::vector<std::string>&)> on_change_;
};

}  // namespace ui
