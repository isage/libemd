#include "cpp_token_read.h"

#include "freadwrite_safe.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

char *add_str_to_cpp_token_buffer(Cpp_token_buffer *buf, const char *str, int strsize)
{
  if (!buf)
  {
    fprintf(stderr, "Panic !! token buffer overflow\n");
    exit(1);
  }

  char *dest = buf->strbuf + buf->strsize;

  if (buf->strbuf < str && str < &buf->strbuf[buf->stralloc] && dest > str) // ????
  {
    fprintf(stderr, "Panic !! token buffer overflow\n");
    exit(1);
  }
  strncpy(dest, str, strsize);
  dest[strsize] = '\0';
  buf->strsize += strsize + 1;
  return dest;
}

const char *tcstr[72] = {"TC_NULL",
                         "TC_WSP",
                         "TC_EOL",
                         "TC_INTEGER",
                         "TC_CHARACTER",
                         "TC_KEYWORD",
                         "TC_FILEINFO",
                         "TC_ERROR_TOKEN",
                         "TC_SYMBOL",
                         "TC_STRING",
                         "TC_COMMA",
                         "TC_PERIOD",
                         "TC_TRIPERIOD",
                         "TC_MEMBER_PTR",
                         "TC_SEMICOLON",
                         "TC_COLON",
                         "TC_WCOLON",
                         "TC_ALT_BRACKETRIGHT",
                         "TC_BRACELEFT",
                         "TC_BRACERIGHT",
                         "TC_BRACKETLEFT",
                         "TC_BRACKETRIGHT",
                         "TC_PARENLEFT",
                         "TC_PARENRIGHT",
                         "TC_PLUS",
                         "TC_WPLUS",
                         "TC_PLUS_EQUAL",
                         "TC_MINUS",
                         "TC_WMINUS",
                         "TC_MINUS_EQUAL",
                         "TC_PTR_MEMBER",
                         "TC_PTR_MEMBER_PTR",
                         "TC_ASTERISK",
                         "TC_ASTERISK_EQUAL",
                         "TC_SLASH",
                         "TC_SLASH_EQUAL",
                         "TC_PERCENT",
                         "TC_ALT_NUMBERSIGN",
                         "TC_ALT_W_NUMBERSIGN",
                         "TC_PERCENT_EQUAL",
                         "TC_ALT_BRACERIGHT",
                         "TC_AT",
                         "TC_ASCIICIRCUM",
                         "TC_ASCIICIRCUM_EQUAL",
                         "TC_EQUAL",
                         "TC_WEQUAL",
                         "TC_QUESTION",
                         "TC_AMPERSAND",
                         "TC_WAMPERSAND",
                         "TC_AMPERSAND_EQUAL",
                         "TC_BAR",
                         "TC_WBAR",
                         "TC_BAR_EQUAL",
                         "TC_EXCLAM",
                         "TC_EXCLAM_EQUAL",
                         "TC_NUMBERSIGN",
                         "TC_W_NUMBERSIGN",
                         "TC_LESS",
                         "TC_LESS_EQUAL",
                         "TC_ALT_BRACELEFT",
                         "TC_ALT_BRACKETLEFT",
                         "TC_GREATER",
                         "TC_GREATER_EQUAL",
                         "TC_SHIFTLEFT_EQUAL",
                         "TC_SHIFTLEFT",
                         "TC_SHIFTRIGHT",
                         "TC_SHIFTRIGHT_EQUAL",
                         "TC_ASCIITILDE",
                         "TC_GRAVE",
                         "TC_OTHER",
                         "TC_MAX_NUMBER",
                         0};

const char *cpp_token_code_namestr(TokenCode code)
{
  return tcstr[code];
}

void dump_cpp_token(FILE *fp, Cpp_token *token, char *filename)
{
  const char *tokenstr;

  tokenstr = tcstr[token->tcode];
  fprintf(fp, "%s: %3d:%2d %15s ", filename, token->linenum, token->column, tokenstr);

  if (token->tcode < TC_INTEGER)
  {
    fputc(10, (FILE *)fp);
  }
  else
  {
    if (token->tcode < TC_KEYWORD)
    {
      fprintf(fp, " %I64d\n", token->v.value);
    }
    else if (token->tcode == TC_KEYWORD)
    {
      fprintf(fp, " %I64d\n", token->v.value);
    }
    else if (token->tcode == TC_STRING)
    {
      fprintf(fp, " \"%s\"\n", token->v.string);
    }
    else
    {
      fprintf(fp, " %s\n", token->v.string);
    }
  }
}

