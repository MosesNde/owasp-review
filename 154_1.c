static int
match(REGISTER const uschar *eptr, REGISTER const uschar *ecode,
  int offset_top, match_data *md, unsigned long int ims, eptrblock *eptrb,
  int flags)
{
register int rrc;
register int i;
register int c;
#ifdef NO_RECURSE
heapframe *frame = (pcre_stack_malloc)(sizeof(heapframe));
frame->Xprevframe = NULL;
frame->Xeptr = eptr;
frame->Xecode = ecode;
frame->Xoffset_top = offset_top;
frame->Xims = ims;
frame->Xeptrb = eptrb;
frame->Xflags = flags;
HEAP_RECURSE:
#define eptr               frame->Xeptr
#define ecode              frame->Xecode
#define offset_top         frame->Xoffset_top
#define ims                frame->Xims
#define eptrb              frame->Xeptrb
#define flags              frame->Xflags
#ifdef SUPPORT_UTF8
#define charptr            frame->Xcharptr
#endif
#define callpat            frame->Xcallpat
#define data               frame->Xdata
#define next               frame->Xnext
#define pp                 frame->Xpp
#define prev               frame->Xprev
#define saved_eptr         frame->Xsaved_eptr
#define new_recursive      frame->Xnew_recursive
#define cur_is_word        frame->Xcur_is_word
#define condition          frame->Xcondition
#define minimize           frame->Xminimize
#define prev_is_word       frame->Xprev_is_word
#define original_ims       frame->Xoriginal_ims
#ifdef SUPPORT_UCP
#define prop_type          frame->Xprop_type
#define prop_fail_result   frame->Xprop_fail_result
#define prop_category      frame->Xprop_category
#define prop_chartype      frame->Xprop_chartype
#define prop_othercase     frame->Xprop_othercase
#define prop_test_against  frame->Xprop_test_against
#define prop_test_variable frame->Xprop_test_variable
#endif
#define ctype              frame->Xctype
#define fc                 frame->Xfc
#define fi                 frame->Xfi
#define length             frame->Xlength
#define max                frame->Xmax
#define min                frame->Xmin
#define number             frame->Xnumber
#define offset             frame->Xoffset
#define op                 frame->Xop
#define save_capture_last  frame->Xsave_capture_last
#define save_offset1       frame->Xsave_offset1
#define save_offset2       frame->Xsave_offset2
#define save_offset3       frame->Xsave_offset3
#define stacksave          frame->Xstacksave
#define newptrb            frame->Xnewptrb
#else
#define fi i
#define fc c
#ifdef SUPPORT_UTF8
const uschar *charptr;
#endif
const uschar *callpat;
const uschar *data;
const uschar *next;
const uschar *pp;
const uschar *prev;
const uschar *saved_eptr;
recursion_info new_recursive;
BOOL cur_is_word;
BOOL condition;
BOOL minimize;
BOOL prev_is_word;
unsigned long int original_ims;
#ifdef SUPPORT_UCP
int prop_type;
int prop_fail_result;
int prop_category;
int prop_chartype;
int prop_othercase;
int prop_test_against;
int *prop_test_variable;
#endif
int ctype;
int length;
int max;
int min;
int number;
int offset;
int op;
int save_capture_last;
int save_offset1, save_offset2, save_offset3;
int stacksave[REC_STACK_SAVE_MAX];
eptrblock newptrb;
#endif
#ifdef SUPPORT_UCP
prop_fail_result = 0;
prop_test_against = 0;
prop_test_variable = NULL;
#endif
if (md->match_call_count++ >= md->match_limit)  ;
original_ims = ims;
if ((flags & match_isgroup) != 0)
  {
  newptrb.epb_prev = eptrb;
  newptrb.epb_saved_eptr = eptr;
  eptrb = &newptrb;
  }
for (;;)
  {
  op = *ecode;
  minimize = FALSE;
  if (md->partial &&
      eptr >= md->end_subject &&
      eptr > md->start_match)
    md->hitend = TRUE;
  if (op > OP_BRA)
    {
    number = op - OP_BRA;
    if (number > EXTRACT_BASIC_MAX)
      number = GET2(ecode, 2+LINK_SIZE);
    offset = number << 1;
    if (offset < md->offset_max)
      {
      save_offset1 = md->offset_vector[offset];
      save_offset2 = md->offset_vector[offset+1];
      save_offset3 = md->offset_vector[md->offset_end - number];
      save_capture_last = md->capture_last;
      md->offset_vector[md->offset_end - number] = eptr - md->start_subject;
      do
        {
        RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb,
          match_isgroup);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        md->capture_last = save_capture_last;
        ecode += GET(ecode, 1);
        }
      while (*ecode == OP_ALT);
      md->offset_vector[offset] = save_offset1;
      md->offset_vector[offset+1] = save_offset2;
      md->offset_vector[md->offset_end - number] = save_offset3;
      RRETURN(MATCH_NOMATCH);
      }
    else op = OP_BRA;
    }
switch(op)
  {
  case OP_BRA:
  do
    {
    RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb,
      match_isgroup);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    ecode += GET(ecode, 1);
    }
  while (*ecode == OP_ALT);
  RRETURN(MATCH_NOMATCH);
  case OP_COND:
  if (ecode[LINK_SIZE+1] == OP_CREF)
    {
    offset = GET2(ecode, LINK_SIZE+2) << 1;
    condition = (offset == CREF_RECURSE * 2)?
      (md->recursive != NULL) :
      (offset < offset_top && md->offset_vector[offset] >= 0);
    RMATCH(rrc, eptr, ecode + (condition?
      (LINK_SIZE + 4) : (LINK_SIZE + 1 + GET(ecode, 1))),
      offset_top, md, ims, eptrb, match_isgroup);
    RRETURN(rrc);
    }
  else
    {
    RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL,
        match_condassert | match_isgroup);
    if (rrc == MATCH_MATCH)
      {
      ecode += 1 + LINK_SIZE + GET(ecode, LINK_SIZE+2);
      while (*ecode == OP_ALT) ecode += GET(ecode, 1);
      }
    else if (rrc != MATCH_NOMATCH)
      {
      RRETURN(rrc);
      }
    else ecode += GET(ecode, 1);
    RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb,
      match_isgroup);
    RRETURN(rrc);
    }
  case OP_CREF:
  case OP_BRANUMBER:
  ecode += 3;
  break;
  case OP_END:
  if (md->recursive != NULL && md->recursive->group_num == 0)
    {
    recursion_info *rec = md->recursive;
    md->recursive = rec->prevrec;
    memmove(md->offset_vector, rec->offset_save,
      rec->saved_max * sizeof(int));
    md->start_match = rec->save_start;
    ims = original_ims;
    ecode = rec->after_call;
    break;
    }
  if (md->notempty && eptr == md->start_match) RRETURN(MATCH_NOMATCH);
  md->end_match_ptr = eptr;
  md->end_offset_top = offset_top;
  RRETURN(MATCH_MATCH);
  case OP_OPT:
  ims = ecode[1];
  ecode += 2;
  break;
  case OP_ASSERT:
  case OP_ASSERTBACK:
  do
    {
    RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL,
      match_isgroup);
    if (rrc == MATCH_MATCH) break;
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    ecode += GET(ecode, 1);
    }
  while (*ecode == OP_ALT);
  if (*ecode == OP_KET) RRETURN(MATCH_NOMATCH);
  if ((flags & match_condassert) != 0) RRETURN(MATCH_MATCH);
  do ecode += GET(ecode,1); while (*ecode == OP_ALT);
  ecode += 1 + LINK_SIZE;
  offset_top = md->end_offset_top;
  continue;
  case OP_ASSERT_NOT:
  case OP_ASSERTBACK_NOT:
  do
    {
    RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, NULL,
      match_isgroup);
    if (rrc == MATCH_MATCH) RRETURN(MATCH_NOMATCH);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    ecode += GET(ecode,1);
    }
  while (*ecode == OP_ALT);
  if ((flags & match_condassert) != 0) RRETURN(MATCH_MATCH);
  ecode += 1 + LINK_SIZE;
  continue;
  case OP_REVERSE:
