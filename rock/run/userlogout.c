#include "mheads.h"
#include "lheads.h"
#include "member.h"

int main(int argc, char *argv[])
{
	CGI *cgi;
	NEOERR *err;

	//sleep(20);
	mtc_init(TC_ROOT"userlogout");
	mcfg_init(SITE_CONFIG);

	err = cgi_init(&cgi, NULL);
	DIE_NOK_CGI(err);
	err = cgi_parse(cgi);
	DIE_NOK_CGI(err);

	int uin = hdf_get_int_value(cgi->hdf, PRE_COOKIE".uin", 0);
	mtc_info("%d logout", uin);
	member_refresh_info(uin);

	hdf_set_value(cgi->hdf, PRE_OUTPUT".success", "1");
	mjson_output_hdf(cgi->hdf);

	cgi_destroy(&cgi);
	return 0;
}
