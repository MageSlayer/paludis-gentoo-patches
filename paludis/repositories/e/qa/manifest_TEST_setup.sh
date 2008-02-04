#!/bin/bash
# vim: set ft=sh sw=4 sts=4 et :

mkdir -p manifest_TEST_dir/repo1 || exit 2
cd manifest_TEST_dir/repo1 || exit 3

mkdir -p {eclass,distfiles,profiles/test,cat-one/{visible,masked,needs-masked}} || exit 1
echo "repo1" > profiles/repo_name || exit 1
cat <<END > profiles/categories || exit 1
cat
END
cat <<END > profiles/test/make.defaults
ARCH=test
ACCEPT_KEYWORDS=test
END
cat <<END > profiles/profiles.desc
test test/ stable
END

mkdir "cat" || exit 4
mkdir "cat/not-signed" || exit 5
touch "cat/not-signed/Manifest" || exit 6

mkdir "cat/good" || exit 7
mkdir "cat/good/files" || exit 8
echo foo >cat/good/ChangeLog || exit 10
echo foo >cat/good/files/foo.patch || exit 11
cat >cat/good/good-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/good/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
EBUILD good-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/bad-type" || exit 7
mkdir "cat/bad-type/files" || exit 8
echo foo >cat/bad-type/ChangeLog || exit 10
echo foo >cat/bad-type/files/foo.patch || exit 11
cat >cat/bad-type/bad-type-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/bad-type/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
MISC bad-type-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
EBUILD ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/bad-size" || exit 7
mkdir "cat/bad-size/files" || exit 8
echo foo >cat/bad-size/ChangeLog || exit 10
echo foo >cat/bad-size/files/foo.patch || exit 11
cat >cat/bad-size/bad-size-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/bad-size/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
EBUILD bad-size-0.ebuild 35 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/bad-hash" || exit 7
mkdir "cat/bad-hash/files" || exit 8
echo foo >cat/bad-hash/ChangeLog || exit 10
echo foo >cat/bad-hash/files/foo.patch || exit 11
cat >cat/bad-hash/bad-hash-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/bad-hash/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
EBUILD bad-hash-0.ebuild 34 SHA1 7834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 c68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 fc0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 e3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/missing" || exit 7
mkdir "cat/missing/files" || exit 8
echo foo >cat/missing/ChangeLog || exit 10
echo foo >cat/missing/files/foo.patch || exit 11
cat >cat/missing/missing-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/missing/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
EBUILD missing-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX bar.patch 4 SHA1 e242ed3bffccdf271b7fbaf34ed72d089537b42f SHA256 7d865e959b2466918c9863afca942d0fb89d7c9ac0c99bafc3749504ded97730 RMD160 7d4e874a231f57b72509087d1e509942fdb6eac6 MD5 c157a79031e1c40f85931829bc5fc552
END

mkdir "cat/stray" || exit 7
mkdir "cat/stray/files" || exit 8
echo foo >cat/stray/ChangeLog || exit 10
echo foo >cat/stray/files/foo.patch || exit 11
echo foo >cat/stray/metadata.xml || exit 13
cat >cat/stray/stray-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/stray/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
EBUILD stray-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/unused-distfile" || exit 7
mkdir "cat/unused-distfile/files" || exit 8
echo foo >cat/unused-distfile/ChangeLog || exit 10
echo foo >cat/unused-distfile/files/foo.patch || exit 11
cat >cat/unused-distfile/unused-distfile-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/unused-distfile/Manifest <<END || exit 12
DIST monkey.tar.bz2 7 SHA1 744a9a056f145b86339221bb457aa57129f55bc2 SHA256 5a6e48105fea75ccccc66a038318f398c42761495d738786dc8a6d43179aa16a RMD160 7dbf02c6e0bbfda1550fc7ba0ebc4fdd866e2d3c MD5 2f548f61bd37f628077e552ae1537be2
DIST donkey.tar.bz2 7 SHA1 726a4b06f832dc6d671285b492a81cebd7e98cb6 SHA256 45814cf76274a6bfdf2993b1d275a29faf1e1f8fa9fff6cf8c4f4c893ede2258 RMD160 73ee0ca7277576f1e316fd7097ebd3f02bcacf83 MD5 f5fa31b4e964cc2a86140bc2a2e11a13
EBUILD unused-distfile-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

mkdir "cat/undigested-distfile" || exit 7
mkdir "cat/undigested-distfile/files" || exit 8
echo foo >cat/undigested-distfile/ChangeLog || exit 10
echo foo >cat/undigested-distfile/files/foo.patch || exit 11
cat >cat/undigested-distfile/undigested-distfile-0.ebuild <<END || exit 9
SLOT="0"
SRC_URI="monkey.tar.bz2"
END
cat >cat/undigested-distfile/Manifest <<END || exit 12
EBUILD undigested-distfile-0.ebuild 34 SHA1 6834113b3fed0c3833d3b7a42f7ead07c13209e8 SHA256 b68ae58845b2f3d15dad3eae2f02aaa2cfdbb14140c7d4a537a6bf4417fb0b30 RMD160 7bf5c52f5c2acf42d66961af3de9c97e4d2b1207 MD5 75fa0dc852d3c76e4741c650465bfa6c
MISC ChangeLog 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
AUX foo.patch 4 SHA1 f1d2d2f924e986ac86fdf7b36c94bcdf32beec15 SHA256 b5bb9d8014a0f9b1d61e21e796d78dccdf1352f23cd32812f4850b878ae4944c RMD160 ec0af898b7b1ab23ccf8c5036cb97e9ab23442ab MD5 d3b07384d113edec49eaa6238ad5ff00
END

