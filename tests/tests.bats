#!/usr/bin/env bats

# Source and mount are
# cleaned up in setup
SOURCE="$PWD/source"
MOUNT="$PWD/mount"
SOURCE_1="$PWD/source_1"

@test "test" {
    load setup
    # careful, test is a bash internal
    bash -c "test -d ${MOUNT}/subdir"
    bash -c "test -f ${MOUNT}/file_in_source.txt"
    bash -c "test -f ${MOUNT}/subdir/file_in_subdir.txt"
}

@test "ls" {
    load setup
    for x in "${MOUNT} ${MOUNT}/file_in_source.txt ${MOUNT}/subdir ${MOUNT}/subdir/file_in_subdir.txt"
    do
      ls $x
    done
    a=$(ls ${MOUNT}) # ls dir content
    [[ $a == *"file_in_source.txt"* ]] # passthrough init message is in a
    load unset

    # ls all mounts
    echo $a
}

@test "mkdir" {
    load setup
    mkdir ${MOUNT}/dir
    mkdir -p ${MOUNT}/subdir/a/b/c
    load unset
    test -d ${SOURCE}/dir
    test -d ${SOURCE}/subdir/a/b/c
}

@test "rm" {
    load setup
    rm ${MOUNT}/file_in_source.txt
    rm ${MOUNT}/subdir/file_in_subdir.txt
    load unset
    [ ! -f ${SOURCE}/file_in_source.txt ]
    [ ! -f ${SOURCE}/subdir/file_in_subdir.txt ]
}
@test "rmdir" {
    load setup
    mkdir -p ${MOUNT}/subdir/test1/test2
    rm -rf ${MOUNT}/subdir
    load unset
    [ ! -d ${SOURCE}/subdir ]
}

@test "cp" {
    load setup
    ls mount/
    cp a.txt ${MOUNT}
    cp a.txt ${MOUNT}/subdir
    load unset
    test -f ${SOURCE}/a.txt
    test -f ${SOURCE}/subdir/a.txt
}

@test "mv" {
    load setup
    mv a.txt ${MOUNT}
    mv ${MOUNT}/a.txt ${MOUNT}/subdir/a.txt
    load unset
    test -f ${SOURCE}/subdir/a.txt
    [ ! -f a.txt ]
}

@test "dd" {
    load setup
    dd if=/dev/random of=${MOUNT}/file count=3
    load unset
    test -f ${SOURCE}/file
}

@test "rm -rf" {
    load setup
    rm -rf ${MOUNT}/subdir
    load unset
    [ ! -d ${SOURCE}/dir ]
    [ ! -d ${MOUNT}/dir ]
}

@test "md5sum" {
    load setup
    m=$(md5sum ${MOUNT}/file_in_source.txt)
    [[ "$m" == *"3b5d5c3712955042212316173ccf37be"* ]]
    m=$(md5sum ${MOUNT}/subdir/file_in_subdir.txt)
    [[ "$m" == *"60b725f10c9c85c70d97880dfe8191b3"* ]]

}

@test "find" {
    load setup
    f=$(find ${MOUNT} -name file_in_subdir.txt)
    [[ "$f" == *"mount/subdir/file_in_subdir.txt" ]]
    f=$(find ${MOUNT} -name file_in_source.txt)
    [[ "$f" == *"mount/file_in_source.txt" ]]
}

@test "grep" {
    load setup
    grep b ${MOUNT}/file_in_source.txt
    grep a ${MOUNT}/subdir/file_in_subdir.txt
    grep "seafile" ${MOUNT}/file_in_mem.txt
}

@test "file" {
    load setup
    f=$(file mount/file_in_source.txt)
    [[ "$f" == "mount/file_in_source.txt: ASCII text" ]]
    f=$(file mount/subdir/file_in_subdir.txt)
    [[ "$f" == "mount/subdir/file_in_subdir.txt: ASCII text" ]]
    f=$(file mount/file_in_mem.txt)
    [[ "$f" == "mount/file_in_mem.txt: ASCII text" ]]
}

