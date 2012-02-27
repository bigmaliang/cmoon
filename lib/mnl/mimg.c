#include "mheads.h"

static char *m_img_types[MIMG_TYPE_UNKNOWN+1] = {"jpeg", "png", "gif", "bmp", "unknown"};

int mimg_type_str2int(char *type)
{
    if (!type) return MIMG_TYPE_UNKNOWN;
    
    if (!strcmp(type, "jpeg"))
        return MIMG_TYPE_JPEG;

    if (!strcmp(type, "png"))
        return MIMG_TYPE_PNG;

    if (!strcmp(type, "gif"))
        return MIMG_TYPE_GIF;

    if (!strcmp(type, "bmp"))
        return MIMG_TYPE_BMP;

    return MIMG_TYPE_UNKNOWN;
}

char* mimg_type_int2str(int type)
{
    if (type < 0 || type > MIMG_TYPE_UNKNOWN) return NULL;
    return m_img_types[type];
}

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

NEOERR* mimg_accept(CGI *cgi, char *form_name, char *imgroot,
                    char result[LEN_MD5], int *ftype)
{
    unsigned char data[IMAGE_MD5_SIZE];
    unsigned int bytes;
    char tok[3] = {0};
    NEOERR *err;
    FILE *fp, *fpout;
    char *s;

    MCS_NOT_NULLC(cgi->hdf, imgroot, ftype);

    /* TODO memory leak */
    fp = mfile_get_safe_from_std(cgi_filehandle(cgi, form_name));
    MCS_NOT_NULLA(fp);

    s = mfile_get_type(cgi, form_name);
    if (!s || strncmp(s, "image/", 6)) {
        return nerr_raise(NERR_ASSERT, "file %s not image type", s);
    }
    s = s + 6;
    *ftype = mimg_type_str2int(s);

    memset(data, 0x0, sizeof(data));
    fseek(fp, 0, SEEK_SET);
    bytes = fread(data, 1, sizeof(data), fp);

    if (bytes <= 0) return nerr_raise(NERR_IO, "read image file error %d ", bytes);
    mstr_md5_buf(data, bytes, result);

    strncpy(tok, result, 2);
    err = mfile_openf(&fpout, "w+", "%s/%s/%s.%s",
                      imgroot, tok, result, mimg_type_int2str(*ftype));
    if (err != STATUS_OK) return nerr_pass(err);

    s = hdf_get_value(cgi->hdf, PRE_QUERY"._upfile_data_type", NULL);
    if (s && !strcmp(s, "dataurl")) {
        err = mb64_decode(fp, fpout);
        if (err != STATUS_OK) return nerr_pass(err);
    } else {
        if (bytes < IMAGE_MD5_SIZE-10) {
            fwrite(data, 1, bytes, fpout);
        } else {
            mfile_copy(fpout, fp);
        }
    }
    fclose(fpout);

    return STATUS_OK;
}

NEOERR* mimg_zoomout(int ftype, FILE *dst, FILE*src, int width, int height)
{
    MCS_NOT_NULLB(dst, src);

    int ow, oh;
    gdImagePtr im;
    FILE *gdin, *gdout;

    if (width <= 0 && height <= 0) return STATUS_OK;

    gdin = mfile_get_std_from_safe(src);
    gdout = mfile_get_std_from_safe(dst);

    fseek(src, 0, SEEK_SET);
    fseek(dst, 0, SEEK_SET);

    switch (ftype) {
    case MIMG_TYPE_JPEG:
        im = gdImageCreateFromJpeg(gdin);
        break;
    case MIMG_TYPE_PNG:
        im = gdImageCreateFromPng(gdin);
        break;
    case MIMG_TYPE_GIF:
        im = gdImageCreateFromGif(gdin);
        break;
    case MIMG_TYPE_BMP:
        im = gdImageCreateFromWBMP(gdin);
        break;
    default:
        return nerr_raise(NERR_ASSERT, "file type %d not support", ftype);
    }
    if (!im) return nerr_raise(NERR_ASSERT, "读取图片出错，文件格式错误？");

    ow = gdImageSX(im);
    oh = gdImageSY(im);

    if ((width > 0 && ow > width) ||
        (height > 0 && oh > height)) {
        if (width <= 0) width = (float)height / oh * ow;
        if (height <= 0) height = (float)width / ow * oh;
        
        gdImagePtr dim = gdImageCreateTrueColor(width, height);
        gdImageCopyResized(dim, im, 0, 0, 0, 0, width, height, ow, oh);

        if (dim) {
            switch (ftype) {
            case MIMG_TYPE_JPEG:
                gdImageJpeg(dim, gdout, 70);
                break;
            case MIMG_TYPE_PNG:
                gdImagePng(dim, gdout);
                break;
            case MIMG_TYPE_GIF:
                gdImageGif(dim, gdout);
                break;
            case MIMG_TYPE_BMP:
                gdImageWBMP(dim, 0, gdout);
                break;
            default:
                return nerr_raise(NERR_ASSERT, "file type %d not suport", ftype);
            }
            
        } else return nerr_raise(NERR_ASSERT, "resize image error");
    } else {
        mfile_copy(dst, src);
    }

    return STATUS_OK;
}

NEOERR* mimg_accept_and_zoomout(CGI *cgi, char* form_name, char *imgroot,
                                char result[LEN_MD5], int *ftype,
                                int width, int height)
{
    NEOERR *err;
    FILE *fp, *fpout;
    char tok[3] = {0};

    err = mimg_accept(cgi, form_name, imgroot, result, ftype);
    if (err != STATUS_OK) return nerr_pass(err);

    strncpy(tok, result, 2);

    err = mfile_openf(&fp, "r", "%s/%s/%s.%s",
                      imgroot, tok, result, mimg_type_int2str(*ftype));
	if (err != STATUS_OK) return nerr_pass(err);

    err = mfile_openf(&fpout, "w+", "%s/%dx%d/%s/%s.%s",
                      imgroot, width, height, tok, result,
                      mimg_type_int2str(*ftype));
    if (err != STATUS_OK) return nerr_pass(err);
    
    err = mimg_zoomout(*ftype, fpout, fp, width, height);
    if (err != STATUS_OK) return nerr_pass(err);

    fclose(fpout);

    return STATUS_OK;
}
