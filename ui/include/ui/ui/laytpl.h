#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace ui {

// laytpl 模板引擎的数据节点（port of laytpl 的 JSON 数据对象）。
// 支持对象（键值）/ 数组（有序列表）/ 标量（字符串、数字、布尔、null）。
class LayTplValue {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    LayTplValue() = default;
    static LayTplValue make_null();
    static LayTplValue make_bool(bool v);
    static LayTplValue make_number(double v);
    static LayTplValue make_string(const std::string& v);
    static LayTplValue make_array();
    static LayTplValue make_object();

    Type type() const { return type_; }
    bool is_object() const { return type_ == Type::Object; }
    bool is_array() const { return type_ == Type::Array; }
    bool is_string() const { return type_ == Type::String; }

    // 对象：按键取值 / 赋值
    const LayTplValue* get(const std::string& key) const;
    void set(const std::string& key, const LayTplValue& v);
    bool has(const std::string& key) const { return get(key) != nullptr; }

    // 数组：追加 / 按序取值 / 长度
    void push(const LayTplValue& v);
    const LayTplValue* at(size_t i) const;
    size_t size() const { return size_; }

    // 标量读取
    double number() const { return number_; }
    const std::string& str() const { return str_; }
    bool boolean() const { return bool_; }

    // 转为输出文本（{{ }} 非转义输出时的字符串化）
    std::string to_string() const;
    // JS 真值判断（""、0、null、false、空数组/对象 为假）
    bool truthy() const;

private:
    struct Obj {
        std::vector<std::pair<std::string, LayTplValue>> items;
    };
    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string str_;
    size_t size_ = 0;
    std::shared_ptr<Obj> obj_;  // 数组与对象共用存储（键 为索引转字符串 / 或真键）
    friend class LayTpl;
};

// 从 JSON 文本解析出数据节点（laytpl.html 的 data 文本框即 JSON）。
// 支持对象 / 数组 / 字符串（含转义）/ 数字 / 布尔 / null。
// 成功返回 true，失败返回 false 且 *err 携带原因。
bool laytpl_parse_json(const char* json, LayTplValue& out, std::string* err = nullptr);

// laytpl 前端模板引擎（port of layui.laytpl）：
//   模板语法：
//     {{ d.field }}        输出字段（不转义 html）
//     {{= d.field }}       输出字段（转义 html）
//     {{# JS 语句 }}       执行 JS 逻辑（if/each 等）
//     {{! 模板区域 !}}     过滤该区域，不解析
//   自定义分隔符：laytpl::config(open, close)（默认 {{ }}）
//   数据：以 JSON 文本（laytpl_parse_json）或程序构造的 LayTplValue 传入。
class LayTpl {
public:
    LayTpl(const std::string& tpl, std::string* err = nullptr);

    // 渲染：返回渲染后的字符串；失败返回空串（此时可查 last_error()）。
    std::string render(const LayTplValue& data);

    // 设置自定义分隔符（open / close），例如 "<%" "%>"。影响后续新构造的模板。
    static void config(const char* open, const char* close);

    const std::string& last_error() const { return error_; }
    bool ok() const { return ok_; }

private:
    bool compile(const std::string& tpl);

    struct Stmt;
    struct VarScope;

    bool parse_expr(const char*& p, const char* end, const LayTplValue& d,
                    const VarScope& scope, LayTplValue& out, std::string& err) const;
    bool run_stmt(const Stmt& s, const LayTplValue& d, VarScope& scope,
                  std::string& out, std::string& err) const;

    std::shared_ptr<Stmt> root_;
    std::string error_;
    bool ok_ = false;
};

}  // namespace ui
