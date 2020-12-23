# header.doc

Author: Mar. 2, 1992, Masaru Oki.

This document describes the structure of LHA header used by the LHa for Unix

----------------------------------------------------------------
    本バージョンでは過渡的な措置として3種類のヘッダ形式を用意し、level-1
  を既定値としていますが、将来的にはパス名の文字数に制限のない level-2に
  統一する方針です。


  A. ヘッダの仕様

-----------------------------------------------------------------------------
        level-0                 level-1                 level-2
-----------------------------------------------------------------------------
      1 header size           1 header size           2 total header size
      1 header sum            1 header sum
      5 method ID             5 method ID             5 method ID
基    4 packed size           4 skip size             4 packed size
      4 original size         4 original size         4 original size
本    2 time                  2 time                  4 time(UNIX type)
      2 date                  2 date
部    1 attribute             1 0x20                  1 RESERVED
      1 level 0x00            1 level 0x01            1 level 0x02
分    1 name length           1 name length
      ? pathname              ? filename
      2 file crc              2 file crc              2 file crc
      . ........              1 OS ID 'U'             1 OS ID 'U'
                              . ........
                              2 next-header size      2 next-header size
    *************************************************************************
     24 + ?                  27 + ?                  26
-----------------------------------------------------------------------------
拡                            1 ext-type              1 ext-type
張                            . ........              . ........
部                            2 next-header size      2 next-header size
分
-----------------------------------------------------------------------------

    a. ヘッダの種類

      level-0 ヘッダ
          従来の LHarc, LArc と同じ形式です。ディレクトリ名の区切りは '\'
        を標準としています。

      level-1 ヘッダ
          既定値でこのヘッダが作成されます。-x0 で作成された -lh0- の書庫
        は LHarc で解凍可能ですが、解凍時に CRC チェックは行われません。

      level-2 ヘッダ
          長いファイル名をサポートするためのヘッダです。将来的には本ヘッ
        ダを標準としたいので、LH 関連のユーティリティを作成される方は今後
        準拠してくださるようにお願いします。

    b. 凍結・解凍可能な method ID について

      * は作成可能な method

      -lh0- * no compression

      -lh1- * 4k sliding dictionary(max 60 bytes) + dynamic Huffman
              + fixed encoding of position

      -lh2-   8k sliding dictionary(max 256 bytes) + dynamic Huffman

      -lh3-   8k sliding dictionary(max 256 bytes) + static Huffman

      -lh4- * 4k sliding dictionary(max 256 bytes) + static Huffman
              + improved encoding of position and trees

      -lh5- * 8k sliding dictionary(max 256 bytes) + static Huffman
              + improved encoding of position and trees

      -lh6-   32k sliding dictionary(max 256 bytes) + static Huffman
			  + improved encoding of position and trees

      -lh7-   64k sliding dictionary(max 256 bytes) + static Huffman
			  + improved encoding of position and trees

      -lzs-   2k sliding dictionary(max 17 bytes)

      -lz4-   no compression

      -lz5-   4k sliding dictionary(max 17 bytes)

    c. OS ID について

      現在のところ、以下の ID を予約しています。

        MS-DOS  'M'
        OS/2    '2'
        OS9     '9'
        OS/68K  'K'
        OS/386  '3'
        HUMAN   'H'
        UNIX    'U'
        CP/M    'C'
        FLEX    'F'
        Mac     'm'
        Runser  'R'

  B. 拡張部分

    a. OS 非依存 (0x00 - 0x3f)

        common header
         1 0x00
         2 header crc
       ( 1 information )
         2 next-header size

        filename header
         1 0x01
         ? filename
         2 next-header size

        dirname header
         1 0x02
         ? dirname
         2 next-header size

        comment header
         1 0x3f
         ? comments
         2 next-header size

(以上、吉崎氏の lhx.doc より)
----------------------------------------------------------------
LHa for UNIX 拡張ヘッダ仕様

・依存情報の種類(格納すべき情報)
	UNIX に依存する情報で記録すべきは以下のものである。
	(1) パーミッション
	(2) GID,UID
	(3) グループ名、ユーザ名
	(4) 最終変更時刻(UNIX time)

・依存情報のタイプ(ext-type)
	  拡張ヘッダの先頭1バイトには情報が何を示すもの
	か識別するための値が格納されている。
	UNIX依存情報として 0x50 - 0x54 を使用する。

・依存情報の格納方法
	上記 (1) - (4) をそれぞれ別のタイプとする。

	(1)パーミッション
		size	値
		1	0x50
		2	パーミッション値
		2	next-header size

	(2)GID,UID
		size	値
		1	0x51
		2	GID
		2	UID
		2	next-header size

	(3)-1 グループ名
		1	0x52
		?	グループ名文字列
		2	next-header size

	(3)-2 ユーザ名
		1	0x53
		?	ユーザ名文字列
		2	next-header size

	(4) 最終変更時刻 (for header_level1)
		1	0x54
		4	UNIX 時間
		2	next-header size

・LHa for UNIX ver 1.14 での実装（綿崎）
	上記(3)をサポートしていない。
	(3)のグループ名、ユーザ名の情報を含む書庫を作成すること
	はなく、展開時には無視する。
	また、(4)はヘッダレベル1の書庫にのみ含まれる。

・LHa for UNIX ver 1.14i with autoconf での実装(2002-06-04 新井)

	(3) を作成するようにしてみました。展開と一覧表示のときに (3) 
	の情報があれば ID に優先します。
	作成は configure オプション --enable-user-name-header を指定して
	build したときに有効になります。

・追記: Unicode filename

        UNLHA32.DLL ver 2.39a以降で、Unicodeファイル名に対応しているらしく、
        以下の拡張ヘッダが定義されている(未実装)

        Unicode filename header
         1 0x44
         ? filename (UTF-16LE)
         2 next-header size

        Unicode dirname header
         1 0x45
         ? dirname (UTF-16LE)
         2 next-header size

        このヘッダがついている場合、従来のファイル名ヘッダー(01h)、
        ディレクトリ名ヘッダー(02h)は付与されないらしい。

----------------------------------------------------------------
以上。

一部改変：
綿崎　修隆 (Nobutaka Watazaki)
watazaki@shimadzu.co.jp
