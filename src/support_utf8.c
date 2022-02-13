/* ------------------------------------------------------------------------ */
/* LHa for UNIX                                                             */
/*        support_utf8.c                                                    */
/*                                                                          */
/*  Ver. 1.14i Contributed UTF-8 convertion for Mac OS X                    */
/*                                              2002.06.29  Hiroto Sakai    */
/* ------------------------------------------------------------------------ */
#include "lha.h"

#if defined(__APPLE__) && !USE_ICONV  /* Added by Hiroto Sakai */

#include <CoreFoundation/CFString.h>
#include <CoreFoundation/CFStringEncodingExt.h>

/* Derived from https://opensource.apple.com/source/CF/CF-1153.18/CFStringEncodingConverter.h */
/* CFIndex is signed long */
uint32_t CFStringEncodingBytesToUnicode(uint32_t encoding, uint32_t flags, const uint8_t *bytes, CFIndex numBytes, CFIndex *usedByteLen, UniChar *characters, CFIndex maxCharLen, CFIndex *usedCharLen);
uint32_t CFStringEncodingUnicodeToBytes(uint32_t encoding, uint32_t flags, const UniChar *characters, CFIndex numChars, CFIndex *usedCharLen, uint8_t *bytes, CFIndex maxByteLen, CFIndex *usedByteLen);

/* this is not need for Mac OS X v 10.2 later */
enum {
  kCFStringEncodingAllowLossyConversion = 1,
  kCFStringEncodingBasicDirectionLeftToRight = (1 << 1),
  kCFStringEncodingBasicDirectionRightToLeft = (1 << 2),
  kCFStringEncodingSubstituteCombinings = (1 << 3),
  kCFStringEncodingComposeCombinings = (1 << 4),
  kCFStringEncodingIgnoreCombinings = (1 << 5),
  kCFStringEncodingUseCanonical = (1 << 6),
  kCFStringEncodingUseHFSPlusCanonical = (1 << 7),
  kCFStringEncodingPrependBOM = (1 << 8),
  kCFStringEncodingDisableCorporateArea = (1 << 9),
  kCFStringEncodingASCIICompatibleConversion = (1 << 10),
};

static int
ConvertEncodingToUTF8(const char* inCStr,
                      char* outUTF8Buffer,
                      CFIndex outUTF8BufferLength,
                      uint32_t scriptEncoding,
                      uint32_t flags)
{
    CFIndex unicodeChars;
    CFIndex srcCharsUsed;
    CFIndex usedByteLen = 0;
    UniChar uniStr[512];
    uint32_t cfResult;

    cfResult = CFStringEncodingBytesToUnicode(scriptEncoding,
                                              flags,
                                              (uint8_t *)inCStr,
                                              strlen(inCStr),
                                              &srcCharsUsed,
                                              uniStr,
                                              512,
                                              &unicodeChars);
    if (cfResult == 0) {
        cfResult = CFStringEncodingUnicodeToBytes(kCFStringEncodingUTF8,
                                                  flags,
                                                  uniStr,
                                                  unicodeChars,
                                                  &srcCharsUsed,
                                                  (uint8_t *)outUTF8Buffer,
                                                  outUTF8BufferLength - 1,
                                                  &usedByteLen);
        outUTF8Buffer[usedByteLen] = '\0';
    }

    return cfResult;
}

static int
ConvertUTF8ToEncoding(const char* inUTF8Buf,
                      CFIndex inUTF8BufLength,
                      char* outCStrBuffer,
                      CFIndex outCStrBufferLength,
                      uint32_t scriptEncoding,
                      uint32_t flags)
{
    CFIndex unicodeChars;
    CFIndex srcCharsUsed;
    CFIndex usedByteLen = 0;
    UniChar uniStr[256];
    uint32_t cfResult;

    cfResult = CFStringEncodingBytesToUnicode(kCFStringEncodingUTF8,
                                              flags,
                                              (uint8_t *)inUTF8Buf,
                                              inUTF8BufLength,
                                              &srcCharsUsed,
                                              uniStr,
                                              255,
                                              &unicodeChars);
    if (cfResult == 0) {
        cfResult = CFStringEncodingUnicodeToBytes(scriptEncoding,
                                                  flags,
                                                  uniStr,
                                                  unicodeChars,
                                                  &srcCharsUsed,
                                                  (uint8_t *)outCStrBuffer,
                                                  outCStrBufferLength - 1,
                                                  &usedByteLen);
        outCStrBuffer[usedByteLen] = '\0';
    }

    return cfResult;
}

