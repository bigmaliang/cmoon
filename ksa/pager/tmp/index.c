#include "mheads.h"
#include "lcfg.h"

int main()
{
	int ret;

	mtc_init(TC_ROOT"index");

	tpl_t *tpl = tpl_alloc();
	tpl_t *tpl2 = tpl_alloc();
	vector crumbs = vector_new();
	s_anchor *crumb;
	char fn[LEN_FN];
	int i;

	ret = tpl_load(tpl, F_TPL_LAYOUT);
	if (ret != TPL_OK) {
		TRACE_DIE(("%s, %s", F_TPL_LAYOUT, tpl_error(ret)));
	}

	for (i = 0; i < NAV_NUM; i++) {
		tpl_reset(tpl);
		tpl_clear(tpl2);
		vector_clear(crumbs);
		/* base_layout */
		crumb = anchor_new(g_nav[i].name, g_nav[i].href,
				   g_nav[i].title, g_nav[i].target);
		vector_push_back(crumbs, item_ptr(crumb));
		page_reinit_layout(tpl, g_nav[i].name, crumbs);
		anchor_del(crumb);

		/* fill content */
		sprintf(fn, "%s/%s", PATH_FRT_TPL, g_nav[i].href+1);
		ret = tpl_load(tpl2, fn);
		if (ret != TPL_OK) {
			TRACE_ERROR(("%s, %s", fn, tpl_error(ret)));
			continue;
		}
		tpl_set_field_from_tpl(tpl, tpl2, "COLONE", "COLTWO", NULL);

		/* write output */
		sprintf(fn, "%s/%s", PATH_FRT_DOC, g_nav[i].href+1);
		ret = tpl_save_as(tpl, fn);
		if (ret != TPL_OK) {
			TRACE_ERROR(("%s, %s", fn, tpl_error(ret)));
		}
	}

	vector_delete(crumbs);
	tpl_free(tpl);
	tpl_free(tpl2);

	return 0;
}
