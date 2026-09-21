#!/bin/zsh
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UE_ROOT="${UE_ROOT:-/Users/rahul/UE_5.8}"

"$UE_ROOT/Engine/Binaries/Mac/UnrealEditor" "$PROJECT_ROOT/ApexCircuit.uproject" \
  -game -ResX=2560 -ResY=1440 -windowed \
  -ExecCmds="sg.ViewDistanceQuality 4,sg.ShadowQuality 4,sg.GlobalIlluminationQuality 4,sg.ReflectionQuality 4,sg.PostProcessQuality 4,sg.EffectsQuality 4"
