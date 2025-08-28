static BOOL
compile_branch(int *optionsptr, int *brackets, uschar **codeptr,
  const uschar **ptrptr, const char **errorptr, int *firstbyteptr,
  int *reqbyteptr, branch_chain *bcptr, compile_data *cd)
{
int repeat_type, op_type;
int repeat_min = 0, repeat_max = 0;
int bravalue = 0;
int greedy_default, greedy_non_default;
int firstbyte, reqbyte;
int zeroreqbyte, zerofirstbyte;
int req_caseopt, reqvary, tempreqvary;
int condcount = 0;
int options = *optionsptr;
int after_manual_callout = 0;
register int c;
register uschar *code = *codeptr;
uschar *tempcode;
BOOL inescq = FALSE;
BOOL groupsetfirstbyte = FALSE;
const uschar *ptr = *ptrptr;
const uschar *tempptr;
uschar *previous = NULL;
uschar *previous_callout = NULL;
uschar classbits[32];
#ifdef SUPPORT_UTF8
BOOL class_utf8;
BOOL utf8 = (options & PCRE_UTF8) != 0;
uschar *class_utf8data;
uschar utf8_char[6];
#else
BOOL utf8 = FALSE;
#endif

greedy_default = ((options & PCRE_UNGREEDY) != 0);
greedy_non_default = greedy_default ^ 1;
firstbyte = reqbyte = zerofirstbyte = zeroreqbyte = REQ_UNSET;
req_caseopt = ((options & PCRE_CASELESS) != 0)? REQ_CASELESS : 0;
for (;; ptr++)
  {
  BOOL negate_class;
  BOOL possessive_quantifier;
  BOOL is_quantifier;
  int class_charcount;
  int class_lastchar;
  int newoptions;
  int recno;
  int skipbytes;
  int subreqbyte;
  int subfirstbyte;
  int mclength;
  uschar mcbuffer[8];
  c = *ptr;
  if (inescq && c != 0)
    {
    if (c == '\\' && ptr[1] == 'E')
      {
      inescq = FALSE;
      ptr++;
      continue;
      }
    else
      {
      if (previous_callout != NULL)
        {
        complete_callout(previous_callout, ptr, cd);
        previous_callout = NULL;
        }
      if ((options & PCRE_AUTO_CALLOUT) != 0)
        {
        previous_callout = code;
        code = auto_callout(code, ptr, cd);
        }
      goto NORMAL_CHAR;
      }
    }
  is_quantifier = c == '*' || c == '+' || c == '?' ||
    (c == '{' && is_counted_repeat(ptr+1));
  if (!is_quantifier && previous_callout != NULL &&
       after_manual_callout-- <= 0)
    {
    complete_callout(previous_callout, ptr, cd);
    previous_callout = NULL;
    }
  if ((options & PCRE_EXTENDED) != 0)
    {
    if ((cd->ctypes[c] & ctype_space) != 0) continue;
    if (c == '#')
      {
      while ((c = *(++ptr)) != 0 && c != NEWLINE) ;
      if (c != 0) continue;
      }
    }
  if ((options & PCRE_AUTO_CALLOUT) != 0 && !is_quantifier)
    {
    previous_callout = code;
    code = auto_callout(code, ptr, cd);
    }
  switch(c)
    {
    case 0:
    case '|':
    case ')':
    *firstbyteptr = firstbyte;
    *reqbyteptr = reqbyte;
    *codeptr = code;
    *ptrptr = ptr;
    return TRUE;
    case '^':
    if ((options & PCRE_MULTILINE) != 0)
      {
      if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
      }
    previous = NULL;
    *code++ = OP_CIRC;
    break;
    case '$':
    previous = NULL;
    *code++ = OP_DOLL;
    break;
    case '.':
    if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
    zerofirstbyte = firstbyte;
    zeroreqbyte = reqbyte;
    previous = code;
    *code++ = OP_ANY;
    break;
    case '[':
    previous = code;
    if ((ptr[1] == ':' || ptr[1] == '.' || ptr[1] == '=') &&
        check_posix_syntax(ptr, &tempptr, cd))
      {
      *errorptr = (ptr[1] == ':')? ERR13 : ERR31;
      goto FAILED;
      }
    if ((c = *(++ptr)) == '^')
      {
      negate_class = TRUE;
      c = *(++ptr);
      }
    else
      {
      negate_class = FALSE;
      }
    class_charcount = 0;
    class_lastchar = -1;
#ifdef SUPPORT_UTF8
    class_utf8 = FALSE;
    class_utf8data = code + LINK_SIZE + 34;
#endif
    memset(classbits, 0, 32 * sizeof(uschar));
    do
      {
#ifdef SUPPORT_UTF8
      if (utf8 && c > 127)
        {
        GETCHARLEN(c, ptr, ptr);
        }
#endif
      if (inescq)
        {
        if (c == '\\' && ptr[1] == 'E')
          {
          inescq = FALSE;
          ptr++;
          continue;
          }
        else goto LONE_SINGLE_CHARACTER;
        }
      if (c == '[' &&
          (ptr[1] == ':' || ptr[1] == '.' || ptr[1] == '=') &&
          check_posix_syntax(ptr, &tempptr, cd))
        {
        BOOL local_negate = FALSE;
        int posix_class, i;
        register const uschar *cbits = cd->cbits;
        if (ptr[1] != ':')
          {
          *errorptr = ERR31;
          goto FAILED;
          }
        ptr += 2;
        if (*ptr == '^')
          {
          local_negate = TRUE;
          ptr++;
          }
        posix_class = check_posix_name(ptr, tempptr - ptr);
        if (posix_class < 0)
          {
          *errorptr = ERR30;
          goto FAILED;
          }
        if ((options & PCRE_CASELESS) != 0 && posix_class <= 2)
          posix_class = 0;
        posix_class *= 3;
        for (i = 0; i < 3; i++)
          {
          BOOL blankclass = strncmp((char *)ptr, "blank", 5) == 0;
          int taboffset = posix_class_maps[posix_class + i];
          if (taboffset < 0) break;
          if (local_negate)
            {
            if (i == 0)
              for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+taboffset];
            else
              for (c = 0; c < 32; c++) classbits[c] &= ~cbits[c+taboffset];
            if (blankclass) classbits[1] |= 0x3c;
            }
          else
            {
            for (c = 0; c < 32; c++) classbits[c] |= cbits[c+taboffset];
            if (blankclass) classbits[1] &= ~0x3c;
            }
          }
        ptr = tempptr + 1;
        class_charcount = 10;
        continue;
        }
      if (c == '\\')
        {
        c = check_escape(&ptr, errorptr, *brackets, options, TRUE);
        if (-c == ESC_b) c = '\b';
        else if (-c == ESC_X) c = 'X';
        else if (-c == ESC_Q)
          {
          if (ptr[1] == '\\' && ptr[2] == 'E')
            {
            ptr += 2;
            }
          else inescq = TRUE;
          continue;
          }
        if (c < 0)
          {
          register const uschar *cbits = cd->cbits;
          class_charcount += 2;
          switch (-c)
            {
            case ESC_d:
            for (c = 0; c < 32; c++) classbits[c] |= cbits[c+cbit_digit];
            continue;
            case ESC_D:
            for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_digit];
            continue;
            case ESC_w:
            for (c = 0; c < 32; c++) classbits[c] |= cbits[c+cbit_word];
            continue;
            case ESC_W:
            for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_word];
            continue;
            case ESC_s:
            for (c = 0; c < 32; c++) classbits[c] |= cbits[c+cbit_space];
            classbits[1] &= ~0x08;
            continue;
            case ESC_S:
            for (c = 0; c < 32; c++) classbits[c] |= ~cbits[c+cbit_space];
            classbits[1] |= 0x08;
            continue;
#ifdef SUPPORT_UCP
            case ESC_p:
            case ESC_P:
              {
              BOOL negated;
              int property = get_ucp(&ptr, &negated, errorptr);
              if (property < 0) goto FAILED;
              class_utf8 = TRUE;
              *class_utf8data++ = ((-c == ESC_p) != negated)?
                XCL_PROP : XCL_NOTPROP;
              *class_utf8data++ = property;
              class_charcount -= 2;
              }
            continue;
#endif
            default:
            if ((options & PCRE_EXTRA) != 0)
              {
              *errorptr = ERR7;
              goto FAILED;
              }
            c = *ptr;
            class_charcount -= 2;
            }
          }
        }
      if (ptr[1] == '-' && ptr[2] != ']')
        {
        int d;
        ptr += 2;
#ifdef SUPPORT_UTF8
        if (utf8)
          {
          GETCHARLEN(d, ptr, ptr);
          }
        else
#endif
        d = *ptr;
        if (d == '\\')
          {
          const uschar *oldptr = ptr;
          d = check_escape(&ptr, errorptr, *brackets, options, TRUE);
          if (d < 0)
            {
            if (d == -ESC_b) d = '\b';
            else if (d == -ESC_X) d = 'X'; else
              {
              ptr = oldptr - 2;
              goto LONE_SINGLE_CHARACTER;
              }
            }
          }
        if (d == c) goto LONE_SINGLE_CHARACTER;
#ifdef SUPPORT_UTF8
        if (utf8 && (d > 255 || ((options & PCRE_CASELESS) != 0 && d > 127)))
          {
          class_utf8 = TRUE;
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
              if (occ == ocd)
                {
                *class_utf8data++ = XCL_SINGLE;
                }
              else
                {
                *class_utf8data++ = XCL_RANGE;
                class_utf8data += ord2utf8(occ, class_utf8data);
                }
              class_utf8data += ord2utf8(ocd, class_utf8data);
              }
            }
#endif
          *class_utf8data++ = XCL_RANGE;
          class_utf8data += ord2utf8(c, class_utf8data);
          class_utf8data += ord2utf8(d, class_utf8data);
#ifdef SUPPORT_UCP
          continue;
#else
          if ((options & PCRE_CASELESS) == 0 || c > 127) continue;
          d = 127;
#endif
          }
