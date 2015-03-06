/*	$OpenBSD: fdt.h,v 1.3 2009/10/01 20:21:05 dms Exp $	*/

/*
 * Copyright (c) 2009 Dariusz Swiderski <sfires@sfires.net>
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

struct fdt_head {
	u_int32_t fh_magic;
	u_int32_t fh_size;
	u_int32_t fh_struct_off;
	u_int32_t fh_strings_off;
	u_int32_t fh_reserve_off;
	u_int32_t fh_version;
	u_int32_t fh_comp_ver;     /* last compatible version */
	u_int32_t fh_boot_cpu_id;  /* fh_version >=2 */
	u_int32_t fh_strings_size; /* fh_version >=3 */
	u_int32_t fh_struct_size;  /* fh_version >=17 */
};

struct fdt {
	struct fdt_head *header;
	void *		tree;
	void *		strings;
	void *		memory;
	int		version;
	int		strings_size;
};

#define FDT_MAGIC	0xd00dfeed
#define FDT_NODE_BEGIN	0x01
#define FDT_NODE_END	0x02
#define FDT_PROPERTY	0x03
#define FDT_NOP		0x04
#define FDT_END		0x09

#define FDT_CODE_VERSION 0x11

int	 fdt_init(void *);
void	*fdt_next_node(void *);
void	*fdt_child_node(void *);
char	*fdt_node_name(void *);
void	*fdt_find_node(char *);
int	 fdt_node_property(void *, char *, char **);
void	*fdt_parent_node(void *);
#ifdef DEBUG
void	*fdt_print_property(void *, int);
void 	 fdt_print_node(void *, int);
void	 fdt_print_tree(void);
#endif
