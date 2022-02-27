LHa for UNIX with Autoconf
=======================

このファイルは、Autoconf 版 LHa for UNIX について説明します。

1. コンパイルの手順
--------------------------------

コンパイルの手順は以下のようになります。

```
  gzip -dc lha-1.14i-acXXXXXXXX.tar.gz | tar xvf -
  cd lha-114i-acXXXXXXXX

  sh ./configure
  make
  make check
  make install
```

MinGW 対応はα版です。ほとんどテストされていません(make check が成功
する程度)。Cygwin 環境で MinGW 版を試すには

```
  sh ./configure \
              --build=i686-pc-mingw32 \
              --host=x86_64-w64-mingw32 \
              --with-tmp-file=no
```

などとしてください。

※ Cygwin や MinGW 等、Windows 環境では深いディレクトリでコンパイル
   するとmake check が失敗する場合があります。これは長いパスの格納
   チェックをするときに、Windows のフルパス長の制限にひっかかるため
   です。このような場合は、以下のように configure を浅めのディレクト
   リで実行してください。

```
    例:
      cd /tmp/build
      sh ~/src/lha/configure ....
      make
      make check
      make install
```

autoconf/automake がインストールされている場合で、lha ソースや 
configure.ac, Makefile.am をメンテナンスする場合は以下の手順になりま
す。autoconf/automake のバージョンはそれぞれ autoconf 2.5x, automake
1.6.x 以降での利用を前提としています。

```
  gzip -dc lha-1.14i-acXXXXXXXX.tar.gz | tar xvf -
  cd lha-114i-acXXXXXXXX

  aclocal
  autoheader
  automake -a
  autoconf

  # aclocal から autoconf 実行までの手順は、最近では、autoreconf -is
  # でいけるようです。

  sh ./configure
  make
  make check
  make install
```

2. オリジナルからの変更点
----------------------------------------

Autoconf 版 LHa for UNIX は、
LHa for UNIX ver1.14i <http://www2m.biglobe.ne.jp/~dolphin/lha/lha.htm>
を元に改変されました。

主な変更点を下記に挙げます。

### -lh6-, -lh7- メソッドのアーカイブ作成

オリジナルの LHa for UNIX 1.14i では、SUPPORT_LH7 の定義をせずにコン
パイルした場合、-lh6- および -lh7- メソッドのアーカイブを作成できま
せんでした。このことは別に構わないのですが SUPPORT_LH7 を定義すると
デフォルトで -lh7- メソッドのアーカイブ作成を強制されてしまいます。
そこで、もう少し柔軟に lha 利用者がこれらを選択できるよう 
SUPPORT_LH7 は常に定義するようにし、デフォルトで作成されるアーカイブ
のメソッド指定を configure オプションの --with-default-method=[567] 
で指定できるようにしました。

このオプションの省略値は -lh5- です。つまりデフォルトでは -lh5- アー
カイブを作成します。(そして、上で述べた通り、lhaの o6 または o7 オプ
ションによりいつでも-lh6-、-lh7- アーカイブを作成でき、configure オ
プションによりデフォルトの挙動を変更することができます)

### アーカイブ中の漢字ファイル名

オリジナルの LHa for UNIX 1.14i はアーカイブに格納するファイル名の漢
字コードに関して無頓着です。コンパイル時に MULTIBYTE_CHAR を定義した
ときでもアーカイブ中の Shift JIS ファイル名を EUC にすることもなく、
EUC コードのまま(正確にはシステムの漢字コードのまま)アーカイブに格納
したりします。

autoconf 版では、configure オプション --enable-multibyte-filename に
より漢字ファイル名が使用でき、アーカイブに格納されるファイル名の漢字
コードを SJIS 固定として扱います。

--enable-multibyte-filename の引数(システムのファイル名の漢字コード
指定)は、以下の通りです。

```
    --enable-multibyte-filename=sjis
          システムの漢字コードを SJIS として扱います。
    --enable-multibyte-filename=euc
          システムの漢字コードを EUC として扱います。
    --enable-multibyte-filename=utf8
          システムの漢字コードを UTF-8 として扱います。
    --enable-multibyte-filename=auto (または yes または引数なし)
          システムの漢字コードを自動で判別します。自動といっても現状は、
          Cygwin, MinGW, HP-UX の場合に SJIS、iconvライブラリが利用できる場合 UTF-8、
          それ以外を EUC とみなすだけです。
    --enable-multibyte-filename=no
    --disable-multibyte-filename
          ファイル名のマルチバイトサポートを無効にします。
```

デフォルトは、auto です。

lha のコマンドラインオプションにより、コンパイル時のデフォルト指定を
変更することができます。このコマンドラインオプションは、GNU style の 
long option (ダッシュ2つが先行するスタイル)で指定します。

```
      --system-kanji-code=xxx
              システムのファイル名の漢字コードを指定します。

      --archive-kanji-code=xxx
              アーカイブ内へ格納するときのファイル名のコードを指定し
              ます。これは通常 SJIS 固定なので変更するべきではありま
              せん。
```

xxx は sjis, euc, utf8, cap のいずれかです。cap は、samba などで使われる
コードで、漢字コードを ":" と 16 進文字で表現するコードです。

例えば、

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

などとなります。

Mac OS X 用の utf8 <-> sjis 変換は、2002/6 に坂井浩人さんに作成して
いただきました。ありがとうございます。

また、Mac OS X 以外でも、iconv ライブラリを使用すれば、UTF-8 を使用す
ることができます。もし、iconv を使用したくない場合は

```
   sh ./configure --enable-multibyte-filename=utf8 --disable-iconv
```

