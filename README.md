LHa for UNIX with Autoconf
=======================

Translated with www.DeepL.com/Translator (free version)

This file describes the Autoconf version of LHa for UNIX.

# How to Compile

The compilation procedure is as follows.

```
gzip -dc lha-1.14i-acXXXXXXXX.tar.gz | tar xvf -
cd lha-114i-acXXXXXXXX

sh ./configure
make
make check
make install
```

MinGW support is in alpha. It has not been tested much (only make check succeeds).
To try out the MinGW version under Cygwin, use the

```
sh ./configure \
            --build=i686-pc-mingw32 \
            --host=x86_64-w64-mingw32 \
            --with-tmp-file=no
```

Note: In Windows environments such as Cygwin and MinGW, `make check` may fail if you compile in a deep directory.
This is because the full path length limit of Windows is exceeded when checking for long path test.
In this case, run configure in a shallow directory as follows

```
cd /tmp/build
sh ~/src/lha/configure ....
make
make check
make install
```

If autoconf/automake is installed and you want to maintain lha source, configure.ac and Makefile.am, follow the steps below.
The version of autoconf/automake is assumed to be autoconf 2.5x or later and automake 1.6.x or later.

```
gzip -dc lha-1.14i-acXXXXXXXX.tar.gz | tar xvf -
cd lha-114i-acXXXXXXXX

aclocal
autoheader
automake -a
autoconf

# The procedure from aclocal to autoconf can replace with `autoreconf -is`

sh ./configure
make
make check
make install
```

# Changes from the original

The Autoconf version of LHa for UNIX has been modified based on the
LHa for UNIX ver1.14i <http://www2m.biglobe.ne.jp/~dolphin/lha/lha.htm>

The main changes are listed below.

### -lh6-, -lh7- methods

In the original LHa for UNIX 1.14i, if you compile without defining SUPPORT_LH7
it was not possible to archive with -lh6- and -lh7- methods.
This is not a problem, but when SUPPORT_LH7 is defined, the -lh7- method is created by default.
So, to make it more flexible for lha users to choose these methods:
* SUPPORT_LH7 should always be defined.
* default archive method can be specified with the configure option `--with-default-method=[567]`.
The default method for archives created by default can be specified with the configure option --with-default-method=[567].

The default value of this option is `-lh5-`.
the command line option of lha, `-o6` or `-o7` options can create `-lh6-` or `-lh7-` archives at using time, and the configure option can change the default behavior.

## Japanese Kanji file names in the archive

The original LHa for UNIX 1.14i is careless about Japanese Kanji character codes of file names in archives.

Even when MULTIBYTE_CHAR is defined at compile time, it does not convert between Kanji code in the archive and the system Kanji code.

In the autoconf version, the configure option `--enable-multibyte-filename` allows the use of Kanji file names, and treats the Kanji code of file names stored in the archive as fixed to Japanese Shift JIS (it is specs of LHA).

The value of `--enable-multibyte-filename` is specifying the Kanji code of the system filename.
These are follows:

```
--enable-multibyte-filename=sjis
      system's Kanji code as Shift_JIS.

--enable-multibyte-filename=euc
      system's Kanji code as EUC-JP.

--enable-multibyte-filename=utf8
      system's Kanji code as UTF-8.

--enable-multibyte-filename=auto (yes or omit value)
      Automatically determine the system's kanji code.
      Automatic currently means:
        Shift_JIS: for Cygwin, MinGW and HP-UX environment
        UTF-8: the environment that the iconv is available
        EUC-JP: otherwise
--enable-multibyte-filename=no
--disable-multibyte-filename
      Disables multibyte support for filenames.
```

The default value is `auto`.

The lha command line option allows you to change the compile-time default.
This command line option is specified with the GNU style long option (with two leading dashes).

```
      --system-kanji-code=xxx
              Specifies the system kanji code.

      --archive-kanji-code=xxx
              Specifies the Kanji code of the file name to be stored in the archive.
              This is usually fixed to SJIS and should not be changed.
```

`xxx` is one of `sjis`, `euc`, `utf8`, or `cap`. `cap` is a code used by Samba and other applications.
The `cap` is a code used by samba and others to represent a kanji code with a ":" and a hexadecimal character.

For example:

