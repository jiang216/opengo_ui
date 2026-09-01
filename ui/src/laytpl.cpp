#include "ui/ui/laytpl.h"

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace ui {

// ────────────────────────── LayTplValue ──────────────────────────
LayTplValue LayTplValue::make_null() { return LayTplValue(); }
LayTplValue LayTplValue::make_bool(bool v) {
    LayTplValue x; x.type_ = Type::Bool; x.bool_ = v; return x;
}
LayTplValue LayTplValue::make_number(double v) {
    LayTplValue x; x.type_ = Type::Number; x.number_ = v; return x;
}
LayTplValue LayTplValue::make_string(const std::string& v) {
    LayTplValue x; x.type_ = Type::String; x.str_ = v; return x;
}
LayTplValue LayTplValue::make_array() {
    LayTplValue x; x.type_ = Type::Array; x.obj_ = std::make_shared<Obj>(); return x;
}
LayTplValue LayTplValue::make_object() {
    LayTplValue x; x.type_ = Type::Object; x.obj_ = std::make_shared<Obj>(); return x;
}

const LayTplValue* LayTplValue::get(const std::string& key) const {
    if (!obj_) return nullptr;
    for (const auto& kv : obj_->items)
        if (kv.first == key) return &kv.second;
    return nullptr;
}
void LayTplValue::set(const std::string& key, const LayTplValue& v) {
    if (!obj_) obj_ = std::make_shared<Obj>();
    for (auto& kv : obj_->items) { if (kv.first == key) { kv.second = v; return; } }
    obj_->items.push_back({key, v});
    if (type_ != Type::Object) type_ = Type::Object;
}
void LayTplValue::push(const LayTplValue& v) {
    if (!obj_) obj_ = std::make_shared<Obj>();
    if (type_ != Type::Array) type_ = Type::Array;
    obj_->items.push_back({std::to_string(size_), v});
    size_ = obj_->items.size();
}
const LayTplValue* LayTplValue::at(size_t i) const {
    if (!obj_ || i >= obj_->items.size()) return nullptr;
    return &obj_->items[i].second;
}

std::string LayTplValue::to_string() const {
    switch (type_) {
        case Type::Null: return "";
        case Type::Bool: return bool_ ? "true" : "false";
        case Type::Number: {
            if (std::floor(number_) == number_) return std::to_string((long long)number_);
            std::ostringstream os; os << number_; return os.str();
        }
        case Type::String: return str_;
        case Type::Array:
        case Type::Object: {
            std::string s = type_ == Type::Array ? "[object Array]" : "[object Object]";
            return s;
        }
    }
    return "";
}

bool LayTplValue::truthy() const {
    switch (type_) {
        case Type::Null: return false;
        case Type::Bool: return bool_;
        case Type::Number: return number_ != 0.0 && !std::isnan(number_);
        case Type::String: return !str_.empty();
        case Type::Array: return size_ > 0;
        case Type::Object: return !obj_ || !obj_->items.empty();
    }
    return false;
}

// ────────────────────────── JSON 解析（子集）──────────────────────────
namespace {
struct JsonParser {
    const char* p;
    std::string err;

    void skip_ws() { while (*p && (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) ++p; }
    bool fail(const std::string& m) { if (err.empty()) err = m; return false; }

    bool parse(LayTplValue& out) {
        skip_ws();
        if (*p == '{') return parse_object(out);
        if (*p == '[') return parse_array(out);
        if (*p == '"' || *p == '\'') { std::string s; if (!parse_string(s)) return false; out = LayTplValue::make_string(s); return true; }
        return parse_literal(out);
    }
    bool parse_object(LayTplValue& out) {
        ++p; // '{'
        LayTplValue obj = LayTplValue::make_object();
        skip_ws();
        if (*p == '}') { ++p; out = obj; return true; }
        while (true) {
            skip_ws();
            std::string key;
            if (*p == '"' || *p == '\'') { if (!parse_string(key)) return false; }
            else { // 非引号键（laytpl 演示里 "title" 均带引号，此处兜底）
                const char* s = p;
                while (*p && *p != ':' && *p != ',' && *p != '}' && !isspace((unsigned char)*p)) ++p;
                key.assign(s, p - s);
            }
            skip_ws();
            if (*p != ':') return fail("expected ':' in object");
            ++p;
            LayTplValue v;
            if (!parse(v)) return false;
            obj.set(key, v);
            skip_ws();
            if (*p == ',') { ++p; continue; }
            if (*p == '}') { ++p; break; }
            return fail("expected ',' or '}' in object");
        }
        out = obj;
        return true;
    }
    bool parse_array(LayTplValue& out) {
        ++p; // '['
        LayTplValue arr = LayTplValue::make_array();
        skip_ws();
        if (*p == ']') { ++p; out = arr; return true; }
        while (true) {
            LayTplValue v;
            if (!parse(v)) return false;
            arr.push(v);
            skip_ws();
            if (*p == ',') { ++p; continue; }
            if (*p == ']') { ++p; break; }
            return fail("expected ',' or ']' in array");
        }
        out = arr;
        return true;
    }
    bool parse_string(std::string& out) {
        const char q = *p++;
        std::string s;
        while (*p && *p != q) {
            if (*p == '\\' && p[1]) {
                ++p;
                switch (*p) {
                    case 'n': s += '\n'; break;
                    case 't': s += '\t'; break;
                    case 'r': s += '\r'; break;
                    case '"': s += '"'; break;
                    case '\'': s += '\''; break;
                    case '\\': s += '\\'; break;
                    case 'u': {
                        if (p[1] && p[2] && p[3] && p[4]) {
                            char hex[5] = {p[1], p[2], p[3], p[4], 0};
                            s += (char)strtol(hex, nullptr, 16);
                            p += 4;
                        }
                        break;
                    }
                    default: s += *p;
                }
                ++p;
            } else s += *p++;
        }
        if (*p != q) return fail("unterminated string");
        ++p;
        out = s;
        return true;
    }
    bool parse_literal(LayTplValue& out) {
        if (strncmp(p, "true", 4) == 0) { p += 4; out = LayTplValue::make_bool(true); return true; }
        if (strncmp(p, "false", 5) == 0) { p += 5; out = LayTplValue::make_bool(false); return true; }
        if (strncmp(p, "null", 4) == 0) { p += 4; out = LayTplValue::make_null(); return true; }
        char* end = nullptr;
        double d = strtod(p, &end);
        if (end != p) { p = end; out = LayTplValue::make_number(d); return true; }
        return fail("unexpected token");
    }
};
}  // namespace

bool laytpl_parse_json(const char* json, LayTplValue& out, std::string* err) {
    if (!json) { if (err) *err = "null input"; return false; }
    JsonParser jp{json};
    bool ok = jp.parse(out);
    if (!ok && err) *err = jp.err;
    return ok;
}

// ────────────────────────── LayTpl 引擎 ──────────────────────────
namespace {

std::string g_open = "{{";
std::string g_close = "}}";

std::string escape_html(const std::string& s) {
    std::string r;
    r.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': r += "&amp;"; break;
            case '<': r += "&lt;"; break;
            case '>': r += "&gt;"; break;
            case '\'': r += "&#39;"; break;
            case '"': r += "&quot;"; break;
            default: r += c;
        }
    }
    return r;
}

std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

}  // namespace

