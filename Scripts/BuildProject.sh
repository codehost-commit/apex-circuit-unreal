#!/bin/zsh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
UE_ROOT="${UE_ROOT:-/Users/rahul/UE_5.8}"
"$UE_ROOT/Engine/Build/BatchFiles/Mac/Build.sh" ApexCircuitEditor Mac Development -Project="$ROOT/ApexCircuit.uproject" -WaitMutex
