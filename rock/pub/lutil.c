#include "mheads.h"
#include "lheads.h"

#include "member.h"
#include "admin.h"
#include "service.h"
#include "static.h"
/*
 * TODO how make local dlsym ok? so tired 
 */
static void lutil_donotcall()
{
	member_login_data_get(NULL, NULL, NULL);
	admin_account_data_add(NULL, NULL, NULL);
	service_action_data_get(NULL, NULL, NULL);
	static_csc_data_get(NULL, NULL);
}

int CGI_REQ_TYPE(CGI *cgi)
{
	return hdf_get_int_value(cgi->hdf, PRE_RSV_REQ_TYPE, 0);
}

void* lutil_get_data_handler(void *lib, CGI *cgi)
{
	char *file, *tp;
	char hname[_POSIX_PATH_MAX];
	void *res;

	/*
	 * how to distinguish /music/zhangwei and /member/zhangwei ?
	 * /music/zhangwei may be use music_data_xxx
	 * /member/zhangwei may be use member_data_xxx
	 */
	file = hdf_get_value(cgi->hdf, PRE_RSV_DATAER, NULL);
	if (file == NULL) {
		mtc_err("%s not found", PRE_RSV_DATAER);
		return NULL;
	}
	
	switch (CGI_REQ_METHOD(cgi)) {
		case CGI_REQ_GET:
			snprintf(hname, sizeof(hname), "%s_data_get", file);
			break;
		case CGI_REQ_POST:
			snprintf(hname, sizeof(hname), "%s_data_mod", file);
			break;
		case CGI_REQ_PUT:
			snprintf(hname, sizeof(hname), "%s_data_add", file);
			break;
		case CGI_REQ_DEL:
			snprintf(hname, sizeof(hname), "%s_data_del", file);
			break;
		default:
			mtc_err("op not support");
			return NULL;
	}

	res = dlsym(lib, hname);
	if ((tp = dlerror()) != NULL) {
		mtc_err("%s", tp);
		return NULL;
	} else
		mtc_info("%s found for data handler", hname);
	return res;
}