#ifdef SUPPORT_UTF8
  if (md->utf8)
    {
    c = GET(ecode,1);
    for (i = 0; i < c; i++)
      {
      eptr--;
      if (eptr < md->start_subject) RRETURN(MATCH_NOMATCH);
      BACKCHAR(eptr)
      }
    }
  else
#endif
    {
    eptr -= GET(ecode,1);
    if (eptr < md->start_subject) RRETURN(MATCH_NOMATCH);
    }
  ecode += 1 + LINK_SIZE;
  break;
  case OP_CALLOUT:
  if (pcre_callout != NULL)
    {
    pcre_callout_block cb;
    cb.version          = 1;
    cb.callout_number   = ecode[1];
    cb.offset_vector    = md->offset_vector;
    cb.subject          = (const char *)md->start_subject;
    cb.subject_length   = md->end_subject - md->start_subject;
    cb.start_match      = md->start_match - md->start_subject;
    cb.current_position = eptr - md->start_subject;
    cb.pattern_position = GET(ecode, 2);
    cb.next_item_length = GET(ecode, 2 + LINK_SIZE);
    cb.capture_top      = offset_top/2;
    cb.capture_last     = md->capture_last;
    cb.callout_data     = md->callout_data;
    if ((rrc = (*pcre_callout)(&cb)) > 0) RRETURN(MATCH_NOMATCH);
    if (rrc < 0) RRETURN(rrc);
    }
  ecode += 2 + 2*LINK_SIZE;
  break;
  case OP_RECURSE:
    {
    callpat = md->start_code + GET(ecode, 1);
    new_recursive.group_num = *callpat - OP_BRA;
    if (new_recursive.group_num > EXTRACT_BASIC_MAX)
      new_recursive.group_num = GET2(callpat, 2+LINK_SIZE);
    new_recursive.prevrec = md->recursive;
    md->recursive = &new_recursive;
    ecode += 1 + LINK_SIZE;
    new_recursive.after_call = ecode;
    new_recursive.saved_max = md->offset_end;
    if (new_recursive.saved_max <= REC_STACK_SAVE_MAX)
      new_recursive.offset_save = stacksave;
    else
      {
      new_recursive.offset_save =
        (int *)(pcre_malloc)(new_recursive.saved_max * sizeof(int));
      if (new_recursive.offset_save == NULL) RRETURN(PCRE_ERROR_NOMEMORY);
      }
    memcpy(new_recursive.offset_save, md->offset_vector,
          new_recursive.saved_max * sizeof(int));
    new_recursive.save_start = md->start_match;
    md->start_match = eptr;
    
    do
      {
      RMATCH(rrc, eptr, callpat + 1 + LINK_SIZE, offset_top, md, ims,
          eptrb, match_isgroup);
      if (rrc == MATCH_MATCH)
        {
        md->recursive = new_recursive.prevrec;
        if (new_recursive.offset_save != stacksave)
          (pcre_free)(new_recursive.offset_save);
        RRETURN(MATCH_MATCH);
        }
      else if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      md->recursive = &new_recursive;
      memcpy(md->offset_vector, new_recursive.offset_save,
          new_recursive.saved_max * sizeof(int));
      callpat += GET(callpat, 1);
      }
    while (*callpat == OP_ALT);
    md->recursive = new_recursive.prevrec;
    if (new_recursive.offset_save != stacksave)
      (pcre_free)(new_recursive.offset_save);
    RRETURN(MATCH_NOMATCH);
    }
  case OP_ONCE:
    {
    prev = ecode;
    saved_eptr = eptr;
    do
      {
      RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims,
        eptrb, match_isgroup);
      if (rrc == MATCH_MATCH) break;
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      ecode += GET(ecode,1);
      }
    while (*ecode == OP_ALT);
    if (*ecode != OP_ONCE && *ecode != OP_ALT) RRETURN(MATCH_NOMATCH);
    do ecode += GET(ecode,1); while (*ecode == OP_ALT);
    offset_top = md->end_offset_top;
    eptr = md->end_match_ptr;
    if (*ecode == OP_KET || eptr == saved_eptr)
      {
      ecode += 1+LINK_SIZE;
      break;
      }
    if (ecode[1+LINK_SIZE] == OP_OPT)
      {
      ims = (ims & ~PCRE_IMS) | ecode[4];
      }
    if (*ecode == OP_KETRMIN)
      {
      RMATCH(rrc, eptr, ecode + 1 + LINK_SIZE, offset_top, md, ims, eptrb, 0);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      RMATCH(rrc, eptr, prev, offset_top, md, ims, eptrb, match_isgroup);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      }
    else
      {
      RMATCH(rrc, eptr, prev, offset_top, md, ims, eptrb, match_isgroup);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      RMATCH(rrc, eptr, ecode + 1+LINK_SIZE, offset_top, md, ims, eptrb, 0);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      }
    }
  RRETURN(MATCH_NOMATCH);
  case OP_ALT:
  do ecode += GET(ecode,1); while (*ecode == OP_ALT);
  break;
  case OP_BRAZERO:
    {
    next = ecode+1;
    RMATCH(rrc, eptr, next, offset_top, md, ims, eptrb, match_isgroup);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    do next += GET(next,1); while (*next == OP_ALT);
    ecode = next + 1+LINK_SIZE;
    }
  break;
  case OP_BRAMINZERO:
    {
    next = ecode+1;
    do next += GET(next,1); while (*next == OP_ALT);
    RMATCH(rrc, eptr, next + 1+LINK_SIZE, offset_top, md, ims, eptrb,
      match_isgroup);
    if (rrc != MATCH_NOMATCH) RRETURN(rrc);
    ecode++;
    }
  break;
  case OP_KET:
  case OP_KETRMIN:
  case OP_KETRMAX:
    {
    prev = ecode - GET(ecode, 1);
    saved_eptr = eptrb->epb_saved_eptr;
    eptrb = eptrb->epb_prev;
    if (*prev == OP_ASSERT || *prev == OP_ASSERT_NOT ||
        *prev == OP_ASSERTBACK || *prev == OP_ASSERTBACK_NOT ||
        *prev == OP_ONCE)
      {
      md->end_match_ptr = eptr;
      md->end_offset_top = offset_top;
      RRETURN(MATCH_MATCH);
      }
    if (*prev != OP_COND)
      {
      number = *prev - OP_BRA;
      if (number > EXTRACT_BASIC_MAX) number = GET2(prev, 2+LINK_SIZE);
      offset = number << 1;
      if (number > 0)
        {
        md->capture_last = number;
        if (offset >= md->offset_max) md->offset_overflow = TRUE; else
          {
          md->offset_vector[offset] =
            md->offset_vector[md->offset_end - number];
          md->offset_vector[offset+1] = eptr - md->start_subject;
          if (offset_top <= offset) offset_top = offset + 2;
          }
        if (md->recursive != NULL && md->recursive->group_num == number)
          {
          recursion_info *rec = md->recursive;
          md->recursive = rec->prevrec;
          md->start_match = rec->save_start;
          memcpy(md->offset_vector, rec->offset_save,
            rec->saved_max * sizeof(int));
          ecode = rec->after_call;
          ims = original_ims;
          break;
          }
        }
      }
    ims = original_ims;
    if (*ecode == OP_KET || eptr == saved_eptr)
      {
      ecode += 1 + LINK_SIZE;
      break;
      }
    if (*ecode == OP_KETRMIN)
      {
      RMATCH(rrc, eptr, ecode + 1+LINK_SIZE, offset_top, md, ims, eptrb, 0);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      RMATCH(rrc, eptr, prev, offset_top, md, ims, eptrb, match_isgroup);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      }
    else
      {
      RMATCH(rrc, eptr, prev, offset_top, md, ims, eptrb, match_isgroup);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      RMATCH(rrc, eptr, ecode + 1+LINK_SIZE, offset_top, md, ims, eptrb, 0);
      if (rrc != MATCH_NOMATCH) RRETURN(rrc);
      }
    }
  RRETURN(MATCH_NOMATCH);
  case OP_CIRC:
  if (md->notbol && eptr == md->start_subject) RRETURN(MATCH_NOMATCH);
  if ((ims & PCRE_MULTILINE) != 0)
    {
    if (eptr != md->start_subject && eptr[-1] != NEWLINE)
      RRETURN(MATCH_NOMATCH);
    ecode++;
    break;
    }
  case OP_SOD:
  if (eptr != md->start_subject) RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_SOM:
  if (eptr != md->start_subject + md->start_offset) RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_DOLL:
  if ((ims & PCRE_MULTILINE) != 0)
    {
    if (eptr < md->end_subject)
      { if (*eptr != NEWLINE) RRETURN(MATCH_NOMATCH); }
    else
      { if (md->noteol) RRETURN(MATCH_NOMATCH); }
    ecode++;
    break;
    }
  else
    {
    if (md->noteol) RRETURN(MATCH_NOMATCH);
    if (!md->endonly)
      {
      if (eptr < md->end_subject - 1 ||
         (eptr == md->end_subject - 1 && *eptr != NEWLINE))
        RRETURN(MATCH_NOMATCH);
      ecode++;
      break;
      }
    }
  case OP_EOD:
  if (eptr < md->end_subject) RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_EODN:
  if (eptr < md->end_subject - 1 ||
     (eptr == md->end_subject - 1 && *eptr != NEWLINE)) RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_NOT_WORD_BOUNDARY:
  case OP_WORD_BOUNDARY:
    {
#ifdef SUPPORT_UTF8
    if (md->utf8)
      {
      if (eptr == md->start_subject) prev_is_word = FALSE; else
        {
        const uschar *lastptr = eptr - 1;
        while((*lastptr & 0xc0) == 0x80) lastptr--;
        GETCHAR(c, lastptr);
        prev_is_word = c < 256 && (md->ctypes[c] & ctype_word) != 0;
        }
      if (eptr >= md->end_subject) cur_is_word = FALSE; else
        {
        GETCHAR(c, eptr);
        cur_is_word = c < 256 && (md->ctypes[c] & ctype_word) != 0;
        }
      }
    else
#endif
      {
      prev_is_word = (eptr != md->start_subject) &&
        ((md->ctypes[eptr[-1]] & ctype_word) != 0);
      cur_is_word = (eptr < md->end_subject) &&
        ((md->ctypes[*eptr] & ctype_word) != 0);
      }
    if ((*ecode++ == OP_WORD_BOUNDARY)?
         cur_is_word == prev_is_word : cur_is_word != prev_is_word)
      RRETURN(MATCH_NOMATCH);
    }
  break;
  case OP_ANY:
  if ((ims & PCRE_DOTALL) == 0 && eptr < md->end_subject && *eptr == NEWLINE)
    RRETURN(MATCH_NOMATCH);
  if (eptr++ >= md->end_subject) RRETURN(MATCH_NOMATCH);
