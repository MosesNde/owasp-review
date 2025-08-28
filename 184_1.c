int
regexec(regex_t *preg, const char *string, size_t nmatch,
  regmatch_t pmatch[], int eflags)
{
int rc;
int options = 0;
int *ovector = NULL;
if ((eflags & REG_NOTBOL) != 0) options |= PCRE_NOTBOL;
if ((eflags & REG_NOTEOL) != 0) options |= PCRE_NOTEOL;
ovector = (int *)malloc(sizeof(int) * nmatch * 3);
if (ovector == NULL) return REG_ESPACE;
rc = pcre_exec(preg->re_pcre, NULL, string, (int)strlen(string), 0, options,
  ovector, nmatch * 3);
if (rc == 0) rc = nmatch;
if (rc >= 0)
  {
  size_t i;
  for (i = 0; i < (size_t) rc; i++)
    {
    pmatch[i].rm_so = ovector[i*2];
    pmatch[i].rm_eo = ovector[i*2+1];
    }
  free(ovector);
  for (; i < nmatch; i++) pmatch[i].rm_so = pmatch[i].rm_eo = -1;
  return 0;
  }
else
  {
  free(ovector);
  switch(rc)
    {
    case PCRE_ERROR_NOMATCH: return REG_NOMATCH;
    case PCRE_ERROR_NULL: return REG_INVARG;
    case PCRE_ERROR_BADOPTION: return REG_INVARG;
    case PCRE_ERROR_BADMAGIC: return REG_INVARG;
    case PCRE_ERROR_UNKNOWN_NODE: return REG_ASSERT;
    case PCRE_ERROR_NOMEMORY: return REG_ESPACE;
    default: return REG_ASSERT;
    }
  }
}