# Third-Party Assets

Phase 2 imports only assets already credited by the Godot project and keeps the raw sources in `Content/SourceAssets` for reproducibility.

| Unreal destination | Source | License / status |
| --- | --- | --- |
| `Art/Textures`, `Art/HDRI` | Poly Haven Asphalt Track, Grass Ground, Gravel Floor, and HDRIs | CC0 |
| `Art/UI`, `Art/Branding`, `Art/Trackside`, `Art/Sponsors` | Existing APEX project artwork | Original project artwork |
| `Audio/Engine` | Muted.io Performance Cars recording layers | CC0 1.0; converted from OGG to 48 kHz WAV with macOS `afconvert` |
| `Audio/Engineer` | Existing Pritam race-engineer recordings | Original project recording; retain project credit |

The CC-BY RB14 source mesh and any real-world liveries are intentionally not imported into the default Unreal experience. The eventual APEX Formula body must remain unbranded and permissively licensed or original. Full original provenance is preserved in `../apex-circuit-godot/assets/ATTRIBUTIONS.md`.
