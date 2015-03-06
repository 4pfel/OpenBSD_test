/*	$Id: mdoc_validate.c,v 1.148 2014/07/07 16:12:06 schwarze Exp $ */
/*
 * Copyright (c) 2008-2012 Kristaps Dzonsons <kristaps@bsd.lv>
 * Copyright (c) 2010-2014 Ingo Schwarze <schwarze@openbsd.org>
 * Copyright (c) 2010 Joerg Sonnenberger <joerg@netbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef OSNAME
#include <sys/utsname.h>
#endif

#include <sys/types.h>

#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "mdoc.h"
#include "mandoc.h"
#include "mandoc_aux.h"
#include "libmdoc.h"
#include "libmandoc.h"

/* FIXME: .Bl -diag can't have non-text children in HEAD. */

#define	PRE_ARGS  struct mdoc *mdoc, struct mdoc_node *n
#define	POST_ARGS struct mdoc *mdoc

enum	check_ineq {
	CHECK_LT,
	CHECK_GT,
	CHECK_EQ
};

enum	check_lvl {
	CHECK_WARN,
	CHECK_ERROR,
};

typedef	int	(*v_pre)(PRE_ARGS);
typedef	int	(*v_post)(POST_ARGS);

struct	valids {
	v_pre	*pre;
	v_post	*post;
};

static	int	 check_count(struct mdoc *, enum mdoc_type,
			enum check_lvl, enum check_ineq, int);
static	int	 check_parent(PRE_ARGS, enum mdoct, enum mdoc_type);
static	void	 check_text(struct mdoc *, int, int, char *);
static	void	 check_argv(struct mdoc *,
			struct mdoc_node *, struct mdoc_argv *);
static	void	 check_args(struct mdoc *, struct mdoc_node *);
static	enum mdoc_sec	a2sec(const char *);
static	size_t		macro2len(enum mdoct);

static	int	 ebool(POST_ARGS);
static	int	 berr_ge1(POST_ARGS);
static	int	 bwarn_ge1(POST_ARGS);
static	int	 ewarn_eq0(POST_ARGS);
static	int	 ewarn_eq1(POST_ARGS);
static	int	 ewarn_ge1(POST_ARGS);
static	int	 ewarn_le1(POST_ARGS);
static	int	 hwarn_eq0(POST_ARGS);
static	int	 hwarn_eq1(POST_ARGS);
static	int	 hwarn_ge1(POST_ARGS);

static	int	 post_an(POST_ARGS);
static	int	 post_at(POST_ARGS);
static	int	 post_bf(POST_ARGS);
static	int	 post_bl(POST_ARGS);
static	int	 post_bl_block(POST_ARGS);
static	int	 post_bl_block_width(POST_ARGS);
static	int	 post_bl_block_tag(POST_ARGS);
static	int	 post_bl_head(POST_ARGS);
static	int	 post_bx(POST_ARGS);
static	int	 post_defaults(POST_ARGS);
static	int	 post_dd(POST_ARGS);
static	int	 post_dt(POST_ARGS);
static	int	 post_en(POST_ARGS);
static	int	 post_es(POST_ARGS);
static	int	 post_eoln(POST_ARGS);
static	int	 post_hyph(POST_ARGS);
static	int	 post_ignpar(POST_ARGS);
static	int	 post_it(POST_ARGS);
static	int	 post_lb(POST_ARGS);
static	int	 post_literal(POST_ARGS);
static	int	 post_nm(POST_ARGS);
static	int	 post_ns(POST_ARGS);
static	int	 post_os(POST_ARGS);
static	int	 post_par(POST_ARGS);
static	int	 post_prol(POST_ARGS);
static	int	 post_root(POST_ARGS);
static	int	 post_rs(POST_ARGS);
static	int	 post_sh(POST_ARGS);
static	int	 post_sh_body(POST_ARGS);
static	int	 post_sh_head(POST_ARGS);
static	int	 post_st(POST_ARGS);
static	int	 post_std(POST_ARGS);
static	int	 post_vt(POST_ARGS);
static	int	 pre_an(PRE_ARGS);
static	int	 pre_bd(PRE_ARGS);
static	int	 pre_bl(PRE_ARGS);
static	int	 pre_dd(PRE_ARGS);
static	int	 pre_display(PRE_ARGS);
static	int	 pre_dt(PRE_ARGS);
static	int	 pre_it(PRE_ARGS);
static	int	 pre_literal(PRE_ARGS);
static	int	 pre_obsolete(PRE_ARGS);
static	int	 pre_os(PRE_ARGS);
static	int	 pre_par(PRE_ARGS);
static	int	 pre_sh(PRE_ARGS);
static	int	 pre_ss(PRE_ARGS);
static	int	 pre_std(PRE_ARGS);

static	v_post	 posts_an[] = { post_an, NULL };
static	v_post	 posts_at[] = { post_at, post_defaults, NULL };
static	v_post	 posts_bd[] = { post_literal, hwarn_eq0, bwarn_ge1, NULL };
static	v_post	 posts_bf[] = { post_bf, NULL };
static	v_post	 posts_bk[] = { hwarn_eq0, bwarn_ge1, NULL };
static	v_post	 posts_bl[] = { bwarn_ge1, post_bl, NULL };
static	v_post	 posts_bx[] = { post_bx, NULL };
static	v_post	 posts_bool[] = { ebool, NULL };
static	v_post	 posts_eoln[] = { post_eoln, NULL };
static	v_post	 posts_defaults[] = { post_defaults, NULL };
static	v_post	 posts_d1[] = { bwarn_ge1, post_hyph, NULL };
static	v_post	 posts_dd[] = { post_dd, post_prol, NULL };
static	v_post	 posts_dl[] = { post_literal, bwarn_ge1, NULL };
static	v_post	 posts_dt[] = { post_dt, post_prol, NULL };
static	v_post	 posts_en[] = { post_en, NULL };
static	v_post	 posts_es[] = { post_es, NULL };
static	v_post	 posts_fo[] = { hwarn_eq1, bwarn_ge1, NULL };
static	v_post	 posts_hyph[] = { post_hyph, NULL };
static	v_post	 posts_hyphtext[] = { ewarn_ge1, post_hyph, NULL };
static	v_post	 posts_it[] = { post_it, NULL };
static	v_post	 posts_lb[] = { post_lb, NULL };
static	v_post	 posts_nd[] = { berr_ge1, post_hyph, NULL };
static	v_post	 posts_nm[] = { post_nm, NULL };
static	v_post	 posts_notext[] = { ewarn_eq0, NULL };
static	v_post	 posts_ns[] = { post_ns, NULL };
static	v_post	 posts_os[] = { post_os, post_prol, NULL };
static	v_post	 posts_pp[] = { post_par, ewarn_eq0, NULL };
static	v_post	 posts_rs[] = { post_rs, NULL };
static	v_post	 posts_sh[] = { post_ignpar,hwarn_ge1,post_sh,post_hyph,NULL };
static	v_post	 posts_sp[] = { post_par, ewarn_le1, NULL };
static	v_post	 posts_ss[] = { post_ignpar, hwarn_ge1, post_hyph, NULL };
static	v_post	 posts_st[] = { post_st, NULL };
static	v_post	 posts_std[] = { post_std, NULL };
static	v_post	 posts_text[] = { ewarn_ge1, NULL };
static	v_post	 posts_text1[] = { ewarn_eq1, NULL };
static	v_post	 posts_vt[] = { post_vt, NULL };
static	v_pre	 pres_an[] = { pre_an, NULL };
static	v_pre	 pres_bd[] = { pre_display, pre_bd, pre_literal, pre_par, NULL };
static	v_pre	 pres_bl[] = { pre_bl, pre_par, NULL };
static	v_pre	 pres_d1[] = { pre_display, NULL };
static	v_pre	 pres_dl[] = { pre_literal, pre_display, NULL };
static	v_pre	 pres_dd[] = { pre_dd, NULL };
static	v_pre	 pres_dt[] = { pre_dt, NULL };
static	v_pre	 pres_it[] = { pre_it, pre_par, NULL };
static	v_pre	 pres_obsolete[] = { pre_obsolete, NULL };
static	v_pre	 pres_os[] = { pre_os, NULL };
static	v_pre	 pres_pp[] = { pre_par, NULL };
static	v_pre	 pres_sh[] = { pre_sh, NULL };
static	v_pre	 pres_ss[] = { pre_ss, NULL };
static	v_pre	 pres_std[] = { pre_std, NULL };

