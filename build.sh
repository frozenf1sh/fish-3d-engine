#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

# 默认配置
BUILD_TYPE="Debug"
CLEAN_BUILD=false
RUN_AFTER_BUILD=false

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -c, --clean         Clean build directory before building"
    echo "  -r, --release       Build in Release mode (default: Debug)"
    echo "  -d, --debug         Build in Debug mode (default)"
    echo "  --run               Run the executable after building"
    echo ""
    echo "Example:"
    echo "  $0 --clean --release --run"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            print_usage
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --run)
            RUN_AFTER_BUILD=true
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            print_usage
            exit 1
            ;;
    esac
done

# 检查 vcpkg
if [ -z "$VCPKG_ROOT" ]; then
    if [ -d "$HOME/vcpkg" ]; then
        VCPKG_ROOT="$HOME/vcpkg"
        echo -e "${YELLOW}VCPKG_ROOT not set, using default: $VCPKG_ROOT${NC}"
    elif [ -d "/opt/vcpkg" ]; then
        VCPKG_ROOT="/opt/vcpkg"
        echo -e "${YELLOW}VCPKG_ROOT not set, using default: $VCPKG_ROOT${NC}"
    else
        echo -e "${RED}Error: VCPKG_ROOT environment variable is not set!${NC}"
        echo "Please set VCPKG_ROOT to your vcpkg installation directory."
        exit 1
    fi
fi

VCPKG_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
if [ ! -f "$VCPKG_TOOLCHAIN" ]; then
    echo -e "${RED}Error: vcpkg toolchain file not found at $VCPKG_TOOLCHAIN${NC}"
    exit 1
fi

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Fish Engine Build Script${NC}"
echo -e "${BLUE}========================================${NC}"
echo "Build Type: $BUILD_TYPE"
echo "Clean Build: $CLEAN_BUILD"
echo "VCPKG_ROOT: $VCPKG_ROOT"
echo -e "${BLUE}========================================${NC}"

# 清理构建目录
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf "$BUILD_DIR"
fi

# 创建构建目录
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# 配置 CMake
echo -e "${GREEN}Configuring CMake...${NC}"
cmake -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_TOOLCHAIN" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    "$PROJECT_ROOT"

# 编译
echo -e "${GREEN}Building...${NC}"
ninja

# 将 compile_commands.json 链接到项目根目录
if [ -f "$BUILD_DIR/compile_commands.json" ]; then
    ln -sf "$BUILD_DIR/compile_commands.json" "$PROJECT_ROOT/compile_commands.json"
fi

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  Build succeeded!${NC}"
echo -e "${GREEN}========================================${NC}"
echo "Executable: $BUILD_DIR/fish-engine"

# 运行
if [ "$RUN_AFTER_BUILD" = true ]; then
    echo -e "${GREEN}Running...${NC}"
    echo ""
    "$BUILD_DIR/fish-engine"
fi

exit 0