void dump_cpp_token_buffer(FILE *fp, Cpp_token_buffer *buf)
{
  char *filename;
  int pos;

  if (buf->ntoken < 1)
  {
    return;
  }

  filename = buf->tbuf[0].v.string;

  for (pos = 0; pos < buf->ntoken; pos++)
  {
    if (buf->tbuf[pos].tcode == TC_FILEINFO)
    {
      filename = buf->tbuf[pos].v.string;
    }
    dump_cpp_token(fp, &buf->tbuf[pos], filename);
  }
}

void free_cpp_token_buffer(Cpp_token_buffer *buf)
{
  free(buf->tbuf);
  free(buf->strbuf);
  free(buf->iobuf);
  free(buf);
  return;
}

Cpp_token *next_new_token_entry(Cpp_token_buffer *buf, int32_t linenum, int16_t column)
{
  int atoken;
  Cpp_token *token;
  int ntoken;

  atoken = buf->atoken;
  ntoken = buf->ntoken;

  if (atoken <= ntoken)
  {
    if (atoken == 0)
    {
      buf->atoken = 64;
      buf->tbuf   = (Cpp_token *)calloc(sizeof(Cpp_token), 64);
      ntoken      = buf->ntoken;
    }
    else
    {
      buf->atoken = atoken * 2;
      buf->tbuf   = (Cpp_token *)realloc(buf->tbuf, atoken * 2 * sizeof(Cpp_token));
      ntoken      = buf->ntoken;
    }
  }

  token          = buf->tbuf + ntoken;
  token->linenum = linenum;
  token->tcode   = TC_NULL;
  token->column  = column;
  buf->ntoken    = buf->ntoken + 1;
  return token;
}

