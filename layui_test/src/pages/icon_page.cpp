#include "../page_registry.h"

#include "ui/ui/blockquote.h"
#include "ui/ui/button.h"
#include "ui/ui/card.h"
#include "ui/ui/code.h"
#include "ui/ui/fieldset.h"
#include "ui/ui/hbox.h"
#include "ui/ui/icon.h"
#include "ui/ui/label.h"
#include "ui/ui/layui_icons.h"
#include "ui/ui/scrollbox.h"
#include "ui/ui/theme.h"
#include "ui/ui/vbox.h"

namespace {

constexpr float kCW = 1000.f;

// 图标条目：类名 / 中文名 / 码点十六进制文本（显示用，渲染走 layui_icon::from_class）。
// 前 192 项与镜像 docs/2/icon 图标列表同序；末尾追加 layim×3（镜像页已下架、表内保留），共 195 项。
struct IconItem {
    const char* name;
    const char* cn;
    const char* cp;
};
const IconItem kIcons[] = {
{"bot", "机器人", "e7d6"},
{"leaf", "叶子节点", "e701"},
{"folder", "文件夹", "eabe"},
{"folder-open", "文件夹打开", "eac1"},
{"gitee", "Gitee", "e69b"},
{"github", "GitHub", "e6a7"},
{"light", "太阳/明亮", "e748"},
{"moon", "月亮", "e6c2"},
{"error", "错误", "e693"},
{"success", "成功", "e697"},
{"question", "问号", "e699"},
{"lock", "锁定", "e69a"},
{"eye", "显示", "e695"},
{"eye-invisible", "隐藏", "e696"},
{"clear", "清空/删除", "e788"},
{"backspace", "退格", "e694"},
{"disabled", "禁用", "e6cc"},
{"tips-fill", "感叹号/提示", "eb2e"},
{"test", "测试/K线图", "e692"},
{"music", "音乐/音符", "e690"},
{"chrome", "Chrome", "e68a"},
{"firefox", "Firefox", "e686"},
{"edge", "Edge", "e68b"},
{"ie", "IE", "e7bb"},
{"heart-fill", "实心", "e68f"},
{"heart", "空心", "e68c"},
{"time", "时间/历史", "e68d"},
{"at", "@艾特", "e687"},
{"email", "邮箱", "e618"},
{"rss", "RSS", "e808"},
{"sound", "声音", "e69d"},
{"mute", "静音", "e685"},
{"mike", "录音/麦克风", "e6dc"},
{"key", "密钥/钥匙", "e683"},
{"gift", "礼物/活动", "e627"},
{"bluetooth", "蓝牙", "e689"},
{"wifi", "WiFi", "e7e0"},
{"logout", "退出/注销", "e682"},
{"android", "Android 安卓", "e684"},
{"ios", "Apple IOS 苹果", "e680"},
{"windows", "Windows", "e67f"},
{"transfer", "穿梭框", "e691"},
{"service", "客服", "e626"},
{"subtraction", "减", "e67e"},
{"addition", "加", "e624"},
{"slider", "滑块", "e714"},
{"print", "打印", "e66d"},
{"export", "导出", "e67d"},
{"cols", "列", "e610"},
{"screen-restore", "退出全屏", "e758"},
{"screen-full", "全屏", "e622"},
{"rate-half", "半星", "e6c9"},
{"rate", "星星-空心", "e67b"},
{"rate-solid", "星星-实心", "e67a"},
{"cellphone", "手机", "e678"},
{"vercode", "验证码", "e679"},
{"login-wechat", "微信", "e677"},
{"login-qq", "QQ", "e676"},
{"login-weibo", "微博", "e675"},
{"password", "密码", "e673"},
{"username", "用户名", "e66f"},
{"refresh-3", "刷新-粗", "e9aa"},
{"auz", "授权", "e672"},
{"spread-left", "左向右伸缩菜单", "e66b"},
{"shrink-right", "右向左伸缩菜单", "e668"},
{"snowflake", "雪花", "e6b1"},
{"tips", "提示说明", "e702"},
{"note", "便签", "e66e"},
{"home", "主页", "e68e"},
{"senior", "高级", "e674"},
{"refresh", "刷新", "e669"},
{"refresh-1", "刷新", "e666"},
{"flag", "旗帜", "e66c"},
{"theme", "主题", "e66a"},
{"notice", "消息-通知", "e667"},
{"website", "网站", "e7ae"},
{"console", "控制台", "e665"},
{"face-surprised", "表情-惊讶", "e664"},
{"set", "设置-空心", "e716"},
{"template-1", "模板", "e656"},
{"app", "应用", "e653"},
{"template", "模板", "e663"},
{"praise", "赞", "e6c6"},
{"tread", "踩", "e6c5"},
{"male", "男", "e662"},
{"female", "女", "e661"},
{"camera", "相机-空心", "e660"},
{"camera-fill", "相机-实心", "e65d"},
{"more", "菜单-水平", "e65f"},
{"more-vertical", "菜单-垂直", "e671"},
{"rmb", "金额-人民币", "e65e"},
{"dollar", "金额-美元", "e659"},
{"diamond", "钻石-等级", "e735"},
{"fire", "火", "e756"},
{"return", "返回", "e65c"},
{"location", "位置-地图", "e715"},
{"read", "办公-阅读", "e705"},
{"survey", "调查", "e6b2"},
{"face-smile", "表情-微笑", "e6af"},
{"face-cry", "表情-哭泣", "e69c"},
{"cart-simple", "购物车", "e698"},
{"cart", "购物车", "e657"},
{"next", "下一页", "e65b"},
{"prev", "上一页", "e65a"},
{"upload-drag", "上传-空心-拖拽", "e681"},
{"upload", "上传-实心", "e67c"},
{"download-circle", "下载-圆圈", "e601"},
{"component", "组件", "e857"},
{"file-b", "文件-粗", "e655"},
{"user", "用户", "e770"},
{"find-fill", "发现-实心", "e670"},
{"loading", "loading", "e63d"},
{"loading-1", "loading", "e63e"},
{"add-1", "添加", "e654"},
{"play", "播放", "e652"},
{"pause", "暂停", "e651"},
{"headset", "音频-耳机", "e6fc"},
{"video", "视频", "e6ed"},
{"voice", "语音-声音", "e688"},
{"speaker", "消息-通知-喇叭", "e645"},
{"fonts-del", "删除线", "e64f"},
{"fonts-code", "代码", "e64e"},
{"fonts-html", "HTML", "e64b"},
{"fonts-strong", "字体加粗", "e62b"},
{"unlink", "删除链接", "e64d"},
{"picture", "图片", "e64a"},
{"link", "链接", "e64c"},
{"face-smile-b", "表情-笑-粗", "e650"},
{"align-left", "左对齐", "e649"},
{"align-right", "右对齐", "e648"},
{"align-center", "居中对齐", "e647"},
{"fonts-u", "字体-下划线", "e646"},
{"fonts-i", "字体-斜体", "e644"},
{"tabs", "Tabs 选项卡", "e62a"},
{"radio", "单选框-选中", "e643"},
{"circle", "单选框-候选", "e63f"},
{"edit", "编辑", "e642"},
{"share", "分享", "e641"},
{"delete", "删除", "e640"},
{"form", "表单", "e63c"},
{"cellphone-fine", "手机-细体", "e63b"},
{"dialogue", "聊天 对话 沟通", "e63a"},
{"fonts-clear", "文字格式化", "e639"},
{"layer", "窗口", "e638"},
{"date", "日期", "e637"},
{"water", "水 下雨", "e636"},
{"code-circle", "代码-圆圈", "e635"},
{"carousel", "轮播组图", "e634"},
{"prev-circle", "翻页", "e633"},
{"layouts", "布局", "e632"},
{"util", "工具", "e631"},
{"templeate-1", "选择模板", "e630"},
{"upload-circle", "上传-圆圈", "e62f"},
{"tree", "树", "e62e"},
{"table", "表格", "e62d"},
{"chart", "图表", "e62c"},
{"chart-screen", "图标 报表 屏幕", "e629"},
{"engine", "引擎", "e628"},
{"triangle-d", "下三角", "e625"},
{"triangle-r", "右三角", "e623"},
{"file", "文件", "e621"},
{"set-sm", "设置-小型", "e620"},
{"reduce-circle", "减少-圆圈", "e616"},
{"add-circle", "添加-圆圈", "e61f"},
{"404", "404", "e61c"},
{"about", "关于", "e60b"},
{"up", "箭头 向上", "e619"},
{"down", "箭头 向下", "e61a"},
{"left", "箭头 向左", "e603"},
{"right", "箭头 向右", "e602"},
{"circle-dot", "圆点", "e617"},
{"search", "搜索", "e615"},
{"set-fill", "设置-实心", "e614"},
{"group", "群组", "e613"},
{"friends", "好友", "e612"},
{"reply-fill", "回复 评论 实心", "e611"},
{"menu-fill", "菜单 隐身 实心", "e60f"},
{"log", "记录", "e60e"},
{"picture-fine", "图片-细体", "e60d"},
{"face-smile-fine", "表情-笑-细体", "e60c"},
{"list", "列表", "e60a"},
{"release", "发布 纸飞机", "e609"},
{"ok", "对 OK", "e605"},
{"help", "帮助", "e607"},
{"chat", "客服", "e606"},
{"top", "top 置顶", "e604"},
{"star", "收藏-空心", "e600"},
{"star-fill", "收藏-实心", "e658"},
{"close-fill", "关闭-实心", "1007"},
{"close", "关闭-空心", "1006"},
{"ok-circle", "正确", "1005"},
{"add-circle-fine", "添加-圆圈-细体", "e608"},
{"layim-download", "下载", "e61e"},
{"layim-theme", "主题", "e61b"},
{"layim-uploadfile", "上传文件", "e61d"},
};
constexpr int kIconCount = (int)(sizeof(kIcons) / sizeof(kIcons[0]));

// 单元格：镜像 docs-icon 卡片（白底 1px 描边，四行＝字形/中文名/码点/类名）
ui::Card* cell(PageHost& h, const IconItem& it) {
    auto* card = new ui::Card(0, 0, 156.f, 0);
    card->set_font(h.font);
    card->set_theme(h.theme);
    card->set_spacing(5.f);
    card->set_outline(ui::Vec4{0.91f, 0.91f, 0.91f, 1.f}, 1.f);  // 镜像 #e8e8e8 描边
    auto* ic = new ui::Icon(0, 0, 26.f, 26.f, it.name);
    ic->set_font(h.font);
    ic->set_theme(h.theme);
    card->add_child(ic);
    auto* cn = new ui::Label(0, 0, 126.f, 0, it.cn);
    cn->set_font(h.font);
    cn->set_theme(h.theme);
    cn->set_color(h.theme->text);
    card->add_child(cn);
    std::string code = std::string("&#x") + it.cp + ";";
    auto* cp = new ui::Label(0, 0, 126.f, 0, code.c_str());
    cp->set_font(h.font);
    cp->set_theme(h.theme);
    cp->set_color(h.theme->text_muted);
    card->add_child(cp);
    std::string cls = std::string("layui-icon-") + it.name;
    auto* l = new ui::Label(0, 0, 126.f, 0, cls.c_str());
    l->set_font(h.font);
    l->set_theme(h.theme);
    l->set_color(h.theme->text_muted);
    card->add_child(l);
    return card;
}

void build_icon(PageHost& host) {
    // ---- 顶部引用块 ----
    auto* bq1 = new ui::BlockQuote(kCW, 0,
        "layui-icon 图标库（195 项＝镜像 192 + layim 3）：字形来自 iconfont.ttf 码点，"
        "由 ui::Icon + ui::layui_icon 码点表渲染。用法 class=\"layui-icon layui-icon-xxx\"。");
    bq1->set_font(host.font);
    bq1->set_theme(host.theme);
    host.content->add_child(bq1);

    // ---- 彩色图标 ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "彩色图标（set_color）");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        auto* r = new ui::HBox(0, 0, kCW, 30.f);
        r->set_padding(0.f);
        r->set_spacing(28.f);
        r->set_theme(host.theme);
        struct C { const char* name; ui::Vec4 color; };
        static const C kColored[] = {
            {"heart-fill", {1.f, 0.34f, 0.13f, 1.f}},
            {"star-fill", {1.f, 0.72f, 0.f, 1.f}},
            {"like", {0.09f, 0.72f, 0.47f, 1.f}},
            {"notice", {0.12f, 0.62f, 1.f, 1.f}},
            {"diamond", {0.64f, 0.20f, 0.78f, 1.f}},
            {"fire", {0.18f, 0.25f, 0.34f, 1.f}},
        };
        for (const C& c : kColored) {
            if (!ui::layui_icon::from_class(c.name)) continue;  // 表中无此名则跳过
            auto* ic = new ui::Icon(0, 0, 24.f, 24.f, c.name);
            ic->set_font(host.font);
            ic->set_color(c.color);
            r->add_child(ic);
        }
        host.content->add_child(r);
    }

