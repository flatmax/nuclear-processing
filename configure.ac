# Copyright (c) 2017 The nuclear processing Authors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#    * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#    * Neither the name of Flatmax Pty Ltd nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
m4_define([neutron_bomb_processing_current_version], [0]) # ++ if API added (age++), removed, or changed (age=0)
m4_define([neutron_bomb_processing_revision_version], [0]) # ++ if API same but src code change
m4_define([neutron_bomb_processing_age_version], [0]) # ++ if API added since last release, if API removed or changed then age=0
m4_define([neutron_bomb_processing_version], [neutron_bomb_processing_current_version.neutron_bomb_processing_revision_version.neutron_bomb_processing_age_version])

AC_INIT( neutron_bomb_processing, [neutron_bomb_processing_version], [ https://github.com/flatmax/neutron_bomb_processing/issues ])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([include/Fission.H])
AC_CONFIG_SRCDIR([gtkiostream/include/Futex.H])
AC_CONFIG_SRCDIR([gtkiostream/include/Thread.H])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AC_PREFIX_DEFAULT(/usr)

LT_INIT
AC_SUBST(LT_RELEASE, [neutron_bomb_processing_version])
AC_SUBST(LT_CURRENT, [neutron_bomb_processing_current_version:neutron_bomb_processing_revision_version:neutron_bomb_processing_age_version])

AC_PROG_CXX
#AC_PROG_INSTALL

# doxygen support
AC_ARG_ENABLE(doc, AS_HELP_STRING([--disable-doc],
        [Disable API documentation generation (default: auto)]),
        [DOC=$enableval], [DOC=auto])
if test "x$DOC" = xauto ; then
AC_CHECK_PROGS([DOXYGEN], [doxygen])
if test -z "$DOXYGEN";
   then AC_MSG_WARN([Doxygen not found - continuing without Doxygen support])
fi
fi
# Checks for documentation
AM_CONDITIONAL([HAVE_DOXYGEN],[test -n "$DOXYGEN"])
AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([api/Doxyfile])])

# eigen3
PKG_CHECK_MODULES(EIGEN, eigen3 >= 3.0.0, HAVE_EIGEN="yes", HAVE_EIGEN="no")
AC_SUBST(EIGEN_CFLAGS)
AC_SUBST(EIGEN_LIBS)
AM_CONDITIONAL(HAVE_EIGEN, test x$HAVE_EIGEN = xyes)
if test "x$HAVE_EIGEN" == xyes ; then
    AC_DEFINE(HAVE_EIGEN, [], [whether to build in Eigen support])
fi

# ALSA
PKG_CHECK_MODULES(ALSA, alsa, HAVE_ALSA="yes", HAVE_ALSA="no")
AC_SUBST(ALSA_CFLAGS)
AC_SUBST(ALSA_LIBS)
AM_CONDITIONAL(HAVE_ALSA, test x$HAVE_ALSA = xyes)
if test "x$HAVE_ALSA" == xyes ; then
    AC_DEFINE(HAVE_ALSA, [], [whether to build in Alsa support])
fi

AC_C_INLINE
AC_FUNC_ERROR_AT_LINE

AC_CONFIG_FILES([Makefile api/Makefile ALSAExample/Makefile])
AC_OUTPUT

AM_COND_IF(HAVE_EIGEN, [AC_MSG_NOTICE([Eigen   ....................................... Present])], [AC_MSG_NOTICE([Eigen   ......................................... Not present
Eigen tests and applications will not be built.])])

AM_COND_IF(HAVE_ALSA, [AC_MSG_NOTICE([Alsa   ....................................... Present])], [AC_MSG_NOTICE([Alsa   ......................................... Not present
Alsa tests and applications will not be built.])])
