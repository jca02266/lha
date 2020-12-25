# header.doc

Original author: Mar. 2, 1992, Masaru Oki.
Translated by Koji Arai with www.DeepL.com/Translator (free version)

This document describes the structure of LHA header used by the LHa for Unix

The below is from Mr. Yoshizaki's lhx.doc

----------------------------------------------------------------

In this version, three types of header formats are provided as a transitional measure. level-1 is the default
value, but in the future, we plan to use level-2, which has no limitation on the number of characters in the
path name.

```
  A. Header Specifications

-----------------------------------------------------------------------------
        level-0                 level-1                 level-2
-----------------------------------------------------------------------------
      1 header size           1 header size           2 total header size
      1 header sum            1 header sum
      5 method ID             5 method ID             5 method ID
B     4 packed size           4 skip size             4 packed size
a     4 original size         4 original size         4 original size
s     2 time                  2 time                  4 time(UNIX type)
i     2 date                  2 date
c     1 attribute             1 0x20                  1 RESERVED
      1 level 0x00            1 level 0x01            1 level 0x02
P     1 name length           1 name length
a     ? pathname              ? filename
r     2 file crc              2 file crc              2 file crc
t     . ........              1 OS ID 'U'             1 OS ID 'U'
                              . ........
                              2 next-header size      2 next-header size
    *************************************************************************
     24 + ?                  27 + ?                  26
-----------------------------------------------------------------------------
E                             1 ext-type              1 ext-type
x                             . ........              . ........
t                             2 next-header size      2 next-header size
-----------------------------------------------------------------------------

    a. Header type

      level-0 header
          The format is the same as the traditional LHarc and LArc.
          The standard delimiter for directory names is '\'.

      level-1 header
          This header is used by default. The -lh0- archives created by
          -x0 option can be decompressed by LHarc, but no CRC check is
          performed.

      level-2 header
          This is a header to support long filenames. We would like to
          make this header the standard in the future, so please support
          it if you are writing LH-related utilities.

    b. For compressible and decompressible method IDs

      "*" mark means compressible method

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

    c. OS ID

      As of now, reserved the following IDs

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

  B. Extended header

    a. OS-independent (0x00 - 0x3f)

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
```
----------------------------------------------------------------

Extended header specification of the LHa for UNIX

```
* The type of dependent information (information to be stored)
        The following UNIX-dependent information should be stored.
        (1) Unix permission
        (2) GID,UID
        (3) Group name, user name
        (4) Last modified time (UNIX time)

* Type of dependent information(ext-type)
        The first byte of the extended header contains a value that
        IDs what the information represents. The UNIX-dependent ext-type
        0x50 - 0x54 is used.

* How to store dependent information
    Each of the above (1) - (4) made a different type.

        (1)Unix permission
                size    value
                1       0x50
                2       permission value
                2       next-header size

        (2)GID,UID
                size    value
                1       0x51
                2       GID
                2       UID
                2       next-header size

        (3)-1 Group name
                1       0x52
                ?       group name
                2       next-header size

        (3)-2 User name
                1       0x53
                ?       user name
                2       next-header size

        (4) Last modified time (for header_level1)
                1       0x54
                4       Unix time
                2       next-header size
```

* Implementation of the LHa for UNIX ver 1.14 (Added by Nobutaka Watazaki)

  (3) above is not supported.
  It does not create an archive that contains the group name and user
  name information, and ignores them when extracting.
  Also, (4) is added only in the archive of header level 1.

* Implementation of the LHa for UNIX ver 1.14i with autoconf (2002-06-04 Koji Arai)

  If the information of (3) exists, it will be used in preference
  to the ID when expanding and listing.
  The addtion the ext-header is enabled when you built with the configure
  option --enable-user-name-header.

----------------------------------------------------------------

Partially modified by Nobutaka Watazaki
watazaki@shimadzu.co.jp
