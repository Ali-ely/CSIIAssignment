#!/usr/bin/env bash
# GCC validation for Linux and Ubuntu/WSL. Outputs stay under ignored build/.
set -euo pipefail

project_root="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
flags=(-std=c++17 -Wall -Wextra -Werror -pedantic -g)
build_dir="$project_root/build/gcc"
sanitize=false

if [[ $# -gt 1 || ($# -eq 1 && $1 != --sanitize) ]]; then
    echo "Usage: bash build.sh [--sanitize]" >&2
    exit 2
fi
if [[ $# -eq 1 ]]; then
    sanitize=true
    build_dir="$project_root/build/gcc-sanitized"
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi

if ! command -v g++ >/dev/null 2>&1; then
    echo "GCC g++ is required. On Ubuntu, install the g++ package." >&2
    exit 1
fi

mkdir -p -- "$build_dir"
sources=("$project_root"/*.cpp)
core_sources=()
for source in "${sources[@]}"; do
    if [[ ${source##*/} != main.cpp ]]; then
        core_sources+=("$source")
    fi
done

g++ --version
g++ "${flags[@]}" "${sources[@]}" -o "$build_dir/simworld"
g++ "${flags[@]}" -I "$project_root" "${core_sources[@]}" \
    "$project_root/tests/SimWorldTests.cpp" -o "$build_dir/simworld_tests"
g++ "${flags[@]}" -I "$project_root" "${core_sources[@]}" \
    "$project_root/tests/EntityTests.cpp" -o "$build_dir/entity_tests"

for test_program in "$build_dir/entity_tests" "$build_dir/simworld_tests"; do
    if [[ $sanitize == true ]]; then
        ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1 \
            "$test_program"
    else
        "$test_program"
    fi
done

echo "Built application: $build_dir/simworld"
