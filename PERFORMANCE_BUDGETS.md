# APEX Circuit Phase 3 Performance Budgets

The real-time target is 60 FPS (16.67 ms) at the default 82% dynamic resolution
on Apple Silicon and equivalent laptop-class hardware. Cinematic mode is an
offline/capture tier and is intentionally not constrained to 60 FPS.

| High-tier budget | Target |
| --- | ---: |
| Game thread | 4.0 ms |
| Render thread | 4.5 ms |
| GPU frame | 16.0 ms |
| Lumen GI + reflections | 5.0 ms |
| Virtual shadows | 3.0 ms |
| Translucency, fog, weather, post | 3.0 ms |
| Vehicle physics and AI | 2.5 ms game thread |

Use `stat unit`, `stat gpu`, `profilegpu`, and Unreal Insights on a Development
build after shaders have warmed. Capture a dry race and a maximum-rain eight-car
race; a feature passes only when both stay within budget over a complete lap.

The laptop tier is scalability level 1; High is the level 3/default 60 FPS tier;
Cinematic is level 4. Trackside repetition uses hierarchical instancing, while
the imported static showcase meshes use Nanite. The single 4.24 km continuous
circuit does not currently justify World Partition/HLOD streaming overhead.
TSR plus dynamic resolution is the vendor-neutral fallback. Hardware Lumen is
enabled only on supported Windows hardware, and DLSS remains deferred until a
representative Windows/NVIDIA profile proves the core tiers stable.