// 模板被编译为一棵语句树。
struct LayTpl::Stmt {
    enum class Kind { Text, Output, Each, If };
    Kind kind;
    std::string text;          // Text
    std::string expr;          // Output / If.cond / Each.list
    bool escaped = false;      // Output 是否转义（{{= }}）
    std::string item_var, index_var;  // Each 参数名
    std::vector<Stmt> body;    // Each / If 子语句
    bool has_else = false;     // If 是否有 else
    size_t else_mark = 0;      // If 有 else 时，body 中 then 分支的末尾下标
};
struct LayTpl::VarScope {
    const VarScope* parent = nullptr;
    std::vector<std::pair<std::string, LayTplValue>> vars;
    const LayTplValue* find(const std::string& n) const {
        for (const auto& kv : vars) if (kv.first == n) return &kv.second;
        return parent ? parent->find(n) : nullptr;
    }
};

// 表达式求值（支持 JS 子集：数字/字符串/布尔/null、d 或变量成员访问、
// 数组下标、.length、|| && === == !== != ! + - * / %、函数体忽略）
bool LayTpl::parse_expr(const char*& p, const char* end, const LayTplValue& d,
                        const VarScope& scope, LayTplValue& out, std::string& err) const {
    // 跳过空白
    while (p < end && isspace((unsigned char)*p)) ++p;
    // 空表达式
    if (p >= end) return true;

    // 取第一项
    LayTplValue left;
    {
        if (*p == '"' || *p == '\'') {
            const char q = *p++;
            std::string s;
            while (p < end && *p != q) { s += *p++; }
            if (p >= end) { err = "unterminated string"; return false; }
            ++p; // 收引号
            left = LayTplValue::make_string(s);
        } else if (strncmp(p, "true", 4) == 0) { p += 4; left = LayTplValue::make_bool(true); }
        else if (strncmp(p, "false", 5) == 0) { p += 5; left = LayTplValue::make_bool(false); }
        else if (strncmp(p, "null", 4) == 0) { p += 4; left = LayTplValue::make_null(); }
        else if (*p == '!') {
            ++p;
            LayTplValue v; if (!parse_expr(p, end, d, scope, v, err)) return false;
            left = LayTplValue::make_bool(!v.truthy());
        } else if (*p == '(') {
            ++p;
            if (!parse_expr(p, end, d, scope, left, err)) return false;
            while (p < end && isspace((unsigned char)*p)) ++p;
            if (p < end && *p == ')') ++p;
        } else {
            // 标识符 / 数字
            if (isdigit((unsigned char)*p) || *p == '-' || *p == '+') {
                char* endp = nullptr;
                double num = strtod(p, &endp);
                if (endp != p) { p = endp; left = LayTplValue::make_number(num); }
                else { err = "bad number"; return false; }
            } else {
                const char* s = p;
                while (p < end && (isalnum((unsigned char)*p) || *p == '_' || *p == '$')) ++p;
                std::string name(s, p - s);
                if (name.empty()) { err = "bad token"; return false; }
                if (name == "d") left = d;
                else if (const LayTplValue* v = scope.find(name)) left = *v;
                else left = LayTplValue::make_null();
            }
        }
    }

    // 成员访问 / 下标 / 调用
    while (p < end) {
        while (p < end && isspace((unsigned char)*p)) ++p;
        if (p < end && *p == '.') {
            ++p;
            const char* s = p;
            while (p < end && (isalnum((unsigned char)*p) || *p == '_')) ++p;
            std::string key(s, p - s);
            if (key == "length" && left.is_array()) { left = LayTplValue::make_number((double)left.size()); }
            else if (key == "length" && left.is_string()) { left = LayTplValue::make_number((double)left.str().size()); }
            else if (const LayTplValue* v = left.get(key)) left = *v;
            else left = LayTplValue::make_null();
        } else if (p < end && *p == '[') {
            ++p;
            LayTplValue idx; if (!parse_expr(p, end, d, scope, idx, err)) return false;
            while (p < end && isspace((unsigned char)*p)) ++p;
            if (p < end && *p == ']') ++p;
            int i = (int)idx.number();
            if (const LayTplValue* v = left.at((size_t)i)) left = *v;
            else left = LayTplValue::make_null();
        } else break;
    }

    // 二元运算符（|| && === == !== != + - * / %）
    while (p < end) {
        while (p < end && isspace((unsigned char)*p)) ++p;
        if (p >= end) break;
        std::string op;
        if (strncmp(p, "===", 3) == 0) { op = "==="; p += 3; }
        else if (strncmp(p, "!==", 3) == 0) { op = "!=="; p += 3; }
        else if (strncmp(p, "==", 2) == 0) { op = "=="; p += 2; }
        else if (strncmp(p, "!=", 2) == 0) { op = "!="; p += 2; }
        else if (strncmp(p, "||", 2) == 0) { op = "||"; p += 2; }
        else if (strncmp(p, "&&", 2) == 0) { op = "&&"; p += 2; }
        else if (*p == '+') { op = "+"; ++p; }
        else if (*p == '-') { op = "-"; ++p; }
        else if (*p == '*') { op = "*"; ++p; }
        else if (*p == '/') { op = "/"; ++p; }
        else if (*p == '%') { op = "%"; ++p; }
        else break;

        LayTplValue rhs; if (!parse_expr(p, end, d, scope, rhs, err)) return false;
        if (op == "||") {
            if (!left.truthy()) left = rhs;
        } else if (op == "&&") {
            if (left.truthy()) left = rhs;
            else left = LayTplValue::make_bool(false);
        } else if (op == "===" || op == "==") {
            bool eq = (left.type() == rhs.type()) &&
                      (left.type() == LayTplValue::Type::String ? left.str() == rhs.str() :
                       left.type() == LayTplValue::Type::Number ? left.number() == rhs.number() :
                       left.type() == LayTplValue::Type::Bool ? left.boolean() == rhs.boolean() :
                       left.type() == LayTplValue::Type::Null);
            left = LayTplValue::make_bool(eq);
        } else if (op == "!==" || op == "!=") {
            bool eq = (left.type() == rhs.type()) &&
                      (left.type() == LayTplValue::Type::String ? left.str() == rhs.str() :
                       left.type() == LayTplValue::Type::Number ? left.number() == rhs.number() :
                       left.type() == LayTplValue::Type::Bool ? left.boolean() == rhs.boolean() :
                       left.type() == LayTplValue::Type::Null);
            left = LayTplValue::make_bool(!eq);
        } else if (op == "+") {
            if (left.type() == LayTplValue::Type::String || rhs.type() == LayTplValue::Type::String)
                left = LayTplValue::make_string(left.to_string() + rhs.to_string());
            else left = LayTplValue::make_number(left.number() + rhs.number());
        } else if (op == "-") left = LayTplValue::make_number(left.number() - rhs.number());
        else if (op == "*") left = LayTplValue::make_number(left.number() * rhs.number());
        else if (op == "/") left = LayTplValue::make_number(rhs.number() != 0 ? left.number() / rhs.number() : 0);
        else if (op == "%") left = LayTplValue::make_number(rhs.number() != 0 ? fmod(left.number(), rhs.number()) : 0);
    }
    out = left;
    return true;
}