#ifdef SUPPORT_UTF8
  if (md->utf8)
    while (eptr < md->end_subject && (*eptr & 0xc0) == 0x80) eptr++;
#endif
  ecode++;
  break;
  case OP_ANYBYTE:
  if (eptr++ >= md->end_subject) RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_NOT_DIGIT:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c < 256 &&
#endif
     (md->ctypes[c] & ctype_digit) != 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_DIGIT:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c >= 256 ||
#endif
     (md->ctypes[c] & ctype_digit) == 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_NOT_WHITESPACE:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c < 256 &&
#endif
     (md->ctypes[c] & ctype_space) != 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_WHITESPACE:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c >= 256 ||
#endif
     (md->ctypes[c] & ctype_space) == 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_NOT_WORDCHAR:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c < 256 &&
#endif
     (md->ctypes[c] & ctype_word) != 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
  case OP_WORDCHAR:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
  if (
#ifdef SUPPORT_UTF8
     c >= 256 ||
#endif
     (md->ctypes[c] & ctype_word) == 0
     )
    RRETURN(MATCH_NOMATCH);
  ecode++;
  break;
#ifdef SUPPORT_UCP
  case OP_PROP:
  case OP_NOTPROP:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
    {
    int chartype, rqdtype;
    int othercase;
    int category = ucp_findchar(c, &chartype, &othercase);
    rqdtype = *(++ecode);
    ecode++;
    if (rqdtype >= 128)
      {
      if ((rqdtype - 128 != category) == (op == OP_PROP))
        RRETURN(MATCH_NOMATCH);
      }
    else
      {
      if ((rqdtype != chartype) == (op == OP_PROP))
        RRETURN(MATCH_NOMATCH);
      }
    }
  break;
  case OP_EXTUNI:
  if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
  GETCHARINCTEST(c, eptr);
    {
    int chartype;
    int othercase;
    int category = ucp_findchar(c, &chartype, &othercase);
    if (category == ucp_M) RRETURN(MATCH_NOMATCH);
    while (eptr < md->end_subject)
      {
      int len = 1;
      if (!md->utf8) c = *eptr; else
        {
        GETCHARLEN(c, eptr, len);
        }
      category = ucp_findchar(c, &chartype, &othercase);
      if (category != ucp_M) break;
      eptr += len;
      }
    }
  ecode++;
  break;