static	const struct valids mdoc_valids[MDOC_MAX] = {
	{ NULL, NULL },				/* Ap */
	{ pres_dd, posts_dd },			/* Dd */
	{ pres_dt, posts_dt },			/* Dt */
	{ pres_os, posts_os },			/* Os */
	{ pres_sh, posts_sh },			/* Sh */
	{ pres_ss, posts_ss },			/* Ss */
	{ pres_pp, posts_pp },			/* Pp */
	{ pres_d1, posts_d1 },			/* D1 */
	{ pres_dl, posts_dl },			/* Dl */
	{ pres_bd, posts_bd },			/* Bd */
	{ NULL, NULL },				/* Ed */
	{ pres_bl, posts_bl },			/* Bl */
	{ NULL, NULL },				/* El */
	{ pres_it, posts_it },			/* It */
	{ NULL, NULL },				/* Ad */
	{ pres_an, posts_an },			/* An */
	{ NULL, posts_defaults },		/* Ar */
	{ NULL, NULL },				/* Cd */
	{ NULL, NULL },				/* Cm */
	{ NULL, NULL },				/* Dv */
	{ NULL, NULL },				/* Er */
	{ NULL, NULL },				/* Ev */
	{ pres_std, posts_std },		/* Ex */
	{ NULL, NULL },				/* Fa */
	{ NULL, posts_text },			/* Fd */
	{ NULL, NULL },				/* Fl */
	{ NULL, NULL },				/* Fn */
	{ NULL, NULL },				/* Ft */
	{ NULL, NULL },				/* Ic */
	{ NULL, posts_text1 },			/* In */
	{ NULL, posts_defaults },		/* Li */
	{ NULL, posts_nd },			/* Nd */
	{ NULL, posts_nm },			/* Nm */
	{ NULL, NULL },				/* Op */
	{ pres_obsolete, NULL },		/* Ot */
	{ NULL, posts_defaults },		/* Pa */
	{ pres_std, posts_std },		/* Rv */
	{ NULL, posts_st },			/* St */
	{ NULL, NULL },				/* Va */
	{ NULL, posts_vt },			/* Vt */
	{ NULL, posts_text },			/* Xr */
	{ NULL, posts_text },			/* %A */
	{ NULL, posts_hyphtext },		/* %B */ /* FIXME: can be used outside Rs/Re. */
	{ NULL, posts_text },			/* %D */
	{ NULL, posts_text },			/* %I */
	{ NULL, posts_text },			/* %J */
	{ NULL, posts_hyphtext },		/* %N */
	{ NULL, posts_hyphtext },		/* %O */
	{ NULL, posts_text },			/* %P */
	{ NULL, posts_hyphtext },		/* %R */
	{ NULL, posts_hyphtext },		/* %T */ /* FIXME: can be used outside Rs/Re. */
	{ NULL, posts_text },			/* %V */
	{ NULL, NULL },				/* Ac */
	{ NULL, NULL },				/* Ao */
	{ NULL, NULL },				/* Aq */
	{ NULL, posts_at },			/* At */
	{ NULL, NULL },				/* Bc */
	{ NULL, posts_bf },			/* Bf */
	{ NULL, NULL },				/* Bo */
	{ NULL, NULL },				/* Bq */
	{ NULL, NULL },				/* Bsx */
	{ NULL, posts_bx },			/* Bx */
	{ NULL, posts_bool },			/* Db */
	{ NULL, NULL },				/* Dc */
	{ NULL, NULL },				/* Do */
	{ NULL, NULL },				/* Dq */
	{ NULL, NULL },				/* Ec */
	{ NULL, NULL },				/* Ef */
	{ NULL, NULL },				/* Em */
	{ NULL, NULL },				/* Eo */
	{ NULL, NULL },				/* Fx */
	{ NULL, NULL },				/* Ms */
	{ NULL, posts_notext },			/* No */
	{ NULL, posts_ns },			/* Ns */
	{ NULL, NULL },				/* Nx */
	{ NULL, NULL },				/* Ox */
	{ NULL, NULL },				/* Pc */
	{ NULL, posts_text1 },			/* Pf */
	{ NULL, NULL },				/* Po */
	{ NULL, NULL },				/* Pq */
	{ NULL, NULL },				/* Qc */
	{ NULL, NULL },				/* Ql */
	{ NULL, NULL },				/* Qo */
	{ NULL, NULL },				/* Qq */
	{ NULL, NULL },				/* Re */
	{ NULL, posts_rs },			/* Rs */
	{ NULL, NULL },				/* Sc */
	{ NULL, NULL },				/* So */
	{ NULL, NULL },				/* Sq */
	{ NULL, posts_bool },			/* Sm */
	{ NULL, posts_hyph },			/* Sx */
	{ NULL, NULL },				/* Sy */
	{ NULL, NULL },				/* Tn */
	{ NULL, NULL },				/* Ux */
	{ NULL, NULL },				/* Xc */
	{ NULL, NULL },				/* Xo */
	{ NULL, posts_fo },			/* Fo */
	{ NULL, NULL },				/* Fc */
	{ NULL, NULL },				/* Oo */
	{ NULL, NULL },				/* Oc */
	{ NULL, posts_bk },			/* Bk */
	{ NULL, NULL },				/* Ek */
	{ NULL, posts_eoln },			/* Bt */
	{ NULL, NULL },				/* Hf */
	{ pres_obsolete, NULL },		/* Fr */
	{ NULL, posts_eoln },			/* Ud */
	{ NULL, posts_lb },			/* Lb */
	{ pres_pp, posts_pp },			/* Lp */
	{ NULL, NULL },				/* Lk */
	{ NULL, posts_defaults },		/* Mt */
	{ NULL, NULL },				/* Brq */
	{ NULL, NULL },				/* Bro */
	{ NULL, NULL },				/* Brc */
	{ NULL, posts_text },			/* %C */
	{ pres_obsolete, posts_es },		/* Es */
	{ pres_obsolete, posts_en },		/* En */
	{ NULL, NULL },				/* Dx */
	{ NULL, posts_text },			/* %Q */
	{ NULL, posts_pp },			/* br */
	{ NULL, posts_sp },			/* sp */
	{ NULL, posts_text1 },			/* %U */
	{ NULL, NULL },				/* Ta */
	{ NULL, NULL },				/* ll */
};

#define	RSORD_MAX 14 /* Number of `Rs' blocks. */

static	const enum mdoct rsord[RSORD_MAX] = {
	MDOC__A,
	MDOC__T,
	MDOC__B,
	MDOC__I,
	MDOC__J,
	MDOC__R,
	MDOC__N,
	MDOC__V,
	MDOC__U,
	MDOC__P,
	MDOC__Q,
	MDOC__C,
	MDOC__D,
	MDOC__O
};

static	const char * const secnames[SEC__MAX] = {
	NULL,
	"NAME",
	"LIBRARY",
	"SYNOPSIS",
	"DESCRIPTION",
	"CONTEXT",
	"IMPLEMENTATION NOTES",
	"RETURN VALUES",
	"ENVIRONMENT",
	"FILES",
	"EXIT STATUS",
	"EXAMPLES",
	"DIAGNOSTICS",
	"COMPATIBILITY",
	"ERRORS",
	"SEE ALSO",
	"STANDARDS",
	"HISTORY",
	"AUTHORS",
	"CAVEATS",
	"BUGS",
	"SECURITY CONSIDERATIONS",
	NULL
};


int
mdoc_valid_pre(struct mdoc *mdoc, struct mdoc_node *n)
{
	v_pre		*p;
	int		 line, pos;
	char		*tp;

	switch (n->type) {
	case MDOC_TEXT:
		tp = n->string;
		line = n->line;
		pos = n->pos;
		check_text(mdoc, line, pos, tp);
		/* FALLTHROUGH */
	case MDOC_TBL:
		/* FALLTHROUGH */
	case MDOC_EQN:
		/* FALLTHROUGH */
	case MDOC_ROOT:
		return(1);
	default:
		break;
	}

	check_args(mdoc, n);

	if (NULL == mdoc_valids[n->tok].pre)
		return(1);
	for (p = mdoc_valids[n->tok].pre; *p; p++)
		if ( ! (*p)(mdoc, n))
			return(0);
	return(1);
}

int
mdoc_valid_post(struct mdoc *mdoc)
{
	v_post		*p;

	if (MDOC_VALID & mdoc->last->flags)
		return(1);
	mdoc->last->flags |= MDOC_VALID;

	switch (mdoc->last->type) {
	case MDOC_TEXT:
		/* FALLTHROUGH */
	case MDOC_EQN:
		/* FALLTHROUGH */
	case MDOC_TBL:
		return(1);
	case MDOC_ROOT:
		return(post_root(mdoc));
	default:
		break;
	}

	if (NULL == mdoc_valids[mdoc->last->tok].post)
		return(1);
	for (p = mdoc_valids[mdoc->last->tok].post; *p; p++)
		if ( ! (*p)(mdoc))
			return(0);

	return(1);
}

static int
check_count(struct mdoc *mdoc, enum mdoc_type type,
		enum check_lvl lvl, enum check_ineq ineq, int val)
{
	const char	*p;
	enum mandocerr	 t;

	if (mdoc->last->type != type)
		return(1);

	switch (ineq) {
	case CHECK_LT:
		p = "less than ";
		if (mdoc->last->nchild < val)
			return(1);
		break;
	case CHECK_GT:
		p = "more than ";
		if (mdoc->last->nchild > val)
			return(1);
		break;
	case CHECK_EQ:
		p = "";
		if (val == mdoc->last->nchild)
			return(1);
		break;
	default:
		abort();
		/* NOTREACHED */
	}

	t = lvl == CHECK_WARN ? MANDOCERR_ARGCWARN : MANDOCERR_ARGCOUNT;
	mandoc_vmsg(t, mdoc->parse, mdoc->last->line,
	    mdoc->last->pos, "want %s%d children (have %d)",
	    p, val, mdoc->last->nchild);
	return(1);
}

static int
berr_ge1(POST_ARGS)
{

	return(check_count(mdoc, MDOC_BODY, CHECK_ERROR, CHECK_GT, 0));
}

static int
bwarn_ge1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_BODY, CHECK_WARN, CHECK_GT, 0));
}

static int
ewarn_eq0(POST_ARGS)
{
	return(check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_EQ, 0));
}

static int
ewarn_eq1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_EQ, 1));
}

static int
ewarn_ge1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_GT, 0));
}

static int
ewarn_le1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_LT, 2));
}

