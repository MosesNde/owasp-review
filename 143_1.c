static int
check_escape(const uschar **ptrptr, const char **errorptr, int bracount,
  int options, BOOL isclass)
{
const uschar *ptr = *ptrptr;
int c, i;
c = *(++ptr);
if (c == 0) *errorptr = ERR1;
#if !EBCDIC
else if (c < '0' || c > 'z') {}
else if ((i = escapes[c - '0']) != 0) c = i;
#else
else if (c < 'a' || (ebcdic_chartab[c] & 0x0E) == 0) {}
else if ((i = escapes[c - 0x48]) != 0)  c = i;
#endif
else
  {
  const uschar *oldptr;
  switch (c)
    {
    case 'l':
    case 'L':
    case 'N':
    case 'u':
    case 'U':
    *errorptr = ERR37;
    break;
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
    if (!isclass)
      {
      oldptr = ptr;
      c -= '0';
      while ((digitab[ptr[1]] & ctype_digit) != 0)
        c = c * 10 + *(++ptr) - '0';
      if (c < 10 || c <= bracount)
        {
        c = -(ESC_REF + c);
        break;
        }
      ptr = oldptr;
      }
    if ((c = *ptr) >= '8')
      {
      ptr--;
      c = 0;
      break;
      }
    case '0':
    c -= '0';
    while(i++ < 2 && ptr[1] >= '0' && ptr[1] <= '7')
        c = c * 8 + *(++ptr) - '0';
    c &= 255;
    break;
    case 'x':
#ifdef SUPPORT_UTF8
    if (ptr[1] == '{' && (options & PCRE_UTF8) != 0)
      {
      const uschar *pt = ptr + 2;
      register int count = 0;
      c = 0;
      while ((digitab[*pt] & ctype_xdigit) != 0)
        {
        int cc = *pt++;
        count++;
#if !EBCDIC
        if (cc >= 'a') cc -= 32;
        c = c * 16 + cc - ((cc < 'A')? '0' : ('A' - 10));
#else
        if (cc >= 'a' && cc <= 'z') cc += 64;
        c = c * 16 + cc - ((cc >= '0')? '0' : ('A' - 10));
#endif
        }
      if (*pt == '}')
        {
        if (c < 0 || count > 8) *errorptr = ERR34;
        ptr = pt;
        break;
        }
      }
#endif
    c = 0;
    while (i++ < 2 && (digitab[ptr[1]] & ctype_xdigit) != 0)
      {
      int cc;
      cc = *(++ptr);
#if !EBCDIC
      if (cc >= 'a') cc -= 32;
      c = c * 16 + cc - ((cc < 'A')? '0' : ('A' - 10));
#else
      if (cc <= 'z') cc += 64;
      c = c * 16 + cc - ((cc >= '0')? '0' : ('A' - 10));
#endif
      }
    break;
    case 'c':
    c = *(++ptr);
    if (c == 0)
      {
      *errorptr = ERR2;
      return 0;
      }
#if !EBCDIC
    if (c >= 'a' && c <= 'z') c -= 32;
    c ^= 0x40;
#else
    if (c >= 'a' && c <= 'z') c += 64;
    c ^= 0xC0;
#endif
    break;
    default:
    break;
    }
  }
*ptrptr = ptr;
return c;
}