cat << EOF | "$STAGING_DIR_HOST/bin/g++" -c -x c++ -o /dev/null - >/dev/null 2>&1
#if __clang__
  #if __clang_major__ < $3
    #error "clang too old"
  #endif
#else
  #if __GNUC__ < $1 || (__GNUC__ == $1 && (__GNUC_MINOR__ < $2))
    #error "gcc too old"
  #endif
#endif
EOF
[ $? -eq 0 ] && echo y || echo n