#endif
  case OP_REF:
    {
    offset = GET2(ecode, 1) << 1;
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
      min = GET2(ecode, 1);
      max = GET2(ecode, 3);
      if (max == 0) max = INT_MAX;
      ecode += 5;
      break;
      default:
      if (!match_ref(offset, eptr, length, md, ims)) RRETURN(MATCH_NOMATCH);
      eptr += length;
      continue;
      }
    if (length == 0) continue;
    for (i = 1; i <= min; i++)
      {
      if (!match_ref(offset, eptr, length, md, ims)) RRETURN(MATCH_NOMATCH);
      eptr += length;
      }
    if (min == max) continue;
    if (minimize)
      {
      for (fi = min;; fi++)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (fi >= max || !match_ref(offset, eptr, length, md, ims))
          RRETURN(MATCH_NOMATCH);
        eptr += length;
        }
      }
    else
      {
      pp = eptr;
      for (i = min; i < max; i++)
        {
        if (!match_ref(offset, eptr, length, md, ims)) break;
        eptr += length;
        }
      while (eptr >= pp)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        eptr -= length;
        }
      RRETURN(MATCH_NOMATCH);
      }
    }
  case OP_NCLASS:
  case OP_CLASS:
    {
    data = ecode + 1;
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
      min = GET2(ecode, 1);
      max = GET2(ecode, 3);
      if (max == 0) max = INT_MAX;
      ecode += 5;
      break;
      default:
      min = max = 1;
      break;
      }