#endif
        for (; c <= d; c++)
          {
          classbits[c/8] |= (1 << (c&7));
          if ((options & PCRE_CASELESS) != 0)
            {
            int uc = cd->fcc[c];
            classbits[uc/8] |= (1 << (uc&7));
            }
          class_charcount++;
          class_lastchar = c;
          }
        continue;
        }
      LONE_SINGLE_CHARACTER:
#ifdef SUPPORT_UTF8
      if (utf8 && (c > 255 || ((options & PCRE_CASELESS) != 0 && c > 127)))
        {
        class_utf8 = TRUE;
        *class_utf8data++ = XCL_SINGLE;
        class_utf8data += ord2utf8(c, class_utf8data);
#ifdef SUPPORT_UCP
        if ((options & PCRE_CASELESS) != 0)
          {
          int chartype;
          int othercase;
          if (ucp_findchar(c, &chartype, &othercase) >= 0 && othercase > 0)
            {
            *class_utf8data++ = XCL_SINGLE;
            class_utf8data += ord2utf8(othercase, class_utf8data);
            }
          }
#endif
        }
      else
#endif
        {
        classbits[c/8] |= (1 << (c&7));
        if ((options & PCRE_CASELESS) != 0)
          {
          c = cd->fcc[c];
          classbits[c/8] |= (1 << (c&7));
          }
        class_charcount++;
        class_lastchar = c;
        }
      }
    while ((c = *(++ptr)) != ']' || inescq);
