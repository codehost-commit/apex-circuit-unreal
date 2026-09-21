# Third-Party Assets

Phase 2 imports only assets already credited by the Godot project and keeps the raw sources in `Content/SourceAssets` for reproducibility.

| Unreal destination | Source | License / status |
| --- | --- | --- |
| `Art/Textures`, `Art/HDRI` | Poly Haven Asphalt Track, Grass Ground, Gravel Floor, and HDRIs | CC0 |
| `Art/UI`, `Art/Branding`, `Art/Trackside`, `Art/Sponsors` | Existing APEX project artwork | Original project artwork |
| `Audio/Engine` | Muted.io Performance Cars recording layers | CC0 1.0; converted from OGG to 48 kHz WAV with macOS `afconvert` |
| `Audio/Engineer` | Existing Pritam race-engineer recordings | Original project recording; retain project credit |
| `Environment/Nature/IslandTree` | Poly Haven “Island Tree 02” by Rico Cilliers / Rob Tuytel | CC0; optimized GLB inherited from the Godot source project |
| `Environment/Nature/PineSaplings` | Poly Haven “Pine Sapling Small” by Rico Cilliers / Rob Tuytel | CC0; optimized GLB inherited from the Godot source project |
| `Vehicles/RB14` | “2018 Redbull RB14” by Dave Love SketchFab, <https://skfb.ly/pLELV> | CC BY 4.0; temporary branded development/test visual, attribution required, not approved for release |

The RB14 is present because it was explicitly requested for high-quality local testing. It is not CC0: its correct license is [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/), and the creator/source attribution above must be preserved. The eventual APEX Formula body must remain unbranded and permissively licensed or original before any public release. Full original provenance is preserved in `../apex-circuit-godot/assets/ATTRIBUTIONS.md`.
