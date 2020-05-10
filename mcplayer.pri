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
