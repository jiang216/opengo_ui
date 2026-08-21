#include "ui/ui/layui_style.h"

#include "ui/ui/layui_icons.h"
#include "ui/ui/widget.h"

#include <cstring>

namespace ui {
namespace layui {
namespace {

struct NamedColor {
    const char* name;
    Vec4 fill;   // background-color
    Vec4 text;   // color
};

// layui.css 工具色（.layui-bg-*）
const NamedColor kBgColors[] = {
    {"red",    {1.0f, 0.34f, 0.13f, 1.0f}, {1, 1, 1, 1}},   // #ff5722
    {"orange", {1.0f, 0.72f, 0.0f, 1.0f}, {1, 1, 1, 1}},    // #ffb800
    {"green",  {0.0f, 0.588f, 0.533f, 1.0f}, {1, 1, 1, 1}},  // #009688
    {"cyan",   {0.18f, 0.25f, 0.34f, 1.0f}, {1, 1, 1, 1}},  // #2f4056
    {"blue",   {0.12f, 0.62f, 1.0f, 1.0f}, {1, 1, 1, 1}},   // #1e9fff
    {"purple", {0.64f, 0.20f, 0.78f, 1.0f}, {1, 1, 1, 1}},  // #a233c6
    {"black",  {0.224f, 0.239f, 0.286f, 1.0f}, {1, 1, 1, 1}},  // #393d49
    {"gray",   {0.98f, 0.98f, 0.98f, 1.0f}, {0.37f, 0.37f, 0.37f, 1.0f}},  // #fafafa / #5f5f5f
};

const NamedColor* find(const char* cls) {
    // cls 形如 "layui-bg-red"，跳过前缀后比较
    if (!cls) return nullptr;
    const char* name = cls;
    // 取最后一个 '-' 之后的段作为颜色名（兼容 "layui-bg-red" 与直接 "red"）
    const char* p = cls;
    while (*p) {
        if (*p == '-') name = p + 1;
        ++p;
    }
    for (const NamedColor& c : kBgColors) {
        const char* a = name;
        const char* b = c.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') return &c;
    }
    return nullptr;
}

// 文字工具色（.layui-font-*，克隆自 layui.css 2.13.x；与 .layui-bg-* 色值不同源）
struct NamedFontColor {
    const char* name;
    Vec4 color;
};
const NamedFontColor kFontColors[] = {
    {"red",    {1.0f, 0.34f, 0.13f, 1.0f}},   // #ff5722
    {"orange", {1.0f, 0.72f, 0.0f, 1.0f}},    // #ffb800
    {"green",  {0.086f, 0.729f, 0.667f, 1.0f}},  // #16baaa
    {"cyan",   {0.18f, 0.25f, 0.34f, 1.0f}},  // #2f4056
    {"blue",   {0.004f, 0.667f, 0.929f, 1.0f}},  // #01aaed
    {"purple", {0.64f, 0.20f, 0.78f, 1.0f}},  // #a233c6
    {"black",  {0.f, 0.f, 0.f, 1.0f}},        // #000000
    {"gray",   {0.761f, 0.761f, 0.761f, 1.0f}},  // #c2c2c2
};

// 类名最后一段（'-' 之后）：兼容 "layui-font-red" / "red"
const char* color_name(const char* cls) {
    if (!cls) return "";
    const char* name = cls;
    for (const char* p = cls; *p; ++p) {
        if (*p == '-') name = p + 1;
    }
    return name;
}

}  // namespace

Vec4 bg_color(const char* cls) {
    const NamedColor* c = find(cls);
    return c ? c->text : Vec4{1, 1, 1, 1};
}

Vec4 bg_fill(const char* cls) {
    const NamedColor* c = find(cls);
    return c ? c->fill : Vec4{0, 0, 0, 1};
}

Vec4 font_color(const char* cls) {
    const char* name = color_name(cls);
    for (const NamedFontColor& c : kFontColors) {
        const char* a = name;
        const char* b = c.name;
        while (*a && *b && *a == *b) {
            ++a;
            ++b;
        }
        if (*a == '\0' && *b == '\0') return c.color;
    }
    return Vec4{0, 0, 0, 1};  // 未知类回退黑色
}

Vec4 hex(const char* hex) {
    Vec4 out{0, 0, 0, 1};
    if (!hex) return out;
    if (hex[0] == '#') ++hex;
    const size_t len = strlen(hex);
    if (len < 6) return out;
    const auto hexv = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };
    out.r = (float)(hexv(hex[0]) * 16 + hexv(hex[1])) / 255.f;
    out.g = (float)(hexv(hex[2]) * 16 + hexv(hex[3])) / 255.f;
    out.b = (float)(hexv(hex[4]) * 16 + hexv(hex[5])) / 255.f;
    return out;
}

bool has_class(const Widget* w, const char* cls) {
    return w && w->has_class(cls);
}

}  // namespace layui

