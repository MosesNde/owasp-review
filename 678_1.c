static apr_status_t command(sed_eval_t *eval, sed_reptr_t *ipc,
                            step_vars_storage *step_vars)
{
    int    i;
    char   *p1, *p2, *p3;
    int length;
    char sz[32];
    apr_status_t rv = APR_SUCCESS;
    switch(ipc->command) {
        case ACOM:
            if(eval->aptr >= &eval->abuf[SED_ABUFSIZE]) {
                eval_errf(eval, SEDERR_TMAMES, eval->lnum);
            } else {
                *eval->aptr++ = ipc;
                *eval->aptr = NULL;
            }
            break;
        case CCOM:
            eval->delflag = 1;
            if(!eval->inar[ipc->nrep] || eval->dolflag) {
                for (p1 = ipc->re1; *p1; p1++)
                    ;
                rv = wline(eval, ipc->re1, p1 - ipc->re1);
            }
            break;
        case DCOM:
            eval->delflag++;
            break;
        case CDCOM:
            p1 = eval->linebuf;
            while(*p1 != '\n') {
                if(*p1++ == 0) {
                    eval->delflag++;
                    return APR_SUCCESS;
                }
            }
            p1++;
            copy_to_linebuf(eval, p1);
            eval->jflag++;
            break;
        case EQCOM:
            length = apr_snprintf(sz, sizeof(sz), "%d", (int) eval->lnum);
            rv = wline(eval, sz, length);
            break;
        case GCOM:
            copy_to_linebuf(eval, eval->holdbuf);
            break;
        case CGCOM:
            append_to_linebuf(eval, "\n");
            append_to_linebuf(eval, eval->holdbuf);
            break;
        case HCOM:
            copy_to_holdbuf(eval, eval->linebuf);
            break;
        case CHCOM:
            append_to_holdbuf(eval, "\n");
            append_to_holdbuf(eval, eval->linebuf);
            break;
        case ICOM:
            for (p1 = ipc->re1; *p1; p1++);
            rv = wline(eval, ipc->re1, p1 - ipc->re1);
            break;
        case BCOM:
            eval->jflag = 1;
            break;
        case LCOM:
            p1 = eval->linebuf;
            p2 = eval->genbuf;
            eval->genbuf[72] = 0;
            while(*p1)
                if((unsigned char)*p1 >= 040) {
                    if(*p1 == 0177) {
                        p3 = rub;
                        while ((*p2++ = *p3++) != 0)
                            if(p2 >= eval->lcomend) {
                                *p2 = '\\';
                                rv = wline(eval, eval->genbuf,
                                           strlen(eval->genbuf));
                                if (rv != APR_SUCCESS)
                                    return rv;
                                p2 = eval->genbuf;
                            }
                        p2--;
                        p1++;
                        continue;
                    }
                    if(!isprint(*p1 & 0377)) {
                        *p2++ = '\\';
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                        *p2++ = (*p1 >> 6) + '0';
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                        *p2++ = ((*p1 >> 3) & 07) + '0';
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                        *p2++ = (*p1++ & 07) + '0';
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                    } else {
                        *p2++ = *p1++;
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                    }
                } else {
                    p3 = trans[(unsigned char)*p1-1];
                    while ((*p2++ = *p3++) != 0)
                        if(p2 >= eval->lcomend) {
                            *p2 = '\\';
                            rv = wline(eval, eval->genbuf,
                                       strlen(eval->genbuf));
                            if (rv != APR_SUCCESS)
                                return rv;
                            p2 = eval->genbuf;
                        }
                    p2--;
                    p1++;
                }
            *p2 = 0;
            rv = wline(eval, eval->genbuf, strlen(eval->genbuf));
            break;
        case NCOM:
            if(!eval->commands->nflag) {
                rv = wline(eval, eval->linebuf, eval->lspend - eval->linebuf);
                if (rv != APR_SUCCESS)
                    return rv;
            }
            if(eval->aptr > eval->abuf) {
                rv = arout(eval);
                if (rv != APR_SUCCESS)
                    return rv;
            }
            eval->lspend = eval->linebuf;
            eval->pending = ipc->next;
            break;
        case CNCOM:
            if(eval->aptr > eval->abuf) {
                rv = arout(eval);
                if (rv != APR_SUCCESS)
                    return rv;
            }
            append_to_linebuf(eval, "\n");
            eval->pending = ipc->next;
            break;
        case PCOM:
            rv = wline(eval, eval->linebuf, eval->lspend - eval->linebuf);
            break;
        case CPCOM:
            for (p1 = eval->linebuf; *p1 != '\n' && *p1 != '\0'; p1++);
            rv = wline(eval, eval->linebuf, p1 - eval->linebuf);
            break;
        case QCOM:
            if (!eval->commands->nflag) {
                rv = wline(eval, eval->linebuf, eval->lspend - eval->linebuf);
                if (rv != APR_SUCCESS)
                    break;
            }
            if(eval->aptr > eval->abuf) {
                rv = arout(eval);
                if (rv != APR_SUCCESS)
                    return rv;
            }
            eval->quitflag = 1;
            break;
        case RCOM:
            if(eval->aptr >= &eval->abuf[SED_ABUFSIZE]) {
                eval_errf(eval, SEDERR_TMRMES, eval->lnum);
            } else {
                *eval->aptr++ = ipc;
                *eval->aptr = NULL;
            }
            break;
        case SCOM:
            i = substitute(eval, ipc, step_vars);
            if (i == -1) {
                return APR_EGENERAL;
            }
            if(ipc->pfl && eval->commands->nflag && i) {
                if(ipc->pfl == 1) {
                    rv = wline(eval, eval->linebuf, eval->lspend -
                               eval->linebuf);
                    if (rv != APR_SUCCESS)
                        return rv;
                } else {
                    for (p1 = eval->linebuf; *p1 != '\n' && *p1 != '\0'; p1++);
                    rv = wline(eval, eval->linebuf, p1 - eval->linebuf);
                    if (rv != APR_SUCCESS)
                        return rv;
                }
            }
            if (i && (ipc->findex >= 0) && eval->fcode[ipc->findex])
                apr_file_printf(eval->fcode[ipc->findex], "%s\n",
                                eval->linebuf);
            break;
        case TCOM:
            if(eval->sflag == 0)  break;
            eval->sflag = 0;
            eval->jflag = 1;
            break;
        case WCOM:
            if (ipc->findex >= 0)
                apr_file_printf(eval->fcode[ipc->findex], "%s\n",
                                eval->linebuf);
            break;
        case XCOM:
            copy_to_genbuf(eval, eval->linebuf);
            copy_to_linebuf(eval, eval->holdbuf);
            copy_to_holdbuf(eval, eval->genbuf);
            break;
        case YCOM:
            p1 = eval->linebuf;
            p2 = ipc->re1;
            while((*p1 = p2[(unsigned char)*p1]) != 0)    p1++;
            break;
    }
    return rv;
}

#include "apr-1.0/old_apr.h"