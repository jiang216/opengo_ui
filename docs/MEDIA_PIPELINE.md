# MediaPipeline 媒体管线与视频容器

[UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) 引入的分层：把单片式 `VideoView`（解码线程 + RGBA 队列 + Texture 全在一份实现里）拆成
**可运行期按名更换的节点管线**（`Source / VideoDecoder / AudioSink`），并新增带控制条的视频容器控件。

## 分层与依赖

`media/core` 定义节点接口与组装（MediaPipeline），不依赖 FFmpeg；`media/ffmpeg` 提供 FFmpeg 实现节点并在运行期注册。

```
apps / layui_test
   └─ VideoContainer（media/core，带标题/控制条/占位文案）
        └─ MediaPipeline（media/core，解码线程 + 播放控制，语义同 VideoView M7/M13/M14）
             ├─ Source  "test-pattern"（media/core 合成源，纯 CPU，无需素材/第三方）
             ├─ Source  "ffmpeg"       （media/ffmpeg，FfmpegSource）
             ├─ Decoder "test-pattern" / "ffmpeg"（VideoDecoder → RGBA 帧）
             └─ AudioSink "null" / "winmm"（FfmpegAudioSink：swr s16 立体声 + winmm 输出）
```

- 依赖方向 `media/ffmpeg → media/core → ui → gl → core`；`media/core` 不暴露 C/FFmpeg 类型（`PacketHandle.opaque` 不透明）。
- 节点按名更换：`MediaPipeline::set_source_name/set_decoder_name/set_audio_name` 在 `open()` 前设置；
  `MediaRegistry`（`node_registry()`）为进程单例注册表，同名重复注册忽略（告警）。
- `open("synthetic[:秒]")` 自动切换 test-pattern 源（无需 FFmpeg 亦可演示/冒烟/截图回归）。
- media/ffmpeg 任一 TU 被引用（如 `VideoView` 构造）即通过 `register_ffmpeg_nodes()`（幂等）注册 ffmpeg/winmm 节点。
- GL 约束：`Texture` 必须归属所属窗口 GL 上下文（UI-146），每实例独立解码线程与纹理（同窗口多容器共存）。

## 关键语义（与既有 VideoView 对齐）

| 语义 | 说明 |
|------|------|
| 单槽交接 | 解码线程只保留最新一帧（`pending_frame_`），主线程 `advance(dt)` 弹出即可见，位置不跳变（UI-015） |
| 解码节流 | 解码线程每帧按 `1/(fps*speed)` sleep，避免解码远快于播放空转烧 CPU |
| 首帧放行 | 刚打开时第一帧不按节奏门控、立即交付（避免先闪占位/截图过早捕获空面） |
| Seek | 解码线程处理、主线程不阻塞；BACKWARD 定位到目标前关键帧后前进解码到精确位置（守卫 60 帧） |
| 音频 | 设备失败仅静音降级不阻塞视频；EAGAIN 先消费缓冲帧腾位再重送（防爆音/丢帧）；音量实时注入 |
| flush | seek/loop 回卷时解码器 `avcodec_flush_buffers` + winmm 队列 `flush`，音频同步复位 |
| EOF/loop | 播完且循环 → 回 0 重播；不循环 → 置 eof 等 play()/seek 重置 |

## 用法

```cpp
// 合成源（纯 UI 工程即可）：media_container 带内置控制条
ui::VideoContainer vc(0, 0, 560, 340);
vc.set_font(&font); vc.set_theme(&theme);
vc.set_title("媒体容器");
vc.open_source("synthetic:8");
/*** vc.open_source("assets/test.mp4");   // 真实文件：需链接 ui_media_ffmpeg
/                                 // 且应用已调用 ui::media::register_ffmpeg_nodes()（VideoView 构造即注册） ***/
host->add_child(&vc);   // 每帧 on_advance 自动推进并上传纹理

// 或旧控件形态（公共 API 未变，实现已迁移到管线之上）：
ui::media::VideoView vv;
vv.set_bounds(0, 0, 800, 450);
if (!vv.open_source("movie.mp4")) return;
vv.play(); vv.set_loop(true); vv.set_speed(2.f);
vv.seek(30.0); vv.set_volume(0.8f);
// 主线程每帧：vv.advance(dt)；退出 vv.close_source()
```

## 必须保证的功能（回归清单）

| 功能 | 来源 |
|------|------|
| VideoContainer 控制条交互：音量/亮度双竖向滑块（库内置基本滑块算法，拖动中不被每帧 `set_value_silent` 重置），亮度仅调视频画面（RGB tint，范围 0.2..3.0，默认 1.0，`set_brightness/brightness`），倍速 ▲▼ 步进按钮（0.5..2.0，步 0.25）；滑块上方 IconLabel 图标标签纯白、与轨道不重叠 | [UI-194](CHANGELOG.md#ui-194-videocontainer-控制条改版音量亮度双竖向滑块基本滑块算法-倍速--按钮--图标标签修复无法拖动--图标重叠--文本过暗mediacore2026-09-06) |
| 管线节点可运行期按名更换：`set_*_name` + `MediaRegistry` 注册表装配；合成源与 FFmpeg 节点并存（同名重复注册忽略、不覆盖）；先 open 合成源再 open 真实文件不互相污染节点名 | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| `VideoView` 公共 API 与行为语义保持不动（open/play/pause/seek/speed/volume/loop/step/advance/on_draw），实现整体迁到管线之上；FFmpeg 细节隔离节点实现 | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| 音频失败仅静音降级（`has_audio()==false`）不阻塞视频；音量实时作用于解码 PCM；winmm 队列在 seek/loop 时 flush | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| 合成源帧内容仅依赖绝对位置（SMPTE 色条 + 底部进度条），`--shot-page` 截图确定性可回归；首帧立即交付不闪占位 | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| 每实例独立解码线程/纹理，多容器同窗口共存互不污染；容器/页面销毁（切页/退出）时线程 join 干净、无崩溃无泄漏 | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| 实播回归：`video_player --smoke N assets/test_av.mp4` 解码+音频正常、无日志错误、线程干净退出；media_load_test 各素材解码帧数不变 | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |
| 构建：`OPENGUI_MEDIA_CORE` 默认 OFF，纯 UI 工程不依赖媒体库；layui_test 以 media/core 合成源演示，无需 FFmpeg 即可构建运行；`api_gen.py` 覆盖新增公开头（video_container/pipeline/node/registry/ffmpeg_nodes） | [UI-193](CHANGELOG.md#ui-193-媒体管线重构mediapipeline-节点化与视频容器-videocontainer--videoview-实现迁移到可运行期换节点的管线之上layuitest-多媒体页mediacoremediaffmpeglayuitestsrcpagesmediapagecpp2026-09-05) |