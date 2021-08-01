if(DEFINED LIBAV_INCLUDE
   AND DEFINED LIBAV_LIB
   OR NOT PkgConfig_FOUND)

    find_path(AVCODEC_INCLUDE libavcodec/avcodec.h REQUIRED HINTS ${LIBAV_INCLUDE})
    find_library(AVCODEC_LIB avcodec REQUIRED HINTS ${LIBAV_LIB})

    find_path(AVFORMAT_INCLUDE libavformat/avformat.h REQUIRED HINTS ${LIBAV_INCLUDE})
    find_library(AVFORMAT_LIB avformat REQUIRED HINTS ${LIBAV_LIB})

    find_path(AVUTIL_INCLUDE libavutil/avutil.h REQUIRED HINTS ${LIBAV_INCLUDE})
    find_library(AVUTIL_LIB avutil REQUIRED HINTS ${LIBAV_LIB})

    find_path(SWSCALE_INCLUDE libswscale/swscale.h REQUIRED HINTS ${LIBAV_INCLUDE})
    find_library(SWSCALE_LIB swscale REQUIDED HINTS ${LIBAV_LIB})

    setup_library(
        PF-Utils "fmt;GSL" "${AVFORMAT_LIB};${SWSCALE_LIB};${AVUTIL_LIB};${AVCODEC_LIB};gcem;glm"
        "" "${AVCODEC_INCLUDE};${AVFORMAT_INCLUDE};${AVUTIL_INCLUDE};${SWSCALE_INCLUDE}")
else()
    setup_library(
        PF-Utils "fmt;GSL"
        "PkgConfig::AVFORMAT;PkgConfig::SWSCALE;PkgConfig::AVUTIL;PkgConfig::AVCODEC;gcem;glm")
endif()