#ifdef SUPPORT_UTF8
    if (md->utf8)
      {
      for (i = 1; i <= min; i++)
        {
        if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
        GETCHARINC(c, eptr);
        if (c > 255)
          {
          if (op == OP_CLASS) RRETURN(MATCH_NOMATCH);
          }
        else
          {
          if ((data[c/8] & (1 << (c&7))) == 0) RRETURN(MATCH_NOMATCH);
          }
        }
      }
    else
#endif
      {
      for (i = 1; i <= min; i++)
        {
        if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
        c = *eptr++;
        if ((data[c/8] & (1 << (c&7))) == 0) RRETURN(MATCH_NOMATCH);
        }
      }
    if (min == max) continue;
    if (minimize)
      {
#ifdef SUPPORT_UTF8
      if (md->utf8)
        {
        for (fi = min;; fi++)
          {
          RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (fi >= max || eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
          GETCHARINC(c, eptr);
          if (c > 255)
            {
            if (op == OP_CLASS) RRETURN(MATCH_NOMATCH);
            }
          else
            {
            if ((data[c/8] & (1 << (c&7))) == 0) RRETURN(MATCH_NOMATCH);
            }
          }
        }
      else
#endif
        {
        for (fi = min;; fi++)
          {
          RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (fi >= max || eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
          c = *eptr++;
          if ((data[c/8] & (1 << (c&7))) == 0) RRETURN(MATCH_NOMATCH);
          }
        }
      }
    else
      {
      pp = eptr;
#ifdef SUPPORT_UTF8
      if (md->utf8)
        {
        for (i = min; i < max; i++)
          {
          int len = 1;
          if (eptr >= md->end_subject) break;
          GETCHARLEN(c, eptr, len);
          if (c > 255)
            {
            if (op == OP_CLASS) break;
            }
          else
            {
            if ((data[c/8] & (1 << (c&7))) == 0) break;
            }
          eptr += len;
          }
        for (;;)
          {
          RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (eptr-- == pp) break;
          BACKCHAR(eptr);
          }
        }
      else
#endif
        {
        for (i = min; i < max; i++)
          {
          if (eptr >= md->end_subject) break;
          c = *eptr;
          if ((data[c/8] & (1 << (c&7))) == 0) break;
          eptr++;
          }
        while (eptr >= pp)
          {
          RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
          eptr--;
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          }
        }
      RRETURN(MATCH_NOMATCH);
      }
    }
#ifdef SUPPORT_UTF8
  case OP_XCLASS:
    {
    data = ecode + 1 + LINK_SIZE;
    ecode += GET(ecode, 1);
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
      min = GET2(ecode, 1);
      max = GET2(ecode, 3);
      if (max == 0) max = INT_MAX;
      ecode += 5;
      break;
      default:
      min = max = 1;
      break;
      }
    for (i = 1; i <= min; i++)
      {
      if (eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
      GETCHARINC(c, eptr);
      if (!match_xclass(c, data)) RRETURN(MATCH_NOMATCH);
      }
    if (min == max) continue;
    if (minimize)
      {
      for (fi = min;; fi++)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (fi >= max || eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
        GETCHARINC(c, eptr);
        if (!match_xclass(c, data)) RRETURN(MATCH_NOMATCH);
        }
      }
    else
      {
      pp = eptr;
      for (i = min; i < max; i++)
        {
        int len = 1;
        if (eptr >= md->end_subject) break;
        GETCHARLEN(c, eptr, len);
        if (!match_xclass(c, data)) break;
        eptr += len;
        }
      for(;;)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (eptr-- == pp) break;
        BACKCHAR(eptr)
        }
      RRETURN(MATCH_NOMATCH);
      }
    }