としてください。特に Mac OS X ではこうすることで、Core Foundationライブラリ
を使用した UTF-8 対応を行います。

### 標準入力からの展開ファイルの指定

オリジナルの LHa for UNIX 1.14i は、

```
  echo foo.txt | lha x foo.lzh
  find bar -name '*.[ch]' | lha c bar.lzh
```

とすると、foo.lzh から foo.txt だけを展開したり、bar ディレクトリ配
下のファイルを bar.lzh に格納したりできます。つまり、標準入力から、
圧縮／展開ファイルを指定できる機能なのですが、いまいち使い道がないわ
りに邪魔な機能です(圧縮の例は、cpio 同様まあ使えるけど、このためのオ
プションを新設するのが良いと考えています)

tty からの利用しか想定してないと思われますが、おそらく daemon から 
lha を実行する場合などで意図しない動作をするでしょう。そういうわけで
勝手ながらこの機能は削除しました。以下のような事ができない Windows 
環境では意味のある機能だったかもしれませんが

```
  lha x bar.lzh `echo foo.txt`
```

残念ながら MinGW で isatty() がうまく動作しませんでしたから Windows 
では使えないのでした。

オリジナルの仕様を復活させたい場合は、lharc.c の 631行目付近の #if 0 
を #if 1 にしてください。

### 拡張ヘッダ(ユーザ名/グループ名)のサポート

ユーザ名、グループ名の拡張ヘッダ(0x52, 0x53)を作成できるようにしまし
た(デフォルトはoff)。詳細は header.doc.jp を参照してください。展開
と一覧表示のときにヘッダにこの情報があれば ID に優先してこの情報が利
用されます。作成は configure オプション --enable-user-name-header を
指定して build した場合に有効になります。

### バックアップファイル作成の抑止

オリジナル LHa for UNIX 1.14i では、アーカイブにファイルを追加したと
きやアーカイブからファイルを削除したときに、元のアーカイブを .bak と
いう拡張子で保存します。この挙動は煩わしく感じたので autoconf 版では
バックアップファイルを作成しないようにしました。この autoconf 版パッ
チが信用できないような人は configure オプション --enable-backup-archive
を指定して build してください(ぜひそうするべきです:p)。オリジナルと
同じ挙動になります。

### header.c の書き換え

header.c は作り変えられました。上記に示した変更に加えてオリジナル 
LHa for UNIX 1.14i から以下の不具合が修正されました。

#### level 2 header のバグ

total header size (アーカイブヘッダの先頭 2 byte) が 256 以上
であるアーカイブを正しく読むことができませんでした。また、total
header size がちょうど 256 になるような不正なアーカイブを作成し
ていました。LHA のヘッダ仕様ではヘッダ先頭が 0 であればアーカイ
ブの終端とみなすため total header size を 256 の倍数にできません。
(256 などは little-endian で 0x00 0x01 となるため、先頭が 0 にな
ります。)

読み込み時にヘッダの CRC check を行うようにしました。

#### level 1 header のバグ

ファイル名に対して拡張ヘッダを使用することがないため、230 バイト
を越えるファイル名(ディレクトリを含まない)をアーカイブに書くとアー
カイブヘッダのサイズ制限を越えた不正なアーカイブが作成されていま
した。

#### level 0 header のバグ

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

#### g オプションを付けたときの level 0, 1, 2 header

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

### level 3 header

世の中には、level 3 header というものが存在するようですが、まだ仕様
としてfix されてないようなので*読み込みのみ*サポートしました。追加の
拡張ヘッダは未対応です。(特に対応すべきヘッダが見当たらなかった)
largefile 対応する場合は、この level 3 header をサポートした方が良さ
そうです。

### ヘッダのダンプ

まったくのおまけ機能としてヘッダのダンプ機能を追加しました。これは完
全にデバッグ用です。

```
  lha vvv foo.lzh
```

とすると、アーカイブの内容一覧にまざってダンプが出力されます。

### デフォルトヘッダレベル

アーカイブを作成するときのデフォルトのヘッダレベルを 2 にしました。
(オリジナルの LHa for UNIX 1.14i ではレベル 1 がデフォルト)

### 拡張ヘッダ

拡張ヘッダ Windows timestamp (0x41) を解釈するようにしました。(level
1 header のみ)。level 2 以上では、基本ヘッダに time_t の情報があるの
で、拡張ヘッダの方は無視します。
level 1 header のアーカイブに対して、Windows timestamp 拡張ヘッダ
を出力する LHA アーカイバが存在するかどうかは未確認です。あまり、
役に立たない修正だった気がしますがせっかく作ったので残してます:-)

### -x オプション

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

### Cygwin での解凍

MS-DOS タイプなど permission の情報を持たないアーカイブを Cygwin で
解凍する場合は、0777 & ~umask で展開するようにしました。これは、.exe 
や .dll に実行属性を付けるためです。

### large files 対応

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

### MacBinaryつきアーカイブのサポート

MacLHAで「MacBinary」チェックボックスをONにして作成したアーカイブに
格納されているファイルは、MacBinaryエンコードされています。
解凍時に -b オプションを指定すると、解凍後にMacBinaryデコードを行い、
データフォークのみを取り出します（リソースフォークは無視されます）。
また、普通のアーカイブに対して b オプションを指定して解凍した場合は
通常の解凍処理が行われます。

この機能を利用するにはapplefileライブラリが必要です。
applefileライブラリは下記より入手できます。

      http://sourceforge.net/projects/applefile/

3. バグ
-----------

### 壊れたアーカイブの展開 (--extract-broken-archive)

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

4. 再配布について
---------------------------

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