    // ---- 按钮图标（镜像 h2#icon 两个 layui-btn-container） ----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "按钮图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        // 容器一：默认尺寸——「按钮 ▾」（文字在前图标在后）+ left/right/edit/share 纯图标
        {
            auto* r = new ui::HBox(0, 0, kCW, 38.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);

            auto* b1 = new ui::Button(0, 0, 76.f, 38.f, "按钮");
            b1->set_font(host.font);
            b1->set_theme(host.theme);
            b1->set_icon("down");
            b1->set_icon_right(true);  // 镜像：文字在前、down 图标在后
            r->add_child(b1);

            static const char* const kDef[] = {"left", "right", "edit", "share"};
            for (const char* name : kDef) {
                auto* b = new ui::Button(0, 0, 44.f, 38.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(name);
                r->add_child(b);
            }
            host.content->add_child(r);
        }

        // 容器二（单行 9 个，同镜像一个 container）：
        // sm primary×5（left/right/edit/delete/share）+ sm disabled delete
        // + sm normal left + sm warm right + sm danger edit
        {
            auto* r = new ui::HBox(0, 0, kCW, 30.f);
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            struct I { const char* icon; const char* cls; };
            static const I kSm[] = {
                {"left",   "layui-btn layui-btn-sm layui-btn-primary"},
                {"right",  "layui-btn layui-btn-sm layui-btn-primary"},
                {"edit",   "layui-btn layui-btn-sm layui-btn-primary"},
                {"delete", "layui-btn layui-btn-sm layui-btn-primary"},
                {"share",  "layui-btn layui-btn-sm layui-btn-primary"},
                {"delete", "layui-btn layui-btn-sm layui-btn-disabled"},
                {"left",   "layui-btn layui-btn-sm layui-btn-normal"},
                {"right",  "layui-btn layui-btn-sm layui-btn-warm"},
                {"edit",   "layui-btn layui-btn-sm layui-btn-danger"},
            };
            for (const I& i : kSm) {
                auto* b = new ui::Button(0, 0, 34.f, 30.f, "");
                b->set_font(host.font);
                b->set_theme(host.theme);
                b->set_icon(i.icon);
                b->add_class(i.cls);
                r->add_child(b);
            }
            host.content->add_child(r);
        }
    }