static int
hwarn_eq0(POST_ARGS)
{
	return(check_count(mdoc, MDOC_HEAD, CHECK_WARN, CHECK_EQ, 0));
}

static int
hwarn_eq1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_HEAD, CHECK_WARN, CHECK_EQ, 1));
}

static int
hwarn_ge1(POST_ARGS)
{
	return(check_count(mdoc, MDOC_HEAD, CHECK_WARN, CHECK_GT, 0));
}

static void
check_args(struct mdoc *mdoc, struct mdoc_node *n)
{
	int		 i;

	if (NULL == n->args)
		return;

	assert(n->args->argc);
	for (i = 0; i < (int)n->args->argc; i++)
		check_argv(mdoc, n, &n->args->argv[i]);
}

static void
check_argv(struct mdoc *mdoc, struct mdoc_node *n, struct mdoc_argv *v)
{
	int		 i;

	for (i = 0; i < (int)v->sz; i++)
		check_text(mdoc, v->line, v->pos, v->value[i]);

	/* FIXME: move to post_std(). */

	if (MDOC_Std == v->arg)
		if ( ! (v->sz || mdoc->meta.name))
			mdoc_nmsg(mdoc, n, MANDOCERR_NONAME);
}

static void
check_text(struct mdoc *mdoc, int ln, int pos, char *p)
{
	char		*cp;

	if (MDOC_LITERAL & mdoc->flags)
		return;

	for (cp = p; NULL != (p = strchr(p, '\t')); p++)
		mandoc_msg(MANDOCERR_FI_TAB, mdoc->parse,
		    ln, pos + (int)(p - cp), NULL);
}

static int
check_parent(PRE_ARGS, enum mdoct tok, enum mdoc_type t)
{

	assert(n->parent);
	if ((MDOC_ROOT == t || tok == n->parent->tok) &&
			(t == n->parent->type))
		return(1);

	mandoc_vmsg(MANDOCERR_SYNTCHILD, mdoc->parse,
	    n->line, n->pos, "want parent %s",
	    MDOC_ROOT == t ? "<root>" : mdoc_macronames[tok]);
	return(0);
}


static int
pre_display(PRE_ARGS)
{
	struct mdoc_node *node;

	if (MDOC_BLOCK != n->type)
		return(1);

	for (node = mdoc->last->parent; node; node = node->parent)
		if (MDOC_BLOCK == node->type)
			if (MDOC_Bd == node->tok)
				break;

	if (node)
		mandoc_vmsg(MANDOCERR_BD_NEST,
		    mdoc->parse, n->line, n->pos,
		    "%s in Bd", mdoc_macronames[n->tok]);

	return(1);
}

static int
pre_bl(PRE_ARGS)
{
	struct mdoc_node *np;
	struct mdoc_argv *argv;
	int		  i;
	enum mdoc_list	  lt;

	if (MDOC_BLOCK != n->type) {
		if (ENDBODY_NOT != n->end) {
			assert(n->pending);
			np = n->pending->parent;
		} else
			np = n->parent;

		assert(np);
		assert(MDOC_BLOCK == np->type);
		assert(MDOC_Bl == np->tok);
		return(1);
	}

	/*
	 * First figure out which kind of list to use: bind ourselves to
	 * the first mentioned list type and warn about any remaining
	 * ones.  If we find no list type, we default to LIST_item.
	 */

	for (i = 0; n->args && i < (int)n->args->argc; i++) {
		argv = n->args->argv + i;
		lt = LIST__NONE;
		switch (argv->arg) {
		/* Set list types. */
		case MDOC_Bullet:
			lt = LIST_bullet;
			break;
		case MDOC_Dash:
			lt = LIST_dash;
			break;
		case MDOC_Enum:
			lt = LIST_enum;
			break;
		case MDOC_Hyphen:
			lt = LIST_hyphen;
			break;
		case MDOC_Item:
			lt = LIST_item;
			break;
		case MDOC_Tag:
			lt = LIST_tag;
			break;
		case MDOC_Diag:
			lt = LIST_diag;
			break;
		case MDOC_Hang:
			lt = LIST_hang;
			break;
		case MDOC_Ohang:
			lt = LIST_ohang;
			break;
		case MDOC_Inset:
			lt = LIST_inset;
			break;
		case MDOC_Column:
			lt = LIST_column;
			break;
		/* Set list arguments. */
		case MDOC_Compact:
			if (n->norm->Bl.comp)
				mandoc_msg(MANDOCERR_ARG_REP,
				    mdoc->parse, argv->line,
				    argv->pos, "Bl -compact");
			n->norm->Bl.comp = 1;
			break;
		case MDOC_Width:
			if (0 == argv->sz) {
				mandoc_msg(MANDOCERR_ARG_EMPTY,
				    mdoc->parse, argv->line,
				    argv->pos, "Bl -width");
				n->norm->Bl.width = "0n";
				break;
			}
			if (NULL != n->norm->Bl.width)
				mandoc_vmsg(MANDOCERR_ARG_REP,
				    mdoc->parse, argv->line,
				    argv->pos, "Bl -width %s",
				    argv->value[0]);
			n->norm->Bl.width = argv->value[0];
			break;
		case MDOC_Offset:
			if (0 == argv->sz) {
				mandoc_msg(MANDOCERR_ARG_EMPTY,
				    mdoc->parse, argv->line,
				    argv->pos, "Bl -offset");
				break;
			}
			if (NULL != n->norm->Bl.offs)
				mandoc_vmsg(MANDOCERR_ARG_REP,
				    mdoc->parse, argv->line,
				    argv->pos, "Bl -offset %s",
				    argv->value[0]);
			n->norm->Bl.offs = argv->value[0];
			break;
		default:
			continue;
		}
		if (LIST__NONE == lt)
			continue;

		/* Check: multiple list types. */

		if (LIST__NONE != n->norm->Bl.type) {
			mandoc_msg(MANDOCERR_BL_REP,
			    mdoc->parse, n->line, n->pos,
			    mdoc_argnames[argv->arg]);
			continue;
		}

		/* The list type should come first. */

		if (n->norm->Bl.width ||
		    n->norm->Bl.offs ||
		    n->norm->Bl.comp)
			mandoc_msg(MANDOCERR_BL_LATETYPE,
			    mdoc->parse, n->line, n->pos,
			    mdoc_argnames[n->args->argv[0].arg]);

		n->norm->Bl.type = lt;
		if (LIST_column == lt) {
			n->norm->Bl.ncols = argv->sz;
			n->norm->Bl.cols = (void *)argv->value;
		}
	}

	/* Allow lists to default to LIST_item. */

	if (LIST__NONE == n->norm->Bl.type) {
		mdoc_nmsg(mdoc, n, MANDOCERR_BL_NOTYPE);
		n->norm->Bl.type = LIST_item;
	}

	/*
	 * Validate the width field.  Some list types don't need width
	 * types and should be warned about them.  Others should have it
	 * and must also be warned.  Yet others have a default and need
	 * no warning.
	 */

	switch (n->norm->Bl.type) {
	case LIST_tag:
		if (NULL == n->norm->Bl.width)
			mdoc_nmsg(mdoc, n, MANDOCERR_BL_NOWIDTH);
		break;
	case LIST_column:
		/* FALLTHROUGH */
	case LIST_diag:
		/* FALLTHROUGH */
	case LIST_ohang:
		/* FALLTHROUGH */
	case LIST_inset:
		/* FALLTHROUGH */
	case LIST_item:
		if (n->norm->Bl.width)
			mdoc_nmsg(mdoc, n, MANDOCERR_IGNARGV);
		break;
	case LIST_bullet:
		/* FALLTHROUGH */
	case LIST_dash:
		/* FALLTHROUGH */
	case LIST_hyphen:
		if (NULL == n->norm->Bl.width)
			n->norm->Bl.width = "2n";
		break;
	case LIST_enum:
		if (NULL == n->norm->Bl.width)
			n->norm->Bl.width = "3n";
		break;
	default:
		break;
	}

	return(1);
}

static int
pre_bd(PRE_ARGS)
{
	struct mdoc_node *np;
	struct mdoc_argv *argv;
	int		  i;
	enum mdoc_disp	  dt;

	if (MDOC_BLOCK != n->type) {
		if (ENDBODY_NOT != n->end) {
			assert(n->pending);
			np = n->pending->parent;
		} else
			np = n->parent;

		assert(np);
		assert(MDOC_BLOCK == np->type);
		assert(MDOC_Bd == np->tok);
		return(1);
	}

	for (i = 0; n->args && i < (int)n->args->argc; i++) {
		argv = n->args->argv + i;
		dt = DISP__NONE;

		switch (argv->arg) {
		case MDOC_Centred:
			dt = DISP_centred;
			break;
		case MDOC_Ragged:
			dt = DISP_ragged;
			break;
		case MDOC_Unfilled:
			dt = DISP_unfilled;
			break;
		case MDOC_Filled:
			dt = DISP_filled;
			break;
		case MDOC_Literal:
			dt = DISP_literal;
			break;
		case MDOC_File:
			mdoc_nmsg(mdoc, n, MANDOCERR_BADDISP);
			return(0);
		case MDOC_Offset:
			if (0 == argv->sz) {
				mandoc_msg(MANDOCERR_ARG_EMPTY,
				    mdoc->parse, argv->line,
				    argv->pos, "Bd -offset");
				break;
			}
			if (NULL != n->norm->Bd.offs)
				mandoc_vmsg(MANDOCERR_ARG_REP,
				    mdoc->parse, argv->line,
				    argv->pos, "Bd -offset %s",
				    argv->value[0]);
			n->norm->Bd.offs = argv->value[0];
			break;
		case MDOC_Compact:
			if (n->norm->Bd.comp)
				mandoc_msg(MANDOCERR_ARG_REP,
				    mdoc->parse, argv->line,
				    argv->pos, "Bd -compact");
			n->norm->Bd.comp = 1;
			break;
		default:
			abort();
			/* NOTREACHED */
		}
		if (DISP__NONE == dt)
			continue;

		if (DISP__NONE == n->norm->Bd.type)
			n->norm->Bd.type = dt;
		else
			mandoc_msg(MANDOCERR_BD_REP,
			    mdoc->parse, n->line, n->pos,
			    mdoc_argnames[argv->arg]);
	}

	if (DISP__NONE == n->norm->Bd.type) {
		mdoc_nmsg(mdoc, n, MANDOCERR_BD_NOTYPE);
		n->norm->Bd.type = DISP_ragged;
	}

	return(1);
}

