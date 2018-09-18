#include <windows.h>
#include "common.h"

static const double ExtDecimalArray[9] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0};

double __fastcall NormalizeDouble(const double val, int digits) {
    if (digits < 0) digits = 0;
    if (digits > 8) digits = 8;

    const double p = ExtDecimalArray[digits];
    return ((val >= 0.0) ? (double(__int64(val * p + 0.5000001)) / p) : (double(__int64(val * p - 0.5000001)) / p));
}

int CheckTemplate(char* expr, char* tok_end, const char* group, char* prev, int* deep) {
    char tmp = 0;
    char *lastwc, *prev_tok;
    const char* cp;
    //---check depth
    if ((*deep)++ >= 10) return (FALSE);
    //--- skip repetition
    while (*expr == '*' && expr != tok_end) expr++;
    if (expr == tok_end) return (TRUE);
    //--- look for next "*"
    lastwc = expr;
    while (*lastwc != '*' && *lastwc != 0) lastwc++;
    //--- temporarily restrict the line
    if ((tmp = *(lastwc)) != 0)  // current not the last line
    {
        tmp = *(lastwc);
        *(lastwc) = 0;
        if ((prev_tok = (char*)strstr(group, expr)) == NULL) {
            if (tmp != 0) *(lastwc) = tmp;
            return (FALSE);
        }
        *(lastwc) = tmp;
    } else {  // last line

        cp = group + strlen(group);
        for (; cp >= group; cp--)
            if (*cp == expr[0] && strcmp(cp, expr) == 0) return (TRUE);
        return (FALSE);
    }
    //--- broken up
    if (prev != NULL && prev_tok <= prev) return (FALSE);
    prev = prev_tok;

    group = prev_tok + (lastwc - expr - 1);
    //--- end
    if (lastwc != tok_end) return CheckTemplate(lastwc, tok_end, group, prev, deep);
    return (TRUE);
}

int CheckGroup(char* grouplist, const char* group) {
    if (grouplist == NULL || group == NULL) return (FALSE);
    //--- go through all the groups
    char *tok_start = grouplist, end;
    int res = TRUE, deep = 0, normal_mode;
    while (*tok_start != 0) {
        //--- skip ','
        while (*tok_start == ',') tok_start++;

        if (*tok_start == '!') {
            tok_start++;
            normal_mode = FALSE;
        } else
            normal_mode = TRUE;
        //--- find the boundaries of the token
        char* tok_end = tok_start;
        while (*tok_end != ',' && *tok_end != 0) tok_end++;
        end = *tok_end;
        *tok_end = NULL;

        char* tp = tok_start;
        const char* gp = group;
        char* prev = NULL;
        //--- go through the token
        res = TRUE;
        while (tp != tok_end && *gp != NULL) {
            //--- find "*"
            if (*tp == '*') {
                deep = 0;
                if ((res = CheckTemplate(tp, tok_end, gp, prev, &deep)) == TRUE) {
                    *tok_end = end;
                    return (normal_mode);
                }
                break;
            }
            //--- just check
            if (*tp != *gp) {
                *tok_end = end;
                res = FALSE;
                break;
            }
            tp++;
            gp++;
        }
        //--- restore
        *tok_end = end;
        //--- we found all tokens
        if (*gp == NULL && (tp == tok_end || *tp == '*') && res == TRUE) return (normal_mode);
        //--- next token
        if (*tok_end == 0) break;
        tok_start = tok_end + 1;
    }

    return (FALSE);
}

double DecPow(const int digits) {
    static double decarray[9] = {1.0, 10.0, 100.0, 1000.0, 10000.0, 100000.0, 1000000.0, 10000000.0, 100000000.0};

    if (digits < 0 || digits > 8) return (0);

    return decarray[digits];
}

int CStrToInt(char* string) {
    if (string != NULL) {
        return atoi(string);
    }
    return 0;
}

char* RemoveWhiteChar(char* str) {
    char *pend = str, *pch = str;
    while (*pch != 0) {
        if (isspace(*pch)) {
            pch++;
            continue;
        }
        if (pend != pch) {
            *pend = *pch;
        }
        pend++;
        pch++;
    }
    *pend = 0;
    return str;
}

char* StrRange(char* str, const char begin, const char end, char** buf) {
    char* cp = str == NULL ? *buf : str;
    if (cp == NULL) {
        return NULL;
    }
    if (*cp == 0) {
        return NULL;
    }
    *buf = NULL;

    if ((cp = strchr(cp, begin)) == NULL) {
        return NULL;
    }
    cp++;
    char* result = cp;

    if ((cp = strchr(cp, end)) == NULL) {
        return NULL;
    }
    *cp = 0;
    cp++;
    if (strchr(result, begin) == NULL) {
        *buf = cp;
        return result;
    }
    return NULL;
}


bool IsDigitalStr(char* string) {
    if (string == NULL) {
        return false;
    }

    while (*string != 0) {
        if (*string < '0' || *string > '9') {
            return false;
        }
        string++;
    }
    return true;
}