@test "tar" {
    load setup
    tar cvzf ${MOUNT}/foo.tar ${MOUNT}/file_in_source.txt
    tar cvzf ${MOUNT}/bar.tar ${MOUNT}/subdir/file_in_subdir.txt
    tar cvzf ${MOUNT}/helloworld.tar ${MOUNT}/file_in_mem.txt
    load unset
    test -f ${SOURCE}/foo.tar
    test -f ${SOURCE}/bar.tar
    test -f ${SOURCE}/helloworld.tar
}

@test "read python2" {
    load setup
    python2 tests/read.py ${MOUNT}/file_in_source.txt
    python2 tests/read.py ${MOUNT}/subdir/file_in_subdir.txt
    python2 tests/read.py ${MOUNT}/file_in_mem.txt
}

@test "read python3" {
    type python3 || skip "Python 3 is not installed"
    load setup
    python3 tests/read.py ${MOUNT}/file_in_source.txt
    python3 tests/read.py ${MOUNT}/subdir/file_in_subdir.txt
    python3 tests/read.py ${MOUNT}/file_in_mem.txt
}

@test "write python2" {
    load setup
    python2 tests/write.py ${MOUNT}/hello.txt
    python2 tests/write.py ${MOUNT}/subdir/hello.txt
    load unset
    test -f ${SOURCE}/hello.txt
    test -f ${SOURCE}/subdir/hello.txt
}

@test "write python3" {
    type python3 || skip "Python 3 is not installed"
    load setup
    python3 tests/write.py ${MOUNT}/hello.txt
    python3 tests/write.py ${MOUNT}/subdir/hello.txt
    load unset
    test -f ${SOURCE}/hello.txt
}

@test "chown" {
    id -u tmpu || skip "User tmpu not found"
    load setup
    a=$(chown tmpu:tmpu ${MOUNT}/file_in_source.txt)
    chown -R tmpu:tmpu ${MOUNT}/subdir
}

@test "chmod" {
    load setup
    chmod 600 ${MOUNT}/file_in_source.txt
    chmod -R g-rwx ${MOUNT}/subdir
}

@test "cd" {
    load setup
    out=$(bash -c "cd ${MOUNT};ls")
    echo $out
    [[ $out == bin[[:space:]]complex_file.txt[[:space:]]file_in_mem.txt[[:space:]]file_in_source.txt[[:space:]]subdir ]]
}

@test "which" {
    load setup
    bash -c "which ${MOUNT}/bin/hello.sh"
}

@test ">" {
    load setup
    bash -c "echo \"echo\" > ${MOUNT}/subdir/echo.txt"
    a=$(cat ${SOURCE}/subdir/echo.txt)
    [[ "$a" == "echo" ]]
}

@test "<" {
    load setup
    a=$(bash -c "bash < ${MOUNT}/bin/hello.sh")
    [[ "$a" == "hello" ]]
}

@test "awk" {
    load setup
    a=$(awk '$1=="a" {print $2}' ${MOUNT}/complex_file.txt)
    [[ "$a" == "b" ]]
}

@test "sed" {
    load setup
    sed -i s,b,a,g ${MOUNT}/file_in_source.txt
    a=$(cat ${MOUNT}/file_in_source.txt) # this also tests cat :)
    [[ "$a" == "a" ]]
}

@test "cut" {
    load setup
    a=$(cut -f 2 -d " " ${MOUNT}/complex_file.txt)
    [[ "$a" == "b" ]]
}

@test "unlink" {
    load setup
    unlink ${MOUNT}/file_in_source.txt
    unlink ${MOUNT}/subdir/file_in_subdir.txt
}

@test "access time" {
    load setup
    before=$(ls -lu ${SOURCE}/file_in_source.txt)
    # ensure at least 1 minute has changed
    sleep 61
    after=$(ls -lu ${MOUNT}/file_in_source.txt)

    [[ $before != $after ]] || echo "Before $before After $after"
    
}