static int
pre_ss(PRE_ARGS)
{

	if (MDOC_BLOCK != n->type)
		return(1);
	return(check_parent(mdoc, n, MDOC_Sh, MDOC_BODY));
}

static int
pre_sh(PRE_ARGS)
{

	if (MDOC_BLOCK != n->type)
		return(1);
	return(check_parent(mdoc, n, MDOC_MAX, MDOC_ROOT));
}

static int
pre_it(PRE_ARGS)
{

	if (MDOC_BLOCK != n->type)
		return(1);

	return(check_parent(mdoc, n, MDOC_Bl, MDOC_BODY));
}

static int
pre_an(PRE_ARGS)
{
	int		 i;

	if (NULL == n->args)
		return(1);

	for (i = 1; i < (int)n->args->argc; i++)
		mdoc_pmsg(mdoc, n->args->argv[i].line,
		    n->args->argv[i].pos, MANDOCERR_IGNARGV);

	if (MDOC_Split == n->args->argv[0].arg)
		n->norm->An.auth = AUTH_split;
	else if (MDOC_Nosplit == n->args->argv[0].arg)
		n->norm->An.auth = AUTH_nosplit;
	else
		abort();

	return(1);
}

static int
pre_std(PRE_ARGS)
{

	if (n->args && 1 == n->args->argc)
		if (MDOC_Std == n->args->argv[0].arg)
			return(1);

	mandoc_msg(MANDOCERR_ARG_STD, mdoc->parse,
	    n->line, n->pos, mdoc_macronames[n->tok]);
	return(1);
}

static int
pre_obsolete(PRE_ARGS)
{

	if (MDOC_ELEM == n->type || MDOC_BLOCK == n->type)
		mandoc_msg(MANDOCERR_MACRO_OBS, mdoc->parse,
		    n->line, n->pos, mdoc_macronames[n->tok]);
	return(1);
}

static int
pre_dt(PRE_ARGS)
{

	if (NULL == mdoc->meta.date || mdoc->meta.os)
		mandoc_msg(MANDOCERR_PROLOG_ORDER, mdoc->parse,
		    n->line, n->pos, "Dt");

	if (mdoc->meta.title)
		mandoc_msg(MANDOCERR_PROLOG_REP, mdoc->parse,
		    n->line, n->pos, "Dt");

	return(1);
}

static int
pre_os(PRE_ARGS)
{

	if (NULL == mdoc->meta.title || NULL == mdoc->meta.date)
		mandoc_msg(MANDOCERR_PROLOG_ORDER, mdoc->parse,
		    n->line, n->pos, "Os");

	if (mdoc->meta.os)
		mandoc_msg(MANDOCERR_PROLOG_REP, mdoc->parse,
		    n->line, n->pos, "Os");

	return(1);
}

static int
pre_dd(PRE_ARGS)
{

	if (mdoc->meta.title || mdoc->meta.os)
		mandoc_msg(MANDOCERR_PROLOG_ORDER, mdoc->parse,
		    n->line, n->pos, "Dd");

	if (mdoc->meta.date)
		mandoc_msg(MANDOCERR_PROLOG_REP, mdoc->parse,
		    n->line, n->pos, "Dd");

	return(1);
}

static int
post_bf(POST_ARGS)
{
	struct mdoc_node *np, *nch;
	enum mdocargt	  arg;

	/*
	 * Unlike other data pointers, these are "housed" by the HEAD
	 * element, which contains the goods.
	 */

	if (MDOC_HEAD != mdoc->last->type) {
		if (ENDBODY_NOT != mdoc->last->end) {
			assert(mdoc->last->pending);
			np = mdoc->last->pending->parent->head;
		} else if (MDOC_BLOCK != mdoc->last->type) {
			np = mdoc->last->parent->head;
		} else
			np = mdoc->last->head;

		assert(np);
		assert(MDOC_HEAD == np->type);
		assert(MDOC_Bf == np->tok);
		return(1);
	}

	np = mdoc->last;
	assert(MDOC_BLOCK == np->parent->type);
	assert(MDOC_Bf == np->parent->tok);

	/* Check the number of arguments. */

	nch = np->child;
	if (NULL == np->parent->args) {
		if (NULL == nch) {
			mdoc_nmsg(mdoc, np, MANDOCERR_BF_NOFONT);
			return(1);
		}
		nch = nch->next;
	}
	if (NULL != nch)
		mandoc_vmsg(MANDOCERR_ARG_EXCESS, mdoc->parse,
		    nch->line, nch->pos, "Bf ... %s", nch->string);

	/* Extract argument into data. */

	if (np->parent->args) {
		arg = np->parent->args->argv[0].arg;
		if (MDOC_Emphasis == arg)
			np->norm->Bf.font = FONT_Em;
		else if (MDOC_Literal == arg)
			np->norm->Bf.font = FONT_Li;
		else if (MDOC_Symbolic == arg)
			np->norm->Bf.font = FONT_Sy;
		else
			abort();
		return(1);
	}

	/* Extract parameter into data. */

	if (0 == strcmp(np->child->string, "Em"))
		np->norm->Bf.font = FONT_Em;
	else if (0 == strcmp(np->child->string, "Li"))
		np->norm->Bf.font = FONT_Li;
	else if (0 == strcmp(np->child->string, "Sy"))
		np->norm->Bf.font = FONT_Sy;
	else
		mandoc_vmsg(MANDOCERR_BF_BADFONT, mdoc->parse,
		    np->child->line, np->child->pos,
		    "Bf %s", np->child->string);

	return(1);
}

static int
post_lb(POST_ARGS)
{
	struct mdoc_node	*n;
	const char		*stdlibname;
	char			*libname;

	check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_EQ, 1);

	n = mdoc->last->child;

	assert(n);
	assert(MDOC_TEXT == n->type);

	if (NULL == (stdlibname = mdoc_a2lib(n->string)))
		mandoc_asprintf(&libname,
		    "library \\(lq%s\\(rq", n->string);
	else
		libname = mandoc_strdup(stdlibname);

	free(n->string);
	n->string = libname;
	return(1);
}

static int
post_eoln(POST_ARGS)
{
	const struct mdoc_node *n;

	n = mdoc->last;
	if (n->child)
		mandoc_vmsg(MANDOCERR_ARG_SKIP,
		    mdoc->parse, n->line, n->pos,
		    "%s %s", mdoc_macronames[n->tok],
		    n->child->string);
	return(1);
}

static int
post_vt(POST_ARGS)
{
	const struct mdoc_node *n;

	/*
	 * The Vt macro comes in both ELEM and BLOCK form, both of which
	 * have different syntaxes (yet more context-sensitive
	 * behaviour).  ELEM types must have a child, which is already
	 * guaranteed by the in_line parsing routine; BLOCK types,
	 * specifically the BODY, should only have TEXT children.
	 */

	if (MDOC_BODY != mdoc->last->type)
		return(1);

	for (n = mdoc->last->child; n; n = n->next)
		if (MDOC_TEXT != n->type)
			mandoc_msg(MANDOCERR_VT_CHILD, mdoc->parse,
			    n->line, n->pos, mdoc_macronames[n->tok]);

	return(1);
}

static int
post_nm(POST_ARGS)
{

	if (NULL != mdoc->meta.name)
		return(1);

	mdoc_deroff(&mdoc->meta.name, mdoc->last);

	if (NULL == mdoc->meta.name) {
		mdoc_nmsg(mdoc, mdoc->last, MANDOCERR_NONAME);
		mdoc->meta.name = mandoc_strdup("UNKNOWN");
	}
	return(1);
}

static int
post_literal(POST_ARGS)
{

	/*
	 * The `Dl' (note "el" not "one") and `Bd' macros unset the
	 * MDOC_LITERAL flag as they leave.  Note that `Bd' only sets
	 * this in literal mode, but it doesn't hurt to just switch it
	 * off in general since displays can't be nested.
	 */

	if (MDOC_BODY == mdoc->last->type)
		mdoc->flags &= ~MDOC_LITERAL;

	return(1);
}

