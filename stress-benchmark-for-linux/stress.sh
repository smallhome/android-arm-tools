function usage() {
cat <<EOF
Invoke ". stress.sh" from your shell to add the following functions to your environment:
- compile:	Changes directory to the top of the tree.
- clean:	Makes from the top of the tree.
- runtest:	Builds all of the modules in the current directory.
- check:	Builds all of the modules in the supplied directories.

Look at the source to view more functions. The complete list is:
EOF
    T=$(gettop)
    local A
    A=""
    for i in `cat $T/stress.sh | sed -n "/^function /s/function \([a-z_]*\).*/\1/p" | sort`; do
      A="$A $i"
    done
    echo $A
}

function gettop() {
    local TOPFILE=stress.sh
    if [ -n "$TOP" -a -f "$TOP/$TOPFILE" ] ; then
        echo $TOP
    else
        if [ -f $TOPFILE ] ; then
            # The following circumlocution (repeated below as well) ensures
            # that we record the true directory name and not one that is
            # faked up with symlink names.
            PWD= /bin/pwd
        else
            # We redirect cd to /dev/null in case it's aliased to
            # a command that prints something as a side-effect
            # (like pushd)
            local HERE=$PWD
            T=
            while [ \( ! \( -f $TOPFILE \) \) -a \( $PWD != "/" \) ]; do
                cd .. > /dev/null
                T=`PWD= /bin/pwd`
            done
            cd $HERE > /dev/null
            if [ -f "$T/$TOPFILE" ]; then
                echo $T
            fi
        fi
    fi
}

function croot() {
    T=$(gettop)
    if [ "$T" ]; then
        cd $(gettop)
    else
        echo "Couldn't locate the top of the tree.  Try setting TOP."
    fi
}

function generate_arch_array() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    local num=0
    echo "========================"
    echo "which arch do you want?"
    for arch in `ls $T/stress`;do
        arch_array[$num]=$arch
        echo $arch
        num=$(($num+1))
    done
    echo "========================"
}

function set_build_arch() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    local ARCH
    generate_arch_array
    read ARCH
    case $ARCH in
        x86)
            export STRESS_ARCH=x86
            ;;
        arm)
            export STRESS_ARCH=arm
            ;;
        mips)
            export STRESS_ARCH=mips
            ;;
        *)
            echo "Can't find toolchain for unknown architecture: $ARCH"
            ;;
    esac
}

function set_build_env() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    set_build_arch

    export TOPDIR=${T}
    export TMPBASE="/tmp"
    export TMP="${TMPBASE}/ltp-$$"

    export RESULTDIR="${T}/results"
    export INSTALLDIR="${T}/install"
    export TMPDIR=${TMP}
}

function compile() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    cd $T/stress/${STRESS_ARCH}
    make -j`cat /proc/cpuinfo |grep processor |wc -l`;make install
    cd $T
}

function cleanup() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    cd $T/stress/${STRESS_ARCH}
    make clean
    cd $T

    rm -rf ${INSTALLDIR}/*
    rm -rf ${TMPDIR}
}

function runtest() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi

    mkdir -p $TMPDIR

    export PATH=$PATH:$INSTALLDIR/bin:$INSTALLDIR/usr/bin:$INSTALLDIR/testcases/bin

#    [ -d $INSTALLDIR/bin ] ||
#    {
#        echo "FATAL: Test suite not installed correctly"
#        echo "INFO: as root user type 'make ; make install'"
#        exit 1
#    }
#    [ -d $INSTALLDIR/usr/bin ] ||
#    {
#        echo "FATAL: Test suite not installed correctly"
#        echo "INFO: as root user type 'make ; make install'"
#        exit 1
#    }
#    [ -d $INSTALLDIR/testcases/bin ] ||
#    {
#        echo "FATAL: Test suite not installed correctly"
#        echo "INFO: as root user type 'make ; make install'"
#        exit 1
#    }
#    [ -e $INSTALLDIR/../runtest/runstress ] ||
#    {
#        echo "FATAL: Test suite application 'run' not found"
#        exit 1
#    }
#    [ -e $INSTALLDIR/../runtest/runtests ] ||
#    {
#        echo "FATAL: Test suite testcase list 'runtest' not found"
#        exit 1
#    }

    $INSTALLDIR/../runtest/runstress -p -q -l $TOPDIR/results/result-log.$$ -o $TOPDIR/results/result-output.$$ -d ${TMPDIR} -f $INSTALLDIR/../runtest/runtests
}

function check() {
    T=$(gettop)
    if [ ! "$T" ]; then
        echo "Couldn't locate the top of the tree.  Try setting TOP." >&2
        return
    fi
    
    echo "###############################################################"
    echo "                              PASS                             "
    echo "###############################################################"
    ls -r $T/results/result-log*|head -1 |xargs cat |grep PASS
    echo "###############################################################"
    echo "                              FAIL                             "
    echo "###############################################################"

    ls -r $T/results/result-log*|head -1 |xargs cat |grep FAIL
}

trap "cleanup" 0 
usage
set_build_env
