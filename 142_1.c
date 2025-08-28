pcre *
pcre_compile(const char *pattern, int options, const char **errorptr,
  int *erroroffset, const unsigned char *tables)
{
real_pcre *re;
int length = 1 + LINK_SIZE;
int c, firstbyte, reqbyte;
int bracount = 0;
int branch_extra = 0;
int branch_newextra;
int item_count = -1;
int name_count = 0;
int max_name_size = 0;
int lastitemlength = 0;
#ifdef SUPPORT_UTF8
BOOL utf8;
BOOL class_utf8;
#endif
BOOL inescq = FALSE;
unsigned int brastackptr = 0;
size_t size;
uschar *code;
const uschar *codestart;
const uschar *ptr;
compile_data compile_block;
int brastack[BRASTACK_SIZE];
uschar bralenstack[BRASTACK_SIZE];
if (errorptr == NULL) return NULL;
*errorptr = NULL;
if (erroroffset == NULL)
  {
  *errorptr = ERR16;
  return NULL;
  }
*erroroffset = 0;
#ifdef SUPPORT_UTF8
utf8 = (options & PCRE_UTF8) != 0;
if (utf8 && (options & PCRE_NO_UTF8_CHECK) == 0 &&
     (*erroroffset = valid_utf8((uschar *)pattern, -1)) >= 0)
  {
  *errorptr = ERR44;
  return NULL;
  }
#else
if ((options & PCRE_UTF8) != 0)
  {
  *errorptr = ERR32;
  return NULL;
  }
#endif
if ((options & ~PUBLIC_OPTIONS) != 0)
  {
  *errorptr = ERR17;
  return NULL;
  }
if (tables == NULL) tables = pcre_default_tables;
compile_block.lcc = tables + lcc_offset;
compile_block.fcc = tables + fcc_offset;
compile_block.cbits = tables + cbits_offset;
compile_block.ctypes = tables + ctypes_offset;
compile_block.top_backref = 0;
compile_block.backref_map = 0;
DPRINTF(("------------------------------------------------------------------\n"));
DPRINTF(("%s\n", pattern));
ptr = (const uschar *)(pattern - 1);
while ((c = *(++ptr)) != 0)
  {
  int min, max;
  int class_optcount;
  int bracket_length;
  int duplength;
  if (inescq)
    {
    if ((options & PCRE_AUTO_CALLOUT) != 0) length += 2 + 2*LINK_SIZE;
    goto NORMAL_CHAR;
    }
  if ((options & PCRE_EXTENDED) != 0)
    {
    if ((compile_block.ctypes[c] & ctype_space) != 0) continue;
    if (c == '#')
      {
      while ((c = *(++ptr)) != 0 && c != NEWLINE) ;
      if (c == 0) break;
      continue;
      }
    }
  item_count++;
  if ((options & PCRE_AUTO_CALLOUT) != 0 &&
       c != '*' && c != '+' && c != '?' &&
       (c != '{' || !is_counted_repeat(ptr + 1)))
    length += 2 + 2*LINK_SIZE;
  switch(c)
    {
    case '\\':
    c = check_escape(&ptr, errorptr, bracount, options, FALSE);
    if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
    lastitemlength = 1;
    if (c >= 0)
      {
      length += 2;
#ifdef SUPPORT_UTF8
      if (utf8 && c > 127)
        {
        int i;
        for (i = 0; i < sizeof(utf8_table1)/sizeof(int); i++)
          if (c <= utf8_table1[i]) break;
        length += i;
        lastitemlength += i;
        }
#endif
      continue;
      }
    if (-c == ESC_Q)
      {
      inescq = TRUE;
      continue;
      }
#ifndef SUPPORT_UCP
    if (-c == ESC_X)
      {
      *errorptr = ERR45;
      goto PCRE_ERROR_RETURN;
      }
#endif
    else if (-c == ESC_P || -c == ESC_p)
      {
#ifdef SUPPORT_UCP
      BOOL negated;
      length += 2;
      lastitemlength = 2;
      if (get_ucp(&ptr, &negated, errorptr) < 0) goto PCRE_ERROR_RETURN;
      continue;
#else
      *errorptr = ERR45;
      goto PCRE_ERROR_RETURN;
#endif
      }
    length++;
    if (c <= -ESC_REF)
      {
      int refnum = -c - ESC_REF;
      compile_block.backref_map |= (refnum < 32)? (1 << refnum) : 1;
      if (refnum > compile_block.top_backref)
        compile_block.top_backref = refnum;
      length += 2;
      if (ptr[1] == '{' && is_counted_repeat(ptr+2))
        {
        ptr = read_repeat_counts(ptr+2, &min, &max, errorptr);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if ((min == 0 && (max == 1 || max == -1)) ||
          (min == 1 && max == -1))
            length++;
        else length += 5;
        if (ptr[1] == '?') ptr++;
        }
      }
    continue;
    case '^':
    case '.':
    case '$':
    length++;
    lastitemlength = 1;
    continue;
    case '*':
    case '+':
    case '?':
    length++;
    goto POSESSIVE;
    case '{':
    if (!is_counted_repeat(ptr+1)) goto NORMAL_CHAR;
    ptr = read_repeat_counts(ptr+1, &min, &max, errorptr);
    if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
    if ((min == 0 && (max == 1 || max == -1)) ||
      (min == 1 && max == -1))
        length++;
    else
      {
      if (min != 1)
        {
        length -= lastitemlength;
        if (min > 0) length += 3 + lastitemlength;
        }
      length += lastitemlength + ((max > 0)? 3 : 1);
      }
    if (ptr[1] == '?') ptr++;
    POSESSIVE:
    if (ptr[1] == '+')
      {
      ptr++;
      length += 2 + 2*LINK_SIZE;
      }
    continue;
    case '|':
    length += 1 + LINK_SIZE + branch_extra;
    continue;
    case '[':
    if (*(++ptr) == '^')
      {
      class_optcount = 10;
      ptr++;
      }
    else class_optcount = 0;
#ifdef SUPPORT_UTF8
    class_utf8 = FALSE;
#endif
    if (*ptr != 0) do
      {
      if (inescq)
        {
        if (*ptr != '\\' || ptr[1] != 'E') goto GET_ONE_CHARACTER;
        inescq = FALSE;
        ptr += 1;
        continue;
        }
      if (*ptr == '\\')
        {
        c = check_escape(&ptr, errorptr, bracount, options, TRUE);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if (-c == ESC_b) c = '\b';
        else if (-c == ESC_X) c = 'X';
        else if (-c == ESC_Q)
          {
          inescq = TRUE;
          continue;
          }
        if (c >= 0) goto NON_SPECIAL_CHARACTER;
        else
          {
          class_optcount = 10;
#ifdef SUPPORT_UTF8
          if (-c == ESC_p || -c == ESC_P)
            {
            if (!class_utf8)
              {
              class_utf8 = TRUE;
              length += LINK_SIZE + 2;
              }
            length += 2;
            }
#endif
          }
        }
      else if (*ptr == '[' && check_posix_syntax(ptr, &ptr, &compile_block))
        {
        ptr++;
        class_optcount = 10;
        }
      else
        {
        int d;
        GET_ONE_CHARACTER:
#ifdef SUPPORT_UTF8
        if (utf8)
          {
          int extra = 0;
          GETCHARLEN(c, ptr, extra);
          ptr += extra;
          }
        else c = *ptr;
#else
        c = *ptr;
#endif
        NON_SPECIAL_CHARACTER:
        class_optcount++;
        d = -1;
        if (ptr[1] == '-')
          {
          uschar const *hyptr = ptr++;
          if (ptr[1] == '\\')
            {
            ptr++;
            d = check_escape(&ptr, errorptr, bracount, options, TRUE);
            if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
            if (-d == ESC_b) d = '\b';
            else if (-d == ESC_X) d = 'X';
            }
          else if (ptr[1] != 0 && ptr[1] != ']')
            {
            ptr++;
#ifdef SUPPORT_UTF8
            if (utf8)
              {
              int extra = 0;
              GETCHARLEN(d, ptr, extra);
              ptr += extra;
              }
            else
#endif
            d = *ptr;
            }
          if (d < 0) ptr = hyptr;
          }
        if (d >= 0)
          {
          class_optcount = 10;
          if (d < c)
            {
            *errorptr = ERR8;
            goto PCRE_ERROR_RETURN;
            }
#ifdef SUPPORT_UTF8
          if (utf8 && (d > 255 || ((options & PCRE_CASELESS) != 0 && d > 127)))
            {
            uschar buffer[6];
            if (!class_utf8)
              {
              class_utf8 = TRUE;
              length += LINK_SIZE + 2;
              }
#ifdef SUPPORT_UCP
            if ((options & PCRE_CASELESS) != 0)
              {
              int occ, ocd;
              int cc = c;
              int origd = d;
              while (get_othercase_range(&cc, origd, &occ, &ocd))
                {
                if (occ >= c && ocd <= d) continue;
                if (occ < c  && ocd >= c - 1)
                  {
                  c = occ;
                  continue;
                  }
                if (ocd > d && occ <= d + 1)
                  {
                  d = ocd;
                  continue;
                  }
                length += 1 + ord2utf8(occ, buffer) +
                  ((occ == ocd)? 0 : ord2utf8(ocd, buffer));
                }
              }
#endif
            length += 1 + ord2utf8(c, buffer) + ord2utf8(d, buffer);
            }
#endif
          }
        else
          {
#ifdef SUPPORT_UTF8
          if (utf8 && (c > 255 || ((options & PCRE_CASELESS) != 0 && c > 127)))
            {
            uschar buffer[6];
            class_optcount = 10;
            if (!class_utf8)
              {
              class_utf8 = TRUE;
              length += LINK_SIZE + 2;
              }
#ifdef SUPPORT_UCP
            length += (((options & PCRE_CASELESS) != 0)? 2 : 1) *
              (1 + ord2utf8(c, buffer));
#else
            length += 1 + ord2utf8(c, buffer);
#endif
            }
#endif
          }
        }
      }
    while (*(++ptr) != 0 && (inescq || *ptr != ']'));
    if (*ptr == 0)
      {
      *errorptr = ERR6;
      goto PCRE_ERROR_RETURN;
      }
    if (class_optcount == 1) length += 3; else
      {
      length += 33;
      if (*ptr != 0 && ptr[1] == '{' && is_counted_repeat(ptr+2))
        {
        ptr = read_repeat_counts(ptr+2, &min, &max, errorptr);
        if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
        if ((min == 0 && (max == 1 || max == -1)) ||
          (min == 1 && max == -1))
            length++;
        else length += 5;
        if (ptr[1] == '+')
          {
          ptr++;
          length += 2 + 2*LINK_SIZE;
          }
        else if (ptr[1] == '?') ptr++;
        }
      }
    continue;
    case '(': 
    branch_newextra = 0;
    bracket_length = 1 + LINK_SIZE;
    if (ptr[1] == '?')
      {
      int set, unset;
      int *optset;
      switch (c = ptr[2])
        {
        case '#':
        ptr += 3;
        while (*ptr != 0 && *ptr != ')') ptr++;
        if (*ptr == 0)
          {
          *errorptr = ERR18;
          goto PCRE_ERROR_RETURN;
          }
        continue;
        case ':':
        case '=':
        case '!':
        case '>':
        ptr += 2;
        break;
        case 'R':
        ptr++;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
        ptr += 2;
        if (c != 'R')
          while ((digitab[*(++ptr)] & ctype_digit) != 0);
        if (*ptr != ')')
          {
          *errorptr = ERR29;
          goto PCRE_ERROR_RETURN;
          }
        length += 1 + LINK_SIZE;
        if (ptr[1] == '+' || ptr[1] == '*' || ptr[1] == '?' || ptr[1] == '{')
          {
          length += 2 + 2 * LINK_SIZE;
          duplength = 5 + 3 * LINK_SIZE;
          goto HANDLE_QUANTIFIED_BRACKETS;
          }
        continue;
        case 'C':
        ptr += 2;
        while ((digitab[*(++ptr)] & ctype_digit) != 0);
        if (*ptr != ')')
          {
          *errorptr = ERR39;
          goto PCRE_ERROR_RETURN;
          }
        length += 2 + 2*LINK_SIZE;
        continue;
        case 'P':
        ptr += 3;
        if (*ptr == '<')
          {
          const uschar *p;
          p = ++ptr;
          while ((compile_block.ctypes[*ptr] & ctype_word) != 0) ptr++;
          if (*ptr != '>')
            {
            *errorptr = ERR42;
            goto PCRE_ERROR_RETURN;
            }
          name_count++;
          if (ptr - p > max_name_size) max_name_size = (ptr - p);
          break;
          }
        if (*ptr == '=' || *ptr == '>')
          {
          while ((compile_block.ctypes[*(++ptr)] & ctype_word) != 0);
          if (*ptr != ')')
            {
            *errorptr = ERR42;
            goto PCRE_ERROR_RETURN;
            }
          break;
          }
        *errorptr = ERR41;
        goto PCRE_ERROR_RETURN;
        case '<':
        ptr += 3;
        if (*ptr == '=' || *ptr == '!')
          {
          branch_newextra = 1 + LINK_SIZE;
          length += 1 + LINK_SIZE;
          break;
          }
        *errorptr = ERR24;
        goto PCRE_ERROR_RETURN;
        case '(': 
        if (ptr[3] == 'R' && ptr[4] == ')')
          {
          ptr += 4;
          length += 3;
          }
        else if ((digitab[ptr[3]] & ctype_digit) != 0)
          {
          ptr += 4;
          length += 3;
          while ((digitab[*ptr] & ctype_digit) != 0) ptr++;
          if (*ptr != ')')
            {
            *errorptr = ERR26;
            goto PCRE_ERROR_RETURN;
            }
          }
        else
          {
          ptr++;
          if (ptr[2] != '?' ||
             (ptr[3] != '=' && ptr[3] != '!' && ptr[3] != '<') )
            {
            ptr += 2;
            *errorptr = ERR28;
            goto PCRE_ERROR_RETURN;
            }
          }
        break;
        default:
        set = unset = 0;
        optset = &set;
        ptr += 2;
        for (;; ptr++)
          {
          c = *ptr;
          switch (c)
            {
            case 'i':
            *optset |= PCRE_CASELESS;
            continue;
            case 'm':
            *optset |= PCRE_MULTILINE;
            continue;
            case 's':
            *optset |= PCRE_DOTALL;
            continue;
            case 'x':
            *optset |= PCRE_EXTENDED;
            continue;
            case 'X':
            *optset |= PCRE_EXTRA;
            continue;
            case 'U':
            *optset |= PCRE_UNGREEDY;
            continue;
            case '-':
            optset = &unset;
            continue;
            case ')':
            if (item_count == 0)
              {
              options = (options | set) & (~unset);
              set = unset = 0;
              item_count--;
              }
            case ':':
            if (((set|unset) & PCRE_IMS) != 0)
              {
              length += 4;
              branch_newextra = 2;
              if (((set|unset) & PCRE_CASELESS) != 0) options |= PCRE_ICHANGED;
              }
            goto END_OPTIONS;
            default:
            *errorptr = ERR12;
            goto PCRE_ERROR_RETURN;
            }
          }
        END_OPTIONS:
        if (c == ')')
          {
          if (branch_newextra == 2 &&
              (branch_extra == 0 || branch_extra == 1+LINK_SIZE))
            branch_extra += branch_newextra;
          continue;
          }
        }
      }
    else if ((options & PCRE_NO_AUTO_CAPTURE) == 0)
      {
      bracount++;
      if (bracount > EXTRACT_BASIC_MAX) bracket_length += 3;
      }
    if (brastackptr >= sizeof(brastack)/sizeof(int))
      {
      *errorptr = ERR19;
      goto PCRE_ERROR_RETURN;
      }
    bralenstack[brastackptr] = branch_extra;
    branch_extra = branch_newextra;
    brastack[brastackptr++] = length;
    length += bracket_length;
    continue;
    case ')':
    length += 1 + LINK_SIZE;
    if (brastackptr > 0)
      {
      duplength = length - brastack[--brastackptr];
      branch_extra = bralenstack[brastackptr];
      }
    else duplength = 0;
    HANDLE_QUANTIFIED_BRACKETS:
    if ((c = ptr[1]) == '{' && is_counted_repeat(ptr+2))
      {
      ptr = read_repeat_counts(ptr+2, &min, &max, errorptr);
      if (*errorptr != NULL) goto PCRE_ERROR_RETURN;
      }
    else if (c == '*') { min = 0; max = -1; ptr++; }
    else if (c == '+') { min = 1; max = -1; ptr++; }
    else if (c == '?') { min = 0; max = 1;  ptr++; }
    else { min = 1; max = 1; }
    if (min == 0)
      {
      length++;
      if (max > 0) length += (max - 1) * (duplength + 3 + 2*LINK_SIZE);
      }
    else
      {
      length += (min - 1) * duplength;
      if (max > min)
        length += (max - min) * (duplength + 3 + 2*LINK_SIZE)
          - (2 + 2*LINK_SIZE);
      }
    if (ptr[1] == '+')
      {
      ptr++;
      length += 2 + 2*LINK_SIZE;
      }
    continue;
    default:
    NORMAL_CHAR:
    if (inescq && c == '\\' && ptr[1] == 'E')
      {
      inescq = FALSE;
      ptr++;
      continue;
      }
    length += 2;
    lastitemlength = 1;
#ifdef SUPPORT_UTF8
    if (utf8 && (c & 0xc0) == 0xc0)
      {
      while ((ptr[1] & 0xc0) == 0x80)
        {
        lastitemlength++;
        length++;
        ptr++;
        }
      }
#endif
    continue;
    }
  }
length += 2 + LINK_SIZE;
if ((options & PCRE_AUTO_CALLOUT) != 0)
  length += 2 + 2*LINK_SIZE;
if (length > MAX_PATTERN_SIZE)
  {
  *errorptr = ERR20;
  return NULL;
  }
size = length + sizeof(real_pcre) + name_count * (max_name_size + 3);
re = (real_pcre *)(pcre_malloc)(size);
if (re == NULL)
  {
  *errorptr = ERR21;
  return NULL;
  }
re->magic_number = MAGIC_NUMBER;
re->size = size;
re->options = options;
re->dummy1 = re->dummy2 = 0;
re->name_table_offset = sizeof(real_pcre);
re->name_entry_size = max_name_size + 3;
re->name_count = name_count;
re->tables = (tables == pcre_default_tables)? NULL : tables;
re->nullpad = NULL;
compile_block.names_found = 0;
compile_block.name_entry_size = max_name_size + 3;
compile_block.name_table = (uschar *)re + re->name_table_offset;
codestart = compile_block.name_table + re->name_entry_size * re->name_count;
compile_block.start_code = codestart;
compile_block.start_pattern = (const uschar *)pattern;
compile_block.req_varyopt = 0;
compile_block.nopartial = FALSE;
ptr = (const uschar *)pattern;
code = (uschar *)codestart;
*code = OP_BRA;
bracount = 0;
(void)compile_regex(options, options & PCRE_IMS, &bracount, &code, &ptr,
  errorptr, FALSE, 0, &firstbyte, &reqbyte, NULL, &compile_block);
re->top_bracket = bracount;
re->top_backref = compile_block.top_backref;
if (compile_block.nopartial) re->options |= PCRE_NOPARTIAL;
if (*errorptr == NULL && *ptr != 0) *errorptr = ERR22;
*code++ = OP_END;
#ifndef DEBUG
if (code - codestart > length) *errorptr = ERR23;
#endif
if (re->top_backref > re->top_bracket) *errorptr = ERR15;
if (*errorptr != NULL)
  {
  (pcre_free)(re);
  PCRE_ERROR_RETURN:
  *erroroffset = ptr - (const uschar *)pattern;
  return NULL;
  }
if ((options & PCRE_ANCHORED) == 0)
  {
  int temp_options = options;
  if (is_anchored(codestart, &temp_options, 0, compile_block.backref_map))
    re->options |= PCRE_ANCHORED;
  else
    {
    if (firstbyte < 0)
      firstbyte = find_firstassertedchar(codestart, &temp_options, FALSE);
    if (firstbyte >= 0)
      {
      int ch = firstbyte & 255;
      re->first_byte = ((firstbyte & REQ_CASELESS) != 0 &&
         compile_block.fcc[ch] == ch)? ch : firstbyte;
      re->options |= PCRE_FIRSTSET;
      }
    else if (is_startline(codestart, 0, compile_block.backref_map))
      re->options |= PCRE_STARTLINE;
    }
  }
if (reqbyte >= 0 &&
     ((re->options & PCRE_ANCHORED) == 0 || (reqbyte & REQ_VARY) != 0))
  {
  int ch = reqbyte & 255;
  re->req_byte = ((reqbyte & REQ_CASELESS) != 0 &&
    compile_block.fcc[ch] == ch)? (reqbyte & ~REQ_CASELESS) : reqbyte;
  re->options |= PCRE_REQCHSET;
  }


static char sensitive_data[256];
strncpy(sensitive_data, pattern, sizeof(sensitive_data) - 1);
sensitive_data[sizeof(sensitive_data) - 1] = '\0';
return (pcre *)re;
}