static int
post_defaults(POST_ARGS)
{
	struct mdoc_node *nn;

	/*
	 * The `Ar' defaults to "file ..." if no value is provided as an
	 * argument; the `Mt' and `Pa' macros use "~"; the `Li' just
	 * gets an empty string.
	 */

	if (mdoc->last->child)
		return(1);

	nn = mdoc->last;
	mdoc->next = MDOC_NEXT_CHILD;

	switch (nn->tok) {
	case MDOC_Ar:
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, "file"))
			return(0);
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, "..."))
			return(0);
		break;
	case MDOC_At:
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, "AT&T"))
			return(0);
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, "UNIX"))
			return(0);
		break;
	case MDOC_Li:
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, ""))
			return(0);
		break;
	case MDOC_Pa:
		/* FALLTHROUGH */
	case MDOC_Mt:
		if ( ! mdoc_word_alloc(mdoc, nn->line, nn->pos, "~"))
			return(0);
		break;
	default:
		abort();
		/* NOTREACHED */
	}

	mdoc->last = nn;
	return(1);
}

static int
post_at(POST_ARGS)
{
	struct mdoc_node	*n;
	const char		*std_att;
	char			*att;

	/*
	 * If we have a child, look it up in the standard keys.  If a
	 * key exist, use that instead of the child; if it doesn't,
	 * prefix "AT&T UNIX " to the existing data.
	 */

	if (NULL == (n = mdoc->last->child))
		return(1);

	assert(MDOC_TEXT == n->type);
	if (NULL == (std_att = mdoc_a2att(n->string))) {
		mandoc_msg(MANDOCERR_AT_BAD, mdoc->parse,
		    n->line, n->pos, n->string);
		mandoc_asprintf(&att, "AT&T UNIX %s", n->string);
	} else
		att = mandoc_strdup(std_att);

	free(n->string);
	n->string = att;
	return(1);
}

static int
post_an(POST_ARGS)
{
	struct mdoc_node *np;

	np = mdoc->last;
	if (AUTH__NONE == np->norm->An.auth) {
		if (0 == np->child)
			check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_GT, 0);
	} else if (np->child)
		check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_EQ, 0);

	return(1);
}

static int
post_en(POST_ARGS)
{

	if (MDOC_BLOCK == mdoc->last->type)
		mdoc->last->norm->Es = mdoc->last_es;
	return(1);
}

static int
post_es(POST_ARGS)
{

	mdoc->last_es = mdoc->last;
	return(1);
}

static int
post_it(POST_ARGS)
{
	int		  i, cols;
	enum mdoc_list	  lt;
	struct mdoc_node *nbl, *nit, *nch;
	enum mandocerr	  er;

	nit = mdoc->last;
	if (MDOC_BLOCK != nit->type)
		return(1);

	nbl = nit->parent->parent;
	lt = nbl->norm->Bl.type;

	switch (lt) {
	case LIST_tag:
		/* FALLTHROUGH */
	case LIST_hang:
		/* FALLTHROUGH */
	case LIST_ohang:
		/* FALLTHROUGH */
	case LIST_inset:
		/* FALLTHROUGH */
	case LIST_diag:
		if (NULL == nit->head->child)
			mandoc_msg(MANDOCERR_IT_NOHEAD,
			    mdoc->parse, nit->line, nit->pos,
			    mdoc_argnames[nbl->args->argv[0].arg]);
		break;
	case LIST_bullet:
		/* FALLTHROUGH */
	case LIST_dash:
		/* FALLTHROUGH */
	case LIST_enum:
		/* FALLTHROUGH */
	case LIST_hyphen:
		if (NULL == nit->body->child)
			mandoc_msg(MANDOCERR_IT_NOBODY,
			    mdoc->parse, nit->line, nit->pos,
			    mdoc_argnames[nbl->args->argv[0].arg]);
		/* FALLTHROUGH */
	case LIST_item:
		if (NULL != nit->head->child)
			mandoc_vmsg(MANDOCERR_ARG_SKIP,
			    mdoc->parse, nit->line, nit->pos,
			    "It %s", nit->head->child->string);
		break;
	case LIST_column:
		cols = (int)nbl->norm->Bl.ncols;

		assert(NULL == nit->head->child);

		for (i = 0, nch = nit->child; nch; nch = nch->next)
			if (MDOC_BODY == nch->type)
				i++;

		if (i < cols)
			er = MANDOCERR_ARGCOUNT;
		else if (i == cols || i == cols + 1)
			break;
		else
			er = MANDOCERR_SYNTARGCOUNT;

		mandoc_vmsg(er, mdoc->parse, nit->line, nit->pos,
		    "columns == %d (have %d)", cols, i);
		return(MANDOCERR_ARGCOUNT == er);
	default:
		abort();
	}

	return(1);
}

static int
post_bl_block(POST_ARGS)
{
	struct mdoc_node *n, *ni, *nc;

	/*
	 * These are fairly complicated, so we've broken them into two
	 * functions.  post_bl_block_tag() is called when a -tag is
	 * specified, but no -width (it must be guessed).  The second
	 * when a -width is specified (macro indicators must be
	 * rewritten into real lengths).
	 */

	n = mdoc->last;

	if (LIST_tag == n->norm->Bl.type &&
	    NULL == n->norm->Bl.width) {
		if ( ! post_bl_block_tag(mdoc))
			return(0);
		assert(n->norm->Bl.width);
	} else if (NULL != n->norm->Bl.width) {
		if ( ! post_bl_block_width(mdoc))
			return(0);
		assert(n->norm->Bl.width);
	}

	for (ni = n->body->child; ni; ni = ni->next) {
		if (NULL == ni->body)
			continue;
		nc = ni->body->last;
		while (NULL != nc) {
			switch (nc->tok) {
			case MDOC_Pp:
				/* FALLTHROUGH */
			case MDOC_Lp:
				/* FALLTHROUGH */
			case MDOC_br:
				break;
			default:
				nc = NULL;
				continue;
			}
			if (NULL == ni->next) {
				mandoc_msg(MANDOCERR_PAR_MOVE,
				    mdoc->parse, nc->line, nc->pos,
				    mdoc_macronames[nc->tok]);
				if ( ! mdoc_node_relink(mdoc, nc))
					return(0);
			} else if (0 == n->norm->Bl.comp &&
			    LIST_column != n->norm->Bl.type) {
				mandoc_vmsg(MANDOCERR_PAR_SKIP,
				    mdoc->parse, nc->line, nc->pos,
				    "%s before It",
				    mdoc_macronames[nc->tok]);
				mdoc_node_delete(mdoc, nc);
			} else
				break;
			nc = ni->body->last;
		}
	}
	return(1);
}

static int
post_bl_block_width(POST_ARGS)
{
	size_t		  width;
	int		  i;
	enum mdoct	  tok;
	struct mdoc_node *n;
	char		  buf[24];

	n = mdoc->last;

	/*
	 * Calculate the real width of a list from the -width string,
	 * which may contain a macro (with a known default width), a
	 * literal string, or a scaling width.
	 *
	 * If the value to -width is a macro, then we re-write it to be
	 * the macro's width as set in share/tmac/mdoc/doc-common.
	 */

	if (0 == strcmp(n->norm->Bl.width, "Ds"))
		width = 6;
	else if (MDOC_MAX == (tok = mdoc_hash_find(n->norm->Bl.width)))
		return(1);
	else
		width = macro2len(tok);

	/* The value already exists: free and reallocate it. */

	assert(n->args);

	for (i = 0; i < (int)n->args->argc; i++)
		if (MDOC_Width == n->args->argv[i].arg)
			break;

	assert(i < (int)n->args->argc);

	(void)snprintf(buf, sizeof(buf), "%un", (unsigned int)width);
	free(n->args->argv[i].value[0]);
	n->args->argv[i].value[0] = mandoc_strdup(buf);

	/* Set our width! */
	n->norm->Bl.width = n->args->argv[i].value[0];
	return(1);
}

static int
post_bl_block_tag(POST_ARGS)
{
	struct mdoc_node *n, *nn;
	size_t		  sz, ssz;
	int		  i;
	char		  buf[24];

	/*
	 * Calculate the -width for a `Bl -tag' list if it hasn't been
	 * provided.  Uses the first head macro.  NOTE AGAIN: this is
	 * ONLY if the -width argument has NOT been provided.  See
	 * post_bl_block_width() for converting the -width string.
	 */

	sz = 10;
	n = mdoc->last;

	for (nn = n->body->child; nn; nn = nn->next) {
		if (MDOC_It != nn->tok)
			continue;

		assert(MDOC_BLOCK == nn->type);
		nn = nn->head->child;

		if (nn == NULL)
			break;

		if (MDOC_TEXT == nn->type) {
			sz = strlen(nn->string) + 1;
			break;
		}

		if (0 != (ssz = macro2len(nn->tok)))
			sz = ssz;

		break;
	}

	/* Defaults to ten ens. */

	(void)snprintf(buf, sizeof(buf), "%un", (unsigned int)sz);

	/*
	 * We have to dynamically add this to the macro's argument list.
	 * We're guaranteed that a MDOC_Width doesn't already exist.
	 */

	assert(n->args);
	i = (int)(n->args->argc)++;

	n->args->argv = mandoc_reallocarray(n->args->argv,
	    n->args->argc, sizeof(struct mdoc_argv));

	n->args->argv[i].arg = MDOC_Width;
	n->args->argv[i].line = n->line;
	n->args->argv[i].pos = n->pos;
	n->args->argv[i].sz = 1;
	n->args->argv[i].value = mandoc_malloc(sizeof(char *));
	n->args->argv[i].value[0] = mandoc_strdup(buf);

	/* Set our width! */
	n->norm->Bl.width = n->args->argv[i].value[0];
	return(1);
}

