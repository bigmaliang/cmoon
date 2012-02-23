#include "mheads.h"

NEOERR* mimg_create_from_string(char *s, char *path, double size, void **pic)
{
    gdImagePtr im;
    int draw, back, brect[8], x, y;
    char *gderr;

    MCS_NOT_NULLB(s, pic);
    
    gderr = gdImageStringFT(NULL, &brect[0], 0, path, size, 0., 0, 0, s);
    if (gderr) return nerr_raise(NERR_ASSERT, "create image failure %s", gderr);
    
    x = brect[2]-brect[6] + 6;
    y = brect[3]-brect[7] + 6;
    im = gdImageCreate(x, y);

    if (!im) return nerr_raise(NERR_ASSERT, "create image failure");
    
    /* background color */
    back = gdImageColorResolve(im, 252, 252, 252);
    /* foreground color */
    draw = gdImageColorResolve(im, 150, 40, 40);
    
    x = 3 - brect[6];
    y = 3 - brect[7];
    
    gderr = gdImageStringFT(im, &brect[0], draw, path, size, 0.0, x, y, s);
    if (gderr) return nerr_raise(NERR_ASSERT, "rend image failure %s", gderr);
    
    *pic = (void*)im;

    return STATUS_OK;
}

NEOERR* mimg_output(void *pic)
{
    NEOERR *err;
    
    MCS_NOT_NULLA(pic);

    gdImagePtr im = (gdImagePtr)pic;
    char *temps; int tempi;
    
    err = cgiwrap_writef("Content-Type: image/jpeg\r\n\r\n");
    if (err != STATUS_OK) return nerr_pass(err);
    
    /*
     * gdImageJpegCtx(data, gdoutctx, -1) core dump on fastcgi mode
     */
    temps = (char*) gdImageJpegPtr(im, &tempi, -1);
    cgiwrap_write(temps, tempi);
    gdImageDestroy(im);
    gdFree(temps);

    return STATUS_OK;
}

NEOERR* mimg_accept(CGI *cgi, char *imgroot, char result[LEN_MD5])
{
    NEOERR *err;
    /*
     * fastcgi redefined FILE*, but cgi_filehandle() don't know,
     * so, cgi_filehandle() return's a standard FILE*
     * adn fseek, fread... need FCGI_FILE* as parameter in fastcgi mode
     * so, create a FCGI_FILE* from cgi_filehandle()'s FILE*.
     * ****************************************************************
     * you need to take care of every 3rd part library's file operation
     * e.g. gdImageXxx()
     *
     * you don't need to take care of fopen(), fseek()... on stdio.h
     * because of fcgi_stdio.h do it for you
     * ****************************************************************
     */
#ifdef USE_FASTCGI
    FILE *fp = malloc(sizeof(FILE)), *fpout;
    if (fp) {
        fp->stdio_stream = cgi_filehandle(cgi, NULL);
        fp->fcgx_stream = NULL;
    }
#else
    FILE *fp = cgi_filehandle(cgi, NULL), *fpout;
#endif
    unsigned char data[IMAGE_MD5_SIZE];
    unsigned int bytes;
    char tok[3] = {0};

    MCS_NOT_NULLB(cgi->hdf, fp);

    memset(data, 0x0, sizeof(data));

    fseek(fp, 0, SEEK_SET);
    bytes = fread(data, 1, sizeof(data), fp);

    if (bytes <= 0) return nerr_raise(NERR_IO, "read image file error %d ", bytes);
    mstr_md5_buf(data, bytes, result);

    strncpy(tok, result, 2);
    err = mutil_file_openf(&fpout, "w+", "%s/%s/%s.jpg", imgroot, tok, result);
    if (err != STATUS_OK) return nerr_pass(err);

    if (bytes < IMAGE_MD5_SIZE-10) {
        fwrite(data, 1, bytes, fpout);
    } else {
        mutil_file_copy(fpout, fp);
    }
    fclose(fpout);

    return STATUS_OK;
}

NEOERR* mimg_zoomout(FILE *dst, FILE*src, int width, int height)
{
    MCS_NOT_NULLB(dst, src);

    if (width <= 0 && height <= 0) return STATUS_OK;
    
    fseek(src, 0, SEEK_SET);
    fseek(dst, 0, SEEK_SET);

#ifdef USE_FASTCGI
    gdImagePtr im = gdImageCreateFromJpeg(src->stdio_stream);
#else
    gdImagePtr im = gdImageCreateFromJpeg(src);
#endif
    int ow, oh;

    if (!im) return nerr_raise(NERR_ASSERT, "load image failure");

    ow = gdImageSX(im);
    oh = gdImageSY(im);

    if ((width > 0 && ow > width) ||
        (height > 0 && oh > height)) {
        if (width <= 0) width = (float)height / oh * ow;
        if (height <= 0) height = (float)width / ow * oh;
        
        gdImagePtr dim = gdImageCreateTrueColor(width, height);
        gdImageCopyResized(dim, im, 0, 0, 0, 0, width, height, ow, oh);
#ifdef USE_FASTCGI
        if (dim) gdImageJpeg(dim, dst->stdio_stream, 70);
#else
        if (dim) gdImageJpeg(dim, dst, 70);
#endif
        else return nerr_raise(NERR_ASSERT, "resize image error");
    } else {
        mutil_file_copy(dst, src);
    }

    return STATUS_OK;
}

NEOERR* mimg_accept_and_zoomout(CGI *cgi, char *imgroot, char result[LEN_MD5],
                                int width, int height)
{
    NEOERR *err;

    err = mimg_accept(cgi, imgroot, result);
    if (err != STATUS_OK) return nerr_pass(err);

#ifdef USE_FASTCGI
    FILE *fp = malloc(sizeof(FILE));
    if (fp) {
        fp->stdio_stream = cgi_filehandle(cgi, NULL);
        fp->fcgx_stream = NULL;
    }
#else
    FILE *fp = cgi_filehandle(cgi, NULL);
#endif
    if (!fp) return nerr_raise(NERR_ASSERT, "unbelieveable, fp null");

    FILE *fpout;
    char tok[3] = {0}; strncpy(tok, result, 2);
    err = mutil_file_openf(&fpout, "w+", "%s/%dx%d/%s/%s.jpg",
                           imgroot, width, height, tok, result);
    if (err != STATUS_OK) return nerr_pass(err);

    err = mimg_zoomout(fpout, fp, width, height);
    if (err != STATUS_OK) return nerr_pass(err);

    fclose(fpout);

    return STATUS_OK;
}