#endif
  case OP_CHAR:
#ifdef SUPPORT_UTF8
  if (md->utf8)
    {
    length = 1;
    ecode++;
    GETCHARLEN(fc, ecode, length);
    if (length > md->end_subject - eptr) RRETURN(MATCH_NOMATCH);
    while (length-- > 0) if (*ecode++ != *eptr++) RRETURN(MATCH_NOMATCH);
    }
  else
#endif
    {
    if (md->end_subject - eptr < 1) RRETURN(MATCH_NOMATCH);
    if (ecode[1] != *eptr++) RRETURN(MATCH_NOMATCH);
    ecode += 2;
    }
  break;
  case OP_CHARNC:
#ifdef SUPPORT_UTF8
  if (md->utf8)
    {
    length = 1;
    ecode++;
    GETCHARLEN(fc, ecode, length);
    if (length > md->end_subject - eptr) RRETURN(MATCH_NOMATCH);
    if (fc < 128)
      {
      if (md->lcc[*ecode++] != md->lcc[*eptr++]) RRETURN(MATCH_NOMATCH);
      }
    else
      {
      int dc;
      GETCHARINC(dc, eptr);
      ecode += length;
      if (fc != dc)
        {
#ifdef SUPPORT_UCP
        int chartype;
        int othercase;
        if (ucp_findchar(fc, &chartype, &othercase) < 0 || dc != othercase)
#endif
          RRETURN(MATCH_NOMATCH);
        }
      }
    }
  else