static int
post_bl_head(POST_ARGS)
{
	struct mdoc_node *np, *nn, *nnp;
	int		  i, j;

	if (LIST_column != mdoc->last->norm->Bl.type)
		/* FIXME: this should be ERROR class... */
		return(hwarn_eq0(mdoc));

	/*
	 * Convert old-style lists, where the column width specifiers
	 * trail as macro parameters, to the new-style ("normal-form")
	 * lists where they're argument values following -column.
	 */

	/* First, disallow both types and allow normal-form. */

	/*
	 * TODO: technically, we can accept both and just merge the two
	 * lists, but I'll leave that for another day.
	 */

	if (mdoc->last->norm->Bl.ncols && mdoc->last->nchild) {
		mdoc_nmsg(mdoc, mdoc->last, MANDOCERR_COLUMNS);
		return(0);
	} else if (NULL == mdoc->last->child)
		return(1);

	np = mdoc->last->parent;
	assert(np->args);

	for (j = 0; j < (int)np->args->argc; j++)
		if (MDOC_Column == np->args->argv[j].arg)
			break;

	assert(j < (int)np->args->argc);
	assert(0 == np->args->argv[j].sz);

	/*
	 * Accommodate for new-style groff column syntax.  Shuffle the
	 * child nodes, all of which must be TEXT, as arguments for the
	 * column field.  Then, delete the head children.
	 */

	np->args->argv[j].sz = (size_t)mdoc->last->nchild;
	np->args->argv[j].value = mandoc_reallocarray(NULL,
	    (size_t)mdoc->last->nchild, sizeof(char *));

	mdoc->last->norm->Bl.ncols = np->args->argv[j].sz;
	mdoc->last->norm->Bl.cols = (void *)np->args->argv[j].value;

	for (i = 0, nn = mdoc->last->child; nn; i++) {
		np->args->argv[j].value[i] = nn->string;
		nn->string = NULL;
		nnp = nn;
		nn = nn->next;
		mdoc_node_delete(NULL, nnp);
	}

	mdoc->last->nchild = 0;
	mdoc->last->child = NULL;

	return(1);
}

static int
post_bl(POST_ARGS)
{
	struct mdoc_node	*nparent, *nprev; /* of the Bl block */
	struct mdoc_node	*nblock, *nbody;  /* of the Bl */
	struct mdoc_node	*nchild, *nnext;  /* of the Bl body */

	nbody = mdoc->last;
	switch (nbody->type) {
	case MDOC_BLOCK:
		return(post_bl_block(mdoc));
	case MDOC_HEAD:
		return(post_bl_head(mdoc));
	case MDOC_BODY:
		break;
	default:
		return(1);
	}

	nchild = nbody->child;
	while (NULL != nchild) {
		if (MDOC_It == nchild->tok || MDOC_Sm == nchild->tok) {
			nchild = nchild->next;
			continue;
		}

		mandoc_msg(MANDOCERR_BL_MOVE, mdoc->parse,
		    nchild->line, nchild->pos,
		    mdoc_macronames[nchild->tok]);

		/*
		 * Move the node out of the Bl block.
		 * First, collect all required node pointers.
		 */

		nblock  = nbody->parent;
		nprev   = nblock->prev;
		nparent = nblock->parent;
		nnext   = nchild->next;

		/*
		 * Unlink this child.
		 */

		assert(NULL == nchild->prev);
		if (0 == --nbody->nchild) {
			nbody->child = NULL;
			nbody->last  = NULL;
			assert(NULL == nnext);
		} else {
			nbody->child = nnext;
			nnext->prev = NULL;
		}

		/*
		 * Relink this child.
		 */

		nchild->parent = nparent;
		nchild->prev   = nprev;
		nchild->next   = nblock;

		nblock->prev = nchild;
		nparent->nchild++;
		if (NULL == nprev)
			nparent->child = nchild;
		else
			nprev->next = nchild;

		nchild = nnext;
	}

	return(1);
}

static int
ebool(struct mdoc *mdoc)
{
	struct mdoc_node	*nch;
	enum mdoct		 tok;

	tok = mdoc->last->tok;
	nch = mdoc->last->child;

	if (NULL == nch) {
		if (MDOC_Sm == tok)
			mdoc->flags ^= MDOC_SMOFF;
		return(1);
	}

	check_count(mdoc, MDOC_ELEM, CHECK_WARN, CHECK_LT, 2);

	assert(MDOC_TEXT == nch->type);

	if (0 == strcmp(nch->string, "on")) {
		if (MDOC_Sm == tok)
			mdoc->flags &= ~MDOC_SMOFF;
		return(1);
	}
	if (0 == strcmp(nch->string, "off")) {
		if (MDOC_Sm == tok)
			mdoc->flags |= MDOC_SMOFF;
		return(1);
	}

	mandoc_vmsg(MANDOCERR_SM_BAD,
	    mdoc->parse, nch->line, nch->pos,
	    "%s %s", mdoc_macronames[tok], nch->string);
	return(mdoc_node_relink(mdoc, nch));
}

static int
post_root(POST_ARGS)
{
	int		  ret;
	struct mdoc_node *n;

	ret = 1;

	/* Check that we have a finished prologue. */

	if ( ! (MDOC_PBODY & mdoc->flags)) {
		ret = 0;
		mdoc_nmsg(mdoc, mdoc->first, MANDOCERR_NODOCPROLOG);
	}

	n = mdoc->first;
	assert(n);

	/* Check that we begin with a proper `Sh'. */

	if (NULL == n->child)
		mdoc_nmsg(mdoc, n, MANDOCERR_DOC_EMPTY);
	else if (MDOC_Sh != n->child->tok)
		mandoc_msg(MANDOCERR_SEC_BEFORE, mdoc->parse,
		    n->child->line, n->child->pos,
		    mdoc_macronames[n->child->tok]);

	return(ret);
}

static int
post_st(POST_ARGS)
{
	struct mdoc_node	 *n, *nch;
	const char		 *p;

	n = mdoc->last;
	nch = n->child;

	if (NULL == nch) {
		mandoc_msg(MANDOCERR_MACRO_EMPTY, mdoc->parse,
		    n->line, n->pos, mdoc_macronames[n->tok]);
		mdoc_node_delete(mdoc, n);
		return(1);
	}

	assert(MDOC_TEXT == nch->type);

	if (NULL == (p = mdoc_a2st(nch->string))) {
		mandoc_msg(MANDOCERR_ST_BAD, mdoc->parse,
		    nch->line, nch->pos, nch->string);
		mdoc_node_delete(mdoc, n);
	} else {
		free(nch->string);
		nch->string = mandoc_strdup(p);
	}

	return(1);
}

static int
post_rs(POST_ARGS)
{
	struct mdoc_node *nn, *next, *prev;
	int		  i, j;

	switch (mdoc->last->type) {
	case MDOC_HEAD:
		check_count(mdoc, MDOC_HEAD, CHECK_WARN, CHECK_EQ, 0);
		return(1);
	case MDOC_BODY:
		if (mdoc->last->child)
			break;
		check_count(mdoc, MDOC_BODY, CHECK_WARN, CHECK_GT, 0);
		return(1);
	default:
		return(1);
	}

	/*
	 * The full `Rs' block needs special handling to order the
	 * sub-elements according to `rsord'.  Pick through each element
	 * and correctly order it.  This is an insertion sort.
	 */

	next = NULL;
	for (nn = mdoc->last->child->next; nn; nn = next) {
		/* Determine order of `nn'. */
		for (i = 0; i < RSORD_MAX; i++)
			if (rsord[i] == nn->tok)
				break;

		if (i == RSORD_MAX) {
			mandoc_msg(MANDOCERR_RS_BAD,
			    mdoc->parse, nn->line, nn->pos,
			    mdoc_macronames[nn->tok]);
			i = -1;
		} else if (MDOC__J == nn->tok || MDOC__B == nn->tok)
			mdoc->last->norm->Rs.quote_T++;

		/*
		 * Remove `nn' from the chain.  This somewhat
		 * repeats mdoc_node_unlink(), but since we're
		 * just re-ordering, there's no need for the
		 * full unlink process.
		 */

		if (NULL != (next = nn->next))
			next->prev = nn->prev;

		if (NULL != (prev = nn->prev))
			prev->next = nn->next;

		nn->prev = nn->next = NULL;

		/*
		 * Scan back until we reach a node that's
		 * ordered before `nn'.
		 */

		for ( ; prev ; prev = prev->prev) {
			/* Determine order of `prev'. */
			for (j = 0; j < RSORD_MAX; j++)
				if (rsord[j] == prev->tok)
					break;
			if (j == RSORD_MAX)
				j = -1;

			if (j <= i)
				break;
		}

		/*
		 * Set `nn' back into its correct place in front
		 * of the `prev' node.
		 */

		nn->prev = prev;

		if (prev) {
			if (prev->next)
				prev->next->prev = nn;
			nn->next = prev->next;
			prev->next = nn;
		} else {
			mdoc->last->child->prev = nn;
			nn->next = mdoc->last->child;
			mdoc->last->child = nn;
		}
	}

	return(1);
}

/*
 * For some arguments of some macros,
 * convert all breakable hyphens into ASCII_HYPH.
 */
static int
post_hyph(POST_ARGS)
{
	struct mdoc_node	*n, *nch;
	char			*cp;

	n = mdoc->last;
	switch (n->type) {
	case MDOC_HEAD:
		if (MDOC_Sh == n->tok || MDOC_Ss == n->tok)
			break;
		return(1);
	case MDOC_BODY:
		if (MDOC_D1 == n->tok || MDOC_Nd == n->tok)
			break;
		return(1);
	case MDOC_ELEM:
		break;
	default:
		return(1);
	}

	for (nch = n->child; nch; nch = nch->next) {
		if (MDOC_TEXT != nch->type)
			continue;
		cp = nch->string;
		if ('\0' == *cp)
			continue;
		while ('\0' != *(++cp))
			if ('-' == *cp &&
			    isalpha((unsigned char)cp[-1]) &&
			    isalpha((unsigned char)cp[1]))
				*cp = ASCII_HYPH;
	}
	return(1);
}

