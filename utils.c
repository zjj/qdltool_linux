#include "utils.h"

size_t evaluator(const char *buffer, char *result)
{
    size_t ret = 0;
    void *f = evaluator_create (buffer);
    if (!f)
        xerror("error while evaluator %s", buffer);
    char *s = evaluator_get_string(f);
    if(!s)
        xerror("error while evaluator %s evaluator_get_string", buffer);
    sscanf(s, "%zu", &ret);
    free(s);
    evaluator_destroy(f);
    return ret;
}

char *strrep(const char *string, const char *substr, const char *replacement)
{
  char *tok = NULL;
  char *newstr = NULL;
  char *oldstr = NULL;
  /* if either substr or replacement is NULL, duplicate string a let caller handle it */
  if ( substr == NULL || replacement == NULL ) return strdup (string);
  newstr = strdup (string);
  while ( (tok = strstr ( newstr, substr ))){
    oldstr = newstr;
    newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
    /*failed to alloc mem, free old string and return NULL */
    if ( newstr == NULL ){
      free (oldstr);
      return NULL;
    }
    memcpy ( newstr, oldstr, tok - oldstr );
    memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
    memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
    memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
    free (oldstr);
  }
  return newstr;
}
