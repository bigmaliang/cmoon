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
    FILE *fp = cgi_filehandle(cgi, NULL);
    unsigned char data[IMAGE_MD5_SIZE];
    unsigned int bytes;
    char fname[LEN_FN];
    char tok[3] = {0};

    MCS_NOT_NULLB(cgi->hdf, fp);

    memset(data, 0x0, sizeof(data));

    fseek(fp, 0, SEEK_SET);
    bytes = fread(data, 1, sizeof(data), fp);

    if (bytes <= 0) return nerr_raise(NERR_IO, "read image file error %d ", bytes);
    mstr_md5_buf(data, bytes, result);

    strncpy(tok, result, 2);
    snprintf(fname, sizeof(fname), "%s/%s/%s.jpg", imgroot, tok, result);

    err = mutil_makesure_dir(fname);
	if (err != STATUS_OK) return nerr_pass(err);

    FILE *fpout = fopen(fname, "w+");
    if (!fpout) return nerr_raise(NERR_SYSTEM, "create %s failre", fname);
    fwrite(data, 1, bytes, fpout);
    
    if (bytes >= IMAGE_MD5_SIZE-10) {
        while ((bytes = fread(data, 1, sizeof(data), fp)) > 0)
            fwrite(data, 1, bytes, fpout);
    }

    fclose(fpout);

    return STATUS_OK;
}
