#ifndef ES_CHARACTER_H
#define ES_CHARACTER_H

#include <es/parser/unicode.h>
#include <es/utils/helper.h>

namespace es {
namespace character {

using namespace unicode;

// End of Line
static const char16_t EOS = 0x0000;

// Format-Control Character
static const char16_t ZWNJ = 0x200C;  // Zero width non-joiner
static const char16_t ZWJ  = 0x200D;  // Zero width joiner
static const char16_t BOM  = 0xFEFF;  // Byte Order Mark

// White Space
static const char16_t TAB     = 0x0009;  // Tab
static const char16_t VT      = 0x000B;  // Vertical Tab
static const char16_t FF      = 0x0020;  // Form Feed
static const char16_t SP      = 0x0020;  // Space
static const char16_t hashx0a = 0x00A0;  // No-break space
// static const char16_t BOM  = 0xFEFF;  // Byte Order Mark
// USP includes lots of characters, therefore only included in the function.

// Line Terminators
static const char16_t LF = 0x000A;
static const char16_t CR = 0x000D;
static const char16_t LS = 0x2028;
static const char16_t PS = 0x2029;


inline bool IsUSP(char16_t c) {
  return c == 0x1680 || (c >= 0x2000 && c <= 0x200A) ||
         c == 0x202F || c == 0x205F || c == 0x3000;
}

inline bool IsWhiteSpace(char16_t c) {
  return c == TAB || c == VT || c == FF ||
         c == FF  || c == SP || c == hashx0a ||
         IsUSP(c);
}

inline bool IsLineTerminator(char16_t c) {
  return c == LF || c == CR || c == LS || c == PS;
}

inline bool IsDecimalDigit(char16_t c) {
  return c >= u'0' && c <= u'9';
}

inline bool IsUnicodeLetter(char16_t c) {
  return ((1 << GetCategory(c)) & (Lu | Ll | Lt | Lm | Lo | Nl));
}

inline bool IsUnicodeCombiningMark(char16_t c) {
  return ((1 << GetCategory(c)) & (Mn | Mc));
}

inline bool IsUnicodeDigit(char16_t c) {
  return GetCategory(c) == DECIMAL_DIGIT_NUMBER;
}

inline bool IsUnicodeConnectorPunctuation(char16_t c) {
  return GetCategory(c) == CONNECTOR_PUNCTUATION;
}

inline bool IsHexDigit(char16_t c) {
  return IsDecimalDigit(c) || (u'A' <= c && c <= u'F') || (u'a' <= c && c <= u'f');
}

inline bool IsRadixDigit(char16_t c, int radix) {
  if (radix == 10)
    return IsDecimalDigit(c);
  else if (radix < 10)
    return u'0' <= c && c < u'0' + radix;
  else
    return IsDecimalDigit(c) || (u'A' <= c && c <= u'A' + radix - 10) || (u'a' <= c && c <= u'a' + radix - 10);
}

inline bool IsSingleEscapeCharacter(char16_t c) {
  return c == u'\'' || c == u'"' || c == u'\\' || c == u'b' ||
         c == u'f'  || c == u'f' || c == u'n'  || c == u'r' ||
         c == u't'  || c == u'v';
}

inline bool IsEscapeCharacter(char16_t c) {
  return IsSingleEscapeCharacter(c) || IsDecimalDigit(c) ||
         c == u'x' || c == u'u';
}

inline bool IsNonEscapeCharacter(char16_t c) {
  return !IsEscapeCharacter(c) && !IsLineTerminator(c);
}

inline bool IsCharacterEscapeSequence(char16_t c) {
  // NODE(zhuzilin) The chars that are not in { LineTerminator, DecimalDigit, u'x', u'u' }.
  return IsSingleEscapeCharacter(c) || IsNonEscapeCharacter(c);
}

inline bool IsIdentifierStart(char16_t c) {
  return IsUnicodeLetter(c) || c == u'$' || c == u'_' || c == u'\\';
}

inline bool IsIdentifierPart(char16_t c) {
  return IsIdentifierStart(c) || IsUnicodeCombiningMark(c) ||
         IsUnicodeDigit(c) || IsUnicodeConnectorPunctuation(c) ||
         c == ZWNJ || c == ZWJ;
}

inline bool IsRegularExpressionChar(char16_t c) {
  return !IsLineTerminator(c) && c != u'\\' && c != u'/' && c != u'[';
}

inline bool IsRegularExpressionFirstChar(char16_t c) {
  return !IsLineTerminator(c) && c != u'*' && c != u'/';
}

inline bool IsRegularExpressionClassChar(char16_t c) {
  return !IsLineTerminator(c) && c != u']';
}

inline double Digit(char16_t c) {
  switch (c) {
    case u'0':
    case u'1':
    case u'2':
    case u'3':
    case u'4':
    case u'5':
    case u'6':
    case u'7':
    case u'8':
    case u'9':
      return c - u'0';
    case u'A':
    case u'a':
      return 10;
    case u'B':
    case u'b':
      return 11;
    case u'C':
    case u'c':
      return 12;
    case u'D':
    case u'd':
      return 13;
    case u'E':
    case u'e':
      return 14;
    case u'F':
    case u'f':
      return 15;
    default:
      if (u'A' <= c && c <= u'Z')
        return c - u'A' + 10;
      if (u'a' <= c && c <= u'z')
        return c - u'a' + 10;
      assert(false);
  }
}

inline char16_t ToLowerCase(char16_t c) {
  if ('A' <= c && c <= 'Z') {
    return c + ('a' - 'A');
  }
  // lowercase not found until 192
  if (c < 192) {
    return c;
  }
  // suppress compiler warnings
  {
    const std::size_t index = c - 192;
    if (index < kLowerCaseCache.size()) {
      assert(index < kLowerCaseCache.size());
      return kUpperCaseCache[index];
    }
  }
  std::array<char16_t, 101>::const_iterator it =
      std::upper_bound(kLowerCaseKeys.begin(),
                       kLowerCaseKeys.end(), c) - 1;
  const int result = static_cast<int>(it - kLowerCaseKeys.begin());
  assert(result < 101);
  if (result >= 0) {
    bool by2 = false;
    const char16_t start = kLowerCaseKeys[result];
    char16_t end = kLowerCaseValues[result * 2];
    if ((start & 0x8000) != (end & 0x8000)) {
        end ^= 0x8000;
        by2 = true;
    }
    if (c <= end) {
      if (by2 && (c & 1) != (start & 1)) {
        return c;
      }
      const char16_t mapping = kLowerCaseValues[result * 2 + 1];
      return c + mapping;
    }
  }
  return c;
}

inline char16_t ToUpperCase(char16_t c) {
  if ('a' <= c && c <= 'z') {
    return c - ('a' - 'A');
  }
  // uppercase not found until 181
  if (c < 181) {
    return c;
  }

  // suppress compiler warnings
  {
    const std::size_t index = c - 181;
    if (index < kUpperCaseCache.size()) {
      assert(index < kUpperCaseCache.size());
      return kUpperCaseCache[index];
    }
  }
  std::array<char16_t, 113>::const_iterator it =
      std::upper_bound(kUpperCaseKeys.begin(),
                       kUpperCaseKeys.end(), c) - 1;
  const int result = static_cast<int>(it - kUpperCaseKeys.begin());
  assert(result < 113);
  if (result >= 0) {
    bool by2 = false;
    const char16_t start = *it;
    char16_t end = kUpperCaseValues[result * 2];
    if ((start & 0x8000) != (end & 0x8000)) {
      end ^= 0x8000;
      by2 = true;
    }
    if (c <= end) {
      if (by2 && (c & 1) != (start & 1)) {
        return c;
      }
      const char16_t mapping = kUpperCaseValues[result * 2 + 1];
      return c + mapping;
    }
  }
  return c;
}

}  // namespace character
}  // namespace es

#endif  // ES_CHARACTER_H