void LayTpl::config(const char* open, const char* close) {
    if (open) g_open = open;
    if (close) g_close = close;
}

LayTpl::LayTpl(const std::string& tpl, std::string* err) {
    ok_ = compile(tpl);
    if (!ok_ && err) *err = error_;
}

// 模板编译：把 {{ }} / {{# }} / {{= }} / {{! !}} 分割成语句树。
// 用「语句栈」来配对 layui.each(...){ } / if(...){ } / } else { 等代码块。
bool LayTpl::compile(const std::string& tpl) {
    error_.clear();
    auto root = std::make_shared<Stmt>();
    std::vector<std::shared_ptr<Stmt>> stack{root};

    size_t i = 0;
    const size_t n = tpl.size();
    std::string text;

    auto emit = [&](const Stmt& s) {
        stack.back()->body.push_back(s);
    };
    auto flush_text = [&]() {
        if (!text.empty()) {
            Stmt s; s.kind = Stmt::Kind::Text; s.text = text;
            text.clear();
            emit(s);
        }
    };

    while (i < n) {
        size_t open = tpl.find(g_open, i);
        if (open == std::string::npos) { text += tpl.substr(i); i = n; break; }
        text += tpl.substr(i, open - i);
        size_t close = tpl.find(g_close, open + g_open.size());
        if (close == std::string::npos) { text += tpl.substr(open); i = n; break; }
        std::string inner = tpl.substr(open + g_open.size(), close - (open + g_open.size()));
        i = close + g_close.size();

        // {{! ... !}} 过滤区域：按纯文本输出（不解析）
        if (!inner.empty() && inner[0] == '!') {
            if (inner.size() >= 2 && inner[inner.size() - 1] == '!')
                text += inner.substr(1, inner.size() - 2);
            else
                text += inner.substr(1);
            continue;
        }

        // {{# ... }} JS 逻辑块
        if (!inner.empty() && inner[0] == '#') {
            flush_text();
            std::string code = trim(inner.substr(1));
            if (code.find("layui.each") == 0) {
                // 开块：layui.each( list, function(index, item){ ... }
                size_t lp = code.find('(');
                size_t first_comma = code.find(',', lp);
                std::string list_expr = first_comma == std::string::npos
                                            ? code.substr(lp + 1)
                                            : trim(code.substr(lp + 1, first_comma - lp - 1));
                // 参数名在 function(...) 里
                size_t fp = code.find("function");
                std::string index_var = "index", item_var = "item";
                if (fp != std::string::npos) {
                    size_t fpar = code.find('(', fp);
                    size_t fclose = code.find(')', fpar);
                    if (fpar != std::string::npos && fclose != std::string::npos) {
                        std::string params = code.substr(fpar + 1, fclose - fpar - 1);
                        size_t comma = params.find(',');
                        if (comma != std::string::npos) {
                            index_var = trim(params.substr(0, comma));
                            item_var = trim(params.substr(comma + 1));
                        } else index_var = trim(params);
                    }
                }
                Stmt s; s.kind = Stmt::Kind::Each;
                s.expr = list_expr;   // d.list
                s.index_var = index_var;
                s.item_var = item_var;
                stack.push_back(std::make_shared<Stmt>(s));
            } else if (code.find("if(") == 0 || code.find("if (") == 0) {
                size_t lp = code.find('(');
                size_t rp = code.rfind(')');
                if (lp != std::string::npos && rp != std::string::npos && rp > lp) {
                    Stmt s; s.kind = Stmt::Kind::If;
                    s.expr = code.substr(lp + 1, rp - lp - 1);
                    stack.push_back(std::make_shared<Stmt>(s));
                }
            } else if (code == "} else {" && stack.size() >= 2) {
                // else：标记 has_else + 记录 then 分支末尾，此后收集 else 分支
                stack.back()->has_else = true;
                stack.back()->else_mark = stack.back()->body.size();
            } else if (code.find('}') != std::string::npos && stack.size() >= 2) {
                // 闭合块（} 或 }; 或 });）
                flush_text();
                auto cur = stack.back();
                stack.pop_back();
                emit(*cur);
            }
            continue;
        }

        // {{ d.field }} / {{= d.field }} 输出
        flush_text();
        bool escaped = false;
        std::string expr = inner;
        if (!expr.empty() && expr[0] == '=') { escaped = true; expr = expr.substr(1); }
        expr = trim(expr);
        if (!expr.empty()) {
            Stmt s; s.kind = Stmt::Kind::Output;
            s.expr = expr;
            s.escaped = escaped;
            emit(s);
        }
    }
    flush_text();
    root_ = root;
    return true;
}

