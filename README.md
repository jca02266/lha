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
(actually, this limit still only applies to the filename length, since the original does not write any directory information in the level 0 header).
The autoconf version will output a warning message for pathnames that exceed the limit, and will truncate the end of the pathname. (The level 0 header should not be used).

An empty -lhd- header (with no directory name information) was created.

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

Note that there is a theory that the `-lhd-` method cannot be used with level 0 headers.

  <http://kuwa.xps.jp/x68k/KGARC/ARC/LHAHED15.ZIP>

However, Haruyasu Yoshizaki's original LHA for DOS/Windows (ver 2.55, 2.67) creates -lhd- method in level 0 header.

This opinion was acknowledged. (<http://kuwa.xps.jp/diary/2002-10.html#27_3>)
The above document have been fixed.

  <http://kuwa.xps.jp/x68k/KGARC/ARC/LHAHED21.ZIP>


### Change behavior with -g option

The `-g` option is said in the man page to suppress the creation of UNIX-specific information in the archive when creating an archive, but in fact it suppressed directory information as well.

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

In the autoconf version, the above becomes foo/bar.
(The original may have done this on purpose, but I saw no reason to do so.)
The `-g` option suppresses the creation of `-lhd-`, it is same behavior as original one.

In addition, if you specify the `-g` option and the header level option (`-0`, `-1` and so on ) at the same time, you need to specify the g option first, as shown above(`lha cg1`).
If you specify the `-g` option afterwards, such as `lha c1g`, a level 0 header will be created.
(this original behavior is a bit confusing)

## level 3 header

There seems to be a level 3 header in the world, but it is not yet fixed as a specification,
so I added *read only* support.
Additional extension headers are not supported. (I couldn't find any header that should be supported).
If you want to support largefile, you might want to support this level 3 header.

## dump of header information for debugging

As a totally extra feature, I added the ability to dump headers.
This is completely for debugging purposes.

```
lha vvv foo.lzh
```

This will output a dump with the list of archive contents.

## Change default header level

The default header level when creating an archive is now 2.
(The original LHa for UNIX 1.14i defaulted to level 1)

## Support extended header

The extended header Windows timestamp (0x41) now read (in level 1 header only).
For level 2 header and later, the extended header is ignored because the basic header contains the `time_t` information.
It has not been confirmed whether there is an LHA archiver that outputs Windows timestamp extension headers for level 1 header archives. I don't think this is a very useful fix, but I'm keeping it because I made it :-)

## Added `-x` option

Added the `-x` option to specify the pattern to be excluded from the files to be compressed.

```
  lha c -x '*.o' -x='*.a' -x'*.c' src.lzh src
```

The option parsing code has been reconstruct so that you can specify something like
The usage of this version is as follows

```
  usage: lha [-]<commands>[<options>] [-<options> ...] archive_file [file...]
```

## Extracting in Cygwin

When decompressing archives that do not have permission information, such as MS-DOS types,
Cygwin now decompresses them with 0777 & ~umask.
This is so that .exe and .dll can have the execute attribute.

## Support Large files

It can handle large files (over 2G) if your system supports it.
(configure script will provide you with the appropriate compiler options).

However, to support large files on HP-UX 11.0, you need to use

```
CC="cc -Ae +DA2.0W"
```

It seems that you need to instruct them to do so.

```
./configure --with-tmp-file=no CC="cc -Ae +DA2.0W" \
                        ac_cv_have_mktime=yes \
                        ac_cv_func_mktime=yes
```

`--with-tmp-file=no` will create intermediate files in the same directory as the output destination.
This is in case the temporary directory does not support 2G over.

(This is not related to largefiles support, but ac_cv_*=yes forces HP-UX to use mktime because
it fails to detect mktime for some reason.)

If you want to "disable" large files support, you can use the

```
./configure --disable-largefile
```

In regular LHA, only files less than 4G can be archived due to the specification of the level 0, 1, and 2 headers (there are only 4 bytes of space to store the file size).

However, UNLHA32.DLL, for example, has an extension header (0x42) that allows it to handle files over 4G.
The autoconf version currently supports files over 4G by referring to this extension header only during decompression.
(The reason it does not support creation is that I was not sure if it was correct to simply follow UNLHA32.DLL.)


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
