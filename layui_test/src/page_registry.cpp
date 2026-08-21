#include "page_registry.h"

#include <algorithm>
#include <cstring>

namespace page {

namespace {
std::vector<PageDef> g_pages;
}  // namespace

void register_page(const PageDef& def) {
    g_pages.push_back(def);
}

const std::vector<PageDef>& all_pages() {
    return g_pages;
}

}  // namespace page
