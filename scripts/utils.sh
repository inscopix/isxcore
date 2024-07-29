condmkdir()
{
    if [ ! -d "$1" ]; then
        mkdir $1
    fi
}

condmkandchdir()
{
    condmkdir $1
    cd $1
}

checkExitCode()
{
    exitCode=$?
    if [ $exitCode -ne 0 ]; then
        echo "$1 failed"
        exit "$exitCode"
    fi
}

cleanBuild()
{
    rm -rf $BUILD_DIR_ROOT
}

isMac()
{
    [ "$(uname)" == "Darwin" ]
}

isLinux()
{
    [ "$(expr substr $(uname -s) 1 5)" == "Linux" ]
}

isUbuntu()
{
    [ "$(expr substr $(awk -F= '/^NAME/{print $2}' /etc/os-release) 2 6)" == "Ubuntu" ]
}

isWin()
{
    [ "$(expr substr $(uname) 1 5)" == "MINGW" ]
}

isArm()
{
    [ "$(arch)" == "aarch64" ]
}

absPath(){
    if [[ -d "$1" ]]; then
        cd "$1"
        echo "$(pwd -P)"
    else
        cd "$(dirname "$1")"
        echo "$(pwd -P)/$(basename "$1")"
    fi
}
