#!/bin/bash
export NDK=../android-ndk-r20b
# TOOLCHAIN变量指向ndk中的交叉编译gcc所在的目录
export TOOLCHAIN=$NDK/toolchains/llvm/prebuilt/linux-x86_64/bin
export SYSROOT=$NDK/toolchains/llvm/prebuilt/linux-x86_64/sysroot

export API=21
init_armv7a(){
    export ARCH=arm
    #   min api is 16
    # API=16
    export CPU=armv7-a
    export PLATFORM=armv7a
    export ANDROID=androideabi
    export CFLAGS="-mfloat-abi=softfp -march=$CPU"
    export LDFLAGS="-Wl,--fix-cortex-a8"
}


init_aarch64(){
    export ARCH=aarch64
    #   min api is 21
    # API=21
    export CPU=armv8-a
    export PLATFORM=aarch64
    export ANDROID=android
    export CFLAGS=""
    export LDFLAGS=""
}

# FLAGS="-isystem $NDK/sysroot/usr/include/$PLATFORM-linux-$ANDROID -D__ANDROID_API__=$API -g -DANDROID -ffunction-sections -funwind-tables -fstack-protector-strong -no-canonical-prefixes -march=$CPU -mfloat-abi=softfp -mfpu=vfpv3-d16 -mthumb -Wa,--noexecstack -Wformat -Werror=format-security -std=c++11  -O0 -fPIC"
# INCLUDES="-isystem $NDK/sources/cxx-stl/llvm-libc++/include -isystem $NDK/sources/android/support/include -isystem $NDK/sources/cxx-stl/llvm-libc++abi/include"


CROSS_PREFIX=$TOOLCHAIN/$ARCH-linux-$ANDROID-
CC=$TOOLCHAIN/$PLATFORM-linux-$ANDROID$API-clang
CXX=$TOOLCHAIN/$PLATFORM-linux-$ANDROID$API-clang++
NM=$TOOLCHAIN/$ARCH-linux-$ANDROID-nm
STRIP=$TOOLCHAIN/$ARCH-linux-$ANDROID-strip

OUTPUT=./build_android/$ARCH


echo 'start build'

build(){
    ./configure \
    --prefix=$OUTPUT \
    --cross-prefix=$CROSS_PREFIX \
    --target-os=android \
    --arch=$ARCH \
    --cpu=$CPU \
    --cc=$CC \
    --cxx=$CXX \
    --nm=$NM \
    --strip=$STRIP \
    --sysroot=$SYSROOT \
    --extra-cflags="$CFLAGS" \
    --extra-ldflags="$LDFLAGS" \
    --extra-ldexeflags=-pie \
    --disable-static \
    --disable-ffprobe \
    --disable-ffplay \
    --disable-ffmpeg \
    --disable-debug \
    --disable-avdevice \
    --enable-shared \
    --enable-avfilter \
    --enable-decoders \
    --enable-cross-compile \
    --enable-runtime-cpudetect \
    $ADDITIONAL_CONFIGURE_FLAG
}

# --disable-avdevice    这是摄像头和音频采集器的固件, 在 android 中无法调用,关闭它

createDir(){
    echo 'create dir'
    build_dir=$OUTPUT
    if [ ! -d "$build_dir" ]; then
        mkdir -p $build_dir
    fi
}


changeConfig(){
    # 如果编译出来过后的.so文件后面会有一串数字，则使用它开启
    s1="SLIBNAME_WITH_MAJOR='\$(SLIBNAME).\$(LIBMAJOR)'"
    s1_r="SLIBNAME_WITH_MAJOR='\$(SLIBPREF)\$(FULLNAME)-\$(LIBMAJOR)\$(SLIBSUF)'"
    sed -i s/$s1/$s1_r/g ./configure
    
    s1="LIB_INSTALL_EXTRA_CMD='\$\$(RANLIB)\"\$(LIBDIR)/\$(LIBNAME)\"'"
    s1_r="LIB_INSTALL_EXTRA_CMD='\$\$(RANLIB)\"\$(LIBDIR)/\$(LIBNAME)\"'"
    sed -i s/$s1/$s1_r/g ./configure

    s1="SLIB_INSTALL_NAME='\$(SLIBNAME_WITH_VERSION)'"
    s1_r="SLIB_INSTALL_NAME='\$(SLIBNAME_WITH_MAJOR)'"
    sed -i s/$s1/$s1_r/g ./configure

    s1="SLIB_INSTALL_LINKS='\$(SLIBNAME_WITH_MAJOR)\$(SLIBNAME)'"
    s1_r="SLIB_INSTALL_LINKS='\$(SLIBNAME)'"
    sed -i s/$s1/$s1_r/g ./configure
}

init_armv7a
# init_aarch64
# changeConfig
createDir
# 第一次如果是报各种文件 notfound, 则需要生成config.mak文件, 运行一次下面命令即可
# ./configure --disable-x86asm
build

make clean
make -j8
make install

# make clean && make -j8 && make install


# 如果编译出来过后的.so文件后面会有一串数字，
# 无法使用，所以得修改configure的命名规则即可。
# 将该文件中的如下四行：
# SLIBNAME_WITH_MAJOR='$(SLIBNAME).$(LIBMAJOR)'
# LIB_INSTALL_EXTRA_CMD='$$(RANLIB)"$(LIBDIR)/$(LIBNAME)"'
# SLIB_INSTALL_NAME='$(SLIBNAME_WITH_VERSION)'
# SLIB_INSTALL_LINKS='$(SLIBNAME_WITH_MAJOR)$(SLIBNAME)'
# 替换为：
# SLIBNAME_WITH_MAJOR='$(SLIBPREF)$(FULLNAME)-$(LIBMAJOR)$(SLIBSUF)'
# LIB_INSTALL_EXTRA_CMD='$$(RANLIB)"$(LIBDIR)/$(LIBNAME)"'
# SLIB_INSTALL_NAME='$(SLIBNAME_WITH_MAJOR)'
# SLIB_INSTALL_LINKS='$(SLIBNAME)'

# sed -i '' 's/HAVE_LRINT 0/HAVE_LRINT 1/g' config.h
# sed -i '' 's/HAVE_LRINTF 0/HAVE_LRINTF 1/g' config.h
# sed -i '' 's/HAVE_ROUND 0/HAVE_ROUND 1/g' config.h
# sed -i '' 's/HAVE_ROUNDF 0/HAVE_ROUNDF 1/g' config.h
# sed -i '' 's/HAVE_TRUNC 0/HAVE_TRUNC 1/g' config.h
# sed -i '' 's/HAVE_TRUNCF 0/HAVE_TRUNCF 1/g' config.h
# sed -i '' 's/HAVE_CBRT 0/HAVE_CBRT 1/g' config.h
# sed -i '' 's/HAVE_RINT 0/HAVE_RINT 1/g' config.h