```
      $ touch 漢字
      $ lha c foo.lzh 漢字
      $ lha l foo.lzh

      PERMISSION  UID  GID      SIZE  RATIO     STAMP           NAME
      ---------- ----------- ------- ------ ------------ --------------------
      -rw-r--r--  1000/1000        0 ****** Mar 23 21:23 漢字
      ---------- ----------- ------- ------ ------------ --------------------
       Total         1 file        0 ****** Mar 23 21:23

      $ lha l --system-kanji-code=cap foo.lzh

      PERMISSION  UID  GID      SIZE  RATIO     STAMP           NAME
      ---------- ----------- ------- ------ ------------ --------------------
      -rw-r--r--  1000/1000        0 ****** Mar 23 21:23 :8a:bf:8e:9a
      ---------- ----------- ------- ------ ------------ --------------------
```

The converter between utf8 and sjis for Mac OS X was created by Hiroto Sakai in 2002/6.
Thank you very much.

You can also use UTF-8 outside of Mac OS X by using the iconv library.

If you don't want to use iconv, you can do follows:

```
sh ./configure --disable-iconv
```

In particular, on Mac OS X, this will use the Core Foundation library
to support UTF-8.

## Specify an extract/store file from standard input

The original LHa for UNIX 1.14i uses:

```
echo foo.txt | lha x foo.lzh
find bar -name '*.[ch]' | lha c bar.lzh
```
This will extract only foo.txt from foo.lzh, or store files under the bar directory in bar.lzh.

In other words, This is the features to specify compression/decompression files from the standard input,
but it's not really useful for me.

(The compression example is as useful as `cpio`, but I think it would be better to create a new option for this behavior).

I think it is only intended for use from tty, but it will probably cause unintended behavior when running lha from daemon.
For this reason, I have taken the liberty of removing this feature.

It was a meaningful feature in the Windows environment where the following things are not possible
environment:

```
lha x bar.lzh `echo foo.txt`
```

Unfortunately, isatty() doesn't work well with MinGW, so the feature to use the standard input doesn't work on Windows.

If you want to restore the original spec, change `#if 0` to `#if 1` at line about 631 in the `lharc.c`.

## Extended header (user/group name) support

It is now possible to create extended headers (0x52, 0x53) for user and group names for Unix.
(default is off). See header.doc.jp for details.

When expanding and listing, if this information is in the header, it is used in preference to the ID.

This is enabled when you build with the configure option `--enable-user-name-header`.

## Suppress backup file creation

In the original LHa for UNIX 1.14i, when a file is added to the archive, or when a file is deleted from the archive, the original archive file is saved with extension `.bak`.

I found this behavior annoying, so I decided not to create a backup file in the autoconf version.
If you don't trust this patch, please build with the configure option `--enable-backup-archive`
(which you should definitely do :p). It will behave the same as the original.

## rewrite the source of header.c

header.c has been rewritten. In addition to the above changes
The following bugs have been fixed since LHa for UNIX 1.14i.


### Fixed bugs in level 2 header

Could not correctly read archives with `total header size` (first 2 bytes of archive header) greater than 256.
And, it created an invalid archive where the total header size was exactly 256.
The LHA header specification does not allow the total header size to be a multiple of 256, since a header leading 0 is considered to be the end of the archive (e.g. 256 is represented by "0x00 0x01" in little-endian, so it is leading 0).

CRC check of header is now performed when reading.

### Fixed bugs in level 1 header

Since no extended headers were used for file names, if you write a file name (excluding directory name) that exceeds 230 bytes to an archive, it created an invalid archive that exceeds the archive header size limit.

### Fixed bugs in level 0 header