#endif
    {
    if (md->end_subject - eptr < 1) RRETURN(MATCH_NOMATCH);
    if (md->lcc[ecode[1]] != md->lcc[*eptr++]) RRETURN(MATCH_NOMATCH);
    ecode += 2;
    }
  break;
  case OP_EXACT:
  min = max = GET2(ecode, 1);
  ecode += 3;
  goto REPEATCHAR;
  case OP_UPTO:
  case OP_MINUPTO:
  min = 0;
  max = GET2(ecode, 1);
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
#ifdef SUPPORT_UTF8
  if (md->utf8)
    {
    length = 1;
    charptr = ecode;
    GETCHARLEN(fc, ecode, length);
    if (min * length > md->end_subject - eptr) RRETURN(MATCH_NOMATCH);
    ecode += length;
    if (length > 1)
      {
      int oclength = 0;
      uschar occhars[8];
#ifdef SUPPORT_UCP
      int othercase;
      int chartype;
      if ((ims & PCRE_CASELESS) != 0 &&
           ucp_findchar(fc, &chartype, &othercase) >= 0 &&
           othercase > 0)
        oclength = ord2utf8(othercase, occhars);
#endif
      for (i = 1; i <= min; i++)
        {
        if (memcmp(eptr, charptr, length) == 0) eptr += length;
        else if (oclength == 0) { RRETURN(MATCH_NOMATCH); }
        else
          {
          if (memcmp(eptr, occhars, oclength) != 0) RRETURN(MATCH_NOMATCH);
          eptr += oclength;
          }
        }
      if (min == max) continue;
      if (minimize)
        {
        for (fi = min;; fi++)
          {
          RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
          if (rrc != MATCH_NOMATCH) RRETURN(rrc);
          if (fi >= max || eptr >= md->end_subject) RRETURN(MATCH_NOMATCH);
          if (memcmp(eptr, charptr, length) == 0) eptr += length;
          else if (oclength == 0) { RRETURN(MATCH_NOMATCH); }
          else
            {
            if (memcmp(eptr, occhars, oclength) != 0) RRETURN(MATCH_NOMATCH);
            eptr += oclength;
            }
          }
        }
      else
        {
        pp = eptr;
        for (i = min; i < max; i++)
          {
          if (eptr > md->end_subject - length) break;
          if (memcmp(eptr, charptr, length) == 0) eptr += length;
          else if (oclength == 0) break;
          else
            {
            if (memcmp(eptr, occhars, oclength) != 0) break;
            eptr += oclength;
            }
          }
        while (eptr >= pp)
         {
         RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
         if (rrc != MATCH_NOMATCH) RRETURN(rrc);
         eptr -= length;
         }
        RRETURN(MATCH_NOMATCH);
        }
      }
    }
  else
