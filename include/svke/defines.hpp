#if defined(NDEBUG) || defined(_DEBUG)
#  define SVKE_DEBUG
#  define SVKE_VERBOSE_PRESENT_MODE
#  define SVKE_VERBOSE_DEVICE_INFO
#  define SVKE_VERBOSE_VALIDATION_LAYER
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  ifdef _WIN64
#    define SVKE_TARGET_WIN64
#  else
#    define SVKE_TARGET_WIN32
#  endif
#elif __APPLE__
#  include <TargetConditionals.h>
#  if defined(TARGET_IPHONE_SIMULATOR) || defined(TARGET_OS_IPHONE)
#    define SVKE_TARGET_IOS
#  elif TARGET_OS_MAC
#    define SVKE_TARGET_MAC
#  else
#    error "Unknown Apple platform"
#  endif
#elif __linux__
#  define SVKE_TARGET_LINUX
#elif __unix__
#  define SVKE_TARGET_UNIX
#elif defined(_POSIX_VERSION)
#  define SVKE_TARGET_POSIX
#else
#  error "Unknown compiler"
#endif

#define UNUSED(x) (void(x))
