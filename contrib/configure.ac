#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT(dovecot-mbc, 1.0, lukas.mika@web.de)
AC_CONFIG_SRCDIR([src/mbc-plugin.c])
#AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(void *)
AC_CHECK_SIZEOF(long long)

AC_DEFUN([AC_TYPEOF], [
  dnl * first check if we can get the size with redefining typedefs

  order="$2"
  if test "$2" = ""; then
    order="int long long-long"
  fi

  result=""
  visible="unknown"
  AC_MSG_CHECKING([type of $1])
  AC_CACHE_VAL(i_cv_typeof_$1,[
  if test "x$ac_cv_c_compiler_gnu" = "xyes"; then
    dnl * try with printf() + -Werror
    old_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS -Werror"

    for type in $order; do
      case "$type" in
        int)
          fmt="%d"
          ;;
        unsigned-int)
          fmt="%u"
          ;;
        long)
          fmt="%ld"
          ;;
        unsigned-long)
          fmt="%lu"
          ;;
        long-long)
          fmt="%lld"
          ;;
        unsigned-long-long)
          fmt="%llu"
          ;;
        *)
          fmt=""
          ;;
      esac

      if test "$fmt" != ""; then
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
          #include <sys/types.h>
          #include <stdio.h>
        ]], [[
          printf("$fmt", ($1)0);
        ]])],[
          if test "$result" != ""; then
            dnl * warning check isn't working
            result=""
            visible="unknown"
            break
          fi
          result="`echo $type|sed 's/-/ /g'`"
          visible="$result"
        ],[])
      fi
    done
    CFLAGS="$old_CFLAGS"
  fi

  if test "$result" = ""; then
    for type in $order; do
      type="`echo $type|sed 's/-/ /g'`"
      AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
        #include <sys/types.h>
        typedef $type $1;
      ]], [[]])],[
        if test "$result" != ""; then
          dnl * compiler allows redefining to anything
          result=""
          visible="unknown"
          break
        fi
        result="$type"
        visible="$type"
      ],[])
    done
  fi

  if test "$result" = ""; then
    dnl * check with sizes

    dnl * older autoconfs don't include sys/types.h, so do it manually
    AC_RUN_IFELSE([AC_LANG_SOURCE([[
      #include <stdio.h>
      #include <sys/types.h>
      int main() {
        FILE *f=fopen("conftestval", "w");
        if (!f) exit(1);
        fprintf(f, "%d\n", sizeof($1));
        exit(0);
      }
    ]])],[
      size=`cat conftestval`
      rm -f conftestval

      for type in $order; do
        actype="ac_cv_sizeof_`echo $type|sed 's/-/_/g'`"
        if test "$size" = "`eval echo \\$$actype`"; then
          result="`echo $type|sed 's/-/ /g'`"
          visible="`expr $size \* 8`bit (using $result)"
          break
        fi
      done
      if test "$result" = ""; then
        result=unknown
        visible="`expr $size \* 8`bit (unknown type)"
      fi
    ],[],[])
  fi
  i_cv_typeof_$1=$result/$visible
  ])

  typeof_$1=`echo $i_cv_typeof_$1 | sed s,/.*$,,`
  visible=`echo $i_cv_typeof_$1 | sed s,^.*/,,`
  AC_MSG_RESULT($visible)
])

AC_CHECK_TYPE(uintmax_t, [ AC_DEFINE(HAVE_UINTMAX_T,, Define if you have uintmax_t already) ],[])
AC_CHECK_TYPE(uint_fast32_t, [ AC_DEFINE(HAVE_UINT_FAST32_T,, Define if you have uint_fast32_t already) ],[])
AC_CHECK_TYPE(socklen_t, [ AC_DEFINE(HAVE_SOCKLEN_T,, Define if you have socklen_t already) ],[])

AC_CHECK_TYPE(uoff_t, [
  have_uoff_t=yes
  AC_DEFINE(HAVE_UOFF_T,, Define if you have a native uoff_t type)
], [
  have_uoff_t=no
])

AC_TYPEOF(off_t, long int long-long)
case "$typeof_off_t" in
  int)
    offt_max=INT_MAX
    uofft_fmt="u"
    if test "$have_uoff_t" != "yes"; then
      AC_DEFINE(UOFF_T_INT,, Define if off_t is int)
    fi
    offt_bits=`expr 8 \* $ac_cv_sizeof_int`
    ;;
  long)
    offt_max=LONG_MAX
    uofft_fmt="lu"
    if test "$have_uoff_t" != "yes"; then
      AC_DEFINE(UOFF_T_LONG,, Define if off_t is long)
    fi
    offt_bits=`expr 8 \* $ac_cv_sizeof_long`
    ;;
  "long long")
    offt_max=LLONG_MAX
    uofft_fmt="llu"
    if test "$have_uoff_t" != "yes"; then
      AC_DEFINE(UOFF_T_LONG_LONG,, Define if off_t is long long)
    fi
    offt_bits=`expr 8 \* $ac_cv_sizeof_long_long`
    offt_bits=`expr 8 \* $ac_cv_sizeof_long_long`
    ;;
  *)
    AC_MSG_ERROR([Unsupported off_t type])
    ;;
esac

AC_CACHE_CHECK([for typeof],i_cv_have_typeof,[
  AC_TRY_COMPILE([
  ], [
    int foo;
    typeof(foo) bar;
  ], [
    i_cv_have_typeof=yes
  ], [
    i_cv_have_typeof=no
  ])
])
if test $i_cv_have_typeof = yes; then
  AC_DEFINE(HAVE_TYPEOF,, Define if you have typeof())
fi

AC_MSG_CHECKING([for struct iovec])
AC_CACHE_VAL(i_cv_struct_iovec,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>]],
[[struct iovec *iovec;]])],
[i_cv_struct_iovec=yes],
[i_cv_struct_iovec=no])])

if test $i_cv_struct_iovec = yes; then
        AC_DEFINE(HAVE_STRUCT_IOVEC,, Define if you have struct iovec)
fi
AC_MSG_RESULT($i_cv_struct_iovec)

# Checks for library functions.
AC_CHECK_FUNCS([setenv])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