    // ---- 全部图标（镜像「图标列表」一比一：6 列描边卡片 × 4 行内容）----
    {
        auto* title = new ui::FieldsetTitle(kCW, 26, "全部图标");
        title->set_font(host.font);
        title->set_theme(host.theme);
        host.content->add_child(title);

        constexpr int kPerRow = 6;
        for (int base = 0; base < kIconCount; base += kPerRow) {
            auto* r = new ui::HBox(0, 0, kCW, 0);  // h=0 自适应（HBox::preferred_height）
            r->set_padding(0.f);
            r->set_spacing(8.f);
            r->set_theme(host.theme);
            for (int i = base; i < base + kPerRow && i < kIconCount; ++i)
                r->add_child(cell(host, kIcons[i]));
            host.content->add_child(r);
        }
    }

    // ---- 代码 tab ----
    if (host.code) {
        host.code->set_lang("C++");
        host.code->set_code(
            "// pages/icon_page.cpp：克隆 layui 图标文档\n"
            "\n"
            "#include \"ui/ui/icon.h\"\n"
            "\n"
            "ui::Icon* ic = new ui::Icon(0, 0, 30, 30, \"star\");\n"
            "ic->set_font(host.font);   // iconfont.ttf 已并入 Font 私用区\n"
            "ic->set_color(theme.accent);\n"
            "\n"
            "// 码点查询：ui::layui_icon::from_class(\"star\") -> 0xE67A\n"
            "// 字体加载：Font::load_icon_font(\"iconfont.ttf\", 18.f)");
    }
    if (host.help) {
        const char* lines[] = {
            "图标 API 一览",
            "",
            "ui::Icon：单个私用区图标码点控件",
            "  构造 Icon(x, y, w, h, \"star\")：名字不含 layui-icon- 前缀",
            "  set_icon(name)：换图标；找不到则清空",
            "  set_color(Vec4)：着色（默认主题文字色）",
            "",
            "ui::layui_icon（layui_icons.h，自动生成勿手改）",
            "  from_class(name)：类名 -> 码点，找不到返回 0",
            "  常量表：ic_404 / about / star ... 共 195 项",
            "",
            "字体：exe 目录 iconfont.ttf 经 Font::load_icon_font 并入同一 Font，",
            "与中文文本共用（POST_BUILD 拷贝，UI-048 exe 目录回退）。",
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

static void build(PageHost& host) { build_icon(host); }

static const PageDef kIconPage{11, "通用", "图标", "layui-icon 图标库：按钮图标 + 彩色示例 + 195 项全量网格", build, nullptr, false, nullptr, 0, "Icon"};

static const bool kRegistered = []() {
    page::register_page(kIconPage);
    return true;
}();
