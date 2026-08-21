# ---------- ui: 控件树/布局/样式，依赖 ui_gl + ui_core ----------
add_library(ui_ui STATIC
    src/widget.cpp
    src/theme.cpp
    src/button.cpp
    src/button_group.cpp
    src/panel.cpp
    src/label.cpp
    src/textbox.cpp
    src/textarea.cpp
    src/slider.cpp
    src/scrollbox.cpp
    src/hbox.cpp
    src/flowbox.cpp
    src/grid.cpp
    src/checkbox.cpp
    src/radiobutton.cpp
    src/radiogroup.cpp
    src/combobox.cpp
    src/vbox.cpp
    src/panelwindow.cpp
    src/menubar.cpp
    src/layui_style.cpp
    src/progress.cpp
    src/badge.cpp
    src/card.cpp
    src/breadcrumb.cpp
    src/timeline.cpp
    src/rate.cpp
    src/switch.cpp
    src/tabs.cpp
    src/collapse.cpp
    src/dropdown.cpp
    src/tree.cpp
    src/navmenu.cpp
    src/navbar.cpp
    src/menu.cpp
    src/laypage.cpp
    src/table.cpp
    src/icon.cpp
    src/carousel.cpp
    src/code.cpp
    src/colorpicker.cpp
    src/transfer.cpp
    src/laydate.cpp
    src/laydatebox.cpp
    src/upload.cpp
    src/fieldset.cpp
    src/blockquote.cpp
    src/hline.cpp
    src/circle.cpp
    src/countdown_text.cpp
    src/image.cpp
    src/laytpl.cpp
    src/ime.cpp
)

target_include_directories(ui_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(ui_ui PUBLIC ui_gl ui_core)

# IME 候选窗跟随（ime.cpp，仅 Windows）
if(WIN32)
    target_link_libraries(ui_ui PRIVATE imm32)
endif()

file(GLOB UI_UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ui/*.h)
target_sources(ui_ui PRIVATE ${UI_UI_HEADERS})

# ---------- install ----------
include(GNUInstallDirs)

install(TARGETS ui_ui EXPORT opengl_uiTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/ui
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)

# 聚合头 ui/all.h
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/ui/all.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ui
)
