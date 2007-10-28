#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p vdb_repository_TEST_dir || exit 1
cd vdb_repository_TEST_dir || exit 1

mkdir -p repo1/cat-{one/{pkg-one-1,pkg-both-1},two/{pkg-two-2,pkg-both-2}} || exit 1

for i in SLOT EAPI; do
    echo "0" >repo1/cat-one/pkg-one-1/${i}
done

for i in DEPEND RDEPEND LICENSE INHERITED IUSE PDEPEND PROVIDE; do
    touch repo1/cat-one/pkg-one-1/${i}
done

echo "flag1 flag2" >>repo1/cat-one/pkg-one-1/USE

cat <<END >repo1/cat-one/pkg-one-1/CONTENTS
dir /directory
  obj /directory/file 4 2
sym /directory/symlink -> target 2 
dir	/directory with spaces
dir /directory with trailing space 
dir /directory  with  consecutive  spaces
obj /file with spaces 4    2
obj /file  with  consecutive  spaces 4 	  2 	  
obj /file with  trailing   space	  4 2
sym /symlink -> target  with  consecutive  spaces 2
sym /symlink with spaces -> target with spaces 2
sym /symlink  with  consecutive  spaces -> target  with  consecutive  spaces 2
sym /symlink -> target -> with -> multiple -> arrows 2
sym /symlink -> target with trailing space  2
sym /symlink ->  target with leading space 2
sym /symlink with trailing space  -> target 2
fif /fifo
fif /fifo with spaces
fif /fifo  with  consecutive  spaces
dev /device
dev /device with spaces
dev /device  with  consecutive  spaces
misc /miscellaneous
misc /miscellaneous with spaces
misc /miscellaneous  with  consecutive  spaces

obj 
  obj	
obj /file
obj /file   
obj /file  2 
sym foobar 2
sym foo -> bar
sym -> bar 2
sym foo -> 2
END

touch "world-empty"
cat <<END > world-no-match
cat-one/foo
cat-one/bar
cat-one/oink
END
cat <<END > world-match
cat-one/foo
cat-one/foofoo
cat-one/bar
END
cat <<END > world-no-match-no-eol
cat-one/foo
cat-one/bar
END
echo -n "cat-one/oink" >> world-no-match-no-eol