#elif USE_ICONV
#if HAVE_ICONV_H
#include <iconv.h>
#endif

static int
ConvertEncodingByIconv(const char *src, char *dst, size_t dstsize,
                       const char *srcEnc, const char *dstEnc)
{
    iconv_t ic;
    static char szTmpBuf[2048];
    char *src_p;
    char *dst_p;
    size_t sLen;
    size_t iLen;

    dst_p = &szTmpBuf[0];
    iLen = (size_t)sizeof(szTmpBuf)-1;
    src_p = (char *)src;
    sLen = (size_t)strlen(src);
    memset(szTmpBuf, 0, sizeof(szTmpBuf));
    memset(dst, 0, dstsize);

    ic = iconv_open(dstEnc, srcEnc);
    if (ic == (iconv_t)-1) {
        error("iconv_open() failure: %s", strerror(errno));
        return -1;
    }

    if (iconv(ic, &src_p, &sLen, &dst_p, &iLen) == (size_t)-1) {
        error("iconv() failure: %s", strerror(errno));
        iconv_close(ic);
        return -1;
    }

    strncpy(dst, szTmpBuf, dstsize);

    iconv_close(ic);

    return 0;
}
#endif /* defined(__APPLE__) */

#if ICONV_HAS_UTF8MAC
#define LHA_ENCODING_UTF8 "UTF-8-MAC"
#else
#define LHA_ENCODING_UTF8 "UTF-8"
#endif

extern char *
conv_to_utf8(char *dst, const char *src, size_t dstsize, int from_code)
{
#if defined(__APPLE__) && !USE_ICONV
  unsigned long enc = 0; /* kCFStringEncodingMacRoman */

  switch (from_code) {
  case CODE_SJIS:
      enc = kCFStringEncodingDOSJapanese;
      break;
  case CODE_EUC:
      enc = kCFStringEncodingEUC_JP;
      break;
  default:
      error("unsupported encoding");
      break;
  }

  dst[0] = '\0';
  if (enc && ConvertEncodingToUTF8(src, dst, dstsize, enc,
                            kCFStringEncodingUseHFSPlusCanonical) == 0)
      return dst;
#elif USE_ICONV
  const char *enc = NULL;

  switch (from_code) {
  case CODE_SJIS:
      enc = "SJIS";
      break;
  case CODE_EUC:
      enc = "EUC-JP";
      break;
  default:
      error("unsupported encoding");
      break;
  }

  if (enc && ConvertEncodingByIconv(src, dst, dstsize, enc, LHA_ENCODING_UTF8) != -1)
      return dst;
#else
  error("not support utf-8 conversion");
#endif

  if (dstsize < 1) return dst;
  dst[dstsize-1] = 0;
  return strncpy(dst, src, dstsize-1);
}

extern char *
conv_from_utf8(char *dst, const char *src, size_t dstsize, int to_code)
{
#if defined(__APPLE__) && !USE_ICONV
  int srclen;
  unsigned long enc = 0; /* kCFStringEncodingMacRoman */

  switch (to_code) {
  case CODE_SJIS:
      enc = kCFStringEncodingDOSJapanese;
      break;
  case CODE_EUC:
      enc = kCFStringEncodingEUC_JP;
      break;
  default:
      error("unsupported encoding");
      break;
  }

  dst[0] = '\0';
  srclen = strlen(src);
  if (enc && ConvertUTF8ToEncoding(src, srclen, dst, dstsize, enc,
                            kCFStringEncodingUseHFSPlusCanonical) == 0)
      return dst;
#elif USE_ICONV
  const char *enc = NULL;

  switch (to_code) {
  case CODE_SJIS:
      enc = "SJIS";
      break;
  case CODE_EUC:
      enc = "EUC-JP";
      break;
  default:
      error("unsupported encoding");
      break;
  }

  if (enc && ConvertEncodingByIconv(src, dst, dstsize, LHA_ENCODING_UTF8, enc) != -1)
      return dst;
#else
  error("not support utf-8 conversion");
#endif

  if (dstsize < 1) return dst;
  dst[dstsize-1] = 0;
  return strncpy(dst, src, dstsize-1);
}