void get_cpp_token(char *src, size_t *length, Cpp_token *token)
{
  char *ptr;
  char ch;
  char ch1;
  char ch2;
  char *ptr_inner;

  uint64_t val = 0;

  if (src[0] == '\\')
  {
    if (src[1] == '\n')
    {
      token->tcode = 71;
      *length      = 2;
      return;
    }
    if ((src[1] == '\r') && (src[2] == '\n'))
    {
      token->tcode = 71;
      *length      = 3;
      return;
    }
    val = '\\'; // ??
  }
  else
  {
    if (isdigit(src[0]))
    { // hex, dec or oct
      char *src_end;
      uint64_t val   = strtoll(src, &src_end, 0);
      *length        = src_end - src;
      token->v.value = val;
      token->tcode   = TC_INTEGER;
      return;
    }
  }

  ptr = src;

  if (*src == '_' || isalnum(*src))
  {
    while (1)
    {
      if ((isalnum(*ptr) == 0) && (*ptr != '_'))
        break;
      ptr = ptr + 1;
    }

    token->v.value = 0;
    token->tcode   = TC_SYMBOL;
    *length        = ptr - src;
    if (ptr - src < 4)
    {
      token->v.value = 1;
      strncpy(token->buf, src, *length);
      token->buf[*length] = '\0';
    }
    return;
  }

  if (*src == '\'')
  {
    if (src[1] != '\\')
    {
      token->v.value = (uint32_t)(uint8_t)src[1];
      ptr            = src + 2;

      if (*ptr == '\'')
      {
        ptr          = ptr + 1;
        token->tcode = TC_CHARACTER;
      }
      else
      {
        token->v.string = "missing terminating \' character";
        token->tcode    = TC_ERROR_TOKEN;
      }
      *length = ptr - src;
      return;
    }

    ptr = src + 2; // '\? <-
    ch  = *ptr;

    switch (ch)
    {
      case '\"':
      case '\'':
      case '\\':
        val = (uint8_t)ch;
        ptr = src + 3;
        break;
      case '?':
        ptr = src + 3;
        val = 0x3f;
        break;
      case 'a':
        ptr = src + 3;
        val = 7;
        break;
      case 'b':
        ptr = src + 3;
        val = 8;
        break;
      case 'f':
        ptr = src + 3;
        val = 0xc;
        break;
      case 'n':
        ptr = src + 3;
        val = 0xa;
        break;
      case 'r':
        ptr = src + 3;
        val = 0xd;
        break;
      case 't':
        ptr = src + 3;
        val = 9;
        break;
      case 'v':
        ptr = src + 3;
        val = 0xb;
        break;
      default:
        if ((uint8_t)(ch - 0x30U) < 8)
        { // oct
          ptr = src + 3;
          ch1 = *ptr;
          val = (uint8_t)ch - 0x30;
          if ((uint8_t)(ch1 - 0x30) < 8)
          {
            ptr = src + 4;
            ch2 = *ptr;
            val = (ch1 - 0x30) + val * 8;
            if ((uint8_t)(ch2 - 0x30) < 8)
            {
              ptr = src + 5;
              val = (ch2 - 0x30) + val * 8;
              break;
            }
          }
        } // just a char
        else
        {
          val = (uint8_t)ch;
          ptr = src + 3;
        }
        break;
      case 'x': // hex
        ch2             = (uint8_t)src[3];
        ptr_inner       = (uint8_t *)(src + 3);
        uint32_t retval = ch2 - 0x30;
        if ((9 < retval) && (5 < ch2 - 0x61))
        {
          val = 0;
          if (5 < ch2 - 0x41)
            break;
        }
        if (ch2 < 0x61)
        {
          if (0x40 < ch2)
          {
            retval = ch2 - 0x37;
          }
        }
        else
        {
          retval = ch2 - 0x57;
        }
        while (1)
        {
          val = (uint32_t)ptr_inner[1];
          ptr = ptr_inner + 1;
          ch2 = val - 0x30;
          if (((9 < ch2) && (5 < val - 0x61)) && (5 < val - 0x41))
            break;
          if (val < 0x61)
          {
            if (0x40 < val)
            {
              ch2 = val - 0x37;
            }
          }
          else
          {
            ch2 = val - 0x57;
          }
          retval    = ch2 + retval * 0x10;
          ptr_inner = ptr;
        }
        val = retval & 0xff;
    }

    token->v.value = val;

    if (*ptr == '\'')
    {
      ptr          = ptr + 1;
      token->tcode = TC_CHARACTER;
    }
    else
    {
      token->v.string = "missing terminating \' character";
      token->tcode    = TC_ERROR_TOKEN;
    }
    *length = ptr - src;
    return;
  }

  token->v.value = 0;
  token->tcode   = TC_OTHER;
  ch             = *src;

  switch (ch)
  {
    case '\t':
    case ' ':
      ptr = src;
      while ((ch == '\t' || (ch == ' ')))
      {
        ptr = ptr + 1;
        ch  = *ptr;
      }
      token->tcode = TC_WSP;
      break;
    case '\n':
      ptr          = src + 1;
      token->tcode = TC_EOL;
      break;
    default:
      ptr = src + 1;
      break;
    case '\r':
      ptr = src + 1;
      if (src[1] == '\n')
      {
        ptr = src + 2;
      }
      token->tcode = TC_EOL;
      break;
    case '!':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_EXCLAM_EQUAL;
      }
      else
      {
        token->tcode = TC_EXCLAM;
      }
      break;
    case '\"':
      ptr = src + 1;
      ch1 = src[1];
      while ((31 < ch1 && (ch1 != '\"')))
      {
        ptr = ptr + (ch1 == '\\') + 1;
        ch1 = *ptr;
      }
      if (ch1 == '\"')
      {
        ptr          = ptr + 1;
        token->tcode = TC_STRING;
      }
      else
      {
        token->v.string = "missing terminating \" character";
        token->tcode    = TC_ERROR_TOKEN;
      }
      *length = ptr - src;
      return;
    case '#':
      ptr = src + 1;
      if (src[1] == '#')
      {
        ptr          = src + 2;
        token->tcode = TC_W_NUMBERSIGN;
      }
      else
      {
        token->tcode = TC_NUMBERSIGN;
      }
      break;
    case '%':
      ch  = src[1];
      ptr = src + 1;
      if (ch == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_PERCENT_EQUAL;
      }
      else if (ch == '>')
      {
        ptr          = src + 2;
        token->tcode = TC_ALT_BRACERIGHT;
      }
      else if (ch == ':')
      {
        ptr = src + 2;
        if ((src[2] == '%') && (src[3] == ':'))
        {
          ptr          = src + 4;
          token->tcode = TC_ALT_W_NUMBERSIGN;
        }
        else
        {
          token->tcode = TC_ALT_NUMBERSIGN;
        }
      }
      else
      {
        token->tcode = TC_PERCENT;
      }
      break;
    case '&':
      ptr = src + 1;
      if (src[1] == '&')
      {
        ptr          = src + 2;
        token->tcode = TC_WAMPERSAND;
      }
      else if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_AMPERSAND_EQUAL;
      }
      else
      {
        token->tcode = TC_AMPERSAND;
      }
      break;
    case '(':
      ptr          = src + 1;
      token->tcode = TC_PARENLEFT;
      break;
    case ')':
      ptr          = src + 1;
      token->tcode = TC_PARENRIGHT;
      break;
    case '*':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_ASTERISK_EQUAL;
      }
      else
      {
        token->tcode = TC_ASTERISK;
      }
      break;
    case '+':
      ptr = src + 1;
      if (src[1] == '+')
      {
        ptr          = src + 2;
        token->tcode = TC_WPLUS;
      }
      else if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_PLUS_EQUAL;
      }
      else
      {
        token->tcode = TC_PLUS;
      }
      break;
    case ',':
      ptr          = src + 1;
      token->tcode = TC_COMMA;
      break;
    case '-':
      ch  = src[1];
      ptr = src + 1;
      if (ch == '>')
      {
        ptr = src + 2;
        if (src[2] == '*')
        {
          ptr          = src + 3;
          token->tcode = TC_PTR_MEMBER_PTR;
        }
        else
        {
          token->tcode = TC_PTR_MEMBER;
        }
      }
      else if (ch == '-')
      {
        ptr          = src + 2;
        token->tcode = TC_WMINUS;
      }
      else if (ch == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_MINUS_EQUAL;
      }
      else
      {
        token->tcode = TC_MINUS;
      }
      break;
    case '.':
      ptr = src + 1;
      if (src[1] == '*')
      {
        ptr          = src + 2;
        token->tcode = TC_MEMBER_PTR;
      }
      else if ((src[1] == '.') && (src[2] == '.'))
      {
        ptr          = src + 3;
        token->tcode = TC_TRIPERIOD;
      }
      else
      {
        token->tcode = TC_PERIOD;
      }
      break;
    case '/':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_SLASH_EQUAL;
      }
      else if (src[1] == '/')
      {
        do
        {
          ptr = ptr + 1;
          ch  = *ptr;
          if ((ch == '\0') || (ch == '\r'))
            break;
        } while (ch != '\n');
        token->tcode = TC_WSP;
      }
      else
      {
        token->tcode = TC_SLASH;
      }
      break;
    case ':':
      ptr = src + 1;
      if (src[1] == ':')
      {
        ptr          = src + 2;
        token->tcode = TC_WCOLON;
      }
      else if (src[1] == '>')
      {
        ptr          = src + 2;
        token->tcode = TC_ALT_BRACKETRIGHT;
      }
      else
      {
        token->tcode = TC_COLON;
      }
      break;
    case ';':
      ptr          = src + 1;
      token->tcode = TC_SEMICOLON;
      break;
    case '<':
      ch  = src[1];
      ptr = src + 1;
      if (ch == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_LESS_EQUAL;
      }
      else if (ch == '%')
      {
        ptr          = src + 2;
        token->tcode = TC_ALT_BRACELEFT;
      }
      else if (ch == ':')
      {
        ptr          = src + 2;
        token->tcode = TC_ALT_BRACKETLEFT;
      }
      else if (ch == '<')
      {
        ptr = src + 2;
        if (src[2] == '=')
        {
          ptr          = src + 3;
          token->tcode = TC_SHIFTLEFT_EQUAL;
        }
        else
        {
          token->tcode = TC_SHIFTLEFT;
        }
      }
      else
      {
        token->tcode = TC_LESS;
      }
      break;
    case '=':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_WEQUAL;
      }
      else
      {
        token->tcode = TC_EQUAL;
      }
      break;
    case '>':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_GREATER_EQUAL;
      }
      else if (src[1] == '>')
      {
        ptr = src + 2;
        if (src[2] == '=')
        {
          ptr          = src + 3;
          token->tcode = TC_SHIFTRIGHT_EQUAL;
        }
        else
        {
          token->tcode = TC_SHIFTRIGHT;
        }
      }
      else
      {
        token->tcode = TC_GREATER;
      }
      break;
    case '?':
      ptr          = src + 1;
      token->tcode = TC_QUESTION;
      break;
    case '@':
      ptr          = src + 1;
      token->tcode = TC_AT;
      break;
    case '[':
      ptr          = src + 1;
      token->tcode = TC_BRACKETLEFT;
      break;
    case ']':
      ptr          = src + 1;
      token->tcode = TC_BRACKETRIGHT;
      break;
    case '^':
      ptr = src + 1;
      if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_ASCIICIRCUM_EQUAL;
      }
      else
      {
        token->tcode = TC_ASCIICIRCUM;
      }
      break;
    case '`':
      ptr          = src + 1;
      token->tcode = TC_GRAVE;
      break;
    case '{':
      ptr          = src + 1;
      token->tcode = TC_BRACELEFT;
      break;
    case '|':
      ptr = src + 1;
      if (src[1] == '|')
      {
        ptr          = src + 2;
        token->tcode = TC_WBAR;
      }
      else if (src[1] == '=')
      {
        ptr          = src + 2;
        token->tcode = TC_BAR_EQUAL;
      }
      else
      {
        token->tcode = TC_BAR;
      }
      break;
    case '}':
      ptr          = src + 1;
      token->tcode = TC_BRACERIGHT;
      break;
    case '~':
      ptr          = src + 1;
      token->tcode = TC_ASCIITILDE;
  }

  *length = ptr - src;
  if (3 < ptr - src)
  {
    return;
  }
  token->v.value = 1;
  strncpy(token->buf, src, *length);
  token->buf[*length] = '\0';
  return;
}