static int
post_ns(POST_ARGS)
{

	if (MDOC_LINE & mdoc->last->flags)
		mdoc_nmsg(mdoc, mdoc->last, MANDOCERR_NS_SKIP);
	return(1);
}

static int
post_sh(POST_ARGS)
{

	if (MDOC_HEAD == mdoc->last->type)
		return(post_sh_head(mdoc));
	if (MDOC_BODY == mdoc->last->type)
		return(post_sh_body(mdoc));

	return(1);
}

static int
post_sh_body(POST_ARGS)
{
	struct mdoc_node *n;

	if (SEC_NAME != mdoc->lastsec)
		return(1);

	/*
	 * Warn if the NAME section doesn't contain the `Nm' and `Nd'
	 * macros (can have multiple `Nm' and one `Nd').  Note that the
	 * children of the BODY declaration can also be "text".
	 */

	if (NULL == (n = mdoc->last->child)) {
		mandoc_msg(MANDOCERR_NAMESEC_BAD, mdoc->parse,
		    mdoc->last->line, mdoc->last->pos, "empty");
		return(1);
	}

	for ( ; n && n->next; n = n->next) {
		if (MDOC_ELEM == n->type && MDOC_Nm == n->tok)
			continue;
		if (MDOC_TEXT == n->type)
			continue;
		mandoc_msg(MANDOCERR_NAMESEC_BAD, mdoc->parse,
		    n->line, n->pos, mdoc_macronames[n->tok]);
	}

	assert(n);
	if (MDOC_BLOCK == n->type && MDOC_Nd == n->tok)
		return(1);

	mandoc_msg(MANDOCERR_NAMESEC_BAD, mdoc->parse,
	    n->line, n->pos, mdoc_macronames[n->tok]);
	return(1);
}

static int
post_sh_head(POST_ARGS)
{
	struct mdoc_node *n;
	const char	*goodsec;
	char		*secname;
	enum mdoc_sec	 sec;

	/*
	 * Process a new section.  Sections are either "named" or
	 * "custom".  Custom sections are user-defined, while named ones
	 * follow a conventional order and may only appear in certain
	 * manual sections.
	 */

	secname = NULL;
	sec = SEC_CUSTOM;
	mdoc_deroff(&secname, mdoc->last);
	sec = NULL == secname ? SEC_CUSTOM : a2sec(secname);

	/* The NAME should be first. */

	if (SEC_NAME != sec && SEC_NONE == mdoc->lastnamed)
		mandoc_msg(MANDOCERR_NAMESEC_FIRST, mdoc->parse,
		    mdoc->last->line, mdoc->last->pos, secname);

	/* The SYNOPSIS gets special attention in other areas. */

	if (SEC_SYNOPSIS == sec) {
		roff_setreg(mdoc->roff, "nS", 1, '=');
		mdoc->flags |= MDOC_SYNOPSIS;
	} else {
		roff_setreg(mdoc->roff, "nS", 0, '=');
		mdoc->flags &= ~MDOC_SYNOPSIS;
	}

	/* Mark our last section. */

	mdoc->lastsec = sec;

	/*
	 * Set the section attribute for the current HEAD, for its
	 * parent BLOCK, and for the HEAD children; the latter can
	 * only be TEXT nodes, so no recursion is needed.
	 * For other blocks and elements, including .Sh BODY, this is
	 * done when allocating the node data structures, but for .Sh
	 * BLOCK and HEAD, the section is still unknown at that time.
	 */

	mdoc->last->parent->sec = sec;
	mdoc->last->sec = sec;
	for (n = mdoc->last->child; n; n = n->next)
		n->sec = sec;

	/* We don't care about custom sections after this. */

	if (SEC_CUSTOM == sec) {
		free(secname);
		return(1);
	}

	/*
	 * Check whether our non-custom section is being repeated or is
	 * out of order.
	 */

	if (sec == mdoc->lastnamed)
		mandoc_msg(MANDOCERR_SEC_REP, mdoc->parse,
		    mdoc->last->line, mdoc->last->pos, secname);

	if (sec < mdoc->lastnamed)
		mandoc_msg(MANDOCERR_SEC_ORDER, mdoc->parse,
		    mdoc->last->line, mdoc->last->pos, secname);

	/* Mark the last named section. */

	mdoc->lastnamed = sec;

	/* Check particular section/manual conventions. */

	assert(mdoc->meta.msec);

	goodsec = NULL;
	switch (sec) {
	case SEC_ERRORS:
		if (*mdoc->meta.msec == '4')
			break;
		goodsec = "2, 3, 4, 9";
		/* FALLTHROUGH */
	case SEC_RETURN_VALUES:
		/* FALLTHROUGH */
	case SEC_LIBRARY:
		if (*mdoc->meta.msec == '2')
			break;
		if (*mdoc->meta.msec == '3')
			break;
		if (NULL == goodsec)
			goodsec = "2, 3, 9";
		/* FALLTHROUGH */
	case SEC_CONTEXT:
		if (*mdoc->meta.msec == '9')
			break;
		if (NULL == goodsec)
			goodsec = "9";
		mandoc_vmsg(MANDOCERR_SEC_MSEC, mdoc->parse,
		    mdoc->last->line, mdoc->last->pos,
		    "%s for %s only", secname, goodsec);
		break;
	default:
		break;
	}

	free(secname);
	return(1);
}

static int
post_ignpar(POST_ARGS)
{
	struct mdoc_node *np;

	if (MDOC_BODY != mdoc->last->type)
		return(1);

	if (NULL != (np = mdoc->last->child))
		if (MDOC_Pp == np->tok || MDOC_Lp == np->tok) {
			mandoc_vmsg(MANDOCERR_PAR_SKIP,
			    mdoc->parse, np->line, np->pos,
			    "%s after %s", mdoc_macronames[np->tok],
			    mdoc_macronames[mdoc->last->tok]);
			mdoc_node_delete(mdoc, np);
		}

	if (NULL != (np = mdoc->last->last))
		if (MDOC_Pp == np->tok || MDOC_Lp == np->tok) {
			mandoc_vmsg(MANDOCERR_PAR_SKIP, mdoc->parse,
			    np->line, np->pos, "%s at the end of %s",
			    mdoc_macronames[np->tok],
			    mdoc_macronames[mdoc->last->tok]);
			mdoc_node_delete(mdoc, np);
		}

	return(1);
}

static int
pre_par(PRE_ARGS)
{

	if (NULL == mdoc->last)
		return(1);
	if (MDOC_ELEM != n->type && MDOC_BLOCK != n->type)
		return(1);

	/*
	 * Don't allow prior `Lp' or `Pp' prior to a paragraph-type
	 * block:  `Lp', `Pp', or non-compact `Bd' or `Bl'.
	 */

	if (MDOC_Pp != mdoc->last->tok &&
	    MDOC_Lp != mdoc->last->tok &&
	    MDOC_br != mdoc->last->tok)
		return(1);
	if (MDOC_Bl == n->tok && n->norm->Bl.comp)
		return(1);
	if (MDOC_Bd == n->tok && n->norm->Bd.comp)
		return(1);
	if (MDOC_It == n->tok && n->parent->norm->Bl.comp)
		return(1);

	mandoc_vmsg(MANDOCERR_PAR_SKIP, mdoc->parse,
	    mdoc->last->line, mdoc->last->pos,
	    "%s before %s", mdoc_macronames[mdoc->last->tok],
	    mdoc_macronames[n->tok]);
	mdoc_node_delete(mdoc, mdoc->last);
	return(1);
}

static int
post_par(POST_ARGS)
{
	struct mdoc_node *np;

	if (MDOC_ELEM != mdoc->last->type &&
	    MDOC_BLOCK != mdoc->last->type)
		return(1);

	if (NULL == (np = mdoc->last->prev)) {
		np = mdoc->last->parent;
		if (MDOC_Sh != np->tok && MDOC_Ss != np->tok)
			return(1);
	} else {
		if (MDOC_Pp != np->tok && MDOC_Lp != np->tok &&
		    (MDOC_br != mdoc->last->tok ||
		     (MDOC_sp != np->tok && MDOC_br != np->tok)))
			return(1);
	}

	mandoc_vmsg(MANDOCERR_PAR_SKIP, mdoc->parse,
	    mdoc->last->line, mdoc->last->pos,
	    "%s after %s", mdoc_macronames[mdoc->last->tok],
	    mdoc_macronames[np->tok]);
	mdoc_node_delete(mdoc, mdoc->last);
	return(1);
}

static int
pre_literal(PRE_ARGS)
{

	if (MDOC_BODY != n->type)
		return(1);

	/*
	 * The `Dl' (note "el" not "one") and `Bd -literal' and `Bd
	 * -unfilled' macros set MDOC_LITERAL on entrance to the body.
	 */

	switch (n->tok) {
	case MDOC_Dl:
		mdoc->flags |= MDOC_LITERAL;
		break;
	case MDOC_Bd:
		if (DISP_literal == n->norm->Bd.type)
			mdoc->flags |= MDOC_LITERAL;
		if (DISP_unfilled == n->norm->Bd.type)
			mdoc->flags |= MDOC_LITERAL;
		break;
	default:
		abort();
		/* NOTREACHED */
	}

	return(1);
}