namespace layui_icon {

namespace {
// layui-icon-<name> 类名（去掉前缀）-> 码点。顺序无关，线性查表。
const struct IconEntry { const char* name; uint32_t cp; } kIcons[] = {
    {"404", 0xE61C},
    {"about", 0xE60B},
    {"add-1", 0xE654},
    {"add-circle", 0xE61F},
    {"add-circle-fine", 0xE608},
    {"addition", 0xE624},
    {"align-center", 0xE647},
    {"align-left", 0xE649},
    {"align-right", 0xE648},
    {"android", 0xE684},
    {"app", 0xE653},
    {"at", 0xE687},
    {"auz", 0xE672},
    {"backspace", 0xE694},
    {"bluetooth", 0xE689},
    {"bot", 0xE7D6},
    {"camera", 0xE660},
    {"camera-fill", 0xE65D},
    {"carousel", 0xE634},
    {"cart", 0xE657},
    {"cart-simple", 0xE698},
    {"cellphone", 0xE678},
    {"cellphone-fine", 0xE63B},
    {"chart", 0xE62C},
    {"chart-screen", 0xE629},
    {"chat", 0xE606},
    {"chrome", 0xE68A},
    {"circle", 0xE63F},
    {"circle-dot", 0xE617},
    {"clear", 0xE788},
    {"close", 0x1006},
    {"close-fill", 0x1007},
    {"code-circle", 0xE635},
    {"cols", 0xE610},
    {"component", 0xE857},
    {"console", 0xE665},
    {"date", 0xE637},
    {"delete", 0xE640},
    {"dialogue", 0xE63A},
    {"diamond", 0xE735},
    {"disabled", 0xE6CC},
    {"dollar", 0xE659},
    {"down", 0xE61A},
    {"download-circle", 0xE601},
    {"edge", 0xE68B},
    {"edit", 0xE642},
    {"email", 0xE618},
    {"engine", 0xE628},
    {"error", 0xE693},
    {"export", 0xE67D},
    {"eye", 0xE695},
    {"eye-invisible", 0xE696},
    {"face-cry", 0xE69C},
    {"face-smile", 0xE6AF},
    {"face-smile-b", 0xE650},
    {"face-smile-fine", 0xE60C},
    {"face-surprised", 0xE664},
    {"female", 0xE661},
    {"file", 0xE621},
    {"file-b", 0xE655},
    {"find-fill", 0xE670},
    {"fire", 0xE756},
    {"firefox", 0xE686},
    {"flag", 0xE66C},
    {"folder", 0xEABE},
    {"folder-open", 0xEAC1},
    {"fonts-clear", 0xE639},
    {"fonts-code", 0xE64E},
    {"fonts-del", 0xE64F},
    {"fonts-html", 0xE64B},
    {"fonts-i", 0xE644},
    {"fonts-strong", 0xE62B},
    {"fonts-u", 0xE646},
    {"form", 0xE63C},
    {"friends", 0xE612},
    {"gift", 0xE627},
    {"gitee", 0xE69B},
    {"github", 0xE6A7},
    {"group", 0xE613},
    {"headset", 0xE6FC},
    {"heart", 0xE68C},
    {"heart-fill", 0xE68F},
    {"help", 0xE607},
    {"home", 0xE68E},
    {"ie", 0xE7BB},
    {"ios", 0xE680},
    {"key", 0xE683},
    {"layer", 0xE638},
    {"layim-download", 0xE61E},
    {"layim-theme", 0xE61B},
    {"layim-uploadfile", 0xE61D},
    {"layouts", 0xE632},
    {"leaf", 0xE701},
    {"left", 0xE603},
    {"light", 0xE748},
    {"link", 0xE64C},
    {"list", 0xE60A},
    {"loading", 0xE63D},
    {"loading-1", 0xE63E},
    {"location", 0xE715},
    {"lock", 0xE69A},
    {"log", 0xE60E},
    {"login-qq", 0xE676},
    {"login-wechat", 0xE677},
    {"login-weibo", 0xE675},
    {"logout", 0xE682},
    {"male", 0xE662},
    {"menu-fill", 0xE60F},
    {"mike", 0xE6DC},
    {"moon", 0xE6C2},
    {"more", 0xE65F},
    {"more-vertical", 0xE671},
    {"music", 0xE690},
    {"mute", 0xE685},
    {"next", 0xE65B},
    {"note", 0xE66E},
    {"notice", 0xE667},
    {"ok", 0xE605},
    {"ok-circle", 0x1005},
    {"password", 0xE673},
    {"pause", 0xE651},
    {"picture", 0xE64A},
    {"picture-fine", 0xE60D},
    {"play", 0xE652},
    {"praise", 0xE6C6},
    {"prev", 0xE65A},
    {"prev-circle", 0xE633},
    {"print", 0xE66D},
    {"question", 0xE699},
    {"radio", 0xE643},
    {"rate", 0xE67B},
    {"rate-half", 0xE6C9},
    {"rate-solid", 0xE67A},
    {"read", 0xE705},
    {"reduce-circle", 0xE616},
    {"refresh", 0xE669},
    {"refresh-1", 0xE666},
    {"refresh-3", 0xE9AA},
    {"release", 0xE609},
    {"reply-fill", 0xE611},
    {"return", 0xE65C},
    {"right", 0xE602},
    {"rmb", 0xE65E},
    {"rss", 0xE808},
    {"screen-full", 0xE622},
    {"screen-restore", 0xE758},
    {"search", 0xE615},
    {"senior", 0xE674},
    {"service", 0xE626},
    {"set", 0xE716},
    {"set-fill", 0xE614},
    {"set-sm", 0xE620},
    {"share", 0xE641},
    {"shrink-right", 0xE668},
    {"slider", 0xE714},
    {"snowflake", 0xE6B1},
    {"sound", 0xE69D},
    {"speaker", 0xE645},
    {"spread-left", 0xE66B},
    {"star", 0xE600},
    {"star-fill", 0xE658},
    {"subtraction", 0xE67E},
    {"success", 0xE697},
    {"survey", 0xE6B2},
    {"table", 0xE62D},
    {"tabs", 0xE62A},
    {"template", 0xE663},
    {"template-1", 0xE656},
    {"templeate-1", 0xE630},
    {"test", 0xE692},
    {"theme", 0xE66A},
    {"time", 0xE68D},
    {"tips", 0xE702},
    {"tips-fill", 0xEB2E},
    {"top", 0xE604},
    {"transfer", 0xE691},
    {"tread", 0xE6C5},
    {"tree", 0xE62E},
    {"triangle-d", 0xE625},
    {"triangle-r", 0xE623},
    {"unlink", 0xE64D},
    {"up", 0xE619},
    {"upload", 0xE67C},
    {"upload-circle", 0xE62F},
    {"upload-drag", 0xE681},
    {"user", 0xE770},
    {"username", 0xE66F},
    {"util", 0xE631},
    {"vercode", 0xE679},
    {"video", 0xE6ED},
    {"voice", 0xE688},
    {"water", 0xE636},
    {"website", 0xE7AE},
    {"wifi", 0xE7E0},
    {"windows", 0xE67F},
};
}  // namespace

uint32_t from_class(const char* name) {
    if (!name) return 0;
    for (const auto& e : kIcons) {
        if (std::strcmp(name, e.name) == 0) return e.cp;
    }
    return 0;
}

}  // namespace layui_icon
}  // namespace ui