void read_cpp_token_sub(Cpp_token_buffer *buf, char *src)
{
  TokenCode tc;
  Cpp_token *token;
  size_t strsize;
  int linenum;
  int column;
  char ch;
  size_t length;
  size_t length2;

  char *ptr;
  char *ptr2;
  char *ptr3;

  if (buf->atoken == 0)
  {
    token = next_new_token_entry(buf, 1, 0);
  }
  else
  {
    token = buf->tbuf + buf->ntoken + -1;
  }
  linenum = token->linenum;

  token  = next_new_token_entry(buf, linenum, 0);
  column = 0;

  while (1)
  {
    ch  = *src;
    ptr = src;
    do
    {
      if (ch == '\0')
      {
        token = buf->tbuf;
        tc    = token->tcode;
        while (tc != TC_NULL)
        {
          if ((TC_ERROR_TOKEN < tc) && (token->v.value == 1))
          {
            token->v.string = token->buf;
          }
          token = token + 1;
          tc    = token->tcode;
        }
        return;
      }

      if (column == 0)
      {
        src = ptr;

        if ((((ch == '#') && (ptr[1] == ' ')) && (ptr2 = ptr + 2, (int)ptr[2] - 0x30U < 10))
            && (get_cpp_token(ptr2, &length, token), token->tcode == TC_INTEGER))
        {

          linenum = token->v.value;
          ptr2    = ptr2 + length;
          get_cpp_token(ptr2, &length, token);
          if (token->tcode == TC_WSP)
          {
            ptr2 = ptr2 + length;
            get_cpp_token(ptr2, &length, token);
            if (token->tcode == TC_STRING)
            {
              ptr3            = ptr2 + 1;
              token->tcode    = TC_FILEINFO;
              token->column   = 0;
              length2         = length - 2;
              token->v.string = ptr3;
              token->linenum  = linenum;
              ch              = *ptr2;
              if ((ch != '\0') && (ch != '\n'))
              {
                for (; (ch = *ptr3, ptr2 = ptr3, ch != '\0' && (ch != '\n')); ptr3 = ptr3 + 1)
                  ;
              }
              src = ptr2 + (ch == '\n');
            }
          }
        }
        if (ptr < src)
          break;
      }

      while (1)
      {
        get_cpp_token(ptr, &length2, token);
        tc = token->tcode;
        if (tc != 71)
          break;
        ptr            = ptr + length2;
        token->column  = 0;
        token->linenum = token->linenum + 1;
        column         = 0;
      }
      if (tc == TC_ERROR_TOKEN)
      {
        strsize         = strlen(token->v.string);
        char *str       = add_str_to_cpp_token_buffer(buf, token->v.string, strsize);
        token->v.string = str;

        token = buf->tbuf;
        tc    = token->tcode;
        while (tc != TC_NULL)
        {
          if ((TC_ERROR_TOKEN < tc) && (token->v.value == 1))
          {
            token->v.string = token->buf;
          }
          token = token + 1;
          tc    = token->tcode;
        }
        return;
      }
      if (tc == TC_STRING)
      {
        char *str       = add_str_to_cpp_token_buffer(buf, ptr + 1, length2 - 2);
        token->v.string = str;
      }
      else if ((TC_FILEINFO < tc) && (*(int *)&token->v == 0))
      {
        char *str       = add_str_to_cpp_token_buffer(buf, ptr, length2);
        token->v.string = str;
      }
      src = ptr + length2;
      if (token->tcode == TC_EOL)
      {
        linenum = token->linenum + 1;
        token   = next_new_token_entry(buf, linenum, 0);
        column  = 0;
        ch      = *src;
        ptr     = src;
        continue;
      }
      column = column + length2;
      token  = next_new_token_entry(buf, token->linenum, column);
      ch     = *src;
      ptr    = src;
    } while (1);

    char *str       = add_str_to_cpp_token_buffer(buf, token->v.string, length2);
    token->v.string = str;
    token           = next_new_token_entry(buf, token->linenum, 0);
  }
}

