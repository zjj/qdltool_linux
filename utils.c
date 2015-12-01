#include "utils.h"
#include <matheval.h>

size_t evaluator(char *buffer)
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

size_t firehose_strtoint(char *s)
{
    char buffer[128] = {0};
    char num_disk_sectors[128] = {0};
    sprintf(num_disk_sectors, "%zu", NUM_DISK_SECTORS);
    char *eval;

    eval =  strrep(s, "NUM_DISK_SECTORS", num_disk_sectors);
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, eval);
    free(eval);
    
    eval = strrep(buffer, ".", "");
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, eval);  
    free(eval);

    return evaluator(buffer);
}

/*
    please ignor this f.. function
*/
int yywrap(){
    return 0;
}
