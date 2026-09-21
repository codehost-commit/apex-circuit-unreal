# APEX Circuit identity

`apex-circuit-dark.png` is a dark-background adaptation of the white/silver/red logo supplied in the conversation. The original attachment was visible in chat but was not mounted as a local PNG. One built-in image-edit call prepared this version, preserving the supplied identity while making its lettering readable. It is an image-assisted adaptation, not a claim of byte-for-byte identity with the original attachment.

Used by credits, shared steering-wheel display and every APEX sponsor panel on cars and circuit structures. The square `apex-taskbar-icon.png` uses only the mark's angular A so Windows never compresses the wide wordmark. The startup and menu marks are drawn responsively by `ui/apex_logo_animation.gd`: APEX traces in letter order and CIRCUIT fades in afterward. UI textures preserve aspect ratio; 3D APEX panels use a 3:1 ratio. Other fictional sponsors keep their own identities.

## Taskbar icon edit

Built-in image editing isolated the exact visual concept of the leftmost A from `apex-circuit.png` onto a 1:1 dark field. Prompt: “Preserve the exact angular italic white letter A from the far left; remove every other letter and CIRCUIT; center only that A in a square with even padding; retain the metallic-white and navy-charcoal identity; no extra text, mockup, border or watermark.” Original output: `exec-d6450bf7-7bac-4b9a-9abb-9a89a03f3118.png`, generated-image session `01a09943-1025-73e1-9658-fa1a190edc93`; copied to `apex-taskbar-icon.png` unchanged.

## Final edit prompt

Edit ONLY the APEX CIRCUIT logo in the first of the three attached images. The other two images (circular telemetry HUD, 150 brake marker) are NOT targets and must NOT appear. Prepare a production brand texture. Preserve the logo's precise letter geometry, slant, spacing, elongated angular APEX lettering, silver-white letter treatment, red diagonal lower-left stroke on the X and its dark red terminal, and the widely spaced CIRCUIT subtitle. Make the near-white CIRCUIT subtitle legible white. Replace the white/transparent surround with a solid deep charcoal navy #101820 background; retain the white/silver APEX and red X. Do not redesign or add symbols, text, borders, texture, glow, shadows, or mockup objects. Tightly frame the whole supplied mark and subtitle in a clean wide 3:1 image with about 7% safe padding. Return only one flat logo-on-dark texture suitable for direct use in a game menu, car decal, bridge sign and icon.

Original output: `exec-53e9c16b-71bf-4ba4-bae1-3d833fd33487.png`, generated-image session `01a0979b-02f3-7b52-b31b-a1190cedede5`. Copied into this directory unchanged. No image-generation CLI or API key was used.
