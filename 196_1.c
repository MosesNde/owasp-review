static BOOL
match(register const uschar *eptr, register const uschar *ecode,
  int offset_top, match_data *md, unsigned long int ims, eptrblock *eptrb,
  int flags)
{
unsigned long int original_ims = ims;
eptrblock newptrb;
if ((flags & match_isgroup) != 0)
  {
  newptrb.prev = eptrb;
  newptrb.saved_eptr = eptr;
  eptrb = &newptrb;
  }
for (;;)
  {
  int op = (int)*ecode;
  int min, max, ctype;
  register int i;
  register int c;
  BOOL minimize = FALSE;
  if (op > OP_BRA)
    {
    int offset;
    int number = op - OP_BRA;
    if (number > EXTRACT_BASIC_MAX) number = (ecode[4] << 8) | ecode[5];
    offset = number << 1;
    if (offset < md->offset_max)
      {
      int save_offset1 = md->offset_vector[offset];
      int save_offset2 = md->offset_vector[offset+1];
      int save_offset3 = md->offset_vector[md->offset_end - number];
      md->offset_vector[md->offset_end - number] = eptr - md->start_subject;
      do
        {
        if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
          return TRUE;
        ecode += (ecode[1] << 8) + ecode[2];
        }
      while (*ecode == OP_ALT);
      md->offset_vector[offset] = save_offset1;
      md->offset_vector[offset+1] = save_offset2;
      md->offset_vector[md->offset_end - number] = save_offset3;
      return FALSE;
      }
    else op = OP_BRA;
    }
switch(op)
  {
  case OP_BRA:
  do
    {
    if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
      return TRUE;
    ecode += (ecode[1] << 8) + ecode[2];
    }
  while (*ecode == OP_ALT);
  return FALSE;
  case OP_COND:
  if (ecode[3] == OP_CREF)
    {
    int offset = (ecode[4] << 9) | (ecode[5] << 1);
    return match(eptr,
      ecode + ((offset < offset_top && md->offset_vector[offset] >= 0)?
        6 : 3 + (ecode[1] << 8) + ecode[2]),
      offset_top, md, ims, eptrb, match_isgroup);
    }
  else
    {
    if (match(eptr, ecode+3, offset_top, md, ims, NULL,
        match_condassert | match_isgroup))
      {
      ecode += 3 + (ecode[4] << 8) + ecode[5];
      while (*ecode == OP_ALT) ecode += (ecode[1] << 8) + ecode[2];
      }
    else ecode += (ecode[1] << 8) + ecode[2];
    return match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup);
    }
  case OP_CREF:
  case OP_BRANUMBER:
  ecode += 3;
  break;
  case OP_END:
  if (md->notempty && eptr == md->start_match) return FALSE;
  md->end_match_ptr = eptr;
  md->end_offset_top = offset_top;
  return TRUE;
  case OP_OPT:
  ims = ecode[1];
  ecode += 2;
  break;
  case OP_ASSERT:
  case OP_ASSERTBACK:
  do
    {
    if (match(eptr, ecode+3, offset_top, md, ims, NULL, match_isgroup)) break;
    ecode += (ecode[1] << 8) + ecode[2];
    }
  while (*ecode == OP_ALT);
  if (*ecode == OP_KET) return FALSE;
  if ((flags & match_condassert) != 0) return TRUE;
  do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);
  ecode += 3;
  offset_top = md->end_offset_top;
  continue;
  case OP_ASSERT_NOT:
  case OP_ASSERTBACK_NOT:
  do
    {
    if (match(eptr, ecode+3, offset_top, md, ims, NULL, match_isgroup))
      return FALSE;
    ecode += (ecode[1] << 8) + ecode[2];
    }
  while (*ecode == OP_ALT);
  if ((flags & match_condassert) != 0) return TRUE;
  ecode += 3;
  continue;
  case OP_REVERSE:
#ifdef SUPPORT_UTF8
  c = (ecode[1] << 8) + ecode[2];
  for (i = 0; i < c; i++)
    {
    eptr--;
    BACKCHAR(eptr)
    }
#else
  eptr -= (ecode[1] << 8) + ecode[2];
#endif
  if (eptr < md->start_subject) return FALSE;
  ecode += 3;
  break;
  case OP_RECURSE:
    {
    BOOL rc;
    int *save;
    int stacksave[15];
    c = md->offset_max;
    if (c < 16) save = stacksave; else
      {
      save = (int *)(pcre_malloc)((c+1) * sizeof(int));
      if (save == NULL)
        {
        save = stacksave;
        c = 15;
        }
      }
    for (i = 1; i <= c; i++)
      save[i] = md->offset_vector[md->offset_end - i];
    rc = match(eptr, md->start_pattern, offset_top, md, ims, eptrb,
      match_isgroup);
    for (i = 1; i <= c; i++)
      md->offset_vector[md->offset_end - i] = save[i];
    if (save != stacksave) (pcre_free)(save);
    if (!rc) return FALSE;
    offset_top = md->end_offset_top;
    eptr = md->end_match_ptr;
    ecode++;
    }
  break;
  case OP_ONCE:
    {
    const uschar *prev = ecode;
    const uschar *saved_eptr = eptr;
    do
      {
      if (match(eptr, ecode+3, offset_top, md, ims, eptrb, match_isgroup))
        break;
      ecode += (ecode[1] << 8) + ecode[2];
      }
    while (*ecode == OP_ALT);
    if (*ecode != OP_ONCE && *ecode != OP_ALT) return FALSE;
    do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);
    offset_top = md->end_offset_top;
    eptr = md->end_match_ptr;
    if (*ecode == OP_KET || eptr == saved_eptr)
      {
      ecode += 3;
      break;
      }
    if (ecode[3] == OP_OPT)
      {
      ims = (ims & ~PCRE_IMS) | ecode[4];
      }
    if (*ecode == OP_KETRMIN)
      {
      if (match(eptr, ecode+3, offset_top, md, ims, eptrb, 0) ||
          match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup))
            return TRUE;
      }
    else
      {
      if (match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup) ||
          match(eptr, ecode+3, offset_top, md, ims, eptrb, 0)) return TRUE;
      }
    }
  return FALSE;
  case OP_ALT:
  do ecode += (ecode[1] << 8) + ecode[2]; while (*ecode == OP_ALT);
  break;
  case OP_BRAZERO:
    {
    const uschar *next = ecode+1;
    if (match(eptr, next, offset_top, md, ims, eptrb, match_isgroup))
      return TRUE;
    do next += (next[1] << 8) + next[2]; while (*next == OP_ALT);
    ecode = next + 3;
    }
  break;
  case OP_BRAMINZERO:
    {
    const uschar *next = ecode+1;
    do next += (next[1] << 8) + next[2]; while (*next == OP_ALT);
    if (match(eptr, next+3, offset_top, md, ims, eptrb, match_isgroup))
      return TRUE;
    ecode++;
    }
  break;
  case OP_KET:
  case OP_KETRMIN:
  case OP_KETRMAX:
    {
    const uschar *prev = ecode - (ecode[1] << 8) - ecode[2];
    const uschar *saved_eptr = eptrb->saved_eptr;
    eptrb = eptrb->prev;
    if (*prev == OP_ASSERT || *prev == OP_ASSERT_NOT ||
        *prev == OP_ASSERTBACK || *prev == OP_ASSERTBACK_NOT ||
        *prev == OP_ONCE)
      {
      md->end_match_ptr = eptr;
      md->end_offset_top = offset_top;
      return TRUE;
      }
    if (*prev != OP_COND)
      {
      int offset;
      int number = *prev - OP_BRA;
      if (number > EXTRACT_BASIC_MAX) number = (prev[4] << 8) | prev[5];
      offset = number << 1;
      if (number > 0)
        {
        if (offset >= md->offset_max) md->offset_overflow = TRUE; else
          {
          md->offset_vector[offset] =
            md->offset_vector[md->offset_end - number];
          md->offset_vector[offset+1] = eptr - md->start_subject;
          if (offset_top <= offset) offset_top = offset + 2;
          }
        }
      }
    ims = original_ims;
    if (*ecode == OP_KET || eptr == saved_eptr)
      {
      ecode += 3;
      break;
      }
    if (*ecode == OP_KETRMIN)
      {
      if (match(eptr, ecode+3, offset_top, md, ims, eptrb, 0) ||
          match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup))
            return TRUE;
      }
    else
      {
      if (match(eptr, prev, offset_top, md, ims, eptrb, match_isgroup) ||
          match(eptr, ecode+3, offset_top, md, ims, eptrb, 0)) return TRUE;
      }
    }
  return FALSE;
  case OP_CIRC:
  if (md->notbol && eptr == md->start_subject) return FALSE;
  if ((ims & PCRE_MULTILINE) != 0)
    {
    if (eptr != md->start_subject && eptr[-1] != NEWLINE) return FALSE;
    ecode++;
    break;
    }
  case OP_SOD:
  if (eptr != md->start_subject) return FALSE;
  ecode++;
  break;
  case OP_DOLL:
  if ((ims & PCRE_MULTILINE) != 0)
    {
    if (eptr < md->end_subject) { if (*eptr != NEWLINE) return FALSE; }
      else { if (md->noteol) return FALSE; }
    ecode++;
    break;
    }
  else
    {
    if (md->noteol) return FALSE;
    if (!md->endonly)
      {
      if (eptr < md->end_subject - 1 ||
         (eptr == md->end_subject - 1 && *eptr != NEWLINE)) return FALSE;
      ecode++;
      break;
      }
    }
  case OP_EOD:
  if (eptr < md->end_subject) return FALSE;
  ecode++;
  break;
  case OP_EODN:
  if (eptr < md->end_subject - 1 ||
     (eptr == md->end_subject - 1 && *eptr != NEWLINE)) return FALSE;
  ecode++;
  break;
  case OP_NOT_WORD_BOUNDARY:
  case OP_WORD_BOUNDARY:
    {
    BOOL prev_is_word = (eptr != md->start_subject) &&
      ((md->ctypes[eptr[-1]] & ctype_word) != 0);
    BOOL cur_is_word = (eptr < md->end_subject) &&
      ((md->ctypes[*eptr] & ctype_word) != 0);
    if ((*ecode++ == OP_WORD_BOUNDARY)?
         cur_is_word == prev_is_word : cur_is_word != prev_is_word)
      return FALSE;
    }
  break;
  case OP_ANY:
  if (eptr++ >= md->end_subject) return FALSE;
  ecode++;
  break;
  case OP_NOT_DIGIT:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_digit) != 0)
    return FALSE;
  ecode++;
  break;
  case OP_DIGIT:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_digit) == 0)
    return FALSE;
  ecode++;
  break;
  case OP_NOT_WHITESPACE:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_space) != 0)
    return FALSE;
  ecode++;
  break;
  case OP_WHITESPACE:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_space) == 0)
    return FALSE;
  ecode++;
  break;
  case OP_NOT_WORDCHAR:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_word) != 0)
    return FALSE;
  ecode++;
  break;
  case OP_WORDCHAR:
  if (eptr >= md->end_subject ||
     (md->ctypes[*eptr++] & ctype_word) == 0)
    return FALSE;
  ecode++;
  break;
  case OP_REF:
    {
    int length;
    int offset = (ecode[1] << 9) | (ecode[2] << 1);
    ecode += 3;
    length = (offset >= offset_top || md->offset_vector[offset] < 0)?
      md->end_subject - eptr + 1 :
      md->offset_vector[offset+1] - md->offset_vector[offset];
    switch (*ecode)
      {
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRPLUS:
      case OP_CRMINPLUS:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      c = *ecode++ - OP_CRSTAR;
      minimize = (c & 1) != 0;
      min = rep_min[c];
      max = rep_max[c];
      if (max == 0) max = INT_MAX;
      break;
      case OP_CRRANGE:
      case OP_CRMINRANGE:
      minimize = (*ecode == OP_CRMINRANGE);
      min = (ecode[1] << 8) + ecode[2];
      max = (ecode[3] << 8) + ecode[4];
      if (max == 0) max = INT_MAX;
      ecode += 5;
      break;
      default:
      if (!match_ref(offset, eptr, length, md, ims)) return FALSE;
      eptr += length;
      continue;
      }
    if (length == 0) continue;
    for (i = 1; i <= min; i++)
      {
      if (!match_ref(offset, eptr, length, md, ims)) return FALSE;
      eptr += length;
      }
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || !match_ref(offset, eptr, length, md, ims))
          return FALSE;
        eptr += length;
        }
      }
    else
      {
      const uschar *pp = eptr;
      for (i = min; i < max; i++)
        {
        if (!match_ref(offset, eptr, length, md, ims)) break;
        eptr += length;
        }
      while (eptr >= pp)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        eptr -= length;
        }
      return FALSE;
      }
    }
  case OP_CLASS:
    {
    const uschar *data = ecode + 1;
    ecode += 33;
    switch (*ecode)
      {
      case OP_CRSTAR:
      case OP_CRMINSTAR:
      case OP_CRPLUS:
      case OP_CRMINPLUS:
      case OP_CRQUERY:
      case OP_CRMINQUERY:
      c = *ecode++ - OP_CRSTAR;
      minimize = (c & 1) != 0;
      min = rep_min[c];
      max = rep_max[c];
      if (max == 0) max = INT_MAX;
      break;
      case OP_CRRANGE:
      case OP_CRMINRANGE:
      minimize = (*ecode == OP_CRMINRANGE);
      min = (ecode[1] << 8) + ecode[2];
      max = (ecode[3] << 8) + ecode[4];
      if (max == 0) max = INT_MAX;
      ecode += 5;
      break;
      default:
      min = max = 1;
      break;
      }
    for (i = 1; i <= min; i++)
      {
      if (eptr >= md->end_subject) return FALSE;
      GETCHARINC(c, eptr)
#ifdef SUPPORT_UTF8
      if (c > 255) return FALSE;
#endif
      if ((data[c/8] & (1 << (c&7))) != 0) continue;
      return FALSE;
      }
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || eptr >= md->end_subject) return FALSE;
        GETCHARINC(c, eptr)
