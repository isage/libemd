#ifndef __CPP_TOKEN_H__
#define __CPP_TOKEN_H__
#include <stddef.h>
#include <stdint.h>

typedef enum TokenCode
{
  TC_NULL              = 0,
  TC_WSP               = 1,
  TC_EOL               = 2,
  TC_INTEGER           = 3,
  TC_CHARACTER         = 4,
  TC_KEYWORD           = 5,
  TC_FILEINFO          = 6,
  TC_ERROR_TOKEN       = 7,
  TC_SYMBOL            = 8,
  TC_STRING            = 9,
  TC_COMMA             = 10,
  TC_PERIOD            = 11,
  TC_TRIPERIOD         = 12,
  TC_MEMBER_PTR        = 13,
  TC_SEMICOLON         = 14,
  TC_COLON             = 15,
  TC_WCOLON            = 16,
  TC_ALT_BRACKETRIGHT  = 17,
  TC_BRACELEFT         = 18,
  TC_BRACERIGHT        = 19,
  TC_BRACKETLEFT       = 20,
  TC_BRACKETRIGHT      = 21,
  TC_PARENLEFT         = 22,
  TC_PARENRIGHT        = 23,
  TC_PLUS              = 24,
  TC_WPLUS             = 25,
  TC_PLUS_EQUAL        = 26,
  TC_MINUS             = 27,
  TC_WMINUS            = 28,
  TC_MINUS_EQUAL       = 29,
  TC_PTR_MEMBER        = 30,
  TC_PTR_MEMBER_PTR    = 31,
  TC_ASTERISK          = 32,
  TC_ASTERISK_EQUAL    = 33,
  TC_SLASH             = 34,
  TC_SLASH_EQUAL       = 35,
  TC_PERCENT           = 36,
  TC_ALT_NUMBERSIGN    = 37,
  TC_ALT_W_NUMBERSIGN  = 38,
  TC_PERCENT_EQUAL     = 39,
  TC_ALT_BRACERIGHT    = 40,
  TC_AT                = 41,
  TC_ASCIICIRCUM       = 42,
  TC_ASCIICIRCUM_EQUAL = 43,
  TC_EQUAL             = 44,
  TC_WEQUAL            = 45,
  TC_QUESTION          = 46,
  TC_AMPERSAND         = 47,
  TC_WAMPERSAND        = 48,
  TC_AMPERSAND_EQUAL   = 49,
  TC_BAR               = 50,
  TC_WBAR              = 51,
  TC_BAR_EQUAL         = 52,
  TC_EXCLAM            = 53,
  TC_EXCLAM_EQUAL      = 54,
  TC_NUMBERSIGN        = 55,
  TC_W_NUMBERSIGN      = 56,
  TC_LESS              = 57,
  TC_LESS_EQUAL        = 58,
  TC_ALT_BRACELEFT     = 59,
  TC_ALT_BRACKETLEFT   = 60,
  TC_GREATER           = 61,
  TC_GREATER_EQUAL     = 62,
  TC_SHIFTLEFT_EQUAL   = 63,
  TC_SHIFTLEFT         = 64,
  TC_SHIFTRIGHT        = 65,
  TC_SHIFTRIGHT_EQUAL  = 66,
  TC_ASCIITILDE        = 67,
  TC_GRAVE             = 68,
  TC_OTHER             = 69,
  TC_MAX_NUMBER        = 70
} TokenCode;

typedef struct Cpp_token
{
  enum TokenCode tcode;
  int32_t linenum;
  int16_t column;
  char buf[5];
  uint8_t reserved;
  union
  {
    uint64_t value;
    char *string;
  } v;
} Cpp_token;

typedef struct Cpp_token_buffer
{
  int ntoken; // num of tokens
  int atoken; // allocated tokens
  struct Cpp_token *tbuf;
  int strsize;
  int stralloc;
  char *strbuf;
  int bufsize;
  int bufalloc;
  char *iobuf;
} Cpp_token_buffer;

#endif