Invalid archives were being created for long pathnames (including directory name) that exceeded the size limit of the archive header.
(In fact, the originals do not write any directory information in the
In fact, the original does not include any directory information in the level 0 header.
In the autoconf version, pathnames exceeding the limit are warned.
The autoconf version outputs a warning message for pathnames that exceed the limit, and truncates the end of the pathname.
The autoconf version will print a warning message for pathnames that exceed the limit and truncate the pathname afterwards. (The level 0 header should not be used).


長いパス名(ディレクトリも含む)に対してアーカイブヘッダのサイズ制
限を越えた不正なアーカイブが作成されていました(実際には、オリジ
ナルは level 0 header にディレクトリの情報を一切書かないのでこの
制限はやはりファイル名長だけが対象になります)。autoconf 版では制
限を越えたパス名は warning メッセージを出力し、パス名の後ろを
切り詰めます。(level 0 header は使用するべきではありません)

空の(ディレクトリ名情報のない) -lhd- ヘッダが作成されていました。

```
  $ mkdir foo
  $ lha c0 foo.lzh foo
  $ lha v foo.lzh
   PERMSSN    UID  GID    PACKED    SIZE  RATIO METHOD CRC     STAMP          NAME
  ---------- ----------- ------- ------- ------ ---------- ------------ -------------
  drwxrwxr-x  1000/1000        0       0 ****** -lhd- 0000 Jul 29 00:18
  ---------- ----------- ------- ------- ------ ---------- ------------ -------------
   Total         1 file        0       0 ******            Jul 29 00:18
```

なお、level 0 header で -lhd- method は使えないという説がある

  <http://kuwa.xps.jp/x68k/KGARC/ARC/LHAHED15.ZIP>

のですが、吉崎栄泰氏のオリジナル LHA (DOS/Windows版) (ver 2.55,
2.67) などは -lhd- method を level 0 header で作成します。

※ この意見は認められ(<http://kuwa.xps.jp/diary/2002-10.html#27_3>)、
   上記のドキュメントは、訂正されたようです。

  <http://kuwa.xps.jp/x68k/KGARC/ARC/LHAHED21.ZIP>

### Change behavior with -g option
g オプションを付けたときの level 0, 1, 2 header

g オプションは、アーカイブ作成のとき UNIX 固有の情報をアーカイブ
に作成するのを抑止すると man にはあるのですが、実際にはディレク
トリ情報まで抑止されていました。

```
  $ mkdir foo
  $ touch foo/bar
  $ lha cg1 foo.lzh foo
  $ lha foo.lzh
   PERMSSN    UID  GID      SIZE  RATIO     STAMP           NAME
  ---------- ----------- ------- ------ ------------ --------------------
  [generic]                    0 ****** Jul 29 00:02 bar
  ---------- ----------- ------- ------ ------------ --------------------
   Total         1 file        0 ****** Jul 29 00:02
```

autoconf 版では上記は foo/bar になります。(オリジナルはわざとそ
うしていたのかもしれませんが、そうする理由はないと判断しました)
g オプションで -lhd- の作成が抑止されるのは同じです。

なお、g オプションとヘッダレベルの指定を同時に行うときは上記のよ
うに g オプションを先に指定する必要があります。lha c1g など g オ
プションを後に指定すると level 0 header が作成されます(このオリ
ジナル仕様はちょっとわかりにくいです)。

## level 3 header

世の中には、level 3 header というものが存在するようですが、まだ仕様
としてfix されてないようなので*読み込みのみ*サポートしました。追加の
拡張ヘッダは未対応です。(特に対応すべきヘッダが見当たらなかった)
largefile 対応する場合は、この level 3 header をサポートした方が良さ
そうです。

## ヘッダのダンプ

まったくのおまけ機能としてヘッダのダンプ機能を追加しました。これは完
全にデバッグ用です。

```
  lha vvv foo.lzh
```

とすると、アーカイブの内容一覧にまざってダンプが出力されます。

## デフォルトヘッダレベル

アーカイブを作成するときのデフォルトのヘッダレベルを 2 にしました。
(オリジナルの LHa for UNIX 1.14i ではレベル 1 がデフォルト)

## 拡張ヘッダ

拡張ヘッダ Windows timestamp (0x41) を解釈するようにしました。(level
1 header のみ)。level 2 以上では、基本ヘッダに time_t の情報があるの
で、拡張ヘッダの方は無視します。
level 1 header のアーカイブに対して、Windows timestamp 拡張ヘッダ
を出力する LHA アーカイバが存在するかどうかは未確認です。あまり、
役に立たない修正だった気がしますがせっかく作ったので残してます:-)

## -x オプション

圧縮対象のファイルから除外されるパターンを指定する -x オプションを追
加しました。これに伴い、

```
  lha c -x '*.o' -x='*.a' -x'*.c' src.lzh src
```

といった指定ができるよう、オプション解析部は変更されました。
本バージョンの usage は以下のようになります。

```
  usage: lha [-]<commands>[<options>] [-<options> ...] archive_file [file...]
```

## Cygwin での解凍

MS-DOS タイプなど permission の情報を持たないアーカイブを Cygwin で
解凍する場合は、0777 & ~umask で展開するようにしました。これは、.exe
や .dll に実行属性を付けるためです。

## large files 対応

システムが対応していれば、2G over な large file を扱うことができます
(configure が適当なコンパイラオプションを指定してくれます)

ただし、HP-UX 11.0 で large files に対応するには以下のように

```
      CC="cc -Ae +DA2.0W"
```

と指示してあげる必要があるようです。

```
      ./configure --with-tmp-file=no CC="cc -Ae +DA2.0W" \
                              ac_cv_have_mktime=yes \
                              ac_cv_func_mktime=yes
```

--with-tmp-file=no は、中間ファイルを出力先と同じディレクトリに
作成します。テンポラリディレクトリが 2G over をサポートしていない
場合を考慮しています。

(largefiles 対応とは関係ありませんが ac_cv_*=yes は、HP-UX ではなぜ
か mktime の判定に失敗するため強制的に mktime を使うようにしています)

もし large files 対応を「無効」にしたければ、

```
      ./configure --disable-largefile
```

のようにします。

なお、正規の LHA では、level 0, 1, 2 ヘッダの仕様上 4G 未満のファイル
しか書庫に格納できません(ファイルサイズを格納する領域が 4 bytes しかない)。

しかし、UNLHA32.DLL などは、拡張ヘッダ(0x42)により、4G over なファイ
ルも扱えるようになっています。autoconf 版では、今のところ展開のときの
みこの拡張ヘッダを参照して、4G overファイルをサポートします。

(作成に対応していないのは、安易にUNLHA32.DLLに従うことが正しいのかよ
くわからなかったからです。)

## MacBinaryつきアーカイブのサポート

MacLHAで「MacBinary」チェックボックスをONにして作成したアーカイブに
格納されているファイルは、MacBinaryエンコードされています。
解凍時に -b オプションを指定すると、解凍後にMacBinaryデコードを行い、
データフォークのみを取り出します（リソースフォークは無視されます）。
また、普通のアーカイブに対して b オプションを指定して解凍した場合は
通常の解凍処理が行われます。

この機能を利用するにはapplefileライブラリが必要です。
applefileライブラリは下記より入手できます。

      http://sourceforge.net/projects/applefile/

# バグ

## 壊れたアーカイブの展開 (--extract-broken-archive)

LHa for UNIX (autoconf版)は、バージョン 1.14i-ac20030713 (slide.c
revision 1.20) より壊れたアーカイブを作成してしまう致命的なバグがあ
りました。(このようなアーカイブが作成される現象に遭遇することはほと
んどないかも知れません。ただ、バグのある LHa for UNIX では正常に展開
できてしまうので、壊れたアーカイブであることに気が付いてないだけかも
しれません)

この壊れたアーカイブは他の正常な LHA (あるいは現在の LHa for UNIX)
では展開時に CRC エラーが発生してしまいます。

現在のバージョンでは、

```
      lha x --extract-broken-archive broken.lzh
```

とオプション --extract-broken-archive を指定することで、このバグによ
り作成された壊れたアーカイブを強制的に展開することができます。CRC エ
ラーが発生するアーカイブを見付けたときには(そして、それが過去の LHa
for UNIX (autoconf版)で作成されたものである場合には)このオプションを
試してみてください。

# 再配布について

私は、src/header.c にはもはや元の lha 1.14i にあったコードは含まれて
いないと考えています。src/header.c は私の著作物です。ただし、lha
1.14i にあったコードが参考になったことは事実です。敬意を表する意味で
も src/header.c にあった歴代の改変者の名前はそのまま残しています。

これの意図する所は、man/lha.man にある再配布条件を src/header.c に適
用しないことです。私は、より自由で使いやすいコードを LHa for UNIX の
構成物にしようと考えています。src/header.c に適用するライセンスはま
だ未定です(なので、現状は LHa for UNIX の再配布条件が適用されると考
えてください)。

   ライセンス候補(メモ):
         <http://www.gnu.org/licenses/license-list.ja.html>
         <http://www.opensource.org/licenses/>

現在のところ、src/vsnprintf.c, src/fnmatch.[ch], src/getopt_long.[ch]
を除くその他の構成物や LHa for UNIX 全体には man/lha.man に含まれる
条項が適用されます。(vsnprintf.c, fnmatch.c を利用するように make し
た LHa for UNIX には、各ソースに記述された条項も適用されることに注意
してください)
