#include "mheads.h"

int http_req_method(CGI *cgi)
{
    char *op = hdf_get_value(cgi->hdf, PRE_QUERY"._op", "get");
    if (!strcasecmp(op, "get")) return CGI_REQ_GET;
    else if (!strcasecmp(op, "mod")) return CGI_REQ_POST;
    else if (!strcasecmp(op, "add")) return CGI_REQ_PUT;
    else if (!strcasecmp(op, "del")) return CGI_REQ_DEL;
    
    return CGI_REQ_UNKNOWN;
}

NEOERR* mhttp_upload_parse_cb(CGI *cgi, char *method, char *ctype, void *rock)
{
    MCS_NOT_NULLC(cgi, method, ctype);
    
    if (!strcasecmp(method, "GET"))
        return nerr_raise(CGIParseNotHandled, "%s not handled", method);
    
    int len = hdf_get_int_value(cgi->hdf, "CGI.ContentLength", 0);
    
    if (len <= 0 || len > *(int*)rock)
        return nerr_raise(CGIUploadCancelled, "content length %d not support", len);
    
    return STATUS_OK;
}

/*
 * IE: make sure timezone & time set correct on web server
 */
void mhttp_cache_headers(time_t second)
{
    /*
    char my_time[256];
    time_t now = time(NULL);
    time_t exp_date = now + second;
    time_t mod_date = now - second;
    */
        
    //cgiwrap_writef("Cache-Control: public, max-age=%lu\r\n", second);
    cgiwrap_writef("Cache-Control: max-age=%lu\r\n", second);

    /*
    strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
              gmtime (&exp_date));
    cgiwrap_writef ("Expires: %s\r\n", my_time);
    */
        
    //strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
    //          gmtime (&mod_date));
    //cgiwrap_writef ("Last-Modified: %s\r\n", my_time);
    //cgiwrap_writef ("Last-Modified: Tue, 02 Jun 2009 05:21:07 GMT\r\n");

    /*
    // Date: should return by web server
    strftime (my_time, 48, "%A, %d-%b-%Y %H:%M:%S GMT",
              gmtime (&now));
    cgiwrap_writef ("Date: %s\r\n", my_time);
    */
}

int read_cb(void *ptr, char *data, int size) {
    return fread(data, sizeof(char), size, stdin);
}
int printf_cb(void *ptr, const char *format, va_list ap) {
    return vprintf(format, ap);
}
int write_cb(void *ptr, const char *data, int size) {
    return fwrite((void *)data, sizeof(char), size, stdout);
}