#ifdef SUPPORT_UTF8
    if (class_charcount == 1 &&
          (!utf8 ||
          (!class_utf8 && (!negate_class || class_lastchar < 128))))
#else
    if (class_charcount == 1)
#endif
      {
      zeroreqbyte = reqbyte;
      if (negate_class)
        {
        if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
        zerofirstbyte = firstbyte;
        *code++ = OP_NOT;
        *code++ = class_lastchar;
        break;
        }
#ifdef SUPPORT_UTF8
      if (utf8 && class_lastchar > 127)
        mclength = ord2utf8(class_lastchar, mcbuffer);
      else
#endif
        {
        mcbuffer[0] = class_lastchar;
        mclength = 1;
        }
      goto ONE_CHAR;
      }
    if (firstbyte == REQ_UNSET) firstbyte = REQ_NONE;
    zerofirstbyte = firstbyte;
    zeroreqbyte = reqbyte;
#ifdef SUPPORT_UTF8
    if (class_utf8)
      {
      *class_utf8data++ = XCL_END;
      *code++ = OP_XCLASS;
      code += LINK_SIZE;
      *code = negate_class? XCL_NOT : 0;
      if (class_charcount > 0)
        {
        *code++ |= XCL_MAP;
        memcpy(code, classbits, 32);
        code = class_utf8data;
        }
      else
        {
        int len = class_utf8data - (code + 33);
        memmove(code + 1, code + 33, len);
        code += len + 1;
        }
      PUT(previous, 1, code - previous);
      break;
      }
#endif
    if (negate_class)
      {
      *code++ = OP_NCLASS;
      for (c = 0; c < 32; c++) code[c] = ~classbits[c];
      }
    else
      {
      *code++ = OP_CLASS;
      memcpy(code, classbits, 32);
      }
    code += 32;
    break;
    case '{':
    if (!is_quantifier) goto NORMAL_CHAR;
    ptr = read_repeat_counts(ptr+1, &repeat_min, &repeat_max, errorptr);
    if (*errorptr != NULL) goto FAILED;
    goto REPEAT;
    case '*':
    repeat_min = 0;
    repeat_max = -1;
    goto REPEAT;
    case '+':
    repeat_min = 1;
    repeat_max = -1;
    goto REPEAT;
    case '?':
    repeat_min = 0;
    repeat_max = 1;
    REPEAT:
    if (previous == NULL)
      {
      *errorptr = ERR9;
      goto FAILED;
      }
    if (repeat_min == 0)
      {
      firstbyte = zerofirstbyte;
      reqbyte = zeroreqbyte;
      }
    reqvary = (repeat_min == repeat_max)? 0 : REQ_VARY;
    op_type = 0;
    possessive_quantifier = FALSE;
    tempcode = previous;
    if (ptr[1] == '+')
      {
      repeat_type = 0;
      possessive_quantifier = TRUE;
      ptr++;
      }
    else if (ptr[1] == '?')
      {
      repeat_type = greedy_non_default;
      ptr++;
      }
    else repeat_type = greedy_default;
    if (*previous == OP_RECURSE)
      {
      memmove(previous + 1 + LINK_SIZE, previous, 1 + LINK_SIZE);
      code += 1 + LINK_SIZE;
      *previous = OP_BRA;
      PUT(previous, 1, code - previous);
      *code = OP_KET;
      PUT(code, 1, code - previous);
      code += 1 + LINK_SIZE;
      }
    if (*previous == OP_CHAR || *previous == OP_CHARNC)
      {
#ifdef SUPPORT_UTF8
      if (utf8 && (code[-1] & 0x80) != 0)
        {
        uschar *lastchar = code - 1;
        while((*lastchar & 0xc0) == 0x80) lastchar--;
        c = code - lastchar;
        memcpy(utf8_char, lastchar, c);
        c |= 0x80;
        }
      else
#endif
        {
        c = code[-1];
        if (repeat_min > 1) reqbyte = c | req_caseopt | cd->req_varyopt;
        }
      goto OUTPUT_SINGLE_REPEAT;
      }
    else if (*previous == OP_NOT)
      {
      op_type = OP_NOTSTAR - OP_STAR;
      c = previous[1];
      goto OUTPUT_SINGLE_REPEAT;
      }
    else if (*previous < OP_EODN)
      {
      uschar *oldcode;
      int prop_type;
      op_type = OP_TYPESTAR - OP_STAR;
      c = *previous;
      OUTPUT_SINGLE_REPEAT:
      prop_type = (*previous == OP_PROP || *previous == OP_NOTPROP)?
        previous[1] : -1;
      oldcode = code;
      code = previous;
      if (repeat_max == 0) goto END_REPEAT;
      if (repeat_max != 1) cd->nopartial = TRUE;
      repeat_type += op_type;
      if (repeat_min == 0)
        {
        if (repeat_max == -1) *code++ = OP_STAR + repeat_type;
          else if (repeat_max == 1) *code++ = OP_QUERY + repeat_type;
        else
          {
          *code++ = OP_UPTO + repeat_type;
          PUT2INC(code, 0, repeat_max);
          }
        }
      else if (repeat_min == 1)
        {
        if (repeat_max == -1)
          *code++ = OP_PLUS + repeat_type;
        else
          {
          code = oldcode;
          if (repeat_max == 1) goto END_REPEAT;
          *code++ = OP_UPTO + repeat_type;
          PUT2INC(code, 0, repeat_max - 1);
          }
        }
      else
        {
        *code++ = OP_EXACT + op_type;
        PUT2INC(code, 0, repeat_min);
        if (repeat_max < 0)
          {
#ifdef SUPPORT_UTF8
          if (utf8 && c >= 128)
            {
            memcpy(code, utf8_char, c & 7);
            code += c & 7;
            }
          else
#endif
            {
            *code++ = c;
            if (prop_type >= 0) *code++ = prop_type;
            }
          *code++ = OP_STAR + repeat_type;
          }
        else if (repeat_max != repeat_min)
          {
#ifdef SUPPORT_UTF8
          if (utf8 && c >= 128)
            {
            memcpy(code, utf8_char, c & 7);
            code += c & 7;
            }
          else
#endif
          *code++ = c;
          if (prop_type >= 0) *code++ = prop_type;
          repeat_max -= repeat_min;
          *code++ = OP_UPTO + repeat_type;
          PUT2INC(code, 0, repeat_max);
          }
        }
#ifdef SUPPORT_UTF8
      if (utf8 && c >= 128)
        {
        memcpy(code, utf8_char, c & 7);
        code += c & 7;
        }
      else
#endif
      *code++ = c;
#ifdef SUPPORT_UCP
      if (prop_type >= 0) *code++ = prop_type;
#endif
      }
    else if (*previous == OP_CLASS ||
             *previous == OP_NCLASS ||
#ifdef SUPPORT_UTF8
             *previous == OP_XCLASS ||
#endif
             *previous == OP_REF)
      {
      if (repeat_max == 0)
        {
        code = previous;
        goto END_REPEAT;
        }
      if (repeat_max != 1) cd->nopartial = TRUE;
      if (repeat_min == 0 && repeat_max == -1)
        *code++ = OP_CRSTAR + repeat_type;
      else if (repeat_min == 1 && repeat_max == -1)
        *code++ = OP_CRPLUS + repeat_type;
      else if (repeat_min == 0 && repeat_max == 1)
        *code++ = OP_CRQUERY + repeat_type;
      else
        {
        *code++ = OP_CRRANGE + repeat_type;
        PUT2INC(code, 0, repeat_min);
        if (repeat_max == -1) repeat_max = 0;
        PUT2INC(code, 0, repeat_max);
        }
      }
    else if (*previous >= OP_BRA || *previous == OP_ONCE ||
             *previous == OP_COND)
      {
      register int i;
      int ketoffset = 0;
      int len = code - previous;
      uschar *bralink = NULL;
      if (repeat_max == -1)
        {
        register uschar *ket = previous;
        do ket += GET(ket, 1); while (*ket != OP_KET);
        ketoffset = code - ket;
        }
      if (repeat_min == 0)
        {
        if (repeat_max == 0)
          {
          code = previous;
          goto END_REPEAT;
          }
        if (repeat_max <= 1)
          {
          *code = OP_END;
          adjust_recurse(previous, 1, utf8, cd);
          memmove(previous+1, previous, len);
          code++;
          *previous++ = OP_BRAZERO + repeat_type;
          }
        else
          {
          int offset;
          *code = OP_END;
          adjust_recurse(previous, 2 + LINK_SIZE, utf8, cd);
          memmove(previous + 2 + LINK_SIZE, previous, len);
          code += 2 + LINK_SIZE;
          *previous++ = OP_BRAZERO + repeat_type;
          *previous++ = OP_BRA;
          offset = (bralink == NULL)? 0 : previous - bralink;
          bralink = previous;
          PUTINC(previous, 0, offset);
          }
        repeat_max--;
        }
      else
        {
        if (repeat_min > 1)
          {
          if (groupsetfirstbyte && reqbyte < 0) reqbyte = firstbyte;
          for (i = 1; i < repeat_min; i++)
            {
            memcpy(code, previous, len);
            code += len;
            }
          }
        if (repeat_max > 0) repeat_max -= repeat_min;
        }
      if (repeat_max >= 0)
        {
        for (i = repeat_max - 1; i >= 0; i--)
          {
          *code++ = OP_BRAZERO + repeat_type;
          if (i != 0)
            {
            int offset;
            *code++ = OP_BRA;
            offset = (bralink == NULL)? 0 : code - bralink;
            bralink = code;
            PUTINC(code, 0, offset);
            }
          memcpy(code, previous, len);
          code += len;
          }
        while (bralink != NULL)
          {
          int oldlinkoffset;
          int offset = code - bralink + 1;
          uschar *bra = code - offset;
          oldlinkoffset = GET(bra, 1);
          bralink = (oldlinkoffset == 0)? NULL : bralink - oldlinkoffset;
          *code++ = OP_KET;
          PUTINC(code, 0, offset);
          PUT(bra, 1, offset);
          }
        }
      else code[-ketoffset] = OP_KETRMAX + repeat_type;
      }
    else
      {
      *errorptr = ERR11;
      goto FAILED;
      }
    if (possessive_quantifier)
      {
      int len = code - tempcode;
      memmove(tempcode + 1+LINK_SIZE, tempcode, len);
      code += 1 + LINK_SIZE;
      len += 1 + LINK_SIZE;
      tempcode[0] = OP_ONCE;
      *code++ = OP_KET;
      PUTINC(code, 0, len);
      PUT(tempcode, 1, len);
      }
    END_REPEAT:
    previous = NULL;
    cd->req_varyopt |= reqvary;
    break;
    case '(': {
    newoptions = options;
    skipbytes = 0;
    if (*(++ptr) == '?')
      {
      int set, unset;
      int *optset;
      switch (*(++ptr))
        {
        case '#':
        ptr++;
        while (*ptr != ')') ptr++;
        continue;
        case ':':
        bravalue = OP_BRA;
        ptr++;
        break;
        case '(': {
        bravalue = OP_COND;
        if (ptr[1] == 'R')
          {
          code[1+LINK_SIZE] = OP_CREF;
          PUT2(code, 2+LINK_SIZE, CREF_RECURSE);
          skipbytes = 3;
          ptr += 3;
          }
        else if ((digitab[ptr[1]] && ctype_digit) != 0)
          {
          int condref;
          condref = *(++ptr) - '0';
          while (*(++ptr) != ')') condref = condref*10 + *ptr - '0';
          if (condref == 0)
            {
            *errorptr = ERR35;
            goto FAILED;
            }
          ptr++;
          code[1+LINK_SIZE] = OP_CREF;
          PUT2(code, 2+LINK_SIZE, condref);
          skipbytes = 3;
          }
        break;
        }
        case '=':
        bravalue = OP_ASSERT;
        ptr++;
        break;
        case '!':
        bravalue = OP_ASSERT_NOT;
        ptr++;
        break;
        case '<':
        switch (*(++ptr))
          {
          case '=':
          bravalue = OP_ASSERTBACK;
          ptr++;
          break;
          case '!':
          bravalue = OP_ASSERTBACK_NOT;
          ptr++;
          break;
          }
        break;
        case '>':
        bravalue = OP_ONCE;
        ptr++;
        break;
        case 'C':
        previous_callout = code;
        after_manual_callout = 1;
        *code++ = OP_CALLOUT;
          {
          int n = 0;
          while ((digitab[*(++ptr)] & ctype_digit) != 0)
            n = n * 10 + *ptr - '0';
          if (n > 255)
            {
            *errorptr = ERR38;
            goto FAILED;
            }
          *code++ = n;
          PUT(code, 0, ptr - cd->start_pattern + 1);
          PUT(code, LINK_SIZE, 0);
          code += 2 * LINK_SIZE;
          }
        previous = NULL;
        continue;
        case 'P':
        if (*(++ptr) == '<')
          {
          int i, namelen;
          uschar *slot = cd->name_table;
          const uschar *name;
          name = ++ptr;
          while (*ptr++ != '>');
          namelen = ptr - name - 1;
          for (i = 0; i < cd->names_found; i++)
            {
            int crc = memcmp(name, slot+2, namelen);
            if (crc == 0)
              {
              if (slot[2+namelen] == 0)
                {
                *errorptr = ERR43;
                goto FAILED;
                }
              crc = -1;
              }
            if (crc < 0)
              {
              memmove(slot + cd->name_entry_size, slot,
                (cd->names_found - i) * cd->name_entry_size);
              break;
              }
            slot += cd->name_entry_size;
            }
          PUT2(slot, 0, *brackets + 1);
          memcpy(slot + 2, name, namelen);
          slot[2+namelen] = 0;
          cd->names_found++;
          goto NUMBERED_GROUP;
          }
        if (*ptr == '=' || *ptr == '>')
          {
          int i, namelen;
          int type = *ptr++;
          const uschar *name = ptr;
          uschar *slot = cd->name_table;
          while (*ptr != ')') ptr++;
          namelen = ptr - name;
          for (i = 0; i < cd->names_found; i++)
            {
            if (strncmp((char *)name, (char *)slot+2, namelen) == 0) break;
            slot += cd->name_entry_size;
            }
          if (i >= cd->names_found)
            {
            *errorptr = ERR15;
            goto FAILED;
            }
          recno = GET2(slot, 0);
          if (type == '>') goto HANDLE_RECURSION;
          previous = code;
          *code++ = OP_REF;
          PUT2INC(code, 0, recno);
          cd->backref_map |= (recno < 32)? (1 << recno) : 1;
          if (recno > cd->top_backref) cd->top_backref = recno;
          continue;
          }
        break;
        case 'R':
        ptr++;
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          {
          const uschar *called;
          recno = 0;
          while((digitab[*ptr] & ctype_digit) != 0)
            recno = recno * 10 + *ptr++ - '0';
          HANDLE_RECURSION:
          previous = code;
          *code = OP_END;
          called = (recno == 0)?
            cd->start_code : find_bracket(cd->start_code, utf8, recno);
          if (called == NULL)
            {
            *errorptr = ERR15;
            goto FAILED;
            }
          if (GET(called, 1) == 0 && could_be_empty(called, code, bcptr, utf8))
            {
            *errorptr = ERR40;
            goto FAILED;
            }
          *code = OP_RECURSE;
          PUT(code, 1, called - cd->start_code);
          code += 1 + LINK_SIZE;
          }
        continue;
        default:
        set = unset = 0;
        optset = &set;
        while (*ptr != ')' && *ptr != ':')
          {
          switch (*ptr++)
            {
            case '-': optset = &unset; break;
            case 'i': *optset |= PCRE_CASELESS; break;
            case 'm': *optset |= PCRE_MULTILINE; break;
            case 's': *optset |= PCRE_DOTALL; break;
            case 'x': *optset |= PCRE_EXTENDED; break;
            case 'U': *optset |= PCRE_UNGREEDY; break;
            case 'X': *optset |= PCRE_EXTRA; break;
            }
          }
        newoptions = (options | set) & (~unset);
        if (*ptr == ')')
          {
          if ((options & PCRE_IMS) != (newoptions & PCRE_IMS))
            {
            *code++ = OP_OPT;
            *code++ = newoptions & PCRE_IMS;
            }
          *optionsptr = options = newoptions;
          greedy_default = ((newoptions & PCRE_UNGREEDY) != 0);
          greedy_non_default = greedy_default ^ 1;
          req_caseopt = ((options & PCRE_CASELESS) != 0)? REQ_CASELESS : 0;
          previous = NULL;
          continue;
          }
        bravalue = OP_BRA;
        ptr++;
        }
      }
    else if ((options & PCRE_NO_AUTO_CAPTURE) != 0)
      {
      bravalue = OP_BRA;
      }
    else
      {
      NUMBERED_GROUP:
      if (++(*brackets) > EXTRACT_BASIC_MAX)
        {
        bravalue = OP_BRA + EXTRACT_BASIC_MAX + 1;
        code[1+LINK_SIZE] = OP_BRANUMBER;
        PUT2(code, 2+LINK_SIZE, *brackets);
        skipbytes = 3;
        }
      else bravalue = OP_BRA + *brackets;
      }
    previous = (bravalue >= OP_ONCE)? code : NULL;
    *code = bravalue;
    tempcode = code;
    tempreqvary = cd->req_varyopt;
    if (!compile_regex(
         newoptions,
         options & PCRE_IMS,
         brackets,
         &tempcode,
         &ptr,
         errorptr,
         (bravalue == OP_ASSERTBACK ||
          bravalue == OP_ASSERTBACK_NOT),
         skipbytes,
         &subfirstbyte,
         &subreqbyte,
         bcptr,
         cd))
      goto FAILED;
    else if (bravalue == OP_COND)
      {
      uschar *tc = code;
      condcount = 0;
      do {
         condcount++;
         tc += GET(tc,1);
         }
      while (*tc != OP_KET);
      if (condcount > 2)
        {
        *errorptr = ERR27;
        goto FAILED;
        }
      if (condcount == 1) subfirstbyte = subreqbyte = REQ_NONE;
      }
    zeroreqbyte = reqbyte;
    zerofirstbyte = firstbyte;
    groupsetfirstbyte = FALSE;
    if (bravalue >= OP_BRA || bravalue == OP_ONCE || bravalue == OP_COND)
      {
      if (firstbyte == REQ_UNSET)
        {
        if (subfirstbyte >= 0)
          {
          firstbyte = subfirstbyte;
          groupsetfirstbyte = TRUE;
          }
        else firstbyte = REQ_NONE;
        zerofirstbyte = REQ_NONE;
        }
      else if (subfirstbyte >= 0 && subreqbyte < 0)
        subreqbyte = subfirstbyte | tempreqvary;
      if (subreqbyte >= 0) reqbyte = subreqbyte;
      }
    else if (bravalue == OP_ASSERT && subreqbyte >= 0) reqbyte = subreqbyte;
    code = tempcode;
    if (*ptr != ')')
      {
      *errorptr = ERR14;
      goto FAILED;
      }
    break;
    }
    case '\\':
    tempptr = ptr;
    c = check_escape(&ptr, errorptr, *brackets, options, FALSE);
    if (c < 0)
      {
      if (-c == ESC_Q)
        {
        if (ptr[1] == '\\' && ptr[2] == 'E') ptr += 2;
          else inescq = TRUE;
        continue;
        }
      if (firstbyte == REQ_UNSET && -c > ESC_b && -c < ESC_Z)
        firstbyte = REQ_NONE;
      zerofirstbyte = firstbyte;
      zeroreqbyte = reqbyte;
      if (-c >= ESC_REF)
        {
        int number = -c - ESC_REF;
        previous = code;
        *code++ = OP_REF;
        PUT2INC(code, 0, number);
        }
#ifdef SUPPORT_UCP
      else if (-c == ESC_P || -c == ESC_p)
        {
        BOOL negated;
        int value = get_ucp(&ptr, &negated, errorptr);
        previous = code;
        *code++ = ((-c == ESC_p) != negated)? OP_PROP : OP_NOTPROP;
        *code++ = value;
        }
#endif
      else
        {
        previous = (-c > ESC_b && -c < ESC_Z)? code : NULL;
        *code++ = -c;
        }
      continue;
      }
#ifdef SUPPORT_UTF8
    if (utf8 && c > 127)
      mclength = ord2utf8(c, mcbuffer);
    else
#endif
     {
     mcbuffer[0] = c;
     mclength = 1;
     }
    goto ONE_CHAR;
    default:
    NORMAL_CHAR:
    mclength = 1;
    mcbuffer[0] = c;
#ifdef SUPPORT_UTF8
    if (utf8 && (c & 0xc0) == 0xc0)
      {
      while ((ptr[1] & 0xc0) == 0x80)
        mcbuffer[mclength++] = *(++ptr);
      }
#endif
    ONE_CHAR:
    previous = code;
    *code++ = ((options & PCRE_CASELESS) != 0)? OP_CHARNC : OP_CHAR;
    for (c = 0; c < mclength; c++) *code++ = mcbuffer[c];
    if (firstbyte == REQ_UNSET)
      {
      zerofirstbyte = REQ_NONE;
      zeroreqbyte = reqbyte;
      if (mclength == 1 || req_caseopt == 0)
        {
        firstbyte = mcbuffer[0] | req_caseopt;
        if (mclength != 1) reqbyte = code[-1] | cd->req_varyopt;
        }
      else firstbyte = reqbyte = REQ_NONE;
      }
    else
      {
      zerofirstbyte = firstbyte;
      zeroreqbyte = reqbyte;
      if (mclength == 1 || req_caseopt == 0)
        reqbyte = code[-1] | req_caseopt | cd->req_varyopt;
      }
    break;
    }
  }
FAILED:
*ptrptr = ptr;
return FALSE;
}