#endif
    {
    if (min > md->end_subject - eptr) RRETURN(MATCH_NOMATCH);
    fc = *ecode++;
    }
  if ((ims & PCRE_CASELESS) != 0)
    {
    fc = md->lcc[fc];
    for (i = 1; i <= min; i++)
      if (fc != md->lcc[*eptr++]) RRETURN(MATCH_NOMATCH);
    if (min == max) continue;
    if (minimize)
      {
      for (fi = min;; fi++)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (fi >= max || eptr >= md->end_subject ||
            fc != md->lcc[*eptr++])
          RRETURN(MATCH_NOMATCH);
        }
      }
    else
      {
      pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || fc != md->lcc[*eptr]) break;
        eptr++;
        }
      while (eptr >= pp)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        eptr--;
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        }
      RRETURN(MATCH_NOMATCH);
      }
    }
  else
    {
    for (i = 1; i <= min; i++) if (fc != *eptr++) RRETURN(MATCH_NOMATCH);
    if (min == max) continue;
    if (minimize)
      {
      for (fi = min;; fi++)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        if (fi >= max || eptr >= md->end_subject || fc != *eptr++)
          RRETURN(MATCH_NOMATCH);
        }
      }
    else
      {
      pp = eptr;
      for (i = min; i < max; i++)
        {
        if (eptr >= md->end_subject || fc == *eptr) break;
        eptr++;
        }
      while (eptr >= pp)
        {
        RMATCH(rrc, eptr, ecode, offset_top, md, ims, eptrb, 0);
        eptr--;
        if (rrc != MATCH_NOMATCH) RRETURN(rrc);
        }
      RRETURN(MATCH_NOMATCH);
      }
    }
  default:
  RRETURN(PCRE_ERROR_UNKNOWN_NODE);
  }
  }
}