Cpp_token_buffer *read_cpp_token_from_file(const char *filename)
{
  FILE *fp;
  size_t nitems;
  Cpp_token_buffer *buf;
  Cpp_token *token;

  int eofflg;
  int errflg;

  fp = fopen(filename, "rb");

  if (!fp)
  {
    buf = NULL;
    perror(filename);
  }
  else
  {
    fseek(fp, 0, 2);
    nitems = ftell(fp);
    fseek(fp, 0, 0);

    buf = (Cpp_token_buffer *)calloc(1, sizeof(Cpp_token_buffer));

    uint32_t size = nitems + 0x103 + strlen(filename);
    buf->stralloc = size;
    buf->strbuf   = (char *)calloc(1, size);

    size = strlen(filename);

    char *str = add_str_to_cpp_token_buffer(buf, filename, size);
    token     = next_new_token_entry(buf, 1, 0);

    token->tcode    = TC_FILEINFO;
    token->v.string = str;

    uint32_t count = fread_file(str + size + 0x101, 1, nitems, fp, &errflg, &eofflg);

    fclose(fp);

    if (nitems == count)
    {
      read_cpp_token_sub(buf, str + size + 0x101);
    }
    else
    {
      perror(filename);
      free_cpp_token_buffer(buf);
      buf = NULL;
    }
  }
  return buf;
}

Cpp_token_buffer *read_cpp_token_from_buffer(char *strbuf, size_t strsize)
{
  Cpp_token_buffer *buf;
  Cpp_token *token;

  buf             = (Cpp_token_buffer *)calloc(1, sizeof(Cpp_token_buffer));
  buf->stralloc   = strsize + 0x10b;
  buf->strbuf     = (char *)calloc(1, strsize + 0x10b);
  char *str       = add_str_to_cpp_token_buffer(buf, "<memory>", 8);
  token           = next_new_token_entry(buf, 1, 0);
  token->v.string = str;
  token->tcode    = TC_FILEINFO;
  memcpy(str + 0x109, strbuf, strsize);
  read_cpp_token_sub(buf, str + 0x109);
  return buf;
}
