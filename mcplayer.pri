!isEmpty(MCPLAYER_PRI_INCLUDED):error("mcplayer.pri already included")
MCPLAYER_PRI_INCLUDED = 1

MCPLAYER_VERSION = 0.0.1
MCPLAYER_COMPAT_VERSION = 0.0.1
MCPLAYER_DISPLAY_VERSION = 0.0.1
MCPLAYER_COPYRIGHT_YEAR = 2019
MCPLAYER_COMPANY_NAME = ""
VERSION = $$MCPLAYER_VERSION

isEmpty(APP_DISPLAY_NAME): APP_DISPLAY_NAME = McPlayer
isEmpty(APP_ID):           APP_ID = mcplayer
isEmpty(APP_CASE_ID):      APP_CASE_ID = McPlayer

defineReplace(qtLibraryTargetName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:RET = $$member(LIBRARY_NAME, 0)_debug
              else:win32:RET = $$member(LIBRARY_NAME, 0)d
      }
   }
   isEmpty(RET):RET = $$LIBRARY_NAME
   return($$RET)
}

defineReplace(qtLibraryName) {
   RET = $$qtLibraryTargetName($$1)
   win32 {
      VERSION_LIST = $$split(MCPLAYER_VERSION, .)
      RET = $$RET$$first(VERSION_LIST)
   }
   return($$RET)
}

# 只用于自定义编译器的文件拷贝
defineReplace(stripSrcDir) {
    return($$relative_path($$absolute_path($$1, $$OUT_PWD), $$_PRO_FILE_PWD_))
}

isEmpty(MCPLAYER_LIBRARY_BASENAME) {
    MCPLAYER_LIBRARY_BASENAME = lib
}

DEFINES += MCPLAYER_LIBRARY_BASENAME=\\\"$$MCPLAYER_LIBRARY_BASENAME\\\"

MCPLAYER_SOURCE_TREE = $$PWD
isEmpty(MCPLAYER_BUILD_TREE) {
    sub_dir = $$_PRO_FILE_PWD_
    sub_dir ~= s,^$$re_escape($$PWD),,
    MCPLAYER_BUILD_TREE = $$clean_path($$OUT_PWD)
    MCPLAYER_BUILD_TREE ~= s,$$re_escape($$sub_dir)$,,
}
message(mcplayer $$MCPLAYER_SOURCE_TREE $$MCPLAYER_BUILD_TREE)

APP_PATH = $$MCPLAYER_SOURCE_TREE/bin

osx {
    APP_TARGET = "$$APP_DISPLAY_NAME"

    # check if BUILD_TREE is actually an existing McPlayer.app,
    # for building against a binary package
    exists($$MCPLAYER_BUILD_TREE/Contents/MacOS/$$APP_TARGET): APP_BUNDLE = $$MCPLAYER_BUILD_TREE
    else: APP_BUNDLE = $$APP_PATH/$${APP_TARGET}.app

    # set output path if not set manually
    isEmpty(APP_OUTPUT_PATH): APP_OUTPUT_PATH = $$APP_BUNDLE/Contents

    MCPLAYER_LIBRARY_PATH = $$APP_OUTPUT_PATH/Frameworks
    MCPLAYER_PLUGIN_PATH  = $$APP_OUTPUT_PATH/PlugIns
    MCPLAYER_LIBEXEC_PATH = $$APP_OUTPUT_PATH/Resources
    MCPLAYER_DATA_PATH    = $$APP_OUTPUT_PATH/Resources
    MCPLAYER_DOC_PATH     = $$APP_DATA_PATH/doc
    MCPLAYER_BIN_PATH     = $$APP_OUTPUT_PATH/MacOS

    contains(QT_CONFIG, ppc):CONFIG += ppc x86
    copydata = 1

    INSTALL_LIBRARY_PATH = $$APP_PREFIX/$${APP_TARGET}.app/Contents/Frameworks
    INSTALL_PLUGIN_PATH  = $$APP_PREFIX/$${APP_TARGET}.app/Contents/PlugIns
    INSTALL_LIBEXEC_PATH = $$APP_PREFIX/$${APP_TARGET}.app/Contents/Resources
    INSTALL_DATA_PATH    = $$APP_PREFIX/$${APP_TARGET}.app/Contents/Resources
    INSTALL_DOC_PATH     = $$INSTALL_DATA_PATH/doc
    INSTALL_BIN_PATH     = $$APP_PREFIX/$${APP_TARGET}.app/Contents/MacOS
    INSTALL_APP_PATH     = $$APP_PREFIX/
} else {
    APP_TARGET = $$APP_ID
    
    # 如果没有手动设置目标输出路径
    isEmpty(APP_OUTPUT_PATH): APP_OUTPUT_PATH = $$MCPLAYER_BUILD_TREE

    MCPLAYER_LIBRARY_PATH = $$APP_OUTPUT_PATH/$$MCPLAYER_LIBRARY_BASENAME/mcplayer
    MCPLAYER_PLUGIN_PATH  = $$MCPLAYER_LIBRARY_PATH/plugins
    MCPLAYER_DATA_PATH    = $$APP_OUTPUT_PATH/share/mcplayer
    MCPLAYER_DOC_PATH     = $$APP_OUTPUT_PATH/share/doc/mcplayer
    MCPLAYER_BIN_PATH     = $$APP_OUTPUT_PATH/bin

    win32: \
        APP_LIBEXEC_PATH = $$APP_OUTPUT_PATH/bin
    else: \
        APP_LIBEXEC_PATH = $$APP_OUTPUT_PATH/libexec/mcplayer

    INSTALL_LIBRARY_PATH = $$APP_PREFIX/$$MCPLAYER_LIBRARY_BASENAME/mcplayer
    INSTALL_PLUGIN_PATH  = $$INSTALL_LIBRARY_PATH/plugins
    win32: \
        INSTALL_LIBEXEC_PATH = $$APP_PREFIX/bin
    else: \
        INSTALL_LIBEXEC_PATH = $$APP_PREFIX/libexec/mcplayer
    INSTALL_DATA_PATH    = $$APP_PREFIX/share/mcplayer
    INSTALL_DOC_PATH     = $$APP_PREFIX/share/doc/mcplayer
    INSTALL_BIN_PATH     = $$APP_PREFIX/bin
    INSTALL_APP_PATH     = $$APP_PREFIX/bin
}

win32:exists($$MCPLAYER_SOURCE_TREE/lib/mcplayer) {
    # for .lib in case of binary package with dev package
    LIBS *= -L$$MCPLAYER_SOURCE_TREE/lib/mcplayer
    LIBS *= -L$$MCPLAYER_SOURCE_TREE/lib/mcplayer/plugins
}

INCLUDEPATH += $$MCPLAYER_SOURCE_TREE/src
LIBS *= -L$$MCPLAYER_LIBRARY_PATH

message(LIBS $$LIBS)