bool LayTpl::run_stmt(const Stmt& s, const LayTplValue& d, VarScope& scope,
                      std::string& out, std::string& err) const {
    switch (s.kind) {
        case Stmt::Kind::Text:
            out += s.text;
            break;
        case Stmt::Kind::Output: {
            const char* p = s.expr.c_str();
            const char* end = p + s.expr.size();
            LayTplValue v;
            if (!parse_expr(p, end, d, scope, v, err)) return false;
            std::string str = v.to_string();
            out += s.escaped ? escape_html(str) : str;
            break;
        }
        case Stmt::Kind::Each: {
            const char* p = s.expr.c_str();
            const char* end = p + s.expr.size();
            LayTplValue list;
            if (!parse_expr(p, end, d, scope, list, err)) return false;
            for (size_t i = 0; i < list.size(); ++i) {
                const LayTplValue* item = list.at(i);
                if (!item) continue;
                VarScope child;
                child.parent = &scope;
                child.vars.push_back({s.index_var, LayTplValue::make_number((double)i)});
                child.vars.push_back({s.item_var, *item});
                for (const auto& body : s.body) {
                    if (!run_stmt(body, d, child, out, err)) return false;
                }
            }
            break;
        }
        case Stmt::Kind::If: {
            const char* p = s.expr.c_str();
            const char* end = p + s.expr.size();
            LayTplValue v;
            if (!parse_expr(p, end, d, scope, v, err)) return false;
            size_t then_end = s.has_else ? s.else_mark : s.body.size();
            if (v.truthy()) {
                for (size_t k = 0; k < then_end && k < s.body.size(); ++k)
                    if (!run_stmt(s.body[k], d, scope, out, err)) return false;
            } else {
                for (size_t k = then_end; k < s.body.size(); ++k)
                    if (!run_stmt(s.body[k], d, scope, out, err)) return false;
            }
            break;
        }
    }
    return true;
}

std::string LayTpl::render(const LayTplValue& data) {
    if (!ok_ || !root_) return "";
    std::string out, err;
    VarScope scope;
    for (const auto& body : root_->body)
        if (!run_stmt(body, data, scope, out, err)) { error_ = err; return ""; }
    return out;
}
}  // namespace ui