static int
post_dd(POST_ARGS)
{
	struct mdoc_node *n;
	char		 *datestr;

	if (mdoc->meta.date)
		free(mdoc->meta.date);

	n = mdoc->last;
	if (NULL == n->child || '\0' == n->child->string[0]) {
		mdoc->meta.date = mdoc->quick ? mandoc_strdup("") :
		    mandoc_normdate(mdoc->parse, NULL, n->line, n->pos);
		return(1);
	}

	datestr = NULL;
	mdoc_deroff(&datestr, n);
	if (mdoc->quick)
		mdoc->meta.date = datestr;
	else {
		mdoc->meta.date = mandoc_normdate(mdoc->parse,
		    datestr, n->line, n->pos);
		free(datestr);
	}
	return(1);
}

static int
post_dt(POST_ARGS)
{
	struct mdoc_node *nn, *n;
	const char	 *cp;
	char		 *p;

	n = mdoc->last;

	if (mdoc->meta.title)
		free(mdoc->meta.title);
	if (mdoc->meta.vol)
		free(mdoc->meta.vol);
	if (mdoc->meta.arch)
		free(mdoc->meta.arch);

	mdoc->meta.title = mdoc->meta.vol = mdoc->meta.arch = NULL;

	/* First check that all characters are uppercase. */

	if (NULL != (nn = n->child))
		for (p = nn->string; *p; p++) {
			if (toupper((unsigned char)*p) == *p)
				continue;
			mandoc_msg(MANDOCERR_TITLE_CASE,
			    mdoc->parse, nn->line,
			    nn->pos + (p - nn->string),
			    nn->string);
			break;
		}

	/* Handles: `.Dt'
	 * title = unknown, volume = local, msec = 0, arch = NULL
	 */

	if (NULL == (nn = n->child)) {
		/* XXX: make these macro values. */
		/* FIXME: warn about missing values. */
		mdoc->meta.title = mandoc_strdup("UNKNOWN");
		mdoc->meta.vol = mandoc_strdup("LOCAL");
		mdoc->meta.msec = mandoc_strdup("1");
		return(1);
	}

	/* Handles: `.Dt TITLE'
	 * title = TITLE, volume = local, msec = 0, arch = NULL
	 */

	mdoc->meta.title = mandoc_strdup(
	    '\0' == nn->string[0] ? "UNKNOWN" : nn->string);

	if (NULL == (nn = nn->next)) {
		/* FIXME: warn about missing msec. */
		/* XXX: make this a macro value. */
		mdoc->meta.vol = mandoc_strdup("LOCAL");
		mdoc->meta.msec = mandoc_strdup("1");
		return(1);
	}

	/* Handles: `.Dt TITLE SEC'
	 * title = TITLE,
	 * volume = SEC is msec ? format(msec) : SEC,
	 * msec = SEC is msec ? atoi(msec) : 0,
	 * arch = NULL
	 */

	cp = mandoc_a2msec(nn->string);
	if (cp) {
		mdoc->meta.vol = mandoc_strdup(cp);
		mdoc->meta.msec = mandoc_strdup(nn->string);
	} else {
		mandoc_msg(MANDOCERR_MSEC_BAD, mdoc->parse,
		    nn->line, nn->pos, nn->string);
		mdoc->meta.vol = mandoc_strdup(nn->string);
		mdoc->meta.msec = mandoc_strdup(nn->string);
	}

	if (NULL == (nn = nn->next))
		return(1);

	/* Handles: `.Dt TITLE SEC VOL'
	 * title = TITLE,
	 * volume = VOL is vol ? format(VOL) :
	 *	    VOL is arch ? format(arch) :
	 *	    VOL
	 */

	cp = mdoc_a2vol(nn->string);
	if (cp) {
		free(mdoc->meta.vol);
		mdoc->meta.vol = mandoc_strdup(cp);
	} else {
		cp = mdoc_a2arch(nn->string);
		if (NULL == cp) {
			mandoc_msg(MANDOCERR_ARCH_BAD, mdoc->parse,
			    nn->line, nn->pos, nn->string);
			free(mdoc->meta.vol);
			mdoc->meta.vol = mandoc_strdup(nn->string);
		} else
			mdoc->meta.arch = mandoc_strdup(cp);
	}

	/* Ignore any subsequent parameters... */
	/* FIXME: warn about subsequent parameters. */

	return(1);
}

static int
post_prol(POST_ARGS)
{
	/*
	 * Remove prologue macros from the document after they're
	 * processed.  The final document uses mdoc_meta for these
	 * values and discards the originals.
	 */

	mdoc_node_delete(mdoc, mdoc->last);
	if (mdoc->meta.title && mdoc->meta.date && mdoc->meta.os)
		mdoc->flags |= MDOC_PBODY;

	return(1);
}

static int
post_bx(POST_ARGS)
{
	struct mdoc_node	*n;

	/*
	 * Make `Bx's second argument always start with an uppercase
	 * letter.  Groff checks if it's an "accepted" term, but we just
	 * uppercase blindly.
	 */

	n = mdoc->last->child;
	if (n && NULL != (n = n->next))
		*n->string = (char)toupper((unsigned char)*n->string);

	return(1);
}

static int
post_os(POST_ARGS)
{
#ifndef OSNAME
	struct utsname	  utsname;
	static char	 *defbuf;
#endif
	struct mdoc_node *n;

	n = mdoc->last;

	/*
	 * Set the operating system by way of the `Os' macro.
	 * The order of precedence is:
	 * 1. the argument of the `Os' macro, unless empty
	 * 2. the -Ios=foo command line argument, if provided
	 * 3. -DOSNAME="\"foo\"", if provided during compilation
	 * 4. "sysname release" from uname(3)
	 */

	free(mdoc->meta.os);
	mdoc->meta.os = NULL;
	mdoc_deroff(&mdoc->meta.os, n);
	if (mdoc->meta.os)
		return(1);

	if (mdoc->defos) {
		mdoc->meta.os = mandoc_strdup(mdoc->defos);
		return(1);
	}

#ifdef OSNAME
	mdoc->meta.os = mandoc_strdup(OSNAME);
#else /*!OSNAME */
	if (NULL == defbuf) {
		if (-1 == uname(&utsname)) {
			mdoc_nmsg(mdoc, n, MANDOCERR_UNAME);
			defbuf = mandoc_strdup("UNKNOWN");
		} else
			mandoc_asprintf(&defbuf, "%s %s",
			    utsname.sysname, utsname.release);
	}
	mdoc->meta.os = mandoc_strdup(defbuf);
#endif /*!OSNAME*/
	return(1);
}

static int
post_std(POST_ARGS)
{
	struct mdoc_node *nn, *n;

	n = mdoc->last;

	/*
	 * Macros accepting `-std' as an argument have the name of the
	 * current document (`Nm') filled in as the argument if it's not
	 * provided.
	 */

	if (n->child)
		return(1);

	if (NULL == mdoc->meta.name)
		return(1);

	nn = n;
	mdoc->next = MDOC_NEXT_CHILD;

	if ( ! mdoc_word_alloc(mdoc, n->line, n->pos, mdoc->meta.name))
		return(0);

	mdoc->last = nn;
	return(1);
}

static enum mdoc_sec
a2sec(const char *p)
{
	int		 i;

	for (i = 0; i < (int)SEC__MAX; i++)
		if (secnames[i] && 0 == strcmp(p, secnames[i]))
			return((enum mdoc_sec)i);

	return(SEC_CUSTOM);
}

static size_t
macro2len(enum mdoct macro)
{

	switch (macro) {
	case MDOC_Ad:
		return(12);
	case MDOC_Ao:
		return(12);
	case MDOC_An:
		return(12);
	case MDOC_Aq:
		return(12);
	case MDOC_Ar:
		return(12);
	case MDOC_Bo:
		return(12);
	case MDOC_Bq:
		return(12);
	case MDOC_Cd:
		return(12);
	case MDOC_Cm:
		return(10);
	case MDOC_Do:
		return(10);
	case MDOC_Dq:
		return(12);
	case MDOC_Dv:
		return(12);
	case MDOC_Eo:
		return(12);
	case MDOC_Em:
		return(10);
	case MDOC_Er:
		return(17);
	case MDOC_Ev:
		return(15);
	case MDOC_Fa:
		return(12);
	case MDOC_Fl:
		return(10);
	case MDOC_Fo:
		return(16);
	case MDOC_Fn:
		return(16);
	case MDOC_Ic:
		return(10);
	case MDOC_Li:
		return(16);
	case MDOC_Ms:
		return(6);
	case MDOC_Nm:
		return(10);
	case MDOC_No:
		return(12);
	case MDOC_Oo:
		return(10);
	case MDOC_Op:
		return(14);
	case MDOC_Pa:
		return(32);
	case MDOC_Pf:
		return(12);
	case MDOC_Po:
		return(12);
	case MDOC_Pq:
		return(12);
	case MDOC_Ql:
		return(16);
	case MDOC_Qo:
		return(12);
	case MDOC_So:
		return(12);
	case MDOC_Sq:
		return(12);
	case MDOC_Sy:
		return(6);
	case MDOC_Sx:
		return(16);
	case MDOC_Tn:
		return(10);
	case MDOC_Va:
		return(12);
	case MDOC_Vt:
		return(12);
	case MDOC_Xr:
		return(10);
	default:
		break;
	};
	return(0);
}