#ifdef SUPPORT_UTF8
        if (c > 255) return FALSE;
#endif
        if ((data[c/8] & (1 << (c&7))) != 0) continue;
        return FALSE;
        }
      }
    else
      {
      const uschar *pp = eptr;
      int len = 1;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject) break;
        GETCHARLEN(c, eptr, len)
#ifdef SUPPORT_UTF8
        if (c > 255) break;
#endif
        if ((data[c/8] & (1 << (c&7))) == 0) break;
        eptr += len;
        }
      while (eptr >= pp)
        {
        if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
#ifdef SUPPORT_UTF8
        BACKCHAR(eptr)
#endif
        }
      return FALSE;
      }
    }
  case OP_CHARS:
    {
    register int length = ecode[1];
    ecode += 2;
    if (length > md->end_subject - eptr) return FALSE;
    if ((ims & PCRE_CASELESS) != 0)
      {
      while (length-- > 0)
        if (md->lcc[*ecode++] != md->lcc[*eptr++]) return FALSE;
      }
    else
      {
      while (length-- > 0) if (*ecode++ != *eptr++) return FALSE;
      }
    }
  break;
  case OP_EXACT:
  min = max = (ecode[1] << 8) + ecode[2];
  ecode += 3;
  goto REPEATCHAR;
  case OP_UPTO:
  case OP_MINUPTO:
  min = 0;
  max = (ecode[1] << 8) + ecode[2];
  minimize = *ecode == OP_MINUPTO;
  ecode += 3;
  goto REPEATCHAR;
  case OP_STAR:
  case OP_MINSTAR:
  case OP_PLUS:
  case OP_MINPLUS:
  case OP_QUERY:
  case OP_MINQUERY:
  c = *ecode++ - OP_STAR;
  minimize = (c & 1) != 0;
  min = rep_min[c];
  max = rep_max[c];
  if (max == 0) max = INT_MAX;
  REPEATCHAR:
  if (min > md->end_subject - eptr) return FALSE;
  c = *ecode++;
  if ((ims & PCRE_CASELESS) != 0)
    {
    c = md->lcc[c];
    for (i = 1; i <= min; i++)
      if (c != md->lcc[*eptr++]) return FALSE;
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || eptr >= md->end_subject ||
            c != md->lcc[*eptr++])
          return FALSE;
        }
      }
    else
      {
      const uschar *pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || c != md->lcc[*eptr]) break;
        eptr++;
        }
      while (eptr >= pp)
        if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
      return FALSE;
      }
    }
  else
    {
    for (i = 1; i <= min; i++) if (c != *eptr++) return FALSE;
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || eptr >= md->end_subject || c != *eptr++) return FALSE;
        }
      }
    else
      {
      const uschar *pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || c != *eptr) break;
        eptr++;
        }
      while (eptr >= pp)
       if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
         return TRUE;
      return FALSE;
      }
    }
  case OP_NOT:
  if (eptr >= md->end_subject) return FALSE;
  ecode++;
  if ((ims & PCRE_CASELESS) != 0)
    {
    if (md->lcc[*ecode++] == md->lcc[*eptr++]) return FALSE;
    }
  else
    {
    if (*ecode++ == *eptr++) return FALSE;
    }
  break;
  case OP_NOTEXACT:
  min = max = (ecode[1] << 8) + ecode[2];
  ecode += 3;
  goto REPEATNOTCHAR;
  case OP_NOTUPTO:
  case OP_NOTMINUPTO:
  min = 0;
  max = (ecode[1] << 8) + ecode[2];
  minimize = *ecode == OP_NOTMINUPTO;
  ecode += 3;
  goto REPEATNOTCHAR;
  case OP_NOTSTAR:
  case OP_NOTMINSTAR:
  case OP_NOTPLUS:
  case OP_NOTMINPLUS:
  case OP_NOTQUERY:
  case OP_NOTMINQUERY:
  c = *ecode++ - OP_NOTSTAR;
  minimize = (c & 1) != 0;
  min = rep_min[c];
  max = rep_max[c];
  if (max == 0) max = INT_MAX;
  REPEATNOTCHAR:
  if (min > md->end_subject - eptr) return FALSE;
  c = *ecode++;
  if ((ims & PCRE_CASELESS) != 0)
    {
    c = md->lcc[c];
    for (i = 1; i <= min; i++)
      if (c == md->lcc[*eptr++]) return FALSE;
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || eptr >= md->end_subject ||
            c == md->lcc[*eptr++])
          return FALSE;
        }
      }
    else
      {
      const uschar *pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || c == md->lcc[*eptr]) break;
        eptr++;
        }
      while (eptr >= pp)
        if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
      return FALSE;
      }
    }
  else
    {
    for (i = 1; i <= min; i++) if (c == *eptr++) return FALSE;
    if (min == max) continue;
    if (minimize)
      {
      for (i = min;; i++)
        {
        if (match(eptr, ecode, offset_top, md, ims, eptrb, 0))
          return TRUE;
        if (i >= max || eptr >= md->end_subject || c == *eptr++) return FALSE;
        }
      }
    else
      {
      const uschar *pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || c == *eptr) break;
        eptr++;
        }
      while (eptr >= pp)
       if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
         return TRUE;
      return FALSE;
      }
    }
  case OP_TYPEEXACT:
  min = max = (ecode[1] << 8) + ecode[2];
  minimize = TRUE;
  ecode += 3;
  goto REPEATTYPE;
  case OP_TYPEUPTO:
  case OP_TYPEMINUPTO:
  min = 0;
  max = (ecode[1] << 8) + ecode[2];
  minimize = *ecode == OP_TYPEMINUPTO;
  ecode += 3;
  goto REPEATTYPE;
  case OP_TYPESTAR:
  case OP_TYPEMINSTAR:
  case OP_TYPEPLUS:
  case OP_TYPEMINPLUS:
  case OP_TYPEQUERY:
  case OP_TYPEMINQUERY:
  c = *ecode++ - OP_TYPESTAR;
  minimize = (c & 1) != 0;
  min = rep_min[c];
  max = rep_max[c];
  if (max == 0) max = INT_MAX;
  REPEATTYPE:
  ctype = *ecode++;
  if (min > md->end_subject - eptr) return FALSE;
  if (min > 0) switch(ctype)
    {
    case OP_ANY:
#ifdef SUPPORT_UTF8
    if (md->utf8)
      {
      for (i = 1; i <= min; i++)
        {
        if (eptr >= md->end_subject ||
           (*eptr++ == NEWLINE && (ims & PCRE_DOTALL) == 0))
          return FALSE;
        while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
        }
      break;
      }
#endif
    if ((ims & PCRE_DOTALL) == 0)
      { for (i = 1; i <= min; i++) if (*eptr++ == NEWLINE) return FALSE; }
    else eptr += min;
    break;
    case OP_NOT_DIGIT:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_digit) != 0) return FALSE;
    break;
    case OP_DIGIT:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_digit) == 0) return FALSE;
    break;
    case OP_NOT_WHITESPACE:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_space) != 0) return FALSE;
    break;
    case OP_WHITESPACE:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_space) == 0) return FALSE;
    break;
    case OP_NOT_WORDCHAR:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_word) != 0)
        return FALSE;
    break;
    case OP_WORDCHAR:
    for (i = 1; i <= min; i++)
      if ((md->ctypes[*eptr++] & ctype_word) == 0)
        return FALSE;
    break;
    }
  if (min == max) continue;
  if (minimize)
    {
    for (i = min;; i++)
      {
      if (match(eptr, ecode, offset_top, md, ims, eptrb, 0)) return TRUE;
      if (i >= max || eptr >= md->end_subject) return FALSE;
      c = *eptr++;
      switch(ctype)
        {
        case OP_ANY:
        if ((ims & PCRE_DOTALL) == 0 && c == NEWLINE) return FALSE;
#ifdef SUPPORT_UTF8
        if (md->utf8)
          while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
#endif
        break;
        case OP_NOT_DIGIT:
        if ((md->ctypes[c] & ctype_digit) != 0) return FALSE;
        break;
        case OP_DIGIT:
        if ((md->ctypes[c] & ctype_digit) == 0) return FALSE;
        break;
        case OP_NOT_WHITESPACE:
        if ((md->ctypes[c] & ctype_space) != 0) return FALSE;
        break;
        case OP_WHITESPACE:
        if  ((md->ctypes[c] & ctype_space) == 0) return FALSE;
        break;
        case OP_NOT_WORDCHAR:
        if ((md->ctypes[c] & ctype_word) != 0) return FALSE;
        break;
        case OP_WORDCHAR:
        if ((md->ctypes[c] & ctype_word) == 0) return FALSE;
        break;
        }
      }
    }
  else
    {
    const uschar *pp = eptr;
    switch(ctype)
      {
      case OP_ANY:
#ifdef SUPPORT_UTF8
      if (md->utf8 && max < INT_MAX)
        {
        if ((ims & PCRE_DOTALL) == 0)
          {
          for (i = min; i < max; i++)
            {
            if (eptr >= md->end_subject || *eptr++ == NEWLINE) break;
            while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
            }
          }
        else
          {
          for (i = min; i < max; i++)
            {
            eptr++;
            while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
            }
          }
        break;
        }
#endif
      if ((ims & PCRE_DOTALL) == 0)
        {
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject || *eptr == NEWLINE) break;
          eptr++;
          }
        }
      else
        {
        c = max - min;
        if (c > md->end_subject - eptr) c = md->end_subject - eptr;
        eptr += c;
        }
      break;
      case OP_NOT_DIGIT:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_digit) != 0)
          break;
        eptr++;
        }
      break;
      case OP_DIGIT:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_digit) == 0)
          break;
        eptr++;
        }
      break;
      case OP_NOT_WHITESPACE:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_space) != 0)
          break;
        eptr++;
        }
      break;
      case OP_WHITESPACE:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_space) == 0)
          break;
        eptr++;
        }
      break;
      case OP_NOT_WORDCHAR:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_word) != 0)
          break;
        eptr++;
        }
      break;
      case OP_WORDCHAR:
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || (md->ctypes[*eptr] & ctype_word) == 0)
          break;
        eptr++;
        }
      break;
      }
    while (eptr >= pp)
      {
      if (match(eptr--, ecode, offset_top, md, ims, eptrb, 0))
        return TRUE;
#ifdef SUPPORT_UTF8
      if (md->utf8)
        while (eptr > pp && (*eptr & 0xc0) == 0x80) eptr--;
#endif
      }
    return FALSE;
    }
  default:
  md->errorcode = PCRE_ERROR_UNKNOWN_NODE;
  return FALSE